#pragma once

#include "IPlaylistHandler.h"

namespace eqd_mp
{
    struct EQD_MP_API LoopPlaylist : public IPlaylistHandler
    {
        std::vector<MusicInfo*> getPlaylist(const std::vector<MusicInfo> &originList);
        protected:
    };
}
