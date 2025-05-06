#include "playlistHandler/RandomPlaylist.h"
#include <utils/random.hpp>

std::vector<eqd_mp::MusicInfo*> eqd_mp::RandomPlaylist::getPlaylist(const std::vector<MusicInfo>& originList)
{
    std::vector<eqd_mp::MusicInfo*> playlist;
    wws::randomUnique<eqd_mp::MusicInfo>(originList,originList.size(),[&playlist](const eqd_mp::MusicInfo& info)->void
    {
        playlist.push_back(const_cast<MusicInfo*>(&info));
    },_seed);    
    return playlist;
}
