//
// Created by wws on 17-7-19.
//

#include "MusicInfo.h"

eqd_mp::MusicInfo::MusicInfo()
{
    this->_type = EType::None;
}

eqd_mp::MusicInfo::EType eqd_mp::MusicInfo::Type() const
{
    return _type;
}

void eqd_mp::MusicInfo::Set_Type(EType Type)
{
    _type = Type;
}

const std::wstring_view& eqd_mp::MusicInfo::Name() const
{
    return _name;
}

void eqd_mp::MusicInfo::Set_Name(const std::wstring_view& Name)
{
    _name = Name;
}

const std::wstring_view& eqd_mp::MusicInfo::Suffix() const
{
    return _suffix;
}

void eqd_mp::MusicInfo::Set_Suffix(const std::wstring_view& Suffix)
{
    _suffix = Suffix;
}

const std::wstring& eqd_mp::MusicInfo::Path() const
{
    return _path;
}

void eqd_mp::MusicInfo::Set_Path(const std::wstring& Path)
{
    _path = Path;
}

bool eqd_mp::operator==(const MusicInfo& Lhs, const MusicInfo& RHS)
{
    return Lhs.Type() == RHS.Type()
        && Lhs.Path() == RHS.Path();
}

bool eqd_mp::operator!=(const MusicInfo& Lhs, const MusicInfo& RHS)
{
    return !(Lhs == RHS);
}
