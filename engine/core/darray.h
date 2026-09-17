#pragma once

#include "debug/debug.h"
#include "types.h"

#include <cstdlib>
#include <cstring>
#include <limits>
#include <type_traits>

namespace cw
{
    template<typename T>
    concept CDArrayElement =
        std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;

    template<CDArrayElement T>
    class darray
    {
        T*    data     = nullptr;
        usize capacity = 0;
        usize count    = 0;

      public:
        darray(const darray<T>& arr)               = delete;
        darray(darray<T>&& arr)                    = delete;
        darray<T>& operator=(const darray<T>& arr) = delete;
        darray<T>& operator=(darray<T>&& arr)      = delete;

        darray() = default;

        ~darray()
        {
            free(data);
        }

        usize Count() const
        {
            return count;
        }

        [[nodiscard]] bool Add(const T& element)
        {
            if (count == capacity)
            {
                if (capacity > std::numeric_limits<usize>::max() / 2)
                {
                    return false;
                }

                const usize newCapacity = capacity == 0 ? 4 : capacity * 2;
                if (!Reserve(newCapacity))
                {
                    return false;
                }
            }

            data[count++] = element;
            return true;
        }

        [[nodiscard]] bool Reserve(usize newCapacity)
        {
            if (newCapacity <= capacity)
            {
                return true;
            }

            if (newCapacity > std::numeric_limits<usize>::max() / sizeof(T))
            {
                return false;
            }

            void* newData = realloc(data, sizeof(T) * newCapacity);
            if (newData == nullptr)
            {
                return false;
            }

            data     = static_cast<T*>(newData);
            capacity = newCapacity;
            return true;
        }

        // Order-preserving: callers that reason about insertion order (resource
        // caches destroying newest first) depend on it.
        void RemoveAt(usize index)
        {
            CW_ASSERT(index < count);

            if (index + 1 < count)
            {
                memmove(data + index, data + index + 1, sizeof(T) * (count - index - 1));
            }

            --count;
        }

        void Clear()
        {
            count = 0;
        }

        const T* Data() const
        {
            return data;
        }

        T* Data()
        {
            return data;
        }

        const T& operator[](usize index) const
        {
            CW_ASSERT(index < count);

            return data[index];
        }

        T& operator[](usize index)
        {
            CW_ASSERT(index < count);

            return data[index];
        }

    };
}
