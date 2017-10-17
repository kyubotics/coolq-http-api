#ifndef SERVER_WS_HPP
#define SERVER_WS_HPP

#include "crypto.hpp"
#include "utility.hpp"

#include <atomic>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_set>

#ifdef USE_STANDALONE_ASIO
#include <asio.hpp>
#include <asio/steady_timer.hpp>
namespace SimpleWeb {
  using error_code = std::error_code;
} // namespace SimpleWeb
#else
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
namespace SimpleWeb {
  namespace asio = boost::asio;
  using error_code = boost::system::error_code;
} // namespace SimpleWeb
#endif

// Late 2017 TODO: remove the following checks and always use std::regex
#ifdef USE_BOOST_REGEX
#include <boost/regex.hpp>
namespace SimpleWeb {
  namespace regex = boost;
}
#else
#include <regex>
namespace SimpleWeb {
  namespace regex = std;
}
#endif

namespace SimpleWeb {
  template <class socket_type>
  class SocketServer;

  template <class socket_type>
  class SocketServerBase {
  public:
    /// The buffer is not consumed during send operations.
    /// Do not alter while sending.
    class SendStream : public std::ostream {
      friend class SocketServerBase<socket_type>;

      asio::streambuf streambuf;

    public:
      SendStream() noexcept : std::ostream(&streambuf) {}

      /// Returns the size of the buffer
      size_t size() const noexcept {
        return streambuf.size();
      }
    };

    class Connection : public std::enable_shared_from_this<Connection> {
      friend class SocketServerBase<socket_type>;
      friend class SocketServer<socket_type>;

    public:
      Connection(std::unique_ptr<socket_type> &&socket) noexcept : socket(std::move(socket)), timeout_idle(0), strand(this->socket->get_io_service()), closed(false) {}

      std::string method, path, query_string, http_version;

      CaseInsensitiveMultimap header;

      regex::smatch path_match;

      std::string remote_endpoint_address;
      unsigned short remote_endpoint_port;

    private:
      template <typename... Args>
      Connection(std::shared_ptr<ScopeRunner> handler_runner, long timeout_idle, Args &&... args) noexcept
          : handler_runner(std::move(handler_runner)), socket(new socket_type(std::forward<Args>(args)...)), timeout_idle(timeout_idle), strand(socket->get_io_service()), closed(false) {}

      std::shared_ptr<ScopeRunner> handler_runner;

      std::unique_ptr<socket_type> socket; // Socket must be unique_ptr since asio::ssl::stream<asio::ip::tcp::socket> is not movable
      std::mutex socket_close_mutex;

      asio::streambuf read_buffer;

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

      bool generate_handshake(const std::shared_ptr<asio::streambuf> &write_buffer) {
        std::ostream handshake(write_buffer.get());

        auto header_it = header.find("Sec-WebSocket-Key");
        if(header_it == header.end())
          return false;

        static auto ws_magic_string = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        auto sha1 = Crypto::sha1(header_it->second + ws_magic_string);

        handshake << "HTTP/1.1 101 Web Socket Protocol Handshake\r\n";
        handshake << "Upgrade: websocket\r\n";
        handshake << "Connection: Upgrade\r\n";
        handshake << "Sec-WebSocket-Accept: " << Crypto::Base64::encode(sha1) << "\r\n";
        handshake << "\r\n";

        return true;
      }

      asio::io_service::strand strand;

      class SendData {
      public:
        SendData(std::shared_ptr<SendStream> header_stream, std::shared_ptr<SendStream> message_stream,
                 std::function<void(const error_code)> &&callback) noexcept
            : header_stream(std::move(header_stream)), message_stream(std::move(message_stream)), callback(std::move(callback)) {}
        std::shared_ptr<SendStream> header_stream;
        std::shared_ptr<SendStream> message_stream;
        std::function<void(const error_code)> callback;
      };

      std::list<SendData> send_queue;

