#include <Windows.h>
#include <iostream>
#include <string>

#include <d3dx9.h>
#include <d3d9.h>

#include "Memory.h"

#include "MemCheckBypass.h"
#include "TaskScheduler.h"
#include "Classes.h"
#include "Functions.h"


void open_console(LPCSTR title = "epxloit") {
	unsigned long ignore = 0;
	VirtualProtect(&FreeConsole, 1, PAGE_EXECUTE_READWRITE, &ignore);
	*(BYTE*)&FreeConsole = 0xC3;
	VirtualProtect(&FreeConsole, 1, ignore, &ignore);

	AllocConsole();
	freopen("conin$", "r", stdin); // Enable input
	freopen("conout$", "w", stdout); // Display output
	freopen("conout$", "w", stderr); // std error handling
	SetConsoleTitleA(title);
}

uintptr_t Workspace_ptr;
uintptr_t func_findFirstChild;

int Hook_findFirstChild(int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10) {
	printf("Hook_findFirstChild: %p, %p, %p %p %p %p %p %p %p\n", a2, a3, a4, a5, a6, a7 ,a8, a9, a10);

	int target = 0;
	__asm mov target, edx; // mov self into target var

	printf("target: %p\n", target);

	int ret = reinterpret_cast<int(__thiscall*)(int, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10)>(func_findFirstChild)(Workspace_ptr, a2, a3, a4,a5,a6,a7,a8,a9,a10);
	
	printf("ret: %p\n", ret);
	
	return ret;
}

uintptr_t targetFunc;


// freezes when hooked function is called - check the lua func thing (this is because fastcall can't be used in a vtable
int __fastcall fastcallHook(int self, void* edx, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int a13, int a14) {
	printf("fastcallHook [ %p ]: %p %p %p %p %p %p %p %p %p %p\n", self, a1, a2, a3, a4, a5, a6,a7,a8,a9,a10);
	system("pause");
	int ret = reinterpret_cast<int(__thiscall*)(int, int, int, int, int, int, int, int, int, int, int, int,int,int,int)>(targetFunc)(self, a1, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14);
	//system("pause");
	printf("ret: %p\n", ret);

	return ret;
}

void __fastcall hhhhhh(int self, void* edx, int a1) {
	printf("hhhhhh: %p args: %p\n", self,a1);

	reinterpret_cast<void(__thiscall*)(int,int)>(targetFunc)(self,a1);
}

void setup_fastcallhook(int addr) {
	targetFunc = readloc(addr);
	*(uintptr_t*)addr = (uintptr_t)&fastcallHook;
}

/*

to-do:
GetMouse


* self = Player
* 1 = return
* 2 = player class descriptor
* 3 = getmouse boundfuncdesc


Raycast https://developer.roblox.com/en-us/api-reference/function/WorldRoot/Raycast
* self = Workspace
* 1 = return
* 2 = Origin
* 3 = Direction
* 4 = raycast params https://developer.roblox.com/en-us/api-reference/datatype/RaycastParams
	- + 1 = IgnoreWater
	- + 4 = FilterType (1 for whitelist, 2 now 0 for blacklist)
	- + 10 = CollisionGroup
	- +30 annd +8 = FilterDescendantsInstances (treat like Children) lol
		//*(BYTE*)(a4 + 2) = 0;
		//*(BYTE*)(a4 + 3) = 0; //for some reason return is nil when these r set.. work later


get properties such as Position, Name


	Position args:
	self = getsetptr
	1 = name of part
	2 = part
	3 = &part
	4 = prop descriptor
	5 =
	*/

void DisableLogs()
{
	auto Module = reinterpret_cast<HMODULE>(GetModuleHandle(L"KERNELBASE.dll"));
	auto EnumProcessModules = reinterpret_cast<uintptr_t>(GetProcAddress(Module, "EnumProcessModules"));
	auto EnumProcessModulesEx = reinterpret_cast<uintptr_t>(GetProcAddress(Module, "EnumProcessModulesEx"));
	auto GetMappedFileNameA = reinterpret_cast<uintptr_t>(GetProcAddress(Module, "GetMappedFileNameA"));
	auto GetMappedFileNameW = reinterpret_cast<uintptr_t>(GetProcAddress(Module, "GetMappedFileNameW"));
	//*reinterpret_cast<BYTE*>(EnumProcessModules) = 0x90;
	//*reinterpret_cast<BYTE*>(EnumProcessModulesEx) = 0x90;
	//*reinterpret_cast<BYTE*>(GetMappedFileNameA) = 0x90;
	//*reinterpret_cast<BYTE*>(GetMappedFileNameW) = 0x90;
	WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(EnumProcessModules), "\x90", 1, NULL);
	WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(EnumProcessModulesEx), "\x90", 1, NULL);
	WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(GetMappedFileNameA), "\x90", 1, NULL);
	WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(GetMappedFileNameW), "\x90", 1, NULL);
}

