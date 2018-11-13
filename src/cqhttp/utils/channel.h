#pragma once

#include <future>
#include <list>

template <class Elem>
class Channel {
public:
    Channel() : max_size_(0) {}
    Channel(const size_t max_size) : max_size_(max_size) {}

    void put(const Elem &elem) {
        std::unique_lock<std::mutex> lock(m_);

        if (promise_queue_.empty()) {
            // there is no promise at present, so we put the element into the queue
            if (max_size_ != 0 && elem_queue_.size() >= max_size_) {
                elem_queue_.pop_front();
            }
            elem_queue_.push_back(elem);
            return;
        }

        auto promise = promise_queue_.front();
        promise_queue_.pop_front();
        lock.unlock();
        promise->set_value(elem);
    }

    bool get(Elem &out, const bool wait = true, const unsigned long long timeout_ms = 0) {
        std::unique_lock<std::mutex> lock(m_);

        if (!elem_queue_.empty()) {
            out = elem_queue_.front();
            elem_queue_.pop_front();
            return true;
        }
        if (!wait) return false;

        auto promise = std::make_shared<std::promise<Elem>>();
        promise_queue_.push_back(promise);
        auto future = promise->get_future();

        lock.unlock(); // we've got the future, let put() call set_value() on the promise

        if (timeout_ms == 0) {
            out = future.get(); // wait until there is a value
            return true;
        }
        auto state = future.wait_for(std::chrono::milliseconds(timeout_ms));
        if (state == std::future_status::ready) {
            out = future.get();
            return true;
        }
        lock.lock();
        promise_queue_.remove(promise);
        return false;
    }

    struct CloseException : std::exception {};

    void close() {
        std::unique_lock<std::mutex> lock(m_);
        for (auto promise : promise_queue_) {
            promise->set_exception(make_exception_ptr(CloseException()));
        }
    }

private:
    std::list<Elem> elem_queue_;
    std::list<std::shared_ptr<std::promise<Elem>>> promise_queue_;
    std::mutex m_;
    size_t max_size_;
};
