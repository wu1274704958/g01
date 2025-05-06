#pragma once

namespace eqd_mp
{
    template <typename T,typename ...Args>
    concept VaildConstructArgs = requires(Args ... args)
    {
        requires new T(args...);
    };
}