float hook_walkspeed() {
//	printf("hook_walkspeed\n");
	return 16.f;
}

int GetPartBetweenPoints(RBX::Instance workspace, static RBX::Vector3 origin, static RBX::Vector3 endpoint, static int ignoreDescendants = 0) {
	static RBX::Vector3 unit = endpoint - origin;
	static RBX::Vector3 direction = unit.unit() * unit.magnitude();

	//printf("{%f %f %f}\n", unit.x, unit.y, unit.z); 

	static int ignoreDescendantInfo = 0;
	if (ignoreDescendants)
		ignoreDescendantInfo = readloc(ignoreDescendants + 0x8);

	static int a = workspace.CallBoundFunc("FindPartOnRay", origin.x, origin.y, origin.z, direction.x, direction.y, direction.z, 0, 0);

	if (a)
		return readloc(readloc(a) + 8);

	//int temp;
	//int ret = reinterpret_cast<int(__thiscall*)(int, int*, float, float, float, float, float, float, int, int)>(FindPartOnRay.Func())(workspace.ptr(), &temp, origin.x, origin.y, origin.z, 0.f,0.f,0.f, 0, 0);

	//printf("%p\n", ret);

	return 0;
}

//Functions::ClosestPlayer ClosestPlayer;

static int cripwalk(RBX::Instance Workspace, static RBX::Vector3 origin, RBX::Vector3 end, RBX::Instance Character = 0) {
	static RBX::Vector3 vec = end - origin;
	static RBX::Vector3 direction = vec.unit() * vec.magnitude();


	static int b = Workspace.CallBoundFunc("FindPartOnRay", origin.x, origin.y, origin.z, direction.x, direction.y, direction.z, 0, 0);
	return b;
	//return readloc(readloc(b) + 8);
}
#include "Aimbot.h"
bool testfunc(int a1, RBX::TaskScheduler::Job self) { // virtual TaskScheduler::StepResult step(const Stats& stats);
	if (RBX::Instance DataModel = self.GetDataModel()) {
		RBX::Instance Players = DataModel.FindFirstChildOfClass("Players");
		RBX::Instance Workspace = DataModel.GetPropertyValue<int>("Workspace");

		RBX::Instance Baseplate = Workspace.FindFirstChild("Baseplate");
		if (RBX::Instance LocalPlayer = Players.GetPropertyValue<int>("LocalPlayer")) { // i hate this sm
			if (RBX::Instance Character = LocalPlayer.GetPropertyValue<int>("Character")) {
				if (RBX::Instance PrimaryPart = Character.GetPropertyValue<int>("PrimaryPart")) {
					if (RBX::Instance CurrentCamera = Workspace.GetPropertyValue<int>("CurrentCamera")) {
						if (RBX::Instance Mouse = LocalPlayer.CallBoundFunc("GetMouse")) {
							RBX::Instance target;
							if (aimbot->GetClosestPlayerToMouse(Players, Mouse, target)) {
								// getclosestplayertomouse succeed

								/*if (RBX::Instance Character = target.GetPropertyValue<int>("Character")) {
									if (RBX::Instance PrimaryPart = Character.GetPropertyValue<int>("PrimaryPart")) {
										RBX::CoordinateFrame cframe = CurrentCamera.GetCustomPropertyValue<RBX::CoordinateFrame>("CFrame");
										cframe.lookAt(PrimaryPart.GetCustomPropertyValue<RBX::Vector3>("Position"));
										CurrentCamera.SetCustomPropertyValue<RBX::CoordinateFrame>("CFrame", cframe);
									}
								}*/
							}
							/*RBX::Instance instance_ret;
							if (ClosestPlayer->Get(Players, Mouse, instance_ret))
								printf("%s\n", instance_ret.Name().c_str());*/

							/*RBX::Vector3 origin = PrimaryPart.GetCustomPropertyValue<RBX::Vector3>("Position");
							RBX::Vector3 vec = Baseplate.GetCustomPropertyValue<RBX::Vector3>("Position") - origin;
							RBX::Vector3 direction = vec.unit() * vec.magnitude();

							int b = Workspace.CallBoundFunc("FindPartOnRay", origin.x, origin.y, origin.z, direction.x, direction.y, direction.z, Character.ptr(), 0);
							RBX::Instance i(readloc(readloc(b) + 8));
							printf("%s\n", i.Name().c_str());

							if (GetAsyncKeyState(0x43)) {
								RBX::Vector2 mousePos{ (float)Mouse.GetPropertyValue<int>("X"), (float)Mouse.GetPropertyValue<int>("Y") };
								std::pair<float, RBX::Instance> best = { 9e9f, RBX::Instance() };

								for (RBX::Instance player : Players.GetChildren()) {
									if (player.ptr() == Players.GetPropertyValue<int>("LocalPlayer"))continue;
									RBX::Instance Character = player.GetPropertyValue<int>("Character");
									if (!Character.ptr()) continue;
									RBX::Instance PrimaryPart = Character.GetPropertyValue<int>("PrimaryPart");
									if (!PrimaryPart.ptr()) continue;
									RBX::Vector3 position = PrimaryPart.GetCustomPropertyValue<RBX::Vector3>("Position");

									DWORD ret = CurrentCamera.CallBoundFunc("WorldToViewportPoint", position.x, position.y, position.z);
									RBX::Vector3 vec3 = *(RBX::Vector3*)(readloc(ret) + 8); // z is magnitude
									RBX::Vector2 vec2{ vec3.x, vec3.y };

									if (((vec2 - mousePos).length() < best.first)) {
										best = { (vec2 - mousePos).length(), player };
									}
								}

								if (best.second.ptr()) {
									if (RBX::Instance Character = best.second.GetPropertyValue<int>("Character")) {
										if (RBX::Instance PrimaryPart = Character.GetPropertyValue<int>("PrimaryPart")) {
											RBX::CoordinateFrame cframe = CurrentCamera.GetCustomPropertyValue<RBX::CoordinateFrame>("CFrame");
											cframe.lookAt(PrimaryPart.GetCustomPropertyValue<RBX::Vector3>("Position"));
											CurrentCamera.SetCustomPropertyValue<RBX::CoordinateFrame>("CFrame", cframe);
										}
									}
								}
							}*/
						}
					}
				}
			}
		}
	}

	bool func_ret = reinterpret_cast<bool(__thiscall*)(int, int)>(targetFunc)(self.ptr(), a1);
	return func_ret;
}

