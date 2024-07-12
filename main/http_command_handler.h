/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "ocs_core/noncopyable.h"
#include "ocs_net/http_server.h"
#include "soil_moisture_monitor.h"

namespace ocs {
namespace app {

class HTTPCommandHandler : public core::NonCopyable<> {
public:
    //! Initialize.
    //!
    //! @params
    //!  - @p server to register HTTP commands.
    //!  - @p monitor to request a new soil moisture reading.
    HTTPCommandHandler(net::HTTPServer& server, SoilMoistureMonitor& monitor);
};

} // namespace app
} // namespace ocs
