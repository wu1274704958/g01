#pragma once
#include <string_view>
#include <vector>

#include "bass.h"
#include "IMusicLoader.h"
#include "api_def.h"

namespace eqd_mp
{
    struct EQD_MP_API FlacLoader : public IMusicLoader<HSTREAM,std::wstring_view>
    {
        std::pair<bool,HSTREAM> loadMusic(const std::span<uint8_t>& data);
        bool isSupport(const std::wstring_view& suffix) const;
    };
}
