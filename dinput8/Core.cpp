#include "Core.h"

CCore* Core;
CItemRandomiser *ItemRandomiser;
CAutoEquip *AutoEquip;
SCore* CoreStruct;

DWORD64 qItemEquipComms = 0;

DWORD64 rItemRandomiser = 0;
DWORD64 rAutoEquip = 0;
DWORD64 rNoWeaponRequirements = 0;
DWORD64 rEquipLock = 0;

VOID CCore::Start() {

	Core = new CCore();
	CoreStruct = new SCore();
	ItemRandomiser = new CItemRandomiser();
	AutoEquip = new CAutoEquip();

	Core->DebugInit();

	CoreStruct->hHeap = HeapCreate(8, 0x10000, 0);
	if (!CoreStruct->hHeap) {
		Core->Panic("Unable to allocate appropriate heap", "...\\Randomiser\\Core\\Core.cpp", FE_MemError, 1);
		int3
	};

	if (!Core->Initialise()){
		Core->Panic("Failed to initialise", "...\\Randomiser\\Core\\Core.cpp", FE_InitFailed, 1);
		int3
	};

	while (true) {
		Core->Run();
		Sleep(2500);
	};

	if (!HeapFree(CoreStruct->hHeap, 8, CoreStruct->pItemArray)) {
		Core->Panic("Given memory block appears invalid, or freed already", "...\\Randomiser\\Core\\Core.cpp", FE_InitFailed, 1);
		int3
	};

	HeapDestroy(CoreStruct->hHeap);

	delete AutoEquip;
	delete ItemRandomiser;
	delete CoreStruct;
	delete Core;

	return;
};

VOID CCore::Run() {

	if ((CoreStruct->dIsAutoSave) && CoreStruct->dIsListChanged) {
		Core->SaveArrayList();
		CoreStruct->dIsListChanged--;
	};

	return;
};

