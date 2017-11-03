#ifndef CLIENT_WS_HPP
#define CLIENT_WS_HPP

#include "crypto.hpp"
#include "utility.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio.hpp>
#include <boost/functional/hash.hpp>

#include <atomic>
#include <iostream>
#include <list>
#include <mutex>
#include <random>

#ifdef USE_STANDALONE_ASIO
#include <asio.hpp>
#include <asio/steady_timer.hpp>
namespace SimpleWeb {
  using error_code = std::error_code;
  using errc = std::errc;
  namespace make_error_code = std;
} // namespace SimpleWeb
#else
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
namespace SimpleWeb {
  namespace asio = boost::asio;
  using error_code = boost::system::error_code;
  namespace errc = boost::system::errc;
  namespace make_error_code = boost::system::errc;
} // namespace SimpleWeb
#endif

namespace SimpleWeb {
  template <class socket_type>
  class SocketClient;

  template <class socket_type>
  class SocketClientBase {
  public:
    /// The buffer is consumed during send operations.
    /// Do not alter while sending.
    class SendStream : public std::iostream {
      friend class SocketClientBase<socket_type>;

      asio::streambuf streambuf;

    public:
      SendStream() noexcept : std::iostream(&streambuf) {}

      /// Returns the size of the buffer
      std::size_t size() const noexcept {
        return streambuf.size();
      }
    };

    class Message;

    class Connection : public std::enable_shared_from_this<Connection> {
      friend class SocketClientBase<socket_type>;
      friend class SocketClient<socket_type>;

    public:
      std::string http_version, status_code;
      CaseInsensitiveMultimap header;
      std::string remote_endpoint_address;
      unsigned short remote_endpoint_port;

    private:
      template <typename... Args>
      Connection(std::shared_ptr<ScopeRunner> handler_runner, long timeout_idle, Args &&... args) noexcept
          : handler_runner(std::move(handler_runner)), socket(new socket_type(std::forward<Args>(args)...)), timeout_idle(timeout_idle), strand(socket->get_io_service()), closed(false) {}

      std::shared_ptr<ScopeRunner> handler_runner;

      std::unique_ptr<socket_type> socket; // Socket must be unique_ptr since asio::ssl::stream<asio::ip::tcp::socket> is not movable
      std::mutex socket_close_mutex;

      std::shared_ptr<Message> message;

      long timeout_idle;
      std::unique_ptr<asio::steady_timer> timer;
      std::mutex timer_mutex;

      void close() noexcept {
        error_code ec;
        std::unique_lock<std::mutex> lock(socket_close_mutex); // The following operations seems to be needed to run sequentially
        socket->lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        socket->lowest_layer().close(ec);
      }

      void set_timeout(long seconds = -1) noexcept {
        bool use_timeout_idle = false;
        if(seconds == -1) {
          use_timeout_idle = true;
          seconds = timeout_idle;
        }

        std::unique_lock<std::mutex> lock(timer_mutex);

        if(seconds == 0) {
          timer = nullptr;
          return;
        }

        timer = std::unique_ptr<asio::steady_timer>(new asio::steady_timer(socket->get_io_service()));
        timer->expires_from_now(std::chrono::seconds(seconds));
        std::weak_ptr<Connection> connection_weak(this->shared_from_this()); // To avoid keeping Connection instance alive longer than needed
        timer->async_wait([connection_weak, use_timeout_idle](const error_code &ec) {
          if(!ec) {
            if(auto connection = connection_weak.lock()) {
              if(use_timeout_idle)
                connection->send_close(1000, "idle timeout"); // 1000=normal closure
              else
                connection->close();
            }
          }
        });
      }

      void cancel_timeout() noexcept {
        std::unique_lock<std::mutex> lock(timer_mutex);
        if(timer) {
          error_code ec;
          timer->cancel(ec);
        }
      }

      asio::io_service::strand strand;

      class SendData {
      public:
        SendData(std::shared_ptr<SendStream> send_stream, std::function<void(const error_code)> &&callback) noexcept
            : send_stream(std::move(send_stream)), callback(std::move(callback)) {}
        std::shared_ptr<SendStream> send_stream;
        std::function<void(const error_code)> callback;
      };

