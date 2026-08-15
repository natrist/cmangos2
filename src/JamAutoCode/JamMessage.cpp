#include "JamAutoCode/JamMessage.h"
#include "Util/Errors.h"

void JamMessage::Get(ByteBuffer&)
{
}

void JamMessage::Put(ByteBuffer&) const
{
    MANGOS_ASSERT(!"JamMessage::Put not implemented for this message");
}
