#pragma once

namespace eqd_mp
{
    template <typename T,typename ...Args>
    concept VaildConstructArgs = requires(Args&& ... args)
    {
        new T(std::forward<Args>(args)...);
    };
}
