#pragma once

#include "IPlaylistHandler.h"

namespace eqd_mp
{
    struct LoopPlaylist : public IPlaylistHandler
    {
        std::vector<MusicInfo*> getPlaylist(const std::vector<MusicInfo> &originList);
        protected:
    };
}
