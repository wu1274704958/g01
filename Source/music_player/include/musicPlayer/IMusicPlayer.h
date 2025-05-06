#pragma once
#include "MusicInfo.h"
#include "../api_def.h"

namespace eqd_mp
{
    struct EQD_MP_API IMusicPlayer
    {
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
    };

    template<typename T>
    concept VaildMusicPlayer = requires
    {
        requires std::is_base_of_v<IMusicPlayer, T>;  
    };
    
}
