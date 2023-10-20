#include <GarrysMod/Lua/Helpers.hpp>
#include <GarrysMod/InterfacePointers.hpp>
#include <GarrysMod/FunctionPointers.hpp>
#include <detouring/hook.hpp>
#include <cstdint>
#include <eiface.h>
#include <player.h>
#include <ctime>

namespace server
{
	typedef void (*HandleClientLuaError_t)(CBasePlayer *player, const char *error);
	static Detouring::Hook HandleClientLuaError_detour;

	static void HandleClientLuaError_d(CBasePlayer *player, const char *error)
	{
		std::stringstream ss;
		std::time_t t = std::time(nullptr);
		auto tm = *std::localtime(&t);

		ss << "[" << (tm.tm_mday) << "/" << (tm.tm_mon + 1) << "/" << (tm.tm_year + 1900) << " " << tm.tm_hour << ":" << tm.tm_min << ":" << tm.tm_sec << "] " << error;

		HandleClientLuaError_detour.GetTrampoline<HandleClientLuaError_t>()(player, ss.str().c_str());
	}

	void Initialize()
	{
		const auto HandleClientLuaError = FunctionPointers::CBasePlayer_HandleClientLuaError();
		if (HandleClientLuaError == nullptr)
		{
			printf("Failed to get CBasePlayer::HandleClientLuaError function pointer!\n");
			return;
		}

		HandleClientLuaError_detour.Create(Detouring::Hook::Target(reinterpret_cast<void *>(HandleClientLuaError)), reinterpret_cast<void *>(&HandleClientLuaError_d));
		HandleClientLuaError_detour.Enable();
	}
}

GMOD_MODULE_OPEN()
{
	try
	{
		server::Initialize();
	}
	catch (const std::exception &e)
	{
		printf("Better Client Errors - Exception thrown: %s\n", e.what());
	}
	return 0;
}

GMOD_MODULE_CLOSE()
{
	return 0;
}
