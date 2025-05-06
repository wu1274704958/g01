#pragma once

#include "api_def.h"
#include <vector>

#include "MusicInfo.h"

namespace eqd_mp
{
    struct EQD_MP_API IPlaylistHandler
    {
        std::vector<MusicInfo*> getPlaylist(const std::vector<MusicInfo> &originList);
    protected:
    };
    
    template <typename T>
        concept VaildPlaylistHandler = requires
    {
        requires std::is_base_of_v<IPlaylistHandler,T>;
    };
}
