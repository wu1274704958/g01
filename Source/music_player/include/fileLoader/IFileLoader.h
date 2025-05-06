#pragma once
#include <string>

#include "../api_def.h"
#include <vector>
#include <span>

namespace eqd_mp
{
    template<typename  F>
    struct IFileLoader
    {
        using PathType = F;
        std::span<uint8_t> loadFormMemory(const F&)
        {
            return {};
        }
    };

    template <typename T,typename PT>
    concept VaildFileLoader = requires
    {
        requires std::is_base_of_v<IFileLoader<PT>,T>;
    };
}
