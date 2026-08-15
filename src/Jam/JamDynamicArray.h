#ifndef MANGOS_JAM_DYNAMIC_ARRAY_H
#define MANGOS_JAM_DYNAMIC_ARRAY_H

#include "Util/ByteBuffer.h"

#include <type_traits>
#include <vector>

// Length-prefixed array on the wire (u32 count + elements).
namespace JamDynamicArray
{
    template <typename T>
    void ReadElement(ByteBuffer& data, T& e)
    {
        if constexpr (std::is_enum_v<T>)
        {
            using U = std::underlying_type_t<T>;
            U raw;
            data >> raw;
            e = static_cast<T>(raw);
        }
        else
            data >> e;
    }

    template <typename T>
    void WriteElement(ByteBuffer& data, T const& e)
    {
        if constexpr (std::is_enum_v<T>)
            data << static_cast<std::underlying_type_t<T>>(e);
        else
            data << e;
    }

    template <typename T>
    void Read(ByteBuffer& data, std::vector<T>& out, uint32 maxCount)
    {
        uint32 n;
        data >> n;
        if (n > maxCount)
            throw ByteBufferException(false, data.rpos(), 0, data.size());
        out.resize(n);
        for (uint32 i = 0; i < n; ++i)
            ReadElement(data, out[i]);
    }

    template <typename T>
    void Write(ByteBuffer& data, std::vector<T> const& v)
    {
        data << static_cast<uint32>(v.size());
        for (T const& e : v)
            WriteElement(data, e);
    }
}

#endif