      std::list<SendData> send_queue;

      void send_from_queue() {
        auto self = this->shared_from_this();
        strand.post([self]() {
          asio::async_write(*self->socket, self->send_queue.begin()->send_stream->streambuf, self->strand.wrap([self](const error_code &ec, std::size_t /*bytes_transferred*/) {
            auto lock = self->handler_runner->continue_lock();
            if(!lock)
              return;
            auto send_queued = self->send_queue.begin();
            if(send_queued->callback)
              send_queued->callback(ec);
            if(!ec) {
              self->send_queue.erase(send_queued);
              if(self->send_queue.size() > 0)
                self->send_from_queue();
            }
            else
              self->send_queue.clear();
          }));
        });
      }

      std::atomic<bool> closed;

      void read_remote_endpoint_data() noexcept {
        try {
          remote_endpoint_address = socket->lowest_layer().remote_endpoint().address().to_string();
          remote_endpoint_port = socket->lowest_layer().remote_endpoint().port();
        }
        catch(const std::exception &e) {
          std::cerr << e.what() << std::endl;
        }
      }

    public:
      /// fin_rsv_opcode: 129=one fragment, text, 130=one fragment, binary, 136=close connection.
      /// See http://tools.ietf.org/html/rfc6455#section-5.2 for more information
      void send(const std::shared_ptr<SendStream> &message_stream, const std::function<void(const error_code &)> &callback = nullptr,
                unsigned char fin_rsv_opcode = 129) {
        cancel_timeout();
        set_timeout();

        // Create mask
        std::vector<unsigned char> mask;
        mask.resize(4);
        std::uniform_int_distribution<unsigned short> dist(0, 255);
        std::random_device rd;
        for(std::size_t c = 0; c < 4; c++)
          mask[c] = static_cast<unsigned char>(dist(rd));

        auto send_stream = std::make_shared<SendStream>();

        std::size_t length = message_stream->size();

        send_stream->put(static_cast<char>(fin_rsv_opcode));
        // Masked (first length byte>=128)
        if(length >= 126) {
          std::size_t num_bytes;
          if(length > 0xffff) {
            num_bytes = 8;
            send_stream->put(static_cast<char>(127 + 128));
          }
          else {
            num_bytes = 2;
            send_stream->put(static_cast<char>(126 + 128));
          }

          for(std::size_t c = num_bytes - 1; c != static_cast<std::size_t>(-1); c--)
            send_stream->put((static_cast<unsigned long long>(length) >> (8 * c)) % 256);
        }
        else
          send_stream->put(static_cast<char>(length + 128));

        for(std::size_t c = 0; c < 4; c++)
          send_stream->put(static_cast<char>(mask[c]));

        for(std::size_t c = 0; c < length; c++)
          send_stream->put(message_stream->get() ^ mask[c % 4]);

        auto self = this->shared_from_this();
        strand.post([self, send_stream, callback]() {
          self->send_queue.emplace_back(send_stream, callback);
          if(self->send_queue.size() == 1)
            self->send_from_queue();
        });
      }

      void send_close(int status, const std::string &reason = "", const std::function<void(const error_code &)> &callback = nullptr) {
        // Send close only once (in case close is initiated by client)
        if(closed)
          return;
        closed = true;

        auto send_stream = std::make_shared<SendStream>();

        send_stream->put(status >> 8);
        send_stream->put(status % 256);

        *send_stream << reason;

        // fin_rsv_opcode=136: message close
        send(send_stream, callback, 136);
      }
    };

    class Message : public std::istream {
      friend class SocketClientBase<socket_type>;
      friend class Connection;

    public:
      unsigned char fin_rsv_opcode;
      std::size_t size() noexcept {
        return length;
      }

      /// Convenience function to return std::string. The stream buffer is consumed.
      std::string string() noexcept {
        try {
          std::stringstream ss;
          ss << rdbuf();
          return ss.str();
        }
        catch(...) {
          return std::string();
        }
      }

