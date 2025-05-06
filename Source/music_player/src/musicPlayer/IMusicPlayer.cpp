#include "musicPlayer/IMusicPlayer.h"

bool eqd_mp::IMusicPlayer::play(const MusicInfo& info,bool loop)
{
    return false;
}

bool eqd_mp::IMusicPlayer::stop()
{
    return false;
}

bool eqd_mp::IMusicPlayer::pause()
{
    return false;
}

bool eqd_mp::IMusicPlayer::resume()
{
    return false;
}

bool eqd_mp::IMusicPlayer::rewind()
{
    return false;
}

bool eqd_mp::IMusicPlayer::isPlaying() const
{
    return false;
}

const eqd_mp::MusicInfo* eqd_mp::IMusicPlayer::getCurrentPlaying() const
{
    return nullptr;
}

bool eqd_mp::IMusicPlayer::isSupport(const MusicInfo&) const
{
    return false;
}
