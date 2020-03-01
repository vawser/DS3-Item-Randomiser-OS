#include "Core.h"

extern CCore* Core;
extern CItemRandomiser *ItemRandomiser;
extern SCore* CoreStruct;
DWORD pProgressionItems[50];
DWORD pLimitedItems[250];
DWORD pPlusRings[200];
DWORD pConsumables[200];
DWORD pConsumablesMax = 151;
DWORD pUninfusableWeapons[200];
DWORD pShields[200];
char pBuffer[MAX_PATH];

VOID fItemRandomiser(UINT_PTR qWorldChrMan, UINT_PTR pItemBuffer, UINT_PTR pItemData, DWORD64 qReturnAddress) {

	if (*(int*)(pItemData) >= 0) ItemRandomiser->RandomiseItem(qWorldChrMan, pItemBuffer, pItemData, qReturnAddress);

	return;
};

VOID CItemRandomiser::RandomiseItem(UINT_PTR qWorldChrMan, UINT_PTR pItemBuffer, UINT_PTR pItemData, DWORD64 qReturnAddress) {

	DWORD dItemAmount = 0;
	DWORD dItemID = 0;
	DWORD dItemQuantity = 0;
	DWORD dItemDurability = 0;
	DWORD dOffsetMax = 0;
	DWORD* pOffsetArray;

	dItemAmount = *(int*)pItemBuffer;
	pItemBuffer += 4;

	if (dItemAmount > 6) {
		Core->Panic("Too many items!", "...\\Source\\ItemRandomiser\\ItemRandomiser.cpp", FE_AmountTooHigh, 1);
		int3
	};

	while (dItemAmount) {
	
		dItemID = *(int*)(pItemBuffer);
		dItemQuantity = *(int*)(pItemBuffer + 0x04);
		dItemDurability = *(int*)(pItemBuffer + 0x08);
		pOffsetArray = CoreStruct->pOffsetArray;

		dOffsetMax = *pOffsetArray;
		pOffsetArray++;

		if (!CoreStruct->dRandomiseKeyItems) { 
			if (IsGameProgressionItem(dItemID)) return;
		};
		if (!CoreStruct->dAllowPlusRings) { 
			if (IsPlusRing(dItemID)) return;
		};
		if (!CoreStruct->dRandomiseEstusShards) { 
			if ((dItemID == ITEM_ESTUS_SHARD)) return;
		};
		if (!CoreStruct->dRandomiseBoneShards) { 
			if ((dItemID == ITEM_UNDEAD_BONE_SHARD)) return;
		};

		if (CoreStruct->pItemArray[0] < dOffsetMax) {
			dItemID = CoreStruct->pItemArray[pOffsetArray[CoreStruct->pItemArray[0]]];
			pOffsetArray[CoreStruct->pItemArray[0]] = 0;
		}
		else {
			dItemID = CoreStruct->pItemArray[RandomiseNumber(1, dOffsetMax)];
		};

		CoreStruct->pItemArray[0]++;

		SortNewItem(&dItemID, &dItemQuantity);

		if ((dItemID == ITEM_ESTUS_SHARD) || (dItemID == ITEM_UNDEAD_BONE_SHARD)) {
			if (!CoreStruct->dRandomiseEstusShards) {
				if (CoreStruct->dShowDebugPrint) {
					sprintf_s(pBuffer, "[Debug] - Randomise Estus Shard\n");
					printf_s(pBuffer);
				}
				dItemID = pConsumables[RandomiseNumber(0, pConsumablesMax)];
			}
			if (!CoreStruct->dRandomiseBoneShards) {
				if (CoreStruct->dShowDebugPrint) {
					sprintf_s(pBuffer, "[Debug] - Randomise Undead Bone Shard\n");
					printf_s(pBuffer);
				}
				dItemID = pConsumables[RandomiseNumber(0, pConsumablesMax)];
			}
		};
		if (!CoreStruct->dRandomiseKeyItems) {
			if (IsGameProgressionItem(dItemID)) {
				if (CoreStruct->dShowDebugPrint) {
					sprintf_s(pBuffer, "[Debug] - Randomise Key Item\n");
					printf_s(pBuffer);
				}
				dItemID = pConsumables[RandomiseNumber(0, pConsumablesMax)];
			}
		};
		if (!CoreStruct->dAllowPlusRings) {
			if (IsPlusRing(dItemID)) {
				if (CoreStruct->dShowDebugPrint) {
					sprintf_s(pBuffer, "[Debug] - Randomise Plus Ring\n");
					printf_s(pBuffer);
				}
				dItemID = pConsumables[RandomiseNumber(0, pConsumablesMax)];
			}
		};

		DebugItemPrint(*(int*)(pItemBuffer), *(int*)(pItemBuffer + 0x04), dItemID, dItemQuantity);
		
		*(int*)(pItemBuffer) = dItemID;
		*(int*)(pItemBuffer + 0x04) = dItemQuantity;
		*(int*)(pItemBuffer + 0x08) = -1;
	
		dItemAmount--;
		pItemBuffer += 0x0C;
	};

	CoreStruct->dIsListChanged++;

	return;

};

