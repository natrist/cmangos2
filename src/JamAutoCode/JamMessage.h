#ifndef JAM_MESSAGE_H
#define JAM_MESSAGE_H

#include "Jam/JamTypes.h"
#include "Services/WowConnection.h"
#include "Util/ByteBuffer.h"

enum JAM_RESULT
{
    JAM_OK     = 0,
    JAM_FAILED = 1,
};

class JamMessage
{
public:
    virtual ~JamMessage() = default;

    WowConnection* Connection() const { return m_connection; }

    virtual void        Get(ByteBuffer& data);
    virtual void        Put(ByteBuffer& data) const;
    virtual u16         GetCode() const = 0;
    virtual char const* GetName() const = 0;
    virtual u32         GetSize() const = 0;

protected:
    JamMessage() = default;
    explicit JamMessage(WowConnection* conn) : m_connection(conn) {}

    WowConnection* m_connection = nullptr;
};

#endif
