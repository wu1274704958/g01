#pragma once
#include "..\MusicInfo.h"
#include <vector>
#include <functional>
namespace eqd_mp
{
    struct EQD_MP_API IInfoProvider
    {
        std::vector<MusicInfo> getRoot();
        std::vector<MusicInfo> getAlbum(const MusicInfo&);
        void addFilter(std::function<bool(const MusicInfo&)> filter);
        void removeAllFilter();
        bool filter(const MusicInfo& info) const;
        static std::string_view subStr(const std::string& src,const std::string& sub);
        static std::wstring_view subStr(const std::wstring& src,const std::wstring& sub);
    protected:
        std::vector<std::function<bool(const MusicInfo&)>> _filters;
    };
}