VOID CItemRandomiser::SortNewItem(DWORD* dItem, DWORD* dQuantity) {

	char pBuffer[MAX_PATH];
	UINT_PTR pPlayerPointer = 0;
	DWORD dItemType = 0;
	DWORD dItemSortID = 0;
	BYTE bPlayerUpgradeLevel = 0;
	
	if (!*dItem) {
		Core->Panic("No item", "...\\Source\\ItemRandomiser\\ItemRandomiser.cpp", HE_InvalidItemType, 0);
		*dItem = pConsumables[RandomiseNumber(0, pConsumablesMax)];
		*dQuantity = 1;
		return;
	};

	dItemType = (*dItem >> 0x1C);
	dItemSortID = (*dItem & 0x0FFFFFF);

	switch (dItemType) {
	
	case(ItemType_Weapon): {

		*dQuantity = 1;
		
		if ((*dItem >> 0x10) == 6) {
			*dQuantity = RandomiseNumber(1, 99);
			return;
		};

		pPlayerPointer = *(UINT_PTR*)(CoreStruct->qLocalPlayer);
		pPlayerPointer = *(UINT_PTR*)(pPlayerPointer + 0x10);
		bPlayerUpgradeLevel = *(BYTE*)(pPlayerPointer + 0xB3); // Get current upgrade level

		if (!bPlayerUpgradeLevel) return;

		// Handle uninfusables separately
		if (IsUninfusableWeapon(*dItem)) {
			// Weapons
			if (!IsShield(*dItem)) {
				if (bPlayerUpgradeLevel < 5) {
					*dItem += RandomiseNumber(0, bPlayerUpgradeLevel); // Reinforcement
				}
				else {
					*dItem += bPlayerUpgradeLevel; // Reinforcement
				}
			}
			// Shields
			else {
				if (bPlayerUpgradeLevel < 5) {
					*dItem += RandomiseNumber(0, bPlayerUpgradeLevel); // Reinforcement
				}
				else {
					*dItem += bPlayerUpgradeLevel; // Reinforcement
				}
			}
		}
		else {
			// Weapons
			if (!IsShield(*dItem)) {
				if (bPlayerUpgradeLevel < 5) {
					*dItem += RandomiseNumber(0, bPlayerUpgradeLevel); // Reinforcement
					*dItem += (RandomiseNumber(0, 14) * 100); // Infusion
				}
				else {
					*dItem += bPlayerUpgradeLevel; // Reinforcement
					*dItem += (RandomiseNumber(0, 14) * 100); // Infusion
				}
			}
			// Shields
			else {
				if (bPlayerUpgradeLevel < 5) {
					*dItem += RandomiseNumber(0, bPlayerUpgradeLevel); // Reinforcement
					*dItem += (RandomiseNumber(0, 10) * 100); // Infusion
				}
				else {
					*dItem += bPlayerUpgradeLevel; // Reinforcement
					*dItem += (RandomiseNumber(0, 10) * 100); // Infusion
				}
			}
		}
		return;
	
	};
	case(ItemType_Protector): {
		*dQuantity = 1;
		return;
	};
	case(ItemType_Accessory): {
		*dQuantity = 1;
		return;
	};
	case(ItemType_Goods): {
		*dQuantity = 1;

		// Limit restricted items to 1
		if (IsRestrictedGoods(*dItem)) return;

		// Otherwise randomise quantity
		*dQuantity = RandomiseNumber(1, 10);
		return;
	};
	default: {
		sprintf_s(pBuffer, "Invalid item type: %i (%08X)", dItemType, *dItem);
		Core->Panic(pBuffer, "...\\Source\\ItemRandomiser\\ItemRandomiser.cpp", HE_InvalidItemType, 0);
		*dItem = ITEM_EMBER;
		*dQuantity = 1;
	};
	
	};

};

