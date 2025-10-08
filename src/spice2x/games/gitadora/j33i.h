#pragma once

#include "extio.h"

namespace games::gitadora {

    class J33IDevice : public GDExtIoDevice {
    private:
        uint8_t unit;
        uint8_t pid;
        uint8_t motor;
        bool start_report;

    public:
        explicit J33IDevice(uint8_t unit);

        bool parse_msg(acioemu::MessageData *msg_in, circular_buffer<uint8_t> *response_buffer) override;

        void report_io(circular_buffer<uint8_t> &response_buffer) override;
    };

}
