/*

///// findFirstChildByName

55 8B EC 51 53 56 8B F1 57 8B 7D 0C 8B
\x55\x8B\xEC\x51\x53\x56\x8B\xF1\x57\x8B\x7D\x0C\x8B 
xxxxxxxxxxxxx

DWORD* __thiscall findFirstChildByName(DWORD self, const char* name, size_t len)
typedef DWORD(__thiscall* findFirstChildByName)(DWORD self, const char* name, size_t len);






Player::onServiceProvider
Player::setCharacter


Workspace::setGravity
Workspace::setPhysicsSteppingMethod


LuaSourceContainer::setCachedRemoteSource


///// PRIMITIVE

// ->getCoordinateFrame()

56 8D 71 60 8B CE
\x56\x8D\x71\x60\x8B\xCE 
xxxxxx





// Camera

CoordinateFrame Camera::getRenderingCoordinateFrame()
char *__thiscall Camera::getRenderingCoordinateFrame(char *this)

56 8D 71 68 8B CE
\x56\x8D\x71\x68\x8B\xCE 
xxxxxx


*/