    private:
      Message() noexcept : std::istream(&streambuf) {}
      std::size_t length;
      asio::streambuf streambuf;
    };

    class Config {
      friend class SocketClientBase<socket_type>;

    private:
      Config() noexcept {}

    public:
      /// Timeout on request handling. Defaults to no timeout.
      long timeout_request = 0;
      /// Idle timeout. Defaults to no timeout.
      long timeout_idle = 0;
      /// Additional header fields to send when performing WebSocket handshake.
      /// Use this variable to for instance set Sec-WebSocket-Protocol.
      CaseInsensitiveMultimap header;
    };
    /// Set before calling start().
    Config config;

    std::function<void(std::shared_ptr<Connection>)> on_open;
    std::function<void(std::shared_ptr<Connection>, std::shared_ptr<Message>)> on_message;
    std::function<void(std::shared_ptr<Connection>, int, const std::string &)> on_close;
    std::function<void(std::shared_ptr<Connection>, const error_code &)> on_error;

    void start() {
      if(!io_service) {
        io_service = std::make_shared<asio::io_service>();
        internal_io_service = true;
      }

      if(io_service->stopped())
        io_service->reset();

      connect();

      if(internal_io_service)
        io_service->run();
    }

    void stop() noexcept {
      {
        std::unique_lock<std::mutex> lock(connection_mutex);
        if(connection)
          connection->close();
      }

      if(internal_io_service)
        io_service->stop();
    }

    virtual ~SocketClientBase() noexcept {
      handler_runner->stop();
      stop();
    }

    /// If you have your own asio::io_service, store its pointer here before running start().
    std::shared_ptr<asio::io_service> io_service;

  protected:
    bool internal_io_service = false;

    std::string host;
    unsigned short port;
    std::string path;

    std::shared_ptr<Connection> connection;
    std::mutex connection_mutex;

    std::shared_ptr<ScopeRunner> handler_runner;

    SocketClientBase(const std::string &host_port_path, unsigned short default_port) noexcept : handler_runner(new ScopeRunner()) {
      std::size_t host_end = host_port_path.find(':');
      std::size_t host_port_end = host_port_path.find('/');
      if(host_end == std::string::npos) {
        host_end = host_port_end;
        port = default_port;
      }
      else {
        if(host_port_end == std::string::npos)
          port = static_cast<unsigned short>(stoul(host_port_path.substr(host_end + 1)));
        else
          port = static_cast<unsigned short>(stoul(host_port_path.substr(host_end + 1, host_port_end - (host_end + 1))));
      }
      if(host_port_end == std::string::npos)
        path = "/";
      else
        path = host_port_path.substr(host_port_end);
      if(host_end == std::string::npos)
        host = host_port_path;
      else
        host = host_port_path.substr(0, host_end);
    }

    virtual void connect() = 0;