BOOL CCore::Initialise() {

	int i = 0;
	char pBuffer[MAX_PATH];
	BOOL bReturn = true;
	INIReader reader("RandomiserPreferences.ini");

	if (reader.ParseError() == -1) {
		MessageBoxA(NULL, "Failed to find 'RandomiserPreferences.ini'.", "Load Error", MB_ICONWARNING);
		int3
	};

	if (MH_Initialize() != MH_OK) return false;

	CoreStruct->dIsAutoSave = reader.GetBoolean("Randomiser", "SaveProgress", true);
	CoreStruct->dRandomEstusMaterial = reader.GetBoolean("Randomiser", "RandomEstusMaterials", false);
	CoreStruct->dRandomKeyItems = reader.GetBoolean("Randomiser", "RandomKeyItems", false);
	CoreStruct->dAllowPlusRings = reader.GetBoolean("Randomiser", "AllowPlusRings", false);
	CoreStruct->dAllowRandomWeaponReinforcement = reader.GetBoolean("Randomiser", "AllowRandomWeaponReinforcement", false);
	CoreStruct->dRandomInfusionsOnWeapons = reader.GetBoolean("Randomiser", "RandomInfusionsOnWeapons", true);

	CoreStruct->dGoodsRandomMin = reader.GetInteger("Values", "GoodsRandomMin", 1);
	CoreStruct->dGoodsRandomMax = reader.GetInteger("Values", "GoodsRandomMax", 99);
	CoreStruct->dScalingReinforcementVariance = reader.GetInteger("Values", "ScalingReinforcementVariance", 1);
	
	CoreStruct->dIsAutoEquip = reader.GetBoolean("AutoEquip", "AutoEquipToggle", false);
	CoreStruct->dAutoEquipWeapon = reader.GetBoolean("AutoEquip", "AutoEquipWeapon", false);
	CoreStruct->dAutoEquipArmor = reader.GetBoolean("AutoEquip", "AutoEquipArmor", false);
	CoreStruct->dAutoEquipRing = reader.GetBoolean("AutoEquip", "AutoEquipRing", false);

	CoreStruct->dLockEquipSlots = reader.GetBoolean("AutoEquip", "LockEquipSlots", false);
	CoreStruct->dIsNoWeaponRequirements = reader.GetBoolean("AutoEquip", "NoWeaponRequirements", false);

	CoreStruct->pOffsetArray = (DWORD*)HeapAlloc(CoreStruct->hHeap, 8, 0x3000);
	CoreStruct->pItemArray = (DWORD*)HeapAlloc(CoreStruct->hHeap, 8, 0x3000);

	if ((!CoreStruct->pItemArray) || (!CoreStruct->pOffsetArray)) {
		Core->Panic("Out of memory", "...\\Randomiser\\Core\\Core.cpp", FE_MemError, 1);
		int3
	};

#ifdef DEBUG
	sprintf_s(pBuffer, "[Randomiser] - SaveProgress = %i\n", CoreStruct->dIsAutoSave);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[Randomiser] - RandomEstusMaterials = %i\n", CoreStruct->dRandomEstusMaterial);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[Randomiser] - RandomKeyItems = %i\n", CoreStruct->dRandomKeyItems);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[Randomiser] - AllowPlusRings = %i\n", CoreStruct->dAllowPlusRings);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[Randomiser] - AllowRandomWeaponReinforcement = %i\n", CoreStruct->dAllowRandomWeaponReinforcement);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[Randomiser] - RandomInfusionsOnWeapons = %i\n", CoreStruct->dRandomInfusionsOnWeapons);
	printf_s(pBuffer);


	sprintf_s(pBuffer, "[Values] - GoodsRandomMin = %i\n", CoreStruct->dGoodsRandomMin);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[Values] - GoodsRandomMax = %i\n", CoreStruct->dGoodsRandomMax);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[Values] - ScalingReinforcementVariance = %i\n", CoreStruct->dScalingReinforcementVariance);
	printf_s(pBuffer);

	sprintf_s(pBuffer, "[AutoEquip] - AutoEquipToggle = %i\n", CoreStruct->dIsAutoEquip);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[AutoEquip] - AutoEquipWeapon = %i\n", CoreStruct->dAutoEquipWeapon);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[AutoEquip] - AutoEquipArmor = %i\n", CoreStruct->dAutoEquipArmor);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[AutoEquip] - AutoEquipRing = %i\n", CoreStruct->dAutoEquipRing);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[AutoEquip] - LockEquipSlots = %i\n", CoreStruct->dLockEquipSlots);
	printf_s(pBuffer);
	sprintf_s(pBuffer, "[AutoEquip] - NoWeaponRequirements = %i\n", CoreStruct->dIsNoWeaponRequirements);
	printf_s(pBuffer);
#endif

	GetArrayList();

	while (!CoreStruct->pOffsetArray[i + 1]) {
		if (CoreStruct->pOffsetArray[0] == i) break;
		CoreStruct->pItemArray[0]++;
		i++;
	}; 

	if (CoreStruct->dLockEquipSlots) {
		LockEquipSlots();
	};

	bReturn &= Hook(0x1407BBA80, (DWORD64)&tItemRandomiser, &rItemRandomiser, 5);

	if (CoreStruct->dIsAutoEquip) bReturn &= Hook(0x1407BBE92, (DWORD64)&tAutoEquip, &rAutoEquip, 6);
	if (CoreStruct->dIsNoWeaponRequirements) bReturn &= Hook(0x140C073B9, (DWORD64)&tNoWeaponRequirements, &rNoWeaponRequirements, 7);

	AutoEquip->EquipItem = (fEquipItem*)0x140AFBBB0;
	Core->DisplayGraveMessage = (fDisplayGraveMessage*)0x140BE1990;

	return bReturn;
};

