#pragma once
#include "MusicInfo.h"
#include "../api_def.h"

namespace eqd_mp
{
    template<typename T>
    struct IMusicPlayer
    {
        using MUSIC_TYPE = T;
    public:
        bool play(const MusicInfo& info,bool loop);
        bool stop();
        bool pause();
        bool resume();
        bool rewind();
        bool isPlaying() const;
        const MusicInfo* getCurrentPlaying() const;
        bool isSupport(const MusicInfo&) const;
        void clear();
        int getError() const;
        bool initialized() const;
        const T& getCurrentMusic() const;
        int getChannelCount() const;
    };

    template<typename T,typename MT>
    concept VaildMusicPlayerWithMT = requires
    {
        requires std::is_base_of_v<IMusicPlayer<MT>, T>;  
    };
    template<typename T>
    concept VaildMusicPlayer = requires
    {
        requires std::is_base_of_v<IMusicPlayer<typename T::MUSIC_TYPE>, T>;  
    };
}

#include "IMusicPlayer.hpp"