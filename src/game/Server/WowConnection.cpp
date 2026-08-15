#include "Services/WowConnection.h"
#include "JamAutoCode/JamMessage.h"
#include "Server/WorldSession.h"
#include "Server/WorldPacket.h"

void WowConnection::Send(JamMessage const& msg) const
{
    if (!m_session)
        return;

    WorldPacket pkt(msg.GetCode(), msg.GetSize());
    msg.Put(pkt);
    m_session->SendPacket(pkt);
}
