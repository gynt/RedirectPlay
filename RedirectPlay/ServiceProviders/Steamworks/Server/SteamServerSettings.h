#pragma once

#include "Utils/fstring.h"

#include "DirectX/dplay.h"
#include "Steam/isteammatchmaking.h"

struct SSteamServerSettings
{
	bool HasPassword() const { return !password.empty(); }

	fstring<DPSESSIONNAMELEN> name;
	fstring<DPPASSWORDLEN>    password;
	ELobbyType                lobbyType = k_ELobbyTypePublic;
	size_t                    maxPlayers = 8;
};