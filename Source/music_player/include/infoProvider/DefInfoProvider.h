#pragma once
#include "IInfoProvider.h"

namespace eqd_mp
{
    struct EQD_MP_API DefInfoProvider : public IInfoProvider
    {
        DefInfoProvider(const MUSIC_INFO_STR_TYPE& rootPath);
        std::vector<MusicInfo> getRoot();
        std::vector<MusicInfo> getAlbum(const MusicInfo&);
        std::vector<MusicInfo> getMusicByPath(const MUSIC_INFO_STR_TYPE& path);
    private:
        MUSIC_INFO_STR_TYPE _rootPath;
    };
}
