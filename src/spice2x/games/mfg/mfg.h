#pragma once

#include "games/game.h"

namespace games::mfg {
    extern std::string MFG_INJECT_ARGS;
    extern std::string MFG_CABINET_TYPE;
    extern bool MFG_NO_IO;
    extern bool MFG_NO_ICCA;

    class MFGGame : public games::Game {
    public:
        MFGGame() : Game("Mahjong Fight Girl") {}

        virtual void attach() override;
        virtual void detach() override;
    };
}
