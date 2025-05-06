#pragma once
#include <vector>
#include <tuple>
#include <span>

namespace eqd_mp
{
    template<typename R,typename S>
    struct IMusicLoader
    {
        using MusicType = R;
        using SuffixType = S;
        std::pair<bool,R> loadMusic(const std::span<uint8_t>& data)
        {
            return {};
        }
        bool isSupport(const S& suffix)
        {
            return false;
        }
    };

    template <typename T,typename MT,typename ST>
   concept VaildMusicLoader = requires
    {
        requires std::is_base_of_v<IMusicLoader<MT,ST>,T>;
    };
}
