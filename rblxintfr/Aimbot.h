#pragma once
#include "Functions.h"

class AimbotClass : public Functions::SingletonHelper {
private:
	struct RaycastResult {
		RBX::Vector3 Position; // The world space point at which the intersection occurred, usually a point directly on the surface of the instance.
		RBX::Vector3 Normal; // The normal vector of the intersected face.
		uintptr_t Material; // The Material at the intersection point. For normal parts this is the BasePart.Material; for Terrain this can vary depending on terrain data.
		float noclue;
		RBX::Instance Instance; // The BasePart or Terrain cell that the ray intersected.
		uintptr_t afk; // thingy of instance
	};

	struct RaycastParams
	{
		char pad_0x0000[0x1]; //0x0000
		bool IgnoreWater = false; //0x0001 
		BYTE dk1 = 0x53; //0x0002 2 n 3 r just random bytes
		BYTE dk2 = 0x12; //0x0003 
		bool WhitelistType = false; //0x0004  Blacklist (0) Whitelist (1)
		char pad_0x0005[0x3]; //0x0005
		DWORD FilterDescendantsInstances; //0x0008 
		char pad_0x000C[0x4]; //0x000C
		std::string CollisionGroup = std::string("Default"); //0x1295888 

	}; //Size=0x0014
public:
	AimbotClass() { initialise(); }

	bool GetClosestPlayerToMouse(RBX::Instance Players, RBX::Instance Mouse, RBX::Instance& target) {
		RBX::Vector2 mousePos{ (float)Mouse.GetPropertyValue<int>("X"), (float)Mouse.GetPropertyValue<int>("Y") };
		static RBX::Instance workspace;  if (!workspace.ptr()) workspace = Players.Parent().GetPropertyValue<int>("Workspace");
		static RBX::Instance CurrentCamera; if(!CurrentCamera.ptr()) CurrentCamera = workspace.GetPropertyValue<int>("CurrentCamera");
		static RBX::Reflection::BoundFuncDescriptor Raycast; if (!Raycast.ptr()) Raycast = workspace.FindBoundFuncDescriptor("Raycast");
		static RBX::Instance localPlayer = Players.GetPropertyValue<int>("LocalPlayer"); if (!localPlayer.ptr()) return false;
		static RBX::Instance localCharacter = localPlayer.GetPropertyValue<int>("Character"); if (!localCharacter.ptr()) return false;
		RBX::Instance localHead = localCharacter.FindFirstChild("Head"); if (!localHead.ptr()) localHead = localCharacter.GetPropertyValue<int>("PrimaryPart");
		RBX::Vector3 origin = localHead.GetCustomPropertyValue<RBX::Vector3>("Position");
		
		std::pair<float, RBX::Instance> best = { 9e9f, RBX::Instance() };
		float closest_mag = 9e9f;
		
		for (RBX::Instance player : Players.GetChildren()) {
			if (!(player.ptr() && player.GetClassDescriptor().ClassName() == "Player")) continue;
			if (RBX::Instance character = player.GetPropertyValue<int>("Character")) {
				RBX::Instance target = character.FindFirstChild(AimPart);
				//if (!target) target = character.GetPropertyValue<int>("PrimaryPart");
				if (!target.ptr()) continue; // can have both but idk

				if (!DoChecks(player)) continue;
				RBX::Vector3 position = target.GetCustomPropertyValue<RBX::Vector3>("Position");

				if (WallCheck) {
					RBX::Vector3 vec = position - origin;
					RBX::Vector3 direction = vec.unit() * vec.magnitude();
					std::vector<uintptr_t> FilterDescendantsInstances = {localCharacter.ptr(), 0, character.ptr(), 0};

					/*uintptr_t raycastParams = (uintptr_t)VirtualAlloc(0, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); //(uintptr_t)malloc(40);
					*(bool*)(raycastParams + 0x1) = false; // IgnoreWater
					*(BYTE*)(raycastParams + 0x2) = 0x53; // dk1
					*(BYTE*)(raycastParams + 0x3) = 0x12; // dk2

					*(bool*)(raycastParams + 0x4) = false; // Blacklist (0) Whitelist (1)
					*(uintptr_t*)(raycastParams + 0x8) = (uintptr_t)&FilterDescendantsInstances;
					*(std::string*)(raycastParams + 0x10) = std::string("Default");*/

					RaycastParams* raycastParams = new RaycastParams();
					raycastParams->FilterDescendantsInstances = (uintptr_t)&FilterDescendantsInstances;

					static RaycastResult ret;
					int lol = reinterpret_cast<int(__thiscall*)(int, RaycastResult*, RBX::Vector3*, RBX::Vector3*, RaycastParams*)>(Raycast.Func())(workspace.ptr(), &ret, &origin, &direction, raycastParams);

					if (ret.Instance) continue;



					//int b = workspace.CallBoundFunc("FindPartOnRay", v1,v2,v3,v4,v5,v6, 0, 0);//workspace.CallBoundFunc("FindPartOnRay", origin.x, origin.y, origin.z, direction.x, direction.y, direction.z,0, 0);
					//printf("%p\n", b);
					//RBX::Instance i(readloc(readloc(b) + 8));
					//printf("%s\n", i.Name().c_str());

					
				}
				
				DWORD ret = CurrentCamera.CallBoundFunc("WorldToViewportPoint", position.x, position.y, position.z);
				RBX::Vector3 vec3 = *(RBX::Vector3*)(readloc(ret) + 8); // z is magnitude
				RBX::Vector2 vec2{ vec3.x, vec3.y };

				if (vec3.z < closest_mag) closest_mag = vec3.z;
				else continue;

				if (((vec2 - mousePos).length() < best.first)) {
					best = { (vec2 - mousePos).length(), player };
				}
			}
		}

		target = best.second;
		return !!best.second.ptr();
	}

	bool isActive() {
		return this->active ? GetAsyncKeyState(aimbotKey) : false;
	}

	const char* AimPart = "Head";
	bool WallCheck = true;
	bool AutoShoot = true;
	bool active = true;
private:
	BYTE aimbotKey = VK_MENU; // https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

	void initialise() {
		AddCheck("LocalPlayerCheck", [](RBX::Instance player) { return player.ptr() != player.Parent().GetPropertyValue<int>("LocalPlayer"); });
		AddCheck("TeamCheck", [](RBX::Instance player) {
			RBX::Instance LocalPlayer = player.Parent().GetPropertyValue<int>("LocalPlayer");
			return player.GetPropertyValue<int>("Team") != LocalPlayer.GetPropertyValue<int>("Team");
			});
	}
};

extern AimbotClass* aimbot = new AimbotClass();