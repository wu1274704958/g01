#pragma once

#include "IFileLoader.h"
#include <string>

namespace eqd_mp
{
    struct DefFileLoader : public IFileLoader<std::string>
    {
        std::span<uint8_t> loadFormMemory(const std::string& filePath);
    protected:
        std::vector<uint8_t> _buffer;
    };
    
}
