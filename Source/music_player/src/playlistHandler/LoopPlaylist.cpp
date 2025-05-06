#include "playlistHandler/LoopPlaylist.h"

std::vector<eqd_mp::MusicInfo*> eqd_mp::LoopPlaylist::getPlaylist(const std::vector<MusicInfo>& originList)
{
    std::vector<eqd_mp::MusicInfo*> playlist;
    for (auto it = originList.begin(); it != originList.end(); it++)
    {
        playlist.push_back(const_cast<MusicInfo*>(&(*it)));
    }
    return playlist;
}
