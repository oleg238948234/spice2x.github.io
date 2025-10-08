#include "extio.h"

#include "gitadora.h"
#include "j33i.h"
#include "j32d.h"
#include "util/logging.h"
#include "util/utils.h"

namespace games::gitadora {

    GDExtIoHandle::GDExtIoHandle(LPCWSTR lpCOMPort, games::gitadora::ExtIoType type, uint8_t unit) {
        this->com_port = lpCOMPort;
        this->type = type;
        this->unit = unit;
        this->device = nullptr;
    }

    bool GDExtIoHandle::open(LPCWSTR lpFileName) {
        if (wcscmp(lpFileName, com_port) != 0) {
            return false;
        }

        if (type == ExtIoType::DETECT) {
            type = is_guitar() ? ExtIoType::J33I : ExtIoType::J32D;
        }

        if (type == ExtIoType::J33I) {
            device = new J33IDevice(unit);
        } else {
            device = new J32DDevice();
        }

        log_info("gitadora", "Opened {} ({} IO)", ws2s(com_port), type == ExtIoType::J33I ? "J33I" : "J32D");
        acio_emu.add_device(device);

        return true;
    }

    int GDExtIoHandle::read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead) {
        auto buffer = reinterpret_cast<uint8_t *>(lpBuffer);

        // game calls ReadFile every ~2ms
        device->report_io(acio_emu.get_response_buffer());

        // read from emu
        DWORD bytes_read = 0;
        while (bytes_read < nNumberOfBytesToRead) {
            auto cur_byte = acio_emu.read();

            if (cur_byte.has_value()) {
                buffer[bytes_read++] = cur_byte.value();
            } else {
                break;
            }
        }

#ifdef ACIOEMU_LOG
        if (bytes_read > 0) {
            log_info("extio", "GDExtIoHandle::read: {}", bin2hex(lpBuffer, bytes_read));
        }
#endif

        // return amount of bytes read
        return (int) bytes_read;
    }

    int GDExtIoHandle::write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite) {
        auto buffer = reinterpret_cast<const uint8_t *>(lpBuffer);

        // write to emu
        for (DWORD i = 0; i < nNumberOfBytesToWrite; i++) {
            acio_emu.write(buffer[i]);
        }

#ifdef ACIOEMU_LOG
        log_info("extio", "GDExtIoHandle::write: {}", bin2hex(lpBuffer, nNumberOfBytesToWrite));
#endif

        // return all data written
        return (int) nNumberOfBytesToWrite;
    }

    int GDExtIoHandle::device_io(DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer,
                                 DWORD nOutBufferSize) {
        return -1;
    }

    bool GDExtIoHandle::close() {
        log_info("gitadora", "Closed {} ({} IO)", ws2s(com_port), type == ExtIoType::J33I ? "J33I" : "J32D");
        return true;
    }

}


