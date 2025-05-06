#include "playlistHandler/NextSongPlaylist.h"

std::vector<eqd_mp::MusicInfo*> eqd_mp::NextSongPlaylist::getPlaylist(const std::vector<MusicInfo>& originList)
{
    auto it = std::find_if(_currentPlaylist.begin(), _currentPlaylist.end(), [this](const eqd_mp::MusicInfo* item)
    {
        return item == &_next_song || *item == _next_song;
    });
    if (it != _currentPlaylist.end())
        _currentPlaylist.erase(it);
    _currentPlaylist.insert(_currentPlaylist.begin(),const_cast<MusicInfo*>(&_next_song));
    return _currentPlaylist;
}
