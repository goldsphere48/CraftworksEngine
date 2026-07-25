#pragma once

#include <concepts>

namespace cw
{
    template<typename T>
    concept CScalar =
        (std::integral<T> || std::floating_point<T>) && !std::same_as<std::remove_cv_t<T>, bool>;

    template<typename T>
    concept CFloating = std::floating_point<T>;

    template<typename T>
    concept CInteger = CScalar<T> && std::integral<T>;
}
