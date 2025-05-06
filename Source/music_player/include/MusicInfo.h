#pragma once

#include <string>
#include <string_view>
#include "api_def.h"
#include <filesystem>

#ifdef PF_WIN32
#define MUSIC_INFO_STR_TYPE std::wstring
#define MUSIC_INFO_STR_VIEW_TYPE std::wstring_view
#define MUSIC_INFO_CHAR_TYPE wchar_t
#else
#define MUSIC_INFO_STR_TYPE std::string
#define MUSIC_INFO_STR_VIEW_TYPE std::string_view
#define MUSIC_INFO_CHAR_TYPE char
#endif

namespace eqd_mp
{
    class EQD_MP_API MusicInfo {
        using CHAR_TYPE = MUSIC_INFO_CHAR_TYPE;
        using STR_TYPE = MUSIC_INFO_STR_TYPE;
        using STR_VIEW_TYPE = MUSIC_INFO_STR_VIEW_TYPE;
    public:
        enum class EType
        {
            None = 0,
            Music = 1,
            Album
        };

    private:

        EType _type;
        MUSIC_INFO_STR_VIEW_TYPE _name;
        MUSIC_INFO_STR_VIEW_TYPE _suffix;
        MUSIC_INFO_STR_TYPE _path;
    public:

        MusicInfo();
        MusicInfo(const MusicInfo &oth) = default;
        MusicInfo(MusicInfo &&oth) = default;
        MusicInfo& operator=(const MusicInfo &oth) = default;
        MusicInfo& operator=(MusicInfo &&oth) = default;


        [[nodiscard]] EType Type() const;

        void Set_Type(EType Type);

        [[nodiscard]] const MUSIC_INFO_STR_VIEW_TYPE& Name() const;

        void Set_Name(const MUSIC_INFO_STR_VIEW_TYPE& Name);

        [[nodiscard]] const MUSIC_INFO_STR_VIEW_TYPE& Suffix() const;

        void Set_Suffix(const MUSIC_INFO_STR_VIEW_TYPE& Suffix);

        [[nodiscard]] const MUSIC_INFO_STR_TYPE& Path() const;

        void Set_Path(const MUSIC_INFO_STR_TYPE& Path);
    };

    bool operator==(const MusicInfo& Lhs, const MusicInfo& RHS);

    bool operator!=(const MusicInfo& Lhs, const MusicInfo& RHS);
}