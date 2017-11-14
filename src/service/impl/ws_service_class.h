#pragma once

#include <thread>

#include "../service_base_class.h"
#include "../pushable_interface.h"
#include "web_server/server_ws.hpp"

class WsService final : public ServiceBase, public IPushable {
public:
	void start() override;
	void stop() override;
	bool good() const override;

	void push_event(const json &payload) const override;

protected:
	void init() override;
	void finalize() override;

private:
	std::shared_ptr<SimpleWeb::SocketServer<SimpleWeb::WS>> server_;
	std::thread thread_;
};
