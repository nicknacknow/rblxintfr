#pragma once
#include "Functions.h"

class AimbotClass : public Functions::SingletonHelper {
private:
	uintptr_t craftRaycastParams(bool ignoreWater, bool whitelistType, std::vector<uintptr_t> filterDescendantsInstances) {
		/*std::vector<uintptr_t> realFilterDescendantsInstances;
		for (RBX::Instance i : filterDescendantsInstances) {
			realFilterDescendantsInstances.push_back(i.ptr());
			realFilterDescendantsInstances.push_back(0);
		}*/

		static uintptr_t raycastParams = (uintptr_t)VirtualAlloc(0, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); //(uintptr_t)malloc(40);
		*(bool*)(raycastParams + 0x1) = ignoreWater; // IgnoreWater
		*(BYTE*)(raycastParams + 0x2) = 0x53; // dk1
		*(BYTE*)(raycastParams + 0x3) = 0x12; // dk2

		*(bool*)(raycastParams + 0x4) = whitelistType; // Blacklist (0) Whitelist (1)
		*(uintptr_t*)(raycastParams + 0x8) = (uintptr_t)&filterDescendantsInstances;
		*(std::string*)(raycastParams + 0x10) = std::string("Default");

		return raycastParams;
	}

	struct RaycastResult {
		RBX::Vector3 Position; // The world space point at which the intersection occurred, usually a point directly on the surface of the instance.
		RBX::Vector3 Normal; // The normal vector of the intersected face.
		uintptr_t Material; // The Material at the intersection point. For normal parts this is the BasePart.Material; for Terrain this can vary depending on terrain data.
		float noclue;
		RBX::Instance Instance; // The BasePart or Terrain cell that the ray intersected.
		uintptr_t afk; // thingy of instance
	};
public:
	AimbotClass() { initialise(); }
	float v1 = 0.f, v2 = 0.f, v3 = 0.f;
	float v4 = 0.f, v5 = 0.f, v6 = 0.f;
	bool GetClosestPlayerToMouse(RBX::Instance Players, RBX::Instance Mouse, RBX::Instance& target) {
		RBX::Vector2 mousePos{ (float)Mouse.GetPropertyValue<int>("X"), (float)Mouse.GetPropertyValue<int>("Y") };
		RBX::Instance workspace = Players.Parent().GetPropertyValue<int>("Workspace");
		RBX::Instance CurrentCamera = workspace.GetPropertyValue<int>("CurrentCamera");
		RBX::Reflection::BoundFuncDescriptor Raycast = workspace.FindBoundFuncDescriptor("Raycast");
		RBX::Instance localPlayer = Players.GetPropertyValue<int>("LocalPlayer"); if (!localPlayer) return false;
		RBX::Instance localCharacter = localPlayer.GetPropertyValue<int>("Character"); if (!localCharacter) return false;
		RBX::Instance localHead = localCharacter.FindFirstChild("Head"); if (!localHead) localHead = localCharacter.GetPropertyValue<int>("PrimaryPart");
		RBX::Vector3 origin = localHead.GetCustomPropertyValue<RBX::Vector3>("Position");
		
		std::pair<float, RBX::Instance> best = { 9e9f, RBX::Instance() };
		
		for (RBX::Instance player : Players.GetChildren()) {
			if (RBX::Instance character = player.GetPropertyValue<int>("Character")) {
				RBX::Instance target = character.FindFirstChild(AimPart);
				//if (!target) target = character.GetPropertyValue<int>("PrimaryPart");
				if (!target.ptr()) continue; // can have both but idk

				if (!DoChecks(player)) continue;
				RBX::Vector3 position = target.GetCustomPropertyValue<RBX::Vector3>("Position");

				if (WallCheck) {
					RBX::Vector3 vec = position - origin;
					RBX::Vector3 direction = vec.unit() * vec.magnitude();
					//std::vector<uintptr_t> FilterDescendantsInstances = {localCharacter.ptr(), 0, character.ptr(), 0};

					//uintptr_t raycastParams = (uintptr_t)VirtualAlloc(0, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); //(uintptr_t)malloc(40);
					/**(bool*)(raycastParams + 0x1) = false; // IgnoreWater
					*(BYTE*)(raycastParams + 0x2) = 0x53; // dk1
					*(BYTE*)(raycastParams + 0x3) = 0x12; // dk2

					*(bool*)(raycastParams + 0x4) = false; // Blacklist (0) Whitelist (1)
					*(uintptr_t*)(raycastParams + 0x8) = (uintptr_t)&FilterDescendantsInstances;
					*(std::string*)(raycastParams + 0x10) = std::string("Default");*/

					//static RaycastResult ret;
					//int lol = reinterpret_cast<int(__thiscall*)(int, RaycastResult*, RBX::Vector3*, RBX::Vector3*, int)>(Raycast.Func())(workspace.ptr(), &ret, &origin, &direction, raycastParams);

					//VirtualFree((LPVOID)raycastParams, 0, MEM_RELEASE);
					//free((void*)raycastParams);

					//printf("%p %p\n", &ret, &lol);

					//if (ret.Instance) continue;



					//int b = workspace.CallBoundFunc("FindPartOnRay", v1,v2,v3,v4,v5,v6, 0, 0);//workspace.CallBoundFunc("FindPartOnRay", origin.x, origin.y, origin.z, direction.x, direction.y, direction.z,0, 0);
					//printf("%p\n", b);
					//RBX::Instance i(readloc(readloc(b) + 8));
					//printf("%s\n", i.Name().c_str());

					
				}
				
				/*DWORD ret = CurrentCamera.CallBoundFunc("WorldToViewportPoint", position.x, position.y, position.z);
				RBX::Vector3 vec3 = *(RBX::Vector3*)(readloc(ret) + 8); // z is magnitude
				RBX::Vector2 vec2{ vec3.x, vec3.y };

				if (((vec2 - mousePos).length() < best.first)) {
					best = { (vec2 - mousePos).length(), player };
				}*/
			}
		}

		//target = best.second;
		//return !!best.second.ptr();
		return false;
	}
private:
	const char* AimPart = "Head";
	bool WallCheck = true;
	BYTE vkKey = 0x43; // c i think ?

	void initialise() {
		//AddCheck("LocalPlayerCheck", [](RBX::Instance player) { return player.ptr() != player.Parent().GetPropertyValue<int>("LocalPlayer"); });
	}
};

extern AimbotClass* aimbot = new AimbotClass();