void init() {
	open_console();
	DisableLogs();

	RBX::TaskScheduler scheduler;

	MemCheckBypass memBypass;
	//memBypass.Enable();

	RBX::TaskScheduler::Job RenderJob = scheduler.FindJobByName("Render");
	targetFunc = vftable_hook(RenderJob.ptr(), (DWORD)&testfunc, 4, 20);

	RBX::Instance workspace = RenderJob.GetDataModel().FindFirstChildOfClass("Workspace");

	RBX::Reflection::BoundFuncDescriptor Raycast = workspace.FindBoundFuncDescriptor("Raycast");
	printf("Raycast: %p\n", Raycast.ptr());
	//targetFunc = Raycast.Func();
	//*(uintptr_t*)(Raycast + 0x40) = (uintptr_t)&fastcallHook;

	/*RBX::Instance PrimaryPart = workspace.FindFirstChild("bobo12").GetPropertyValue<int>("PrimaryPart");
	RBX::Vector3 pos = PrimaryPart.GetCustomPropertyValue<RBX::Vector3>("Position");
	RBX::Vector3 straightDown{ 0.f, -90.f, 0.f };

	std::vector<RBX::Instance> FilterDescendantsInstances;

	uintptr_t raycastParams = (uintptr_t)VirtualAlloc(0, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); //(uintptr_t)malloc(40);
	*(bool*)(raycastParams + 0x1) = false; // IgnoreWater
	*(BYTE*)(raycastParams + 0x2) = 0x53; // dk1
	*(BYTE*)(raycastParams + 0x3) = 0x12; // dk2

	*(bool*)(raycastParams + 0x4) = false; // Blacklist (0) Whitelist (1)
	*(uintptr_t*)(raycastParams + 0x8) = (uintptr_t)&FilterDescendantsInstances;
	*(std::string*)(raycastParams + 0x10) = std::string("Default");

	printf("raycastParams : %p\n" , raycastParams);

	//Sleep(1000 * 30);

	//int RaycastResult = workspace.CallBoundFunc("Raycast", &pos, &straightDown, raycastParams);

	//printf("RaycastResult : %p %p\n", RaycastResult,&RaycastResult);

	static int ret = 0;
	static int lol = reinterpret_cast<bool(__thiscall*)(int, int*, RBX::Vector3*, RBX::Vector3*, int)>(Raycast.Func())(workspace.ptr(), &ret, &pos, &straightDown, raycastParams);

	printf("%p %p\n", ret, &lol);*/

	// ret from hooking raycast is corect but this isnt ^ ? - is from CallBoundFunc, 

	//printf("targetFunc:  %p\n", targetFunc);
	
	// work on hooking Job.Step - use the vftable class to add hooking etc
	// work on : GetProperty "Name", CallBoundFunc "IsA"
	// hook BoundFunc's that pass a func as a param to see how funcs r handled
	// use MemCheckBypass to call functions like roblox's FindFirstChild so it doesn't wipe it

	// work on : remote-related calls etc
	/*
		*arg is argument sizeof = 44 idk maybe 48 yea its
		
	*/

	// you can get luastate by hooking the vftable of any descriptor !

	/*RBX::Instance DataModel = scheduler.FindJobByName("Render").GetDataModel();
	RBX::Instance Players = DataModel.FindFirstChildOfClass("Players");
	RBX::Instance Workspace = DataModel.GetPropertyValue<int>("Workspace");
	RBX::Instance RunService = DataModel.FindFirstChildOfClass("RunService"); // need to work on GetService

	RBX::Instance Baseplate = Workspace.FindFirstChild("Baseplate"); printf("Baseplate: %p\n", Baseplate);
	RBX::Instance LocalPlayer = Players.GetPropertyValue<int>("LocalPlayer"); printf("LocalPlayer: %p\n", LocalPlayer);
	RBX::Instance Character = LocalPlayer.GetPropertyValue<int>("Character"); printf("Character: %p\n", Character);

	RBX::Instance CurrentCamera = Workspace.GetPropertyValue<int>("CurrentCamera"); printf("CurrentCamera: %p\n", CurrentCamera);
	RBX::Instance Mouse = LocalPlayer.CallBoundFunc("GetMouse"); printf("Mouse %p\n", Mouse); // causes crashes sometimes (CallBoundFunc) finally fixed holy fuk
	RBX::Instance Humanoid = Character.FindFirstChildOfClass("Humanoid");
	RBX::Instance PrimaryPart = Character.GetPropertyValue<int>("PrimaryPart");

	printf("GetMouse :  %p\n", LocalPlayer.FindBoundFuncDescriptor("GetMouse").ptr());*/

	// lets work on GetService
	//RBX::Reflection::BoundFuncDescriptor GetServiceDescriptor = DataModel.FindBoundFuncDescriptor("GetService");

	/*RBX::Instance RemoteEvent = workspace.FindFirstChild("RemoteEvent");
	RBX::Reflection::BoundFuncDescriptor FireServer = RemoteEvent.FindBoundFuncDescriptor("FireServer");
	targetFunc = FireServer.Func();
	*(uintptr_t*)(FireServer + 0x40) = (uintptr_t)&fastcallHook;*/

	//static int a = GetPartBetweenPoints(Workspace, Baseplate.FindFirstChild("A").GetCustomPropertyValue<RBX::Vector3>("Position"), Baseplate.FindFirstChild("B").GetCustomPropertyValue<RBX::Vector3>("Position"));
	//printf("GetPartBetweenPoints a = %p\n", a);


	/*while (true) {
		RBX::Vector3 origin = PrimaryPart.GetCustomPropertyValue<RBX::Vector3>("Position");
		RBX::Vector3 vec = Baseplate.GetCustomPropertyValue<RBX::Vector3>("Position") - origin;
		RBX::Vector3 direction = vec.unit() * vec.magnitude();

		int b = Workspace.CallBoundFunc("FindPartOnRay", origin.x, origin.y, origin.z, direction.x, direction.y, direction.z, Character.ptr(), 0);
		RBX::Instance i(readloc(readloc(b) + 8));
		printf("%s\n", i.Name().c_str());
	}*/

	//RBX::Instance LocalScript = DataModel.FindFirstChildOfClass("ReplicatedFirst").FindFirstChildOfClass("LocalScript");

	//printf("CachedRemoteSource %p\n", LocalScript.GetCustomPropertyValue<int>("CachedRemoteSource"));

	/*std::string str;
	while (true) {
		//RBX::Vector3 vec3 = PrimaryPart.GetCustomPropertyValue<RBX::Vector3>("Position");
		//printf("%f %f %f\n", vec3.x, vec3.y, vec3.z);


		RBX::CoordinateFrame cframe = CurrentCamera.GetCustomPropertyValue<RBX::CoordinateFrame>("CFrame");
		cframe.lookAt(Baseplate.GetCustomPropertyValue<RBX::Vector3>("Position"));
		CurrentCamera.SetCustomPropertyValue<RBX::CoordinateFrame>("CFrame", cframe);

		//RBX::CoordinateFrame CFrame = Baseplate.GetCustomPropertyValue<RBX::CoordinateFrame>("CFrame");
		//printf("cframe: %p\n", &CFrame);
	}*/

	//RBX::Vector3 origin = PrimaryPart.GetCustomPropertyValue<RBX::Vector3>("Position");
	//printf("%s\n", origin.tostring());
	//RBX::Vector3 direction = { 0, -90, 0 };

	//printf("%p\n", GetPartBetweenPoints(Workspace, origin, direction, Character.ptr())); // why does this crash when 2nd call

	/*int a = Workspace.CallBoundFunc("FindPartOnRay", origin.x, origin.y, origin.z, direction.x, direction.y, direction.z, 0, 0);
	printf("FindPartOnRay %p\n", a);

	a = Workspace.CallBoundFunc("FindPartOnRay", origin.x, origin.y, origin.z, direction.x, direction.y, direction.z, 0, 0);
	printf("FindPartOnRay %p\n", a);*/

	/*int temp;
	int ret = reinterpret_cast<int(__thiscall*)(int, int*, float,float,float,float,float,float,int,int)>(Raycast.Func())(Workspace.ptr(), &temp, 0.f, -30.f, 0.f, 0.f, 30.f, 0.f,0,0);
	printf("ret: %p - %p\n", ret, temp);

	int CollidedPart = readloc(readloc(readloc(ret)) + 8);
	printf("%p\n", CollidedPart);*/

	// ClassDescriptor.FindBoundFuncDescriptor("Destroy").Call<void>(Character.ptr()); - correct call

}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    if (reason == DLL_PROCESS_ATTACH) return CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)init, 0, 0, 0));
	//if (reason == DLL_PROCESS_DETACH) { /*FreeLibraryAndExitThread(hModule, TRUE);*/ exit(0); }
    return TRUE;
}




