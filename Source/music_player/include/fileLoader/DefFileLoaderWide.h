#pragma once
#include "IFileLoader.h"
#include <string>

namespace eqd_mp
{
    struct DefFileLoaderWide : public IFileLoader<std::wstring>
    {
        std::span<uint8_t> loadFormMemory(const std::wstring& path);
    protected:
        std::vector<uint8_t> _buffer;
    };
    
}

