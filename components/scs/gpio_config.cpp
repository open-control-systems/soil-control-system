/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstring>

#include "scs/gpio_config.h"

namespace ocs {
namespace app {

namespace {

const gpio_num_t RELAY_GPIO = GPIO_NUM_26;
const unsigned GPIO_OUTPUT_PIN_SEL = ((1ULL << RELAY_GPIO));

} // namespace

GpioConfig::GpioConfig() {
    memset(&config_, 0, sizeof(config_));

    // disable interrupt
    config_.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    config_.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set,
    config_.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    // enable pull-down mode
    config_.pull_down_en = GPIO_PULLDOWN_ENABLE;
    // disable pull-up mode
    config_.pull_up_en = GPIO_PULLUP_DISABLE;
    // configure GPIO with the given settings
    gpio_config(&config_);
}

} // namespace app
} // namespace ocs
