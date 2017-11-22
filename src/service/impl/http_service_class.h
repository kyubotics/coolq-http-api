//#pragma once
//
//#include "../service_base_class.h"
//#include "web_server/server_http.hpp"
//
//class HttpService final : public ServiceBase {
//public:
//    void start() override;
//    void stop() override;
//    bool good() const override;
//
//protected:
//    void init() override;
//    void finalize() override;
//
//private:
//    std::shared_ptr<SimpleWeb::Server<SimpleWeb::HTTP>> server_;
//    std::thread thread_;
//};
