/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "driver/gpio.h"

#include "ocs_core/noncopyable.h"

namespace ocs {
namespace app {

class GpioConfig : public core::NonCopyable<> {
public:
    //! Initialize system wide GPIO configuration.
    GpioConfig();

private:
    gpio_config_t config_ {};
};

} // namespace app
} // namespace ocs
