#pragma once
#include "IPlaylistHandler.h"

namespace eqd_mp
{
    struct NextSongPlaylist : public IPlaylistHandler
    {
        NextSongPlaylist(std::vector<MusicInfo*>& currentPlaylist,const MusicInfo& next_song):
            _currentPlaylist(currentPlaylist),_next_song(next_song)
        {}
        std::vector<MusicInfo*> getPlaylist(const std::vector<MusicInfo> &originList);
    protected:
        std::vector<MusicInfo*>& _currentPlaylist;
        const MusicInfo& _next_song;
    };
}
