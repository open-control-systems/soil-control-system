/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "esp_log.h"

#include "ocs_status/code_to_str.h"
#include "ocs_system/default_clock.h"
#include "ocs_system/default_rebooter.h"
#include "ocs_system/delay_rebooter.h"
#include "scs/adc_reader.h"
#include "scs/console_telemetry_writer.h"
#include "scs/control_pipeline.h"
#include "scs/yl69_moisture_reader.h"

namespace ocs {
namespace app {

namespace {

const char* log_tag = "control-pipeline";

} // namespace

ControlPipeline::ControlPipeline() {
    adc_reader_.reset(new (std::nothrow) ADCReader(ADCReader::Params {
        .channel = static_cast<adc_channel_t>(CONFIG_SMC_SENSOR_ADC_CHANNEL),
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_10,
    }));

    moisture_reader_.reset(
        new (std::nothrow) YL69MoistureReader(CONFIG_SMC_SENSOR_THRESHOLD, *adc_reader_));

    flash_initializer_.reset(new (std::nothrow) storage::FlashInitializer());

    http_server_pipeline_.reset(new (std::nothrow) iot::HTTPServerPipeline());

    default_clock_.reset(new (std::nothrow) system::DefaultClock());

    fanout_reboot_handler_.reset(new (std::nothrow) system::FanoutRebootHandler());

    default_rebooter_.reset(new (std::nothrow)
                                system::DefaultRebooter(*fanout_reboot_handler_));

    delay_rebooter_.reset(
        new (std::nothrow) system::DelayRebooter(pdMS_TO_TICKS(500), *default_rebooter_));

    fanout_telemetry_writer_.reset(new (std::nothrow) FanoutTelemetryWriter());

    telemetry_holder_.reset(new (std::nothrow) TelemetryHolder());
    fanout_telemetry_writer_->add(*telemetry_holder_);

    telemetry_formatter_.reset(new (std::nothrow) TelemetryFormatter());
    telemetry_formatter_->fanout().add(*telemetry_holder_);

    http_telemetry_handler_.reset(new (std::nothrow) HTTPTelemetryHandler(
        http_server_pipeline_->server(), *telemetry_formatter_, "/telemetry",
        "http-telemetry-handler"));

    console_telemetry_writer_.reset(new (std::nothrow)
                                        ConsoleTelemetryWriter(*telemetry_formatter_));
    fanout_telemetry_writer_->add(*console_telemetry_writer_);

    gpio_config_.reset(new (std::nothrow) GPIOConfig());

    soil_moisture_monitor_.reset(new (std::nothrow) SoilMoistureMonitor(
        SoilMoistureMonitor::Params {
            .power_on_delay_interval =
                (1000 * CONFIG_SMC_POWER_ON_DELAY_INTERVAL) / portTICK_PERIOD_MS,
            .read_interval = (1000 * CONFIG_SMC_READ_INTERVAL) / portTICK_PERIOD_MS,
            .relay_gpio = static_cast<gpio_num_t>(CONFIG_SMC_RELAY_GPIO),
        },
        *moisture_reader_, *fanout_telemetry_writer_));

    http_command_handler_.reset(new (std::nothrow) HTTPCommandHandler(
        *delay_rebooter_, http_server_pipeline_->server(), *soil_moisture_monitor_));

    registration_formatter_.reset(
        new (std::nothrow) RegistrationFormatter(http_server_pipeline_->network()));

    http_registration_handler_.reset(new (std::nothrow) HTTPRegistrationHandler(
        http_server_pipeline_->server(), *registration_formatter_, "/registration",
        "http-registration-handler"));

    storage_builder_.reset(new (std::nothrow) storage::StorageBuilder());

    system_counter_storage_ = storage_builder_->make("system_counter");
    configASSERT(system_counter_storage_);

    counter_json_formatter_.reset(new (std::nothrow) iot::CounterJSONFormatter());

    system_counter_pipeline_.reset(new (std::nothrow) iot::SystemCounterPipeline(
        *default_clock_, *system_counter_storage_, *fanout_reboot_handler_,
        *counter_json_formatter_));

    telemetry_formatter_->fanout().add(*counter_json_formatter_);

    soil_counter_storage_ = storage_builder_->make("soil_counter");
    configASSERT(soil_counter_storage_);

    soil_status_monitor_.reset(new (std::nothrow) SoilStatusMonitor(
        *default_clock_, *soil_counter_storage_, *fanout_reboot_handler_,
        *counter_json_formatter_));

    fanout_telemetry_writer_->add(*soil_status_monitor_);
}

void ControlPipeline::start() {
    const auto code = http_server_pipeline_->start();
    if (code != status::StatusCode::OK) {
        ESP_LOGE(log_tag, "failed to start HTTP server pipeline: code=%s",
                 status::code_to_str(code));
    } else {
        register_mdns_endpoints_();
    }

    soil_moisture_monitor_->start();
}

void ControlPipeline::register_mdns_endpoints_() {
    http_server_pipeline_->mdns().add_service_txt_records(
        "_http", "_tcp",
        net::MDNSProvider::TxtRecordList {
            {
                "telemetry",
                "/telemetry",
            },
            {
                "registration",
                "/registration",
            },
            {
                "command_reboot",
                "/commands/reboot",
            },
            {
                "command_reload",
                "/commands/reload",
            },
        });
}

} // namespace app
} // namespace ocs