BOOL CItemRandomiser::IsGameProgressionItem(DWORD dItemID) {

	int i = 0;

	while (pProgressionItems[i]) {
		if (dItemID == pProgressionItems[i]) return true;
		i++;
	};

	return false;
};

BOOL CItemRandomiser::IsRestrictedGoods(DWORD dItemID) {

	int i = 0;

	while (pLimitedItems[i]) {
		if (dItemID == pLimitedItems[i]) return true;
		i++;
	};

	return false;
};

BOOL CItemRandomiser::IsPlusRing(DWORD dItemID) {

	int i = 0;

	while (pPlusRings[i]) {
		if (dItemID == pPlusRings[i]) return true;
		i++;
	};

	return false;
};

BOOL CItemRandomiser::IsUninfusableWeapon(DWORD dItemID) {

	int i = 0;

	while (pUninfusableWeapons[i]) {
		if (dItemID == pUninfusableWeapons[i]) return true;
		i++;
	};

	return false;
};

BOOL CItemRandomiser::IsShield(DWORD dItemID) {

	int i = 0;

	while (pShields[i]) {
		if (dItemID == pShields[i]) return true;
		i++;
	};

	return false;
};

DWORD CItemRandomiser::RandomiseNumber(DWORD dMin, DWORD dMax) {

	char pBuffer[MAX_PATH];
	DWORD dGen = 0;

	if (dMin > dMax) {
		sprintf_s(pBuffer, "Defined minimum > maximum! %i > %i", dMin, dMax);
		Core->Panic(pBuffer, "...\\Source\\ItemRandomiser\\ItemRandomiser.cpp", HE_Undefined, 0);
		return 1;
	};

	dGen = (DWORD)(__rdtsc() % dMax);

	if ((!dMin) || (dGen > dMin)) return dGen;

	return dMin;
};

VOID CItemRandomiser::DebugItemPrint(DWORD dOldItem, DWORD dOldQuantity, DWORD dItem, DWORD dQuantity) {
#ifdef DEBUG
	char pBuffer[MAX_PATH];
	sprintf_s(pBuffer, "[%i] Item randomised | Old %08X (%i) | New %08X (%i)\n", CoreStruct->pItemArray[0], dOldItem, dOldQuantity, dItem, dQuantity);
	printf_s(pBuffer);
#endif
};

extern DWORD pProgressionItems[50] = {
	0x400007D1,
	0x400007D5,
	0x400007D8,
	0x400007DA,
	0x400007DE,
	0x4000084B,
	0x4000084C,
	0x4000084D,
	0x4000084E,
	0x4000087A,
	0x4000087B,
	0x40000BD1,
	0x40000BBE,
	0x40000BBF,
	0x40000BC0,
	0x40000BC1,
	0x40000BC2,
	0x40000BC3,
	0x40000BC4,
	0x40000BC5,
	0x40000836,
	0x4000083B,
	0x4000083C,
	0x4000083D,
	0x4000083E,
	0x4000083F,
	0x40000840,
	0x40000841,
	0x40000842,
	0x40000843,
	0x40000844,
	0x40000845,
	0x40000846,
	0x40000847,
	0x40000848,
	0x40000849,
	0x400007D7,
	0x400007D9,
	0x400007DB,
	0x400007DD,
	0x400001EA,
	0x40000097,
	0x400000BF,
	0x00000000
};

