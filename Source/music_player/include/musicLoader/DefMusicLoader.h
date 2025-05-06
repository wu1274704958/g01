#pragma once
#include "DefMusicLoader.h"
#include <bass.h>
#include <string>
#include "IMusicLoader.h"

namespace eqd_mp
{
    
    struct DefMusicLoader : public eqd_mp::IMusicLoader<HSTREAM,std::wstring_view>
    {
        std::pair<bool,HSTREAM> loadMusic(const std::span<uint8_t>& data);
        bool isSupport(const std::wstring_view& suffix);
    };
}

