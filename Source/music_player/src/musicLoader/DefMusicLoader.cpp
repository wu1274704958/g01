#include "musicLoader/DefMusicLoader.h"

std::pair<bool,HSTREAM> eqd_mp::DefMusicLoader::loadMusic(const std::span<uint8_t>& data)
{
    auto stream = BASS_StreamCreateFile(true,data.data(),0,data.size(),0);
    return std::make_pair(stream != 0,stream);
}

bool eqd_mp::DefMusicLoader::isSupport(const std::wstring_view& suffix)
{
    return suffix == L".mp3" || suffix == L".wav";
}
