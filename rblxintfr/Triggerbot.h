#pragma once
#include "Functions.h"

class TriggerbotClass : public Functions::SingletonHelper {
private:
	RBX::Instance GetPlayerFromCharacter(RBX::Instance players, RBX::Instance obj) {
		for (RBX::Instance player : players.GetChildren()) {
			if (!player.ptr()) continue;
			if (player.GetPropertyValue<int>("Character") == obj.ptr()) return player;
		}
		return RBX::Instance();
	}
public:
	TriggerbotClass() { initialise(); }

	bool Step(RBX::Instance Players, RBX::Instance Mouse) {
		RBX::Instance target = Mouse.GetPropertyValue<int>("Target"); if (!target.ptr()) return false;
		RBX::Instance LocalPlayer = Players.GetPropertyValue<int>("LocalPlayer"); if (!LocalPlayer.ptr()) return false;

		RBX::Instance character = target.Parent(); if (!character.ptr()) return false;

		RBX::Instance plr = GetPlayerFromCharacter(Players, character); if (!plr.ptr()) return false;
		//printf("%s\n", target.Name().c_str());

		if (DoChecks(plr))
			return true;

		// cant call
		//uintptr_t plr = Players.CallBoundFunc("GetPlayerFromCharacter", character.ptr(), character.Ref());

		//printf("%p\n", plr);

		/*if (RBX::Instance plr = Players.CallBoundFunc("GetPlayerFromCharacter", character.ptr(), character.Ref())) {
			if (DoChecks(plr))
				return true;
		}*/

		return false;
	}

	void Action() {
		if (GetForegroundWindow() == FindWindow(0, L"Roblox")) {
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

			/*
			calling all connections to Button1Down mouse signal will work - need to work out how to get connections
			
			*/
		}
	}

	bool isActive() {
		return this->active;
	}
private:
	bool active = false;

	void initialise() {
		AddCheck("LocalPlayerCheck", [](RBX::Instance player) { return player.ptr() != player.Parent().GetPropertyValue<int>("LocalPlayer"); });
		AddCheck("TeamCheck", [](RBX::Instance player) {
			RBX::Instance LocalPlayer = player.Parent().GetPropertyValue<int>("LocalPlayer");
			return player.GetPropertyValue<int>("Team") != LocalPlayer.GetPropertyValue<int>("Team");
			});
	}
};

extern TriggerbotClass* triggerbot = new TriggerbotClass();