#ifndef MANGOS_JAM_DYNAMIC_STRING_H
#define MANGOS_JAM_DYNAMIC_STRING_H

#include "Common.h"

#include <string>

class ByteBuffer;

// NUL-terminated string on the wire.
class JamDynamicString
{
public:
    JamDynamicString() = default;
    JamDynamicString(char const* s) { if (s) m_str = s; }

    char const* c_str() const { return m_str.c_str(); }
    size_t      size()  const { return m_str.size(); }
    bool        empty() const { return m_str.empty(); }

    void Read(ByteBuffer& data, size_t maxLen);
    void Write(ByteBuffer& data) const;

private:
    std::string m_str;
};

ByteBuffer& operator<<(ByteBuffer& b, JamDynamicString const& s);

#endif
