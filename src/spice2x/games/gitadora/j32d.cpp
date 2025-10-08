#include "j32d.h"

#include "acioemu/device.h"
#include "rawinput/rawinput.h"
#include "games/io.h"
#include "io.h"

using namespace acioemu;

namespace games::gitadora {

    static const uint8_t ACIO_ADDRESS = 1;

    enum J32DCommand : uint16_t {
        J32D_REPORT_START = 0x0120,
        J32D_REPORT_DATA = 0x012F,
    };

#pragma pack(push, 1)
    struct J32DReport {
        uint16_t analog[8];
        uint8_t btn;
    };
#pragma pack(pop)

    J32DDevice::J32DDevice() {
        this->node_count = 1;
        this->start_report = false;
        this->pid = 1;
    }

    void J32DDevice::report_io(circular_buffer<uint8_t> &response_buffer) {
        if (!this->start_report) {
            return;
        }

        J32DReport report;
        memset(&report, 0, sizeof(report));

        RI_MGR->devices_flush_output();

        auto &buttons = get_buttons();
        auto ht = (uint16_t) (GameAPI::Buttons::getVelocity(RI_MGR, buttons[Buttons::DrumHiTom]) * 1023);
        auto lt = (uint16_t) (GameAPI::Buttons::getVelocity(RI_MGR, buttons[Buttons::DrumLowTom]) * 1023);
        auto sn = (uint16_t) (GameAPI::Buttons::getVelocity(RI_MGR, buttons[Buttons::DrumSnare]) * 1023);
        auto ft = (uint16_t) (GameAPI::Buttons::getVelocity(RI_MGR, buttons[Buttons::DrumFloorTom]) * 1023);
        auto lc = (uint16_t) (GameAPI::Buttons::getVelocity(RI_MGR, buttons[Buttons::DrumLeftCymbal]) * 1023);
        auto rc = (uint16_t) (GameAPI::Buttons::getVelocity(RI_MGR, buttons[Buttons::DrumRightCymbal]) * 1023);
        auto hh = (uint16_t) (GameAPI::Buttons::getVelocity(RI_MGR, buttons[Buttons::DrumHiHat]) * 1023);

#define ENCODE_ANALOG(__val) ((__val & 0x3f) << 10) | ((__val & 0x3c0) >> 6)
        report.analog[0] = ENCODE_ANALOG(ht);
        report.analog[1] = ENCODE_ANALOG(lt);
        report.analog[2] = ENCODE_ANALOG(sn);
        report.analog[3] = ENCODE_ANALOG(ft);
        report.analog[4] = ENCODE_ANALOG(lc);
        report.analog[5] = ENCODE_ANALOG(rc);
        report.analog[6] = ENCODE_ANALOG(hh);
#undef ENCODE_ANALOG

        if (GameAPI::Buttons::getState(RI_MGR, buttons[Buttons::DrumBassPedal]))
            report.btn |= 1;

        if (GameAPI::Buttons::getState(RI_MGR, buttons[Buttons::DrumLeftPedal]))
            report.btn |= 2;

        auto msg = create_msg(ACIO_ADDRESS, J32D_REPORT_DATA, ++pid, sizeof(J32DReport),
                              reinterpret_cast<uint8_t *>(&report));
        write_msg(msg, &response_buffer);
        delete msg;
    }

    bool J32DDevice::parse_msg(acioemu::MessageData *msg_in, circular_buffer<uint8_t> *response_buffer) {
        switch (msg_in->cmd.code) {
            case ACIO_CMD_GET_VERSION: {
                // send version data
                auto msg = create_msg(ACIO_ADDRESS, ACIO_CMD_GET_VERSION, ++pid, MSG_VERSION_SIZE);
                set_version(msg, 0x020A, 0, 0, 1, 0, "J32D");
                write_msg(msg, response_buffer);
                delete msg;
                break;
            }
            case J32D_REPORT_START: {
                this->start_report = true;
                auto msg = create_msg_status(ACIO_ADDRESS, J32D_REPORT_START, ++pid, 0);
                write_msg(msg, response_buffer);
                delete msg;
                break;
            }
            case ACIO_CMD_STARTUP: {
                // send status 0
                this->start_report = false;
                auto msg = create_msg_status(ACIO_ADDRESS, ACIO_CMD_STARTUP, ++pid, 0);
                write_msg(msg, response_buffer);
                delete msg;
                break;
            }
            default:
                return false;
        }

        return true;
    }

}
