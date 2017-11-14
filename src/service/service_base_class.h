#pragma once

class ServiceBase {
public:
	virtual ~ServiceBase() = default;
	virtual void start() = 0;
	virtual void stop() = 0;

	bool initialized() const { return initialized_; }
	bool started() const { return started_; }
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
