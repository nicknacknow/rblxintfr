#pragma once
#include <Windows.h>
#include <string>
#include <vector>

#include "Memory.h"

#include "G3DCore.h"

class ClassHandler {
public:
	ClassHandler() {}
	~ClassHandler() { instance = NULL; }
	ClassHandler(void* p) : instance(reinterpret_cast<uintptr_t>(p)) {}
	ClassHandler(uintptr_t p) : instance(p) {}

	ClassHandler(const ClassHandler& n) : instance(n.instance) {}

	uintptr_t operator+(int a) { return instance + static_cast<uintptr_t>(a); }
	uintptr_t operator-(int a) { return instance - static_cast<uintptr_t>(a); }

	bool operator==(const ClassHandler& n) {
		return n.instance == this->instance;
	}

	operator bool() {
		return !!this->ptr();
	}

	uintptr_t ptr() { return instance; }
	void set(uintptr_t p) { instance = p; }
private:
	uintptr_t instance = NULL;
};

// make VTable handler? 

class VTableHandler {
public:
	VTableHandler() {}
	VTableHandler(uintptr_t p) : main_class(p), ptr(readloc(p)) {}

	uintptr_t GetVFuncFromIndex(int index) {
		return readloc(this->ptr + (index * 4));
	}
private:
	uintptr_t main_class;
	uintptr_t ptr;
};

namespace RBX {
	namespace Reflection {
		class GetSet : public ClassHandler {
		public:
			GetSet() : ClassHandler() {} // SetValue https://i.imgur.com/A3GRAC0.png
			GetSet(uintptr_t p) : ClassHandler(readloc(p)), vtable(this->ptr()) { 
				this->GetFunc = vtable.GetVFuncFromIndex(3);
				this->SetFunc = vtable.GetVFuncFromIndex(4);
			}

			uintptr_t GetFunc;
			uintptr_t SetFunc;

			bool isReadOnly = false;
			bool isWriteOnly = false;
		private:
			VTableHandler vtable; // make it hookable - so i can hook returns and sets for OP shit 
			/*
			index(0) = NewTypedPropertyDescriptor
			index(1) = GetRobloxLocked
			index(3) = GetRobloxLocked
			index(3) = GetValue
			index(4) = SetValue
			index(5) = EqualValue
			*/
		};

		class Descriptor : public ClassHandler {
		public:
			Descriptor() {}
			Descriptor(uintptr_t p) : ClassHandler(readloc(p)), vtable(this->ptr()) {}

			/*std::string Name() {
				//printf("ok : %p - \n", this->ptr());
				//printf("%s\n", (*(std::string*)(this->ptr() + 0x4)).c_str());
				return *(const char**)(this->ptr() + 0x4); //*(std::string*)(this->ptr() + 0x4);
			}*/

			addstr(Name, 0x4); // this can crash .. need2 fix
			addstr(Category, 0x18);

			VTableHandler vtable;
		};

		class PropertyDescriptor : public Descriptor {
		public:
			PropertyDescriptor(uintptr_t p) : Descriptor(p), getset(this->ptr() + *(BYTE*)(vtable.GetVFuncFromIndex(2) + 2)) {} //, getset(this->ptr() + *(BYTE*)(vtable.GetVFuncFromIndex(2)) + 0x2) { } //this->getset.init(this->ptr() + 0x34);
			
			mem_add(IsEnum, bool, 0x20);

			template<class ret, class... T>
			ret callGet(T... arg) {
				return reinterpret_cast<ret(__thiscall*)(uintptr_t, T...)>(this->getGetSet().GetFunc)(this->getGetSet().ptr(), arg...);
			}

			template<class... T>
			void callSet(T... arg) {
				reinterpret_cast<void(__thiscall*)(uintptr_t, T...)>(this->getGetSet().SetFunc)(this->getGetSet().ptr(), arg...);
			}

			GetSet getGetSet() { return this->getset; }

			std::string get_type() {
				uintptr_t type = readloc(this->ptr() + 0x28); 
				return read_string(type + 0x4);
			}
		private:
			GetSet getset;
		};

		class BoundFuncDescriptor : public Descriptor {
		public:
			BoundFuncDescriptor() : Descriptor() {}
			BoundFuncDescriptor(uintptr_t p) : Descriptor(p) { CallFunc = readloc(this->ptr() + 0x40); } // GetVFuncFromIndex(2); +148? is offset is the lua func thats called

			addptr(Func, uintptr_t, 0x40); // was 48 now 58 , 17/11/21 now 40

			template<class... T>
			uintptr_t Call(uintptr_t obj, T... arg) {
				return reinterpret_cast<uintptr_t(__thiscall*)(uintptr_t, T...)>(CallFunc)(obj, arg...);
			}
		private:
			uintptr_t CallFunc;
		};

		class ClassDescriptor : public ClassHandler {
		public:
			ClassDescriptor(uintptr_t p) : ClassHandler((p)) {}
			addstr(ClassName, 0x4);