extern DWORD pLimitedItems[250] = {
	0x400007D1,
	0x400007D5,
	0x400007D8,
	0x400007DA,
	0x400007DE,
	0x4000084B,
	0x4000084C,
	0x4000084D,
	0x4000084E,
	0x4000087A,
	0x4000087B,
	0x40000BD1,
	0x40000BBE,
	0x40000BBF,
	0x40000BC0,
	0x40000BC1,
	0x40000BC2,
	0x40000BC3,
	0x40000BC4,
	0x40000BC5,
	0x40000836,
	0x4000083B,
	0x4000083C,
	0x4000083D,
	0x4000083E,
	0x4000083F,
	0x40000840,
	0x40000841,
	0x40000842,
	0x40000843,
	0x40000844,
	0x40000845,
	0x40000846,
	0x40000847,
	0x40000848,
	0x40000849,
	0x400007D7,
	0x400007D9,
	0x400007DB,
	0x400007DD,
	0x400001EA,
	0x40000834,
	0x40000208,
	0x40000209,
	0x4000020A,
	0x4000020B,
	0x4000020C,
	0x40000875,
	0x40000876,
	0x40000877,
	0x4000084F,
	0x40000850,
	0x40000851,
	0x40000852,
	0x40000853,
	0x40000854,
	0x40000855,
	0x40000856,
	0x40000857,
	0x40000859,
	0x4000085A,
	0x4000085B,
	0x4000085C,
	0x4000085D,
	0x4000085E,
	0x4000085F,
	0x40000860,
	0x40000861,
	0x40000862,
	0x40000863,
	0x40000864,
	0x40000865,
	0x40000866,
	0x40000867,
	0x40000868,
	0x40000181,
	0x40000183,
	0x40000184,
	0x40000179,
	0x4000017A,
	0x40000173,
	0x4000015F,
	0x40124F80,
	0x40127690,
	0x4013D620,
	0x4013DA08,
	0x4013DDF0,
	0x4013E1D8,
	0x4013E5C0,
	0x4013E9A8,
	0x4013ED90,
	0x4013F178,
	0x4013F560,
	0x4013F948,
	0x4013FD30,
	0x40140118,
	0x40140500,
	0x40144B50,
	0x40144F38,
	0x40147260,
	0x40147648,
	0x40149970,
	0x4014A528,
	0x4014A910,
	0x4014ACF8,
	0x4014B0E0,
	0x4014E790,
	0x4014EF60,
	0x4014F348,
	0x4014F730,
	0x4014FB18,
	0x401875B8,
	0x40189CC8,
	0x4018B820,
	0x40193138,
	0x401A8CE0,
	0x401B7740,
	0x401B7B28,
	0x401B7F10,
	0x401B82F8,
	0x401B8AC8,
	0x401B8EB0,
	0x401B9298,
	0x401B9680,
	0x401B9A68,
	0x401B9E50,
	0x401BA238,
	0x401BA620,
	0x401BAA08,
	0x401BADF0,
	0x401BB1D8,
	0x401BB5C0,
	0x401BB9A8,
	0x401BBD90,
	0x401BCD30,
	0x40249F00,
	0x4024A6D0,
	0x4024AAB8,
	0x4024B288,
	0x4024BA58,
	0x4024C9F8,
	0x4024ED20,
	0x4024F108,
	0x4024F4F0,
	0x40251430,
	0x40251818,
	0x402527B8,
	0x40252BA0,
	0x40253B40,
	0x40256250,
	0x40256638,
	0x40256A20,
	0x40256E08,
	0x402575D8,
	0x402579C0,
	0x4025B070,
	0x402717D0,
	0x4027D350,
	0x4027FA60,
	0x40282170,
	0x40284880,
	0x40286F90,
	0x402936C8,
	0x40293AB0,
	0x40294E38,
	0x40295220,
	0x40295608,
	0x402959F0,
	0x40295DD8,
	0x402961C0,
	0x402965A8,
	0x40296990,
	0x403540D0,
	0x403567E0,
	0x40356BC8,
	0x40356FB0,
	0x40357398,
	0x40357780,
	0x40357B68,
	0x40358338,
	0x40358720,
	0x4035B600,
	0x4035B9E8,
	0x4035DD10,
	0x4035E0F8,
	0x4035E4E0,
	0x40360420,
	0x40362B30,
	0x40362F18,
	0x40363300,
	0x403636E8,
	0x40363AD0,
	0x40363EB8,
	0x40365240,
	0x40365628,
	0x40365DF8,
	0x4036A448,
	0x4036C770,
	0x4036CB58,
	0x40378AC0,
	0x40387520,
	0x40389C30,
	0x4038C340,
	0x4038EA50,
	0x40395F80,
	0x40398690,
	0x4039ADA0,
	0x403A0390,
	0x403A0778,
	0x403A0F48,
	0x403A1330,
	0x403A1718,
	0x403A26B8,
	0x403A2AA0,
	0x40000097,
	0x400000BF,
	0x00000000
};

