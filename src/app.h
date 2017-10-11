#pragma once

#include "cqp/cqp.h"

#define CQAPP_ID "io.github.richardchien.coolqhttpapi"
#define CQAPP_NAME "CoolQ HTTP API Plugin"
#define CQAPP_VERSION "3.0.0-dev-1"
#define CQAPP_FULLNAME CQAPP_NAME " v" CQAPP_VERSION
#define CQAPP_USER_AGENT "CoolQHttpApi/" CQAPP_VERSION

#include "common.h"

class Sdk;
extern std::optional<Sdk> sdk;
#include "sdk_class.h"

#include "log_class.h"
