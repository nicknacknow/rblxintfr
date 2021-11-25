#pragma once
#include <Windows.h>
#include <iostream>
#include <string>

#include <d3dx9.h>
#include <d3d9.h>

#include "Memory.h"

#include "MemCheckBypass.h"
#include "TaskScheduler.h"
#include "Classes.h"

namespace Functions {
	class SingletonHelper {
	private:
		typedef bool(CheckFunc)(RBX::Instance player);

		struct Check {
			Check(CheckFunc f, std::string n) : func(f), name(n) {}
			CheckFunc* func;
			std::string name;
			bool enabled = true;
		};
	public:
		void AddCheck(Check check) {
			Checks.emplace_back(check);
		}
		void AddCheck(std::string name, CheckFunc func) {
			Check check{ func, name };
			return AddCheck(check);
		}
		template <class lambda>
		void AddCheck(std::string name, lambda func) {
			Check check{ func, name };
			return AddCheck(check);
		}
		bool DoChecks(RBX::Instance player) {
			for (Check check : Checks)
				if (check.enabled && !check.func(player)) return false;
			return true;
		}
		Check SearchCheck(std::string name) {
			for (Check check : Checks) if (!check.name.compare(name)) return check;
		}
	private:
		std::vector<Check> Checks;
	};
}

// create a class for GetClosestPlayer - allowing adding and removing etc
// allow functions to be added which are looped through and given args to help specify ret
// eg these functions could check if localplayer, check team, wallcheck, whitelist/blacklist
// would be good to do w2s check too in one of these funcs (like broomhub aimbot)