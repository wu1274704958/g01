#include "musicLoader/FlacLoader.h"
#include <bassflac.h>

std::pair<bool,HSTREAM> eqd_mp::FlacLoader::loadMusic(const std::span<uint8_t>& data)
{
    auto stream = BASS_FLAC_StreamCreateFile(true,data.data(),0,data.size(),0);
    return std::make_pair(stream != 0,stream);
}

bool eqd_mp::FlacLoader::isSupport(const std::wstring_view& suffix)
{
    return suffix == L".flac";
}
