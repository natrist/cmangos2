/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "UserClient.h"
#include "JamAutoCode/JamUserClient.h"
#include "JamAutoCode/JamClient.h"
#include "Entities/Player.h"
#include "Globals/ObjectMgr.h"
#include "Server/WorldSession.h"

void UserClient::InstallHandlers()
{
    UserClientWorldTeleport::s_handler = WorldTeleportHandler;
    UserClientGmResurrect::s_handler   = GmResurrectHandler;
}

JAM_RESULT UserClient::WorldTeleportHandler(WowConnection* conn, UserClientWorldTeleport* msg)
{
    WorldSession* session = conn->GetSession();
    if (!session)
        return JAM_FAILED;

    if (session->GetSecurity() < SEC_MODERATOR)
    {
        session->SendPermissionFailure();
        return JAM_FAILED;
    }

    Player* player = session->ActivePlayerPtr();
    if (!player)
        return JAM_FAILED;

    player->TeleportTo(msg->mapId,
                       msg->position.x,
                       msg->position.y,
                       msg->position.z,
                       msg->facing);
    return JAM_OK;
}

JAM_RESULT UserClient::GmResurrectHandler(WowConnection* conn, UserClientGmResurrect* msg)
{
    WorldSession* session = conn->GetSession();
    if (!session)
        return JAM_FAILED;

    ClientGmResurrectFailed response;

    Player* player = sObjectMgr.GetPlayer(msg->name.c_str());
    if (!player || player->IsAlive())
    {
        response.failed = true;
        conn->Send(response);
        return JAM_FAILED;
    }

    player->Resurrect(100.0f);

    if (Corpse* corpse = player->GetCorpse())
        corpse->DestroyOnClientsIAmAt();

    response.failed = false;
    conn->Send(response);
    return JAM_OK;
}
