#pragma once
#include "IPlaylistHandler.h"

namespace eqd_mp
{
    struct EQD_MP_API RandomPlaylist : public IPlaylistHandler
    {
        RandomPlaylist(int seed = 0) : _seed(seed) {}
        std::vector<MusicInfo*> getPlaylist(const std::vector<MusicInfo> &originList);
    protected:
        int _seed;
    };
    
}
