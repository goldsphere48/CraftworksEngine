#pragma once

#include "types.h"
#include "concepts.h"

namespace cw
{
    template<CScalar T>
    struct AccumulatorType
    {
        using Type = T;
    };

    template<CInteger T>
    struct AccumulatorType<T>
    {
        using Type = std::conditional_t<
            std::is_signed_v<T>,
            int64,
            uint64
        >;
    };

    template<CScalar T>    
    using AccumulatorT = typename AccumulatorType<T>::Type;
}