BOOL CCore::GetArrayList() {

	DWORD i = 0;

	std::ifstream readfileA("Data_Item_Order.txt");
	std::ifstream readfileB("Data_Item_List.txt");
	
	DWORD* pOffsetList = CoreStruct->pOffsetArray;
	DWORD* pItemList = CoreStruct->pItemArray;

	if (readfileA.is_open()) {

		readfileA >> pOffsetList[0];
		i++;

		while (i <= *pOffsetList) {
			readfileA >> pOffsetList[i];
			i++;
		};
		readfileA.close();
	}
	else MessageBoxA(NULL, "Failed to find 'Data_Item_Order.txt'", "Load Error", MB_ICONWARNING);

	i = 1;

	if (readfileB.is_open()) {

		while (i <= *pOffsetList) {
			readfileB >> std::hex >> pItemList[i];
			i++;
		};
		readfileB.close();
		return true;
	}
	else MessageBoxA(NULL, "Failed to find 'Data_Item_List.txt'", "Load Error", MB_ICONWARNING);

	return false;
};

BOOL CCore::SaveArrayList() {

	DWORD i = 0;

	std::ofstream outfile("Data_Item_Order.txt");

	DWORD* pOffsetList = CoreStruct->pOffsetArray;
	DWORD* pItemList = CoreStruct->pItemArray;

	if (outfile.is_open()) {

		while (i <= *pOffsetList) {
			outfile << pOffsetList[i] << std::endl;
			i++;
		};
		outfile.close();
		return true;
	};

	CoreStruct->dIsAutoSave = 0;
	MessageBoxA(NULL, "Failed to find 'Data_Item_Order.txt'", "Save Error", MB_ICONWARNING);


	return false;

};

BOOL CCore::Hook(DWORD64 qAddress, DWORD64 qDetour, DWORD64* pReturn, DWORD dByteLen) {

	if (MH_CreateHook((LPVOID)qAddress, (LPVOID)qDetour, 0) != MH_OK) return false;
	if (MH_EnableHook((LPVOID)qAddress) != MH_OK) return false;

	*pReturn = (qAddress + dByteLen);

	return true;
};

VOID CCore::Panic(char* pMessage, char* pSort, DWORD dError, DWORD dIsFatalError) {

	char pOutput[MAX_PATH];
	char pTitle[MAX_PATH];

	sprintf_s(pOutput, "%s -> %s (%i)", pSort, pMessage, dError);

	if (IsDebuggerPresent()) {
		OutputDebugStringA(pOutput);
	};

	if (CoreStruct->dIsDebug) {
		printf_s("CCore::Panic is outputting debug-mode error information\n");
		sprintf_s(pOutput, "%s\n", pOutput);
		printf_s(pOutput);
	}
	else {
		if (dIsFatalError){
			sprintf_s(pTitle, "[Item Randomiser - Fatal Error]");
		} 
		else {
			sprintf_s(pTitle, "[Item Randomiser - Error]");
		}; 
		
		MessageBoxA(NULL, pOutput, pTitle, MB_ICONERROR);
	};

	if (dIsFatalError) *(int*)0 = 0;

	return;
};

VOID CCore::LockEquipSlots() {

	DWORD dOldProtect = 0;
	DWORD64 qEquip = 0x140B70F45;
	DWORD64 qUnequip = 0x140B736EA;

	if (!VirtualProtect((LPVOID)qEquip, 1, PAGE_EXECUTE_READWRITE, &dOldProtect)) return;
	if (!VirtualProtect((LPVOID)qUnequip, 1, PAGE_EXECUTE_READWRITE, &dOldProtect)) return;

	*(BYTE*)qEquip = 0x30;
	*(BYTE*)qUnequip = 0x30;

	if (!VirtualProtect((LPVOID)qEquip, 1, dOldProtect, &dOldProtect)) return;
	if (!VirtualProtect((LPVOID)qUnequip, 1, dOldProtect, &dOldProtect)) return;

	return;
};

VOID CCore::DebugInit() {
	CoreStruct->dIsDebug = 0;
#ifdef DEBUG
	FILE* fp;

	AllocConsole();
	SetConsoleTitleA("Dark Souls III - Item Randomiser Debug Console");
	freopen_s(&fp, "CONOUT$", "w", stdout);
	printf_s("Starting...\n");
	CoreStruct->dIsDebug = 1;
#endif

	return;
};