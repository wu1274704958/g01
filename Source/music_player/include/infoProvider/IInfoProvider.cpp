#include "IInfoProvider.h"

std::vector<eqd_mp::MusicInfo> eqd_mp::IInfoProvider::getRoot()
{
    return {};
}

std::vector<eqd_mp::MusicInfo> eqd_mp::IInfoProvider::getAlbum(const MusicInfo&)
{
    return {};
}

void eqd_mp::IInfoProvider::addFilter(std::function<bool(const MusicInfo&)> filter)
{
    _filters.push_back(filter);
}

void eqd_mp::IInfoProvider::removeAllFilter()
{
    _filters.clear();
}

bool eqd_mp::IInfoProvider::filter(const MusicInfo& info) const
{
    for (const auto& filter : _filters)
    {
        if (!filter(info))
            return false;
    }
    return true;
}

std::string_view eqd_mp::IInfoProvider::subStr(const std::string& src, const std::string& sub)
{
    std::string_view sv(src);
    auto pos = sv.find(sub);
    if (pos == std::string::npos)
        return {};
    return sv.substr(pos, sub.size());
}

std::wstring_view eqd_mp::IInfoProvider::subStr(const std::wstring& src, const std::wstring& sub)
{
    std::wstring_view sv(src);
    auto pos = sv.find(sub);
    if (pos == std::string::npos)
        return {};
    return sv.substr(pos, sub.size());
}



