#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <bitset>
#include "INIReader.h"
#include "MinHook/include/MinHook.h"

#define ItemType_Weapon 0
#define ItemType_Protector 1
#define ItemType_Accessory 2
#define ItemType_Goods 4
#define int3 __debugbreak();
#define DEBUG

#define FE_InitFailed 0
#define FE_AmountTooHigh 1
#define FE_NullPtr 2
#define FE_NullArray 3
#define FE_BadFunc 4
#define FE_MemError 5
#define HE_InvalidItemType 6
#define HE_InvalidInventoryEquipID 7
#define HE_Undefined 8
#define HE_NoPlayerChar 9

#define MAX_LIST_ITEMS 3000

#define ITEM_EMBER 0x400001F4
#define ITEM_ESTUS_SHARD 0x4000085D
#define ITEM_UNDEAD_BONE_SHARD 0x4000085F
#define ITEM_ASHEN_ESTUS 0x400000BF

struct SCore;
struct SEquipBuffer;

typedef VOID fEquipItem(DWORD dSlot, SEquipBuffer* E);
typedef VOID fDisplayGraveMessage(DWORD dEvent);
typedef VOID fDisplayInfoMsg(DWORD64* pBuffer);

class CCore {
public:
	static VOID Start();
	virtual VOID Run();
	virtual BOOL Initialise();
	virtual BOOL GetArrayList();
	virtual BOOL SaveArrayList();
	virtual BOOL Hook(DWORD64 qAddress, DWORD64 qDetour, DWORD64* pReturn, DWORD dByteLen);
	virtual VOID Panic(char* pMessage, char* pSort, DWORD dError, DWORD dIsFatalError);
	virtual VOID DebugInit();
	virtual VOID LockEquipSlots();
	fDisplayInfoMsg* DisplayEquipLockMsg; //0x14075BC70
private:
	fDisplayGraveMessage* DisplayGraveMessage;
};

class CItemRandomiser {
public:
	virtual VOID RandomiseItem(UINT_PTR qWorldChrMan, UINT_PTR pItemBuffer, UINT_PTR pItemData, DWORD64 qReturnAddress);
	virtual VOID SortNewItem(DWORD* dItem, DWORD* dQuantity);
	virtual BOOL IsKeyGood(DWORD dItemID);
	virtual BOOL IsRestrictedGoods(DWORD dItemID);
	virtual BOOL IsPlusRing(DWORD dItemID);
	virtual BOOL IsUninfusableWeapon(DWORD dItemID);
	virtual BOOL IsShield(DWORD dItemID);
	virtual BOOL IsNormalWeapon(DWORD dItemID);
	virtual BOOL IsSpell(DWORD dItemID);
	virtual DWORD RandomiseNumber(DWORD dMin, DWORD dMax);
};

class CAutoEquip {
public:
	virtual VOID AutoEquipItem(UINT_PTR pItemBuffer, DWORD64 qReturnAddress);
	virtual BOOL SortItem(DWORD dItemID, SEquipBuffer* E);
	virtual BOOL FindEquipType(DWORD dItem, DWORD* pArray);
	virtual DWORD GetInventorySlotID(DWORD dItemID);
	virtual VOID LockUnlockEquipSlots(int iIsUnlock);
	fEquipItem* EquipItem; //0x140AFBBB0

};

struct SCore {
	DWORD dIsDebug;
	DWORD dIsAutoSave;

	DWORD dRandomEstusMaterial;
	DWORD dRandomKeyItems;
	DWORD dAllowPlusRings;
	DWORD dAllowRandomWeaponReinforcement;
	DWORD dRandomInfusionsOnWeapons;

	DWORD dGoodsRandomMin;
	DWORD dGoodsRandomMax;
	DWORD dScalingReinforcementVariance;

	DWORD dIsAutoEquip;
	DWORD dAutoEquipWeapon;
	DWORD dAutoEquipArmor;
	DWORD dAutoEquipRing;
	DWORD dLockEquipSlots;
	DWORD dIsNoWeaponRequirements;

	DWORD dIsListChanged;

	UINT_PTR qLocalPlayer = 0x144740178; // Base A
	UINT_PTR qWorldChrMan = 0x144768E78; // Base B
	UINT_PTR qSprjLuaEvent = 0x14473A9C8;
	UINT_PTR qBaseC = 0x144743AB0;
	UINT_PTR qBaseD = 0x144743A80;
	UINT_PTR qParam = 0x144782838;
	UINT_PTR qBaseE = 0x14473FD08;
	UINT_PTR qBaseF = 0x14473AD78;
	UINT_PTR qBaseZ = 0x144768F98;
	UINT_PTR qLockBonus = 0x1408C06A6;
	UINT_PTR qPickupPtr = 0x13FFA0021;

	HANDLE hHeap;
	DWORD* pOffsetArray;
	DWORD* pItemArray;
};

struct SEquipBuffer {
	DWORD dUn1;
	DWORD dUn2;
	DWORD dEquipSlot;
	char unkBytes[0x2C];
	DWORD dInventorySlot;
	char paddingBytes[0x60];
};

extern "C" DWORD64 qItemEquipComms;

extern "C" DWORD64 rItemRandomiser;
extern "C" VOID tItemRandomiser();
extern "C" VOID fItemRandomiser(UINT_PTR qWorldChrMan, UINT_PTR pItemBuffer, UINT_PTR pItemData, DWORD64 qReturnAddress);

extern "C" DWORD64 rAutoEquip;
extern "C" VOID tAutoEquip();
extern "C" VOID fAutoEquip(UINT_PTR pItemBuffer, DWORD64 pItemData, DWORD64 qReturnAddress);

extern "C" DWORD64 rNoWeaponRequirements;
extern "C" VOID tNoWeaponRequirements();
extern "C" VOID fNoWeaponRequirements(DWORD* pRequirementPtr);

