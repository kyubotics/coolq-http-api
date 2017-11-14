#include "./hub_class.h"
#include "./impl/service_impl_common.h"

#include "app.h"

#include <algorithm>

#include "./impl/http_service_class.h"
#include "./impl/ws_service_class.h"

using namespace std;

void ServiceHub::start() {
	if (config.use_http) {
		auto service = make_shared<HttpService>();
		services_["http"] = service;
		service->start();
	}

	if (config.use_ws) {
		auto service = make_shared<WsService>();
		services_["ws"] = service;
		pushable_services_.push_back(service);
		service->start();
	}

	Log::d(TAG, u8"已开启 API 服务");
}

void ServiceHub::stop() {
	for (auto &item : services_) {
		item.second->stop();
	}
	services_.clear();

	Log::d(TAG, u8"已关闭 API 服务");
}

bool ServiceHub::good() const {
	return all_of(services_.cbegin(), services_.cend(), [](auto item) {
		return item.second->good();
	});
}

void ServiceHub::push_event(const json &payload) const {
	for (const auto &service : pushable_services_) {
		service->push_event(payload);
	}
}
