#pragma once
#include <string_view>
#include <vector>

#include "bass.h"
#include "IMusicLoader.h"

namespace eqd_mp
{
    struct FlacLoader : public IMusicLoader<HSTREAM,std::wstring_view>
    {
        std::pair<bool,HSTREAM> loadMusic(const std::span<uint8_t>& data);
        bool isSupport(const std::wstring_view& suffix);
    };
}