			std::vector<PropertyDescriptor> GetPropertyDescriptors() {
				std::vector<PropertyDescriptor> PropertyDescriptors{};

				for (uintptr_t i = readloc(this->ptr() + 0x18); i != readloc(this->ptr() + 0x1C); i += 4) {
					PropertyDescriptors.emplace_back(i);
				} 

				return PropertyDescriptors;
			} // 0x70 is EventDesc list, 0xC8 = BoundFuncDesc list, 0x120 = BoundYieldFuncDesc list, 0x178 = BoundAsyncCallbackDesc
			// above has updated - can automatically grab these from ClassDescriptors 1st (and only) vfunc
			std::vector<BoundFuncDescriptor> GetBoundFuncDescriptors() {
				std::vector<BoundFuncDescriptor> BoundFuncDescriptors{};

				for (uintptr_t i = readloc(this->ptr() + 0xD8); i != readloc(this->ptr() + 0xDC); i += 4) {
					BoundFuncDescriptors.emplace_back(i);
				}

				return BoundFuncDescriptors;
			}

			PropertyDescriptor FindPropertyDescriptor(std::string str) {
				for (PropertyDescriptor pd : this->GetPropertyDescriptors())
					if (!pd.Name().compare(str)) return pd;
			}

			BoundFuncDescriptor FindBoundFuncDescriptor(std::string str) {
				for (BoundFuncDescriptor bfd : this->GetBoundFuncDescriptors()) {
					//printf("%s\n", bfd.Name().c_str());
					if (!bfd.Name().compare(str)) return bfd;
				}
			}
		};
	}

	class Instance : public ClassHandler {
	public:
		Instance() : ClassHandler() {}
		Instance(uintptr_t p) : ClassHandler(p) {}

		mem_add(Archivable, bool, 0x21);
		mem_add(Locked, bool, 0xE5);
		mem_add(CastShadow, bool, 0xCB);
		mem_add(Transparency, float, 0xCC);
		mem_add(Reflectance, float, 0xD0);

		addptr(Parent, Instance, 0x34);

		addstr(Name, 0x28);

		Reflection::ClassDescriptor GetClassDescriptor() { return Reflection::ClassDescriptor(readloc(this->ptr() + 0xC)); }

		RBX::Reflection::PropertyDescriptor FindPropertyDescriptor(std::string name) {
			return this->GetClassDescriptor().FindPropertyDescriptor(name);
		}

		RBX::Reflection::BoundFuncDescriptor FindBoundFuncDescriptor(std::string name) {
			return this->GetClassDescriptor().FindBoundFuncDescriptor(name);
		}

		template<class T>
		T GetPropertyValue(std::string name) {
			RBX::Reflection::PropertyDescriptor PropertyDescriptor = this->FindPropertyDescriptor(name);
			return PropertyDescriptor.callGet<T>(this->ptr());
		}

		template<class ret >
		ret GetCustomPropertyValue(std::string name) { // used for custom roblox variables, such as Vector3
			RBX::Reflection::PropertyDescriptor prop_descriptor = this->FindPropertyDescriptor(name);

			static ret val;
			reinterpret_cast<void(__thiscall*)(uintptr_t, ret*, int)>(prop_descriptor.getGetSet().GetFunc)(prop_descriptor.getGetSet().ptr(), &val, this->ptr());
			return val;
		}

		template<class T>
		void SetCustomPropertyValue(std::string name, T val) {
			RBX::Reflection::PropertyDescriptor prop_descriptor = this->FindPropertyDescriptor(name);
			reinterpret_cast<void(__thiscall*)(uintptr_t, uintptr_t, T*)>(prop_descriptor.getGetSet().SetFunc)(prop_descriptor.getGetSet().ptr(), this->ptr(), &val);

		}

		template<class T> // will need to work on this
		void SetPropertyValue(std::string name, T val) {
			RBX::Reflection::PropertyDescriptor PropertyDescriptor = this->FindPropertyDescriptor(name);
			PropertyDescriptor.callSet(this->ptr(), &val);
		}

		template <class... T>
		uintptr_t CallBoundFunc(std::string name, T... args) {
			RBX::Reflection::BoundFuncDescriptor Descriptor = this->FindBoundFuncDescriptor(name);

			static int callr;
			//printf("ok\n");
			Descriptor.Call(this->ptr(), &callr, args...);
			return callr;
		}

		std::vector<Instance> GetChildren() {
			std::vector<Instance> Children;

			uintptr_t pChildArray = readloc(this->ptr() + 0x2C);
			uintptr_t pEndArray = readloc(pChildArray + 0x4);

			for (uintptr_t p = readloc(pChildArray); p != pEndArray; p += 8) {
				Children.emplace_back(readloc(p));
			}

			return Children;
		}

		Instance FindFirstChild(std::string childname) {
			for (Instance child : this->GetChildren())
				if (child.Name() == childname)
					return child;
			return NULL;
		}

		Instance FindFirstChildOfClass(std::string classname) {
			for (Instance child : this->GetChildren())
				if (child.GetClassDescriptor().ClassName() == classname)
					return child;
			return NULL;
		}
	};

	class Model : public Instance {
	public:
		addptr(PrimaryPart, Instance, 0xEC);
	};
}