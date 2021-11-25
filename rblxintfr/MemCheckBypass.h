#pragma once
#include "TaskScheduler.h"

static bool enabled_memcheckbypass = false;
static DWORD orig_memcheckfn = NULL;

static bool hook_memcheckfn(int a1, RBX::TaskScheduler::Job self) {
	bool ret = false;

	if (!enabled_memcheckbypass)
		ret = reinterpret_cast<bool(__thiscall*)(int, int)>(orig_memcheckfn)(self.ptr(), a1);

	return ret;
}

class MemCheckBypass {
public:
	MemCheckBypass() {
		RBX::TaskScheduler scheduler;
		orig_memcheckfn = vftable_hook(scheduler.FindJobByName("US14116").ptr(), (DWORD)&hook_memcheckfn, 4, 20);
	}
	void Enable() {
		enabled_memcheckbypass = true;
	}
	void Disable() {
		enabled_memcheckbypass = false;
	}
};