extern DWORD pPlusRings[200] = {
	0x20004E21,
	0x20004E22,
	0x20004E23,
	0x20004E2B,
	0x20004E2C,
	0x20004E2D,
	0x20004E35,
	0x20004E36,
	0x20004E37,
	0x20004E3F,
	0x20004E40,
	0x20004E41,
	0x20004E49,
	0x20004E4A,
	0x20004E4B,
	0x20004E53,
	0x20004E54,
	0x20004E55,
	0x20004E5D,
	0x20004E5E,
	0x20004E5F,
	0x20004E67,
	0x20004E68,
	0x20004E69,
	0x20004E71,
	0x20004E72,
	0x20004E73,
	0x20004E7B,
	0x20004E7C,
	0x20004E7D,
	0x20004EAD,
	0x20004EAE,
	0x20004EAF,
	0x20004EB7,
	0x20004EB8,
	0x20004EB9,
	0x20004EC1,
	0x20004EC2,
	0x20004EC3,
	0x20004EDF,
	0x20004EE0,
	0x20004EE1,
	0x20004F2F,
	0x20004F30,
	0x20004F31,
	0x20004F39,
	0x20004F3A,
	0x20004F3B,
	0x20004F4D,
	0x20004F4E,
	0x20004F4F,
	0x20004F93,
	0x20004F94,
	0x20004F95,
	0x20004FA7,
	0x20004FA8,
	0x20004FA9,
	0x20004FB1,
	0x20004FB2,
	0x20004FB3,
	0x20004FCF,
	0x20004FD0,
	0x20004FD1,
	0x20004FED,
	0x20004FEE,
	0x20004FEF,
	0x20004FF7,
	0x20004FF8,
	0x20004FF9,
	0x20005001,
	0x20005002,
	0x20005003,
	0x2000500B,
	0x2000500C,
	0x2000500D,
	0x20005015,
	0x20005016,
	0x20005017,
	0x2000501F,
	0x20005020,
	0x20005021,
	0x20005029,
	0x2000502A,
	0x2000502B,
	0x20005083,
	0x20005084,
	0x20005085,
	0x200050F1,
	0x200050F2,
	0x200050F3,
	0x2000515F,
	0x20005160,
	0x20005161,
	0x20007563,
	0x20007564,
	0x20007565,
	0x20007581,
	0x20007582,
	0x20007583,
	0x20007595,
	0x20007596,
	0x20007597,
	0x20007603,
	0x20007604,
	0x20007605,
	0x200076B7,
	0x200076B8,
	0x200076B9,
	0x200076C1,
	0x200076C2,
	0x200076C3,
	0x200076CB,
	0x200076CC,
	0x200076CD,
	0x200076D5,
	0x200076D6,
	0x200076D7,
	0x200076DF,
	0x200076E0,
	0x200076E1,
	0x200077C5,
	0x200077C6,
	0x200077C7,
	0x200077CF,
	0x200077D0,
	0x200077D1,
	0x200077D9,
	0x200077DA,
	0x200077DB,
	0x200077E3,
	0x200077E4,
	0x200077E5,
	0x200077ED,
	0x200077EE,
	0x200077EF
};