    void handshake(const std::shared_ptr<Connection> &connection) {
      connection->read_remote_endpoint_data();

      auto write_buffer = std::make_shared<asio::streambuf>();

      std::ostream request(write_buffer.get());

      request << "GET " << path << " HTTP/1.1"
              << "\r\n";
      request << "Host: " << host << "\r\n";
      request << "Upgrade: websocket\r\n";
      request << "Connection: Upgrade\r\n";

      // Make random 16-byte nonce
      std::string nonce;
      nonce.resize(16);
      std::uniform_int_distribution<unsigned short> dist(0, 255);
      std::random_device rd;
      for(std::size_t c = 0; c < 16; c++)
        nonce[c] = static_cast<char>(dist(rd));

      auto nonce_base64 = std::make_shared<std::string>(Crypto::Base64::encode(nonce));
      request << "Sec-WebSocket-Key: " << *nonce_base64 << "\r\n";
      request << "Sec-WebSocket-Version: 13\r\n";
      for(auto &header_field : config.header)
        request << header_field.first << ": " << header_field.second << "\r\n";
      request << "\r\n";

      connection->message = std::shared_ptr<Message>(new Message());

      connection->set_timeout(config.timeout_request);
      asio::async_write(*connection->socket, *write_buffer, [this, connection, write_buffer, nonce_base64](const error_code &ec, std::size_t /*bytes_transferred*/) {
        connection->cancel_timeout();
        auto lock = connection->handler_runner->continue_lock();
        if(!lock)
          return;
        if(!ec) {
          connection->set_timeout(this->config.timeout_request);
          asio::async_read_until(*connection->socket, connection->message->streambuf, "\r\n\r\n", [this, connection, nonce_base64](const error_code &ec, std::size_t /*bytes_transferred*/) {
            connection->cancel_timeout();
            auto lock = connection->handler_runner->continue_lock();
            if(!lock)
              return;
            if(!ec) {
              if(!ResponseMessage::parse(*connection->message, connection->http_version, connection->status_code, connection->header) ||
                 connection->status_code.empty() || connection->status_code.compare(0, 4, "101 ") != 0) {
                this->connection_error(connection, make_error_code::make_error_code(errc::protocol_error));
                return;
              }
              auto header_it = connection->header.find("Sec-WebSocket-Accept");
              static auto ws_magic_string = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
              if(header_it != connection->header.end() &&
                 Crypto::Base64::decode(header_it->second) == Crypto::sha1(*nonce_base64 + ws_magic_string)) {
                this->connection_open(connection);
                read_message(connection);
              }
              else
                this->connection_error(connection, make_error_code::make_error_code(errc::protocol_error));
            }
            else
              this->connection_error(connection, ec);
          });
        }
        else
          this->connection_error(connection, ec);
      });
    }

    void read_message(const std::shared_ptr<Connection> &connection) {
      asio::async_read(*connection->socket, connection->message->streambuf, asio::transfer_exactly(2), [this, connection](const error_code &ec, std::size_t bytes_transferred) {
        auto lock = connection->handler_runner->continue_lock();
        if(!lock)
          return;
        if(!ec) {
          if(bytes_transferred == 0) { // TODO: This might happen on server at least, might also happen here
            this->read_message(connection);
            return;
          }
          std::vector<unsigned char> first_bytes;
          first_bytes.resize(2);
          connection->message->read(reinterpret_cast<char *>(&first_bytes[0]), 2);

          connection->message->fin_rsv_opcode = first_bytes[0];

          // Close connection if masked message from server (protocol error)
          if(first_bytes[1] >= 128) {
            const std::string reason("message from server masked");
            connection->send_close(1002, reason);
            this->connection_close(connection, 1002, reason);
            return;
          }

          std::size_t length = (first_bytes[1] & 127);

          if(length == 126) {
            // 2 next bytes is the size of content
            asio::async_read(*connection->socket, connection->message->streambuf, asio::transfer_exactly(2), [this, connection](const error_code &ec, std::size_t /*bytes_transferred*/) {
              auto lock = connection->handler_runner->continue_lock();
              if(!lock)
                return;
              if(!ec) {
                std::vector<unsigned char> length_bytes;
                length_bytes.resize(2);
                connection->message->read(reinterpret_cast<char *>(&length_bytes[0]), 2);

                std::size_t length = 0;
                std::size_t num_bytes = 2;
                for(std::size_t c = 0; c < num_bytes; c++)
                  length += static_cast<std::size_t>(length_bytes[c]) << (8 * (num_bytes - 1 - c));

                connection->message->length = length;
                this->read_message_content(connection);
              }
              else
                this->connection_error(connection, ec);
            });
          }
          else if(length == 127) {
            // 8 next bytes is the size of content
            asio::async_read(*connection->socket, connection->message->streambuf, asio::transfer_exactly(8), [this, connection](const error_code &ec, std::size_t /*bytes_transferred*/) {
              auto lock = connection->handler_runner->continue_lock();
              if(!lock)
                return;
              if(!ec) {
                std::vector<unsigned char> length_bytes;
                length_bytes.resize(8);
                connection->message->read(reinterpret_cast<char *>(&length_bytes[0]), 8);

                std::size_t length = 0;
                std::size_t num_bytes = 8;
                for(std::size_t c = 0; c < num_bytes; c++)
                  length += static_cast<std::size_t>(length_bytes[c]) << (8 * (num_bytes - 1 - c));

                connection->message->length = length;
                this->read_message_content(connection);
              }
              else
                this->connection_error(connection, ec);
            });
          }
          else {
            connection->message->length = length;
            this->read_message_content(connection);
          }
        }
        else
          this->connection_error(connection, ec);
      });
    }

