#include "j33i.h"

#include "acioemu/device.h"
#include "rawinput/rawinput.h"
#include "games/io.h"
#include "io.h"

using namespace acioemu;

namespace games::gitadora {

    static const uint8_t ACIO_ADDRESS = 1;

    enum J33ICommand : uint16_t {
        J33I_REPORT_START = 0x0120,
        J33I_REPORT_DATA = 0x012F,
        J33I_SET_MOTOR = 0x0130,
    };

#pragma pack(push, 1)
    struct J33IReport {
        uint16_t gyro_x;
        uint16_t gyro_y;
        uint16_t gyro_z;

        bool : 3;
        bool p : 1;
        bool y : 1;
        bool b : 1;
        bool g : 1;
        bool r : 1;

        uint8_t knob : 4;
        bool pick_down : 1;
        bool pick_up : 1;
        bool : 2;

        uint8_t motor;
    };
#pragma pack(pop)

    J33IDevice::J33IDevice(uint8_t unit) {
        this->node_count = 1;
        this->start_report = false;
        this->unit = unit;
        this->pid = 1;
        this->motor = 0;
    }

    void J33IDevice::report_io(circular_buffer<uint8_t> &response_buffer) {
        if (!this->start_report) {
            return;
        }

        J33IReport report;
        memset(&report, 0, sizeof(report));

        RI_MGR->devices_flush_output();

        auto &buttons = get_buttons();
        auto btn_offset = unit * GUITAR_NUM_BUTTONS;
        report.r = GameAPI::Buttons::getState(RI_MGR, buttons[Buttons::GuitarP1R + btn_offset]);
        report.g = GameAPI::Buttons::getState(RI_MGR, buttons[Buttons::GuitarP1G + btn_offset]);
        report.b = GameAPI::Buttons::getState(RI_MGR, buttons[Buttons::GuitarP1B + btn_offset]);
        report.y = GameAPI::Buttons::getState(RI_MGR, buttons[Buttons::GuitarP1Y + btn_offset]);
        report.p = GameAPI::Buttons::getState(RI_MGR, buttons[Buttons::GuitarP1P + btn_offset]);
        report.pick_up = GameAPI::Buttons::getState(RI_MGR, buttons[Buttons::GuitarP1PickUp + btn_offset]);
        report.pick_down = GameAPI::Buttons::getState(RI_MGR, buttons[Buttons::GuitarP1PickDown + btn_offset]);

        auto &analogs = get_analogs();
        auto analog_offset = unit * GUITAR_NUM_ANALOGS;
        auto x = (int16_t) ((GameAPI::Analogs::getState(RI_MGR, analogs[Analogs::GuitarP1WailX + analog_offset]) - 1024) * 2047);
        auto y = (int16_t) ((GameAPI::Analogs::getState(RI_MGR, analogs[Analogs::GuitarP1WailY + analog_offset]) - 1024) * 2047);
        auto z = (int16_t) ((GameAPI::Analogs::getState(RI_MGR, analogs[Analogs::GuitarP1WailZ + analog_offset]) - 1024) * 2047);
        report.gyro_x = (uint8_t)((int8_t)(x >> 3)) | ((x & 0x07) << 12);
        report.gyro_y = (uint8_t)((int8_t)(y >> 3)) | ((y & 0x07) << 12);
        report.gyro_z = (uint8_t)((int8_t)(z >> 3)) | ((z & 0x07) << 12);

        // TODO knob
        //  it's not used by arena model it seems so i can't test the encoding

        report.motor = this->motor;

        auto msg = create_msg(ACIO_ADDRESS, J33I_REPORT_DATA, ++pid, sizeof(J33IReport),
                              reinterpret_cast<uint8_t *>(&report));
        write_msg(msg, &response_buffer);
        delete msg;
    }

    bool J33IDevice::parse_msg(acioemu::MessageData *msg_in, circular_buffer<uint8_t> *response_buffer) {
        switch (msg_in->cmd.code) {
            case ACIO_CMD_GET_VERSION: {
                // send version data
                auto msg = create_msg(ACIO_ADDRESS, ACIO_CMD_GET_VERSION, ++pid, MSG_VERSION_SIZE);
                set_version(msg, 0x010A, 0, 1, 0, 4, "J33I");
                write_msg(msg, response_buffer);
                delete msg;
                break;
            }
            case J33I_REPORT_START: {
                this->start_report = true;
                auto msg = create_msg_status(ACIO_ADDRESS, J33I_REPORT_START, ++pid, 0);
                write_msg(msg, response_buffer);
                delete msg;
                break;
            }
            case J33I_SET_MOTOR: {
                if (msg_in->cmd.data_size >= 1) {
                    this->motor = msg_in->cmd.raw[0];
                    auto &lights = get_lights();
                    GameAPI::Lights::writeLight(RI_MGR, lights[Lights::GuitarP1Motor + unit],
                                                (float) this->motor / 255.0f);
                }
                auto msg = create_msg_status(ACIO_ADDRESS, J33I_SET_MOTOR, ++pid, 0);
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