extern DWORD pConsumables[200] = {
	0x400000F0,
	0x400000F1,
	0x40000104,
	0x40000105,
	0x40000106,
	0x4000010E,
	0x4000010F,
	0x40000110,
	0x40000112,
	0x40000113,
	0x40000114,
	0x40000118,
	0x40000122,
	0x40000124,
	0x40000125,
	0x40000126,
	0x40000128,
	0x40000129,
	0x4000012B,
	0x4000012C,
	0x4000012E,
	0x4000012F,
	0x40000130,
	0x40000136,
	0x40000137,
	0x4000014A,
	0x4000014B,
	0x4000014E,
	0x4000014F,
	0x40000150,
	0x40000154,
	0x40000155,
	0x40000157,
	0x40000158,
	0x4000015E,
	0x4000016E,
	0x4000016F,
	0x40000170,
	0x40000171,
	0x40000172,
	0x40000174,
	0x40000175,
	0x40000176,
	0x40000177,
	0x40000898,
	0x40000899,
	0x40000BB8,
	0x40000BB9,
	0x40000BBC,
	0x40000BBD,
	0x40000BCA,
	0x40000BCB,
	0x40000BCC,
	0x40000BCD,
	0x40000BCE,
	0x40000BCF,
	0x40000BD0,
	0x40000FA0,
	0x40000FA1,
	0x40000FA2,
	0x40000FA3,
	0x40000FA4,
	0x40000FA5,
	0x40000FA6,
	0x40000FA7,
	0x40000FA8,
	0x40000FA9,
	0x40000FAA,
	0x40000FAB,
	0x40000FAC,
	0x40000FAD,
	0x40000FAE,
	0x40000FAF,
	0x40000FB0,
	0x40000FB1,
	0x40000FB2,
	0x40000FB3,
	0x40000870,
	0x40000871,
	0x40000872,
	0x40000873,
	0x40000874,
	0x400003E8,
	0x400003E9,
	0x400003EA,
	0x400003EB,
	0x400003F2,
	0x400003FC,
	0x40000406,
	0x40000424,
	0x400004E2,
	0x400002C8,
	0x400002C9,
	0x400002CA,
	0x400002CB,
	0x400002CD,
	0x400002CE,
	0x400002CF,
	0x400002D0,
	0x400002D1,
	0x400002D2,
	0x400002D3,
	0x400002D4,
	0x400002D5,
	0x400002D6,
	0x400002D7,
	0x400002D8,
	0x400002D9,
	0x400002DB,
	0x400002DC,
	0x400002DD,
	0x400002E3,
	0x400002E6,
	0x400002E7,
	0x400002EC,
	0x400001F4,
	0x400001C4,
	0x400001C5,
	0x400001C6,
	0x400001C7,
	0x400001C8,
	0x400001C9,
	0x400001CA,
	0x400001CB,
	0x400001CC,
	0x400001CD,
	0x400001CE,
	0x400001CF,
	0x40000186,
	0x40000190,
	0x40000191,
	0x40000192,
	0x40000193,
	0x40000194,
	0x40000195,
	0x40000196,
	0x40000197,
	0x40000198,
	0x40000199,
	0x4000019A,
	0x4000019B,
	0x4000019C,
	0x4000019D,
	0x4000019E,
	0x4000019F,
	0x400001A0,
	0x400001A1,
	0x400001A2,
	0x400001A3,
	0x400001A4,
	0x400001B8,
	0x4000017C,
	0x40000BBA,
	0x4000041A,
	0x4000042E,
	0x40000438,
	0x40000442
};

