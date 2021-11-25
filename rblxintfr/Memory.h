#pragma once
#include "mscan.h"

#define readloc(addr) *(uintptr_t*)(addr)

#define IDABASE 0x400000
#define BASE 0x950000
#define rebase(offset, base) (offset - base + (uintptr_t)GetModuleHandleA(0))

DWORD vftable_hook(DWORD Instance, DWORD pFunc, int index, int vtable_size) {
	DWORD main_vtable = *(DWORD*)Instance;
	DWORD* our_vtable = new DWORD[vtable_size];
	memcpy(our_vtable, (void*)main_vtable, vtable_size * sizeof DWORD);

	our_vtable[index] = pFunc;

	DWORD old;
	VirtualProtect((LPVOID)Instance, 4, PAGE_READWRITE, &old);
	*(DWORD*)Instance = (DWORD)our_vtable;
	VirtualProtect((LPVOID)Instance, 4, old, &old);

	return *(DWORD*)(main_vtable + index * sizeof DWORD);
}

bool is_char_valid(BYTE x) {
	return 0x20 <= x && x <= 0x7E;
}

bool is_string_valid(std::string str) {
	for (int i = 0; i < str.length(); i++)
		if (!is_char_valid(str[i]))
			return false;

	return true;
}

std::string read_string(DWORD addr, bool loc = true) {
	std::string ret = *(std::string*)addr;
	//int index = 0;

	//while (BYTE c = read<BYTE>(readloc(addr) + index++))
	//	ret += c;

	/*while (true) {
		BYTE c = *(BYTE*)(loc ? readloc(addr) : addr + index++);
		if (c == 0)
			if (index != 1) break;
		ret += c;
	}*/

	return is_string_valid(ret) ? ret : read_string(readloc(addr));
}

#define mem_add(name, type, offset) \
auto name() { \
	return (type)(this->ptr() + offset); \
} 

#define addptr(name, type, offset) \
auto name() { \
	return type(readloc(this->ptr() + offset)); \
} 

#define addstr(name, offset) \
auto name(bool ptr = false) { \
	return ptr ? read_string((this->ptr() + offset)) : read_string(readloc(this->ptr() + offset)); \
}

/*#define addfunc(name, ret, offset, callconv) \
template<typename... T> \
ret name(T...arg) { \
	return (ret)reinterpret_cast<ret(callconv*)(T...)>(this->ptr() + offset)(arg...); \
}*/