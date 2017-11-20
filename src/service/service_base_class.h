#pragma once

class ServiceBase {
public:
    ServiceBase() = default;
    ServiceBase(const ServiceBase &) = delete;
    void operator=(const ServiceBase &) = delete;
    virtual ~ServiceBase() = default;
    virtual void start() = 0;
    virtual void stop() = 0;

    virtual bool initialized() const { return initialized_; }
    virtual bool started() const { return started_; }
    virtual bool good() const { return true; }

protected:
    bool initialized_ = false;
    bool started_ = false;

    virtual void init() {
        initialized_ = true;
    }

    virtual void finalize() {
        started_ = false;
        initialized_ = false;
    }
};