/*RBX::Reflection::PropertyDescriptor Position = Baseplate.FindPropertyDescriptor("Position");
	RBX::Reflection::GetSet posGetSet = Position.getGetSet();

	targetFunc = (posGetSet.SetFunc);

	vftable_hook(posGetSet.ptr(), (DWORD)&fastcallHook, 4, 8);*/


// GETS PLACELAUNCHER AUTOMATICALLY !

	/*const char* aob = "C706????????85C97475"; //"558BEC6AFF68????????64A1????????50648925????????83EC0853568BD957895DECC703????????";

	std::vector<int> scan_results = memscan::scan(aob, false);

	printf("scan_results (%d): \n", scan_results.size());
	for (int i : scan_results) {
		DWORD loc = i;
		printf("	%p\n", loc);
		printf("		%p\n", *(DWORD*)(loc + 2));
		const char* a = (const char*)(loc + 2);
		std::vector<int> aob_scan = memscan::scan(a,false);
		printf("size:%d\n", aob_scan.size());
		for (int x : aob_scan) printf("			%p\n", x);
		printf("%02X %02X %02X %02X\n", a[0], a[1], a[2], a[3]);
	}
	printf("}\n");*/




	/*DWORD ret = CurrentCamera.CallBoundFunc("WorldToViewportPoint", pos.x, pos.y, pos.z); // why this crashing ?

	RBX::Vector3 vec3 = *(RBX::Vector3*)(readloc(ret) + 8);

	float X = vec3.x, Y = vec3.y, magnitude = vec3.z;

	//printf("{%f, %f}\n", width, height);

	RECT rect = { 0 };
	GetWindowRect(FindWindow(0, L"Roblox"), &rect);

	if (GetAsyncKeyState(0x42)) {
		SetCursorPos(rect.left + int(X), rect.top + int(Y) + 25);

		INPUT Input = { 0 };
		Input.type = INPUT_MOUSE;
s		Input.mi.dx = 1;
		Input.mi.dy = 1;
		Input.mi.dwFlags = MOUSEEVENTF_MOVE;
		SendInput(1, &Input, sizeof(Input));
		Input.mi.dx = -1;
		Input.mi.dy = -1;
		SendInput(1, &Input, sizeof(Input));
	}
	if (GetAsyncKeyState(0x43)) {
		RBX::CoordinateFrame cframe = CurrentCamera.GetCustomPropertyValue<RBX::CoordinateFrame>("CFrame");
		cframe.lookAt(Baseplate.GetCustomPropertyValue<RBX::Vector3>("Position"));
		CurrentCamera.SetCustomPropertyValue<RBX::CoordinateFrame>("CFrame", cframe);
	}
	*/