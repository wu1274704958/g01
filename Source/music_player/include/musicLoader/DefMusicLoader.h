#pragma once
#include "DefMusicLoader.h"
#include <bass.h>
#include <string>
#include "IMusicLoader.h"
#include "api_def.h"

namespace eqd_mp
{
    
    struct EQD_MP_API DefMusicLoader : public eqd_mp::IMusicLoader<HSTREAM,std::wstring_view>
    {
        std::pair<bool,HSTREAM> loadMusic(const std::span<uint8_t>& data);
        bool isSupport(const std::wstring_view& suffix) const;
    };
}

