#pragma once

#include "common.h"

#include "./service_base_class.h"
#include "./pushable_interface.h"

class ServiceHub final : public ServiceBase, public IPushable {
public:
    void start() override;
    void stop() override;
    bool good() const override;

    void push_event(const json &payload) const override;
    bool has_pushable_services() const { return !pushable_services_.empty(); }

    using ServiceMap = std::map<std::string, std::shared_ptr<ServiceBase>>;

    const ServiceMap &get_services() const { return services_; }

private:
    ServiceMap services_;
    std::vector<std::shared_ptr<IPushable>> pushable_services_;
};
