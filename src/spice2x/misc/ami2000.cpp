#include "ami2000.h"

#include "acio/icca/icca.h"
#include "util/detour.h"
#include "util/libutils.h"
#include "util/logging.h"
#include "eamuse.h"

static const auto ROM_VERSION = "DUMMY";

static HINSTANCE AMI2000_INSTANCE;

static struct {
    bool card_pressed;
    uint32_t card_type;
} readers[8] = {};

static uint64_t AMI2000_GetCardIdentifier(uint32_t unit_id) {
    auto &reader = readers[unit_id];
    if (eamuse_card_insert_consume(1, unit_id)) {
        if (!reader.card_pressed) {
            reader.card_pressed = true;

            // get and check if valid card
            uint64_t card_id = 0;
            if (!eamuse_get_card(1, unit_id, (uint8_t *) &card_id)) {
                return 0;
            }

            reader.card_type = is_card_uid_felica((uint8_t *) &card_id) ? 2 : 1;

            return _byteswap_uint64(card_id);
        }
    } else {
        reader.card_pressed = false;
        reader.card_type = 0;
    }

    return 0;
}

static uint32_t AMI2000_GetCardType(uint32_t unit_id) {
    return readers[unit_id].card_type;
}

static void __fastcall AMI2000_Initialize(void *a1) {
}

static void __fastcall AMI2000_Startup(uint32_t unit_id, void *a2, uint32_t a3) {
}

static bool __fastcall AMI2000_IsDeviceError(uint32_t unit_id) {
    return false;
}

static int64_t __fastcall AMI2000_ReadCtrl(uint32_t unit_id, uint8_t state) {
    return 0;
}

static void __fastcall AMI2000_Cleanup(uint32_t unit_id) {
}

static const char *__fastcall AMI2000_GetRomVersion(uint32_t unit_id) {
    return ROM_VERSION;
}

void ami2000_attach() {
    AMI2000_INSTANCE = libutils::try_library("iccr-ami2000-api.dll");
    if (!AMI2000_INSTANCE) {
        return;
    }

    log_info("ami2000", "ami2000 attached");

    detour::inline_hook(AMI2000_Initialize, libutils::try_proc(AMI2000_INSTANCE,
                                                               "AMI2000_Initialize"));
    detour::inline_hook(AMI2000_Startup, libutils::try_proc(AMI2000_INSTANCE,
                                                            "AMI2000_Startup"));
    detour::inline_hook(AMI2000_IsDeviceError, libutils::try_proc(AMI2000_INSTANCE,
                                                                  "AMI2000_IsDeviceError"));
    detour::inline_hook(AMI2000_ReadCtrl, libutils::try_proc(AMI2000_INSTANCE,
                                                             "AMI2000_ReadCtrl"));
    detour::inline_hook(AMI2000_Cleanup, libutils::try_proc(AMI2000_INSTANCE,
                                                            "AMI2000_Cleanup"));
    detour::inline_hook(AMI2000_GetRomVersion, libutils::try_proc(AMI2000_INSTANCE,
                                                                  "AMI2000_GetRomVersion"));
    detour::inline_hook(AMI2000_GetCardType, libutils::try_proc(AMI2000_INSTANCE,
                                                                "AMI2000_GetCardType"));
    detour::inline_hook(AMI2000_GetCardIdentifier, libutils::try_proc(AMI2000_INSTANCE,
                                                                      "AMI2000_GetCardIdentifier"));
}

void ami2000_detach() {
    // TODO
}
