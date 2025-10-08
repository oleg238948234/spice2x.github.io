#pragma once

#include "acioemu/acioemu.h"
#include "hooks/devicehook.h"

namespace games::gitadora {

    enum ExtIoType {
        J32D,
        J33I,
        DETECT
    };

    class GDExtIoDevice : public acioemu::ACIODeviceEmu {
    public:
        virtual void report_io(circular_buffer<uint8_t> &response_buffer) = 0;
    };

    class GDExtIoHandle : public CustomHandle {
    private:
        LPCWSTR com_port;
        ExtIoType type;
        GDExtIoDevice *device;
        acioemu::ACIOEmu acio_emu;
        uint8_t unit;

    public:
        GDExtIoHandle(LPCWSTR lpCOMPort, ExtIoType type, uint8_t unit);
        bool open(LPCWSTR lpFileName) override;
        int read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead) override;
        int write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite) override;
        int device_io(DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer,
                      DWORD nOutBufferSize) override;
        bool close() override;
    };

}
