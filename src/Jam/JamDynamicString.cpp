#include "Jam/JamDynamicString.h"
#include "Util/ByteBuffer.h"

void JamDynamicString::Read(ByteBuffer& data, size_t maxLen)
{
    m_str.clear();

    size_t const start = data.rpos();
    size_t const end   = data.size();
    size_t       i     = start;

    while (i < end)
    {
        if (data.contents()[i] == 0)
        {
            size_t const len = i - start;
            if (len > maxLen)
                throw ByteBufferException(false, data.rpos(), 1, data.size());
            m_str.assign(reinterpret_cast<char const*>(data.contents() + start), len);
            data.rpos(i + 1);
            return;
        }
        ++i;
    }

    throw ByteBufferException(false, data.rpos(), 1, data.size());
}

void JamDynamicString::Write(ByteBuffer& data) const
{
    data.append(m_str.c_str(), m_str.size() + 1);
}

ByteBuffer& operator<<(ByteBuffer& b, JamDynamicString const& s)
{
    s.Write(b);
    return b;
}
