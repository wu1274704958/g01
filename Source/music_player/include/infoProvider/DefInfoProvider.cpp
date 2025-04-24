#include "DefInfoProvider.h"
#include <filesystem>
namespace fs = std::filesystem; 

eqd_mp::DefInfoProvider::DefInfoProvider(const std::wstring& rootPath)
{
    _rootPath = rootPath;
}

std::vector<eqd_mp::MusicInfo> eqd_mp::DefInfoProvider::getRoot()
{
    return getMusicByPath(_rootPath);
}

std::vector<eqd_mp::MusicInfo> eqd_mp::DefInfoProvider::getAlbum(const MusicInfo& info)
{
    return getMusicByPath(info.Path());
}

std::vector<eqd_mp::MusicInfo> eqd_mp::DefInfoProvider::getMusicByPath(const std::wstring& path)
{
    std::vector<eqd_mp::MusicInfo> result;
    std::filesystem::path rootPath(path);
    if (!is_directory(rootPath))
        return result;
    for (const auto& entry : fs::directory_iterator(rootPath)) {
        if (entry.is_regular_file()) {
#ifdef PF_WIN32
                auto filePath = entry.path().wstring();
                auto fileExtension = entry.path().extension().wstring();
                auto fileName = entry.path().filename().wstring();
#else
                auto filePath = entry.path().string();
                auto fileExtension = entry.path().string();
                auto fileName = entry.path().filename().string();
#endif
            
            eqd_mp::MusicInfo musicInfo;
            musicInfo.Set_Type(eqd_mp::MusicInfo::EType::Music);
            musicInfo.Set_Path(filePath);
            musicInfo.Set_Name(subStr(musicInfo.Path(), fileName));
            musicInfo.Set_Suffix(subStr(musicInfo.Path(), fileExtension));
           
            if (filter(musicInfo)) 
                result.push_back(musicInfo);
        } else if (entry.is_directory()) {
#ifdef PF_WIN32
                auto filePath = entry.path().wstring();
                auto fileName = entry.path().filename().wstring();
#else
                auto filePath = entry.path().string();
                auto fileName = entry.path().filename().string();
#endif
            eqd_mp::MusicInfo musicInfo;
            musicInfo.Set_Type(eqd_mp::MusicInfo::EType::Album);
            musicInfo.Set_Path(filePath);
            musicInfo.Set_Name(subStr(musicInfo.Path(), fileName));
            if (filter(musicInfo)) 
                result.push_back(musicInfo);
        }
    }
    return result;
}