extern DWORD pUninfusableWeapons[200] = {
	0x00C72090,
	0x00C747A0,
	0x00C76EB0,
	0x00C795C0,
	0x00C8F550,
	0x00C99190,
	0x00C9B8A0,
	0x00CA7BF0,
	0x00CACA10,
	0x00D02140,
	0x00D04850,
	0x00D5C690,
	0x00D5EDA0,
	0x00D63BC0,
	0x00D689E0,
	0x00D74D30,
	0x00D83790,
	0x01DADA80,
	0x00C7E3E0,
	0x00C80AF0,
	0x00C83200,
	0x00C88020,
	0x00C8CE40,
	0x00C91C60,
	0x00C94370,
	0x00C96A80,
	0x00C9DFB0,
	0x00CA06C0,
	0x00CA2DD0,
	0x00CA54E0,
	0x00CAA300,
	0x00CAF120,
	0x00CC77C0,
	0x00CD3B10,
	0x00CD6220,
	0x00CDFE60,
	0x00CE2570,
	0x00CE4C80,
	0x00CE7390,
	0x00CE9AA0,
	0x00CEC1B0,
	0x00CEE8C0,
	0x00CF0FD0,
	0x00CF36E0,
	0x00CF5DF0,
	0x00CF8500,
	0x00CFAC10,
	0x00CFD320,
	0x00CFFA30,
	0x00D614B0,
	0x00D662D0,
	0x00D6B0F0,
	0x00D6FF10,
	0x00D72620,
	0x00D79B50,
	0x00D7C260,
	0x00D7E970,
	0x00D8ACC0,
	0x00D8D3D0,
	0x009B55A0,
	0x00CC9ED0,
	0x00CCC5E0,
	0x00D885B0,
	0x00D77440
};

extern DWORD pShields[200] = {
	0x01312D00,
	0x01315410,
	0x0131A230,
	0x0131C940,
	0x01323E70,
	0x01326580,
	0x0132DAB0,
	0x013301C0,
	0x013328D0,
	0x01339E00,
	0x0133C510,
	0x0133EC20,
	0x01341330,
	0x01343A40,
	0x01346150,
	0x01348860,
	0x0134AF70,
	0x0134D680,
	0x0134FD90,
	0x013524A0,
	0x01354BB0,
	0x01409650,
	0x01410B80,
	0x014180B0,
	0x01426B10,
	0x01429220,
	0x0142B930,
	0x0142E040,
	0x01435570,
	0x0143A390,
	0x0143F1B0,
	0x014418C0,
	0x01443FD0,
	0x01448DF0,
	0x0144DC10,
	0x014FD890,
	0x014FFFA0,
	0x0150EA00,
	0x01513820,
	0x0151AD50,
	0x0151D460,
	0x01522280,
	0x01544560,
	0x01546C70,
	0x013376F0,
	0x013572C0,
	0x013599D0,
	0x014159A0,
	0x0141F5E0,
	0x01421CF0,
	0x01424400,
	0x01430750,
	0x01432E60,
	0x01437C80,
	0x0143CAA0,
	0x014466E0,
	0x0144B500,
	0x01504DC0,
	0x015074D0,
	0x01509BE0,
	0x0150C2F0,
	0x01511110,
	0x01515F30,
	0x01518640,
	0x0151FB70,
	0x01524990,
	0x015270A0,
	0x015297B0,
	0x0152BEC0,
	0x0152E5D0,
	0x01530CE0,
	0x015333F0,
	0x01535B00,
	0x01538210,
	0x0153A920,
	0x0153D030,
	0x0153F740,
	0x01541E50,
	0x0135C0E0,
	0x01450320,
	0x01452A30
};