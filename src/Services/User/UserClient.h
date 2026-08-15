/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CMANGOS_USER_CLIENT_H
#define CMANGOS_USER_CLIENT_H

#include "JamAutoCode/JamMessage.h"

class UserClientWorldTeleport;
class UserClientGmResurrect;

class UserClient
{
public:
    static void InstallHandlers();

private:
    static JAM_RESULT WorldTeleportHandler(WowConnection* conn, UserClientWorldTeleport* msg);
    static JAM_RESULT GmResurrectHandler(WowConnection* conn, UserClientGmResurrect* msg);
};

#endif
