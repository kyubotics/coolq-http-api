#pragma once

#include "common.h"

class IPushable {
public:
    virtual ~IPushable() = default;
    virtual void push_event(const json &payload) const = 0;
};