      void send_from_queue() {
        auto self = this->shared_from_this();
        strand.post([self]() {
          asio::async_write(*self->socket, self->send_queue.begin()->header_stream->streambuf, self->strand.wrap([self](const error_code &ec, size_t /*bytes_transferred*/) {
            auto lock = self->handler_runner->continue_lock();
            if(!lock)
              return;
            if(!ec) {
              asio::async_write(*self->socket, self->send_queue.begin()->message_stream->streambuf.data(), self->strand.wrap([self](const error_code &ec, size_t /*bytes_transferred*/) {
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
            }
            else {
              auto send_queued = self->send_queue.begin();
              if(send_queued->callback)
                send_queued->callback(ec);
              self->send_queue.clear();
            }
          }));
        });
      }

      std::atomic<bool> closed;

      void read_remote_endpoint_data() noexcept {
        try {
          remote_endpoint_address = socket->lowest_layer().remote_endpoint().address().to_string();
          remote_endpoint_port = socket->lowest_layer().remote_endpoint().port();
        }
        catch(...) {
        }
      }

    public:
      /// fin_rsv_opcode: 129=one fragment, text, 130=one fragment, binary, 136=close connection.
      /// See http://tools.ietf.org/html/rfc6455#section-5.2 for more information
      void send(const std::shared_ptr<SendStream> &message_stream, const std::function<void(const error_code &)> &callback = nullptr,
                unsigned char fin_rsv_opcode = 129) {
        cancel_timeout();
        set_timeout();

        auto header_stream = std::make_shared<SendStream>();

        size_t length = message_stream->size();

        header_stream->put(static_cast<char>(fin_rsv_opcode));
        // Unmasked (first length byte<128)
        if(length >= 126) {
          size_t num_bytes;
          if(length > 0xffff) {
            num_bytes = 8;
            header_stream->put(127);
          }
          else {
            num_bytes = 2;
            header_stream->put(126);
          }

          for(size_t c = num_bytes - 1; c != static_cast<size_t>(-1); c--)
            header_stream->put((static_cast<unsigned long long>(length) >> (8 * c)) % 256);
        }
        else
          header_stream->put(static_cast<char>(length));

        auto self = this->shared_from_this();
        strand.post([self, header_stream, message_stream, callback]() {
          self->send_queue.emplace_back(header_stream, message_stream, callback);
          if(self->send_queue.size() == 1)
            self->send_from_queue();
        });
      }

      void send_close(int status, const std::string &reason = "", const std::function<void(const error_code &)> &callback = nullptr) {
        // Send close only once (in case close is initiated by server)
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
      friend class SocketServerBase<socket_type>;

    public:
      unsigned char fin_rsv_opcode;
      size_t size() noexcept {
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
      size_t length;
      asio::streambuf streambuf;
    };

    class Endpoint {
      friend class SocketServerBase<socket_type>;

    private:
      std::unordered_set<std::shared_ptr<Connection>> connections;
      std::mutex connections_mutex;

    public:
      std::function<void(std::shared_ptr<Connection>)> on_open;
      std::function<void(std::shared_ptr<Connection>, std::shared_ptr<Message>)> on_message;
      std::function<void(std::shared_ptr<Connection>, int, const std::string &)> on_close;
      std::function<void(std::shared_ptr<Connection>, const error_code &)> on_error;

      std::unordered_set<std::shared_ptr<Connection>> get_connections() noexcept {
        std::unique_lock<std::mutex> lock(connections_mutex);
        auto copy = connections;
        return copy;
      }
    };

    class Config {
      friend class SocketServerBase<socket_type>;

    private:
      Config(unsigned short port) noexcept : port(port) {}

    public:
      /// Port number to use. Defaults to 80 for HTTP and 443 for HTTPS.
      unsigned short port;
      /// If io_service is not set, number of threads that the server will use when start() is called.
      /// Defaults to 1 thread.
      size_t thread_pool_size = 1;
      /// Timeout on request handling. Defaults to 5 seconds.
      long timeout_request = 5;
      /// Idle timeout. Defaults to no timeout.
      long timeout_idle = 0;
      /// IPv4 address in dotted decimal form or IPv6 address in hexadecimal notation.
      /// If empty, the address will be any address.
      std::string address;
      /// Set to false to avoid binding the socket to an address that is already in use. Defaults to true.
      bool reuse_address = true;
    };
    /// Set before calling start().
    Config config;

  private:
    class regex_orderable : public regex::regex {
      std::string str;

    public:
      regex_orderable(const char *regex_cstr) : regex::regex(regex_cstr), str(regex_cstr) {}
      regex_orderable(const std::string &regex_str) : regex::regex(regex_str), str(regex_str) {}
      bool operator<(const regex_orderable &rhs) const noexcept {
        return str < rhs.str;
      }
    };

  public:
    /// Warning: do not add or remove endpoints after start() is called
    std::map<regex_orderable, Endpoint> endpoint;

    virtual void start() {
      if(!io_service) {
        io_service = std::make_shared<asio::io_service>();
        internal_io_service = true;
      }

      if(io_service->stopped())
        io_service->reset();

      asio::ip::tcp::endpoint endpoint;
      if(config.address.size() > 0)
        endpoint = asio::ip::tcp::endpoint(asio::ip::address::from_string(config.address), config.port);
      else
        endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v4(), config.port);

      if(!acceptor)
        acceptor = std::unique_ptr<asio::ip::tcp::acceptor>(new asio::ip::tcp::acceptor(*io_service));
      acceptor->open(endpoint.protocol());
      acceptor->set_option(asio::socket_base::reuse_address(config.reuse_address));
      acceptor->bind(endpoint);
      acceptor->listen();

      accept();

      if(internal_io_service) {
        // If thread_pool_size>1, start m_io_service.run() in (thread_pool_size-1) threads for thread-pooling
        threads.clear();
        for(size_t c = 1; c < config.thread_pool_size; c++) {
          threads.emplace_back([this]() {
            io_service->run();
          });
        }
        // Main thread
        if(config.thread_pool_size > 0)
          io_service->run();

        // Wait for the rest of the threads, if any, to finish as well
        for(auto &t : threads)
          t.join();
      }
    }

    void stop() noexcept {
      if(acceptor) {
        error_code ec;
        acceptor->close(ec);

        for(auto &pair : endpoint) {
          std::unique_lock<std::mutex> lock(pair.second.connections_mutex);
          for(auto &connection : pair.second.connections)
            connection->close();
          pair.second.connections.clear();
        }

        if(internal_io_service)
          io_service->stop();
      }
    }

    virtual ~SocketServerBase() noexcept {}

    std::unordered_set<std::shared_ptr<Connection>> get_connections() noexcept {
      std::unordered_set<std::shared_ptr<Connection>> all_connections;
      for(auto &e : endpoint) {
        std::unique_lock<std::mutex> lock(e.second.connections_mutex);
        all_connections.insert(e.second.connections.begin(), e.second.connections.end());
      }
      return all_connections;
    }

    /**
     * Upgrades a request, from for instance Simple-Web-Server, to a WebSocket connection.
     * The parameters are moved to the Connection object.
     * See also Server::on_upgrade in the Simple-Web-Server project.
     * The socket's io_service is used, thus running start() is not needed.
     *
     * Example use:
     * server.on_upgrade=[&socket_server] (auto socket, auto request) {
     *   auto connection=std::make_shared<SimpleWeb::SocketServer<SimpleWeb::WS>::Connection>(std::move(socket));
     *   connection->method=std::move(request->method);
     *   connection->path=std::move(request->path);
     *   connection->query_string=std::move(request->query_string);
     *   connection->http_version=std::move(request->http_version);
     *   connection->header=std::move(request->header);
     *   connection->remote_endpoint_address=std::move(request->remote_endpoint_address);
     *   connection->remote_endpoint_port=request->remote_endpoint_port;
     *   socket_server.upgrade(connection);
     * }
     */
    void upgrade(const std::shared_ptr<Connection> &connection) {
      connection->handler_runner = handler_runner;
      connection->timeout_idle = config.timeout_idle;
      write_handshake(connection);
    }

    /// If you have your own asio::io_service, store its pointer here before running start().
    std::shared_ptr<asio::io_service> io_service;

  protected:
    bool internal_io_service = false;

    std::unique_ptr<asio::ip::tcp::acceptor> acceptor;
    std::vector<std::thread> threads;

    std::shared_ptr<ScopeRunner> handler_runner;

    SocketServerBase(unsigned short port) noexcept : config(port), handler_runner(new ScopeRunner()) {}

    virtual void accept() = 0;

    void read_handshake(const std::shared_ptr<Connection> &connection) {
      connection->read_remote_endpoint_data();

      connection->set_timeout(config.timeout_request);
      asio::async_read_until(*connection->socket, connection->read_buffer, "\r\n\r\n", [this, connection](const error_code &ec, size_t /*bytes_transferred*/) {
        connection->cancel_timeout();
        auto lock = connection->handler_runner->continue_lock();
        if(!lock)
          return;
        if(!ec) {
          std::istream stream(&connection->read_buffer);
          if(RequestMessage::parse(stream, connection->method, connection->path, connection->query_string, connection->http_version, connection->header))
            write_handshake(connection);
        }
      });
    }

    void write_handshake(const std::shared_ptr<Connection> &connection) {
      for(auto &regex_endpoint : endpoint) {
        regex::smatch path_match;
        if(regex::regex_match(connection->path, path_match, regex_endpoint.first)) {
          auto write_buffer = std::make_shared<asio::streambuf>();

          if(connection->generate_handshake(write_buffer)) {
            connection->path_match = std::move(path_match);
            connection->set_timeout(config.timeout_request);
            asio::async_write(*connection->socket, *write_buffer, [this, connection, write_buffer, &regex_endpoint](const error_code &ec, size_t /*bytes_transferred*/) {
              connection->cancel_timeout();
              auto lock = connection->handler_runner->continue_lock();
              if(!lock)
                return;
              if(!ec) {
                connection_open(connection, regex_endpoint.second);
                read_message(connection, regex_endpoint.second);
              }
              else
                connection_error(connection, regex_endpoint.second, ec);
            });
          }
          return;
        }
      }
    }

    void read_message(const std::shared_ptr<Connection> &connection, Endpoint &endpoint) const {
      asio::async_read(*connection->socket, connection->read_buffer, asio::transfer_exactly(2), [this, connection, &endpoint](const error_code &ec, size_t bytes_transferred) {
        auto lock = connection->handler_runner->continue_lock();
        if(!lock)
          return;
        if(!ec) {
          if(bytes_transferred == 0) { // TODO: why does this happen sometimes?
            read_message(connection, endpoint);
            return;
          }
          std::istream stream(&connection->read_buffer);

          std::vector<unsigned char> first_bytes;
          first_bytes.resize(2);
          stream.read((char *)&first_bytes[0], 2);

          unsigned char fin_rsv_opcode = first_bytes[0];

          // Close connection if unmasked message from client (protocol error)
          if(first_bytes[1] < 128) {
            const std::string reason("message from client not masked");
            connection->send_close(1002, reason);
            connection_close(connection, endpoint, 1002, reason);
            return;
          }

          size_t length = (first_bytes[1] & 127);

          if(length == 126) {
            // 2 next bytes is the size of content
            asio::async_read(*connection->socket, connection->read_buffer, asio::transfer_exactly(2), [this, connection, &endpoint, fin_rsv_opcode](const error_code &ec, size_t /*bytes_transferred*/) {
              auto lock = connection->handler_runner->continue_lock();
              if(!lock)
                return;
              if(!ec) {
                std::istream stream(&connection->read_buffer);

                std::vector<unsigned char> length_bytes;
                length_bytes.resize(2);
                stream.read((char *)&length_bytes[0], 2);

                size_t length = 0;
                size_t num_bytes = 2;
                for(size_t c = 0; c < num_bytes; c++)
                  length += static_cast<size_t>(length_bytes[c]) << (8 * (num_bytes - 1 - c));

                read_message_content(connection, length, endpoint, fin_rsv_opcode);
              }
              else
                connection_error(connection, endpoint, ec);
            });
          }
          else if(length == 127) {
            // 8 next bytes is the size of content
            asio::async_read(*connection->socket, connection->read_buffer, asio::transfer_exactly(8), [this, connection, &endpoint, fin_rsv_opcode](const error_code &ec, size_t /*bytes_transferred*/) {
              auto lock = connection->handler_runner->continue_lock();
              if(!lock)
                return;
              if(!ec) {
                std::istream stream(&connection->read_buffer);

                std::vector<unsigned char> length_bytes;
                length_bytes.resize(8);
                stream.read((char *)&length_bytes[0], 8);

                size_t length = 0;
                size_t num_bytes = 8;
                for(size_t c = 0; c < num_bytes; c++)
                  length += static_cast<size_t>(length_bytes[c]) << (8 * (num_bytes - 1 - c));

                read_message_content(connection, length, endpoint, fin_rsv_opcode);
              }
              else
                connection_error(connection, endpoint, ec);
            });
          }
          else
            read_message_content(connection, length, endpoint, fin_rsv_opcode);
        }
        else
          connection_error(connection, endpoint, ec);
      });
    }

    void read_message_content(const std::shared_ptr<Connection> &connection, size_t length, Endpoint &endpoint, unsigned char fin_rsv_opcode) const {
      asio::async_read(*connection->socket, connection->read_buffer, asio::transfer_exactly(4 + length), [this, connection, length, &endpoint, fin_rsv_opcode](const error_code &ec, size_t /*bytes_transferred*/) {
        auto lock = connection->handler_runner->continue_lock();
        if(!lock)
          return;
        if(!ec) {
          std::istream raw_message_data(&connection->read_buffer);

          // Read mask
          std::vector<unsigned char> mask;
          mask.resize(4);
          raw_message_data.read((char *)&mask[0], 4);

          std::shared_ptr<Message> message(new Message());
          message->length = length;
          message->fin_rsv_opcode = fin_rsv_opcode;

          std::ostream message_data_out_stream(&message->streambuf);
          for(size_t c = 0; c < length; c++) {
            message_data_out_stream.put(raw_message_data.get() ^ mask[c % 4]);
          }

          // If connection close
          if((fin_rsv_opcode & 0x0f) == 8) {
            int status = 0;
            if(length >= 2) {
              unsigned char byte1 = message->get();
              unsigned char byte2 = message->get();
              status = (byte1 << 8) + byte2;
            }

            auto reason = message->string();
            connection->send_close(status, reason);
            connection_close(connection, endpoint, status, reason);
            return;
          }
          else {
            // If ping
            if((fin_rsv_opcode & 0x0f) == 9) {
              // Send pong
              auto empty_send_stream = std::make_shared<SendStream>();
              connection->send(empty_send_stream, nullptr, fin_rsv_opcode + 1);
            }
            else if(endpoint.on_message) {
              connection->cancel_timeout();
              connection->set_timeout();
              endpoint.on_message(connection, message);
            }

            // Next message
            read_message(connection, endpoint);
          }
        }
        else
          connection_error(connection, endpoint, ec);
      });
    }

    void connection_open(const std::shared_ptr<Connection> &connection, Endpoint &endpoint) const {
      connection->cancel_timeout();
      connection->set_timeout();

      {
        std::unique_lock<std::mutex> lock(endpoint.connections_mutex);
        endpoint.connections.insert(connection);
      }

      if(endpoint.on_open)
        endpoint.on_open(connection);
    }

    void connection_close(const std::shared_ptr<Connection> &connection, Endpoint &endpoint, int status, const std::string &reason) const {
      connection->cancel_timeout();
      connection->set_timeout();

      {
        std::unique_lock<std::mutex> lock(endpoint.connections_mutex);
        endpoint.connections.erase(connection);
      }

      if(endpoint.on_close)
        endpoint.on_close(connection, status, reason);
    }

    void connection_error(const std::shared_ptr<Connection> &connection, Endpoint &endpoint, const error_code &ec) const {
      connection->cancel_timeout();
      connection->set_timeout();

      {
        std::unique_lock<std::mutex> lock(endpoint.connections_mutex);
        endpoint.connections.erase(connection);
      }

      if(endpoint.on_error)
        endpoint.on_error(connection, ec);
    }
  };

  template <class socket_type>
  class SocketServer : public SocketServerBase<socket_type> {};

  using WS = asio::ip::tcp::socket;

  template <>
  class SocketServer<WS> : public SocketServerBase<WS> {
  public:
    SocketServer() noexcept : SocketServerBase<WS>(80) {}

  protected:
    void accept() override {
      std::shared_ptr<Connection> connection(new Connection(handler_runner, config.timeout_idle, *io_service));

      acceptor->async_accept(*connection->socket, [this, connection](const error_code &ec) {
        auto lock = connection->handler_runner->continue_lock();
        if(!lock)
          return;
        // Immediately start accepting a new connection (if io_service hasn't been stopped)
        if(ec != asio::error::operation_aborted)
          accept();

        if(!ec) {
          asio::ip::tcp::no_delay option(true);
          connection->socket->set_option(option);

          read_handshake(connection);
        }
      });
    }
  };
} // namespace SimpleWeb

#endif /* SERVER_WS_HPP */