    void read_message_content(const std::shared_ptr<Connection> &connection) {
      asio::async_read(*connection->socket, connection->message->streambuf, asio::transfer_exactly(connection->message->length), [this, connection](const error_code &ec, std::size_t /*bytes_transferred*/) {
        auto lock = connection->handler_runner->continue_lock();
        if(!lock)
          return;
        if(!ec) {
          // If connection close
          if((connection->message->fin_rsv_opcode & 0x0f) == 8) {
            int status = 0;
            if(connection->message->length >= 2) {
              unsigned char byte1 = connection->message->get();
              unsigned char byte2 = connection->message->get();
              status = (byte1 << 8) + byte2;
            }

            auto reason = connection->message->string();
            connection->send_close(status, reason);
            this->connection_close(connection, status, reason);
            return;
          }
          // If ping
          else if((connection->message->fin_rsv_opcode & 0x0f) == 9) {
            // Send pong
            auto empty_send_stream = std::make_shared<SendStream>();
            connection->send(empty_send_stream, nullptr, connection->message->fin_rsv_opcode + 1);
          }
          else if(this->on_message) {
            connection->cancel_timeout();
            connection->set_timeout();
            this->on_message(connection, connection->message);
          }

          // Next message
          connection->message = std::shared_ptr<Message>(new Message());
          this->read_message(connection);
        }
        else
          this->connection_error(connection, ec);
      });
    }

    void connection_open(const std::shared_ptr<Connection> &connection) const {
      connection->cancel_timeout();
      connection->set_timeout();

      if(on_open)
        on_open(connection);
    }

    void connection_close(const std::shared_ptr<Connection> &connection, int status, const std::string &reason) const {
      connection->cancel_timeout();
      connection->set_timeout();

      if(on_close)
        on_close(connection, status, reason);
    }

    void connection_error(const std::shared_ptr<Connection> &connection, const error_code &ec) const {
      connection->cancel_timeout();
      connection->set_timeout();

      if(on_error)
        on_error(connection, ec);
    }
  };

  template <class socket_type>
  class SocketClient : public SocketClientBase<socket_type> {};

  using WS = asio::ip::tcp::socket;

  template <>
  class SocketClient<WS> : public SocketClientBase<WS> {
  public:
    SocketClient(const std::string &server_port_path) noexcept : SocketClientBase<WS>::SocketClientBase(server_port_path, 80){};

  protected:
    void connect() override {
      std::unique_lock<std::mutex> lock(connection_mutex);
      auto connection = this->connection = std::shared_ptr<Connection>(new Connection(handler_runner, config.timeout_idle, *io_service));
      lock.unlock();
      asio::ip::tcp::resolver::query query(host, std::to_string(port));
      auto resolver = std::make_shared<asio::ip::tcp::resolver>(*io_service);
      connection->set_timeout(config.timeout_request);
      resolver->async_resolve(query, [this, connection, resolver](const error_code &ec, asio::ip::tcp::resolver::iterator it) {
        connection->cancel_timeout();
        auto lock = connection->handler_runner->continue_lock();
        if(!lock)
          return;
        if(!ec) {
          connection->set_timeout(this->config.timeout_request);
          asio::async_connect(*connection->socket, it, [this, connection, resolver](const error_code &ec, asio::ip::tcp::resolver::iterator /*it*/) {
            connection->cancel_timeout();
            auto lock = connection->handler_runner->continue_lock();
            if(!lock)
              return;
            if(!ec) {
              asio::ip::tcp::no_delay option(true);
              connection->socket->set_option(option);

              this->handshake(connection);
            }
            else
              this->connection_error(connection, ec);
          });
        }
        else
          this->connection_error(connection, ec);
      });
    }
  };
} // namespace SimpleWeb

#endif /* CLIENT_WS_HPP */
