#ifndef MANGOS_WOW_CONNECTION_H
#define MANGOS_WOW_CONNECTION_H

class WorldSession;
class JamMessage;

// Peer handle for Jam handlers (client session today; other peers later).
class WowConnection
{
public:
    explicit WowConnection(WorldSession* session = nullptr) : m_session(session) {}

    WorldSession* GetSession() const { return m_session; }

    // Serialize and send an outbound Jam message to this peer.
    void Send(JamMessage const& msg) const;

private:
    WorldSession* m_session;
};

#endif
