// 
// application_class.h : Define Application class,
// which is used to maintain the app's lifecircle.
// 
// Copyright (C) 2017  Richard Chien <richardchienthebest@gmail.com>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 

#pragma once

#include "common.h"

class Application {
public:
    void initialize(int32_t auth_code);
    void enable();
    void disable();

    bool is_initialized() const { return initialized_; }
    bool is_enabled() const { return enabled_; }

private:
    bool initialized_ = false;
    bool enabled_ = false;
};
