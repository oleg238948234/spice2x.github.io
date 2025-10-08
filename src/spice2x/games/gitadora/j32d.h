#pragma once

#include "extio.h"

namespace games::gitadora {

    class J32DDevice : public GDExtIoDevice {
    private:
        uint8_t pid;
        bool start_report;

    public:
        J32DDevice();

        bool parse_msg(acioemu::MessageData *msg_in, circular_buffer<uint8_t> *response_buffer) override;

        void report_io(circular_buffer<uint8_t> &response_buffer) override;
    };

}
