#include "Core.h"

extern CCore* Core;
extern CItemRandomiser *ItemRandomiser;
extern SCore* CoreStruct;
DWORD pProgressionItems[50];
DWORD pLimitedItems[250];
DWORD pSpecialWeapons[300];
DWORD pPlusRings[150];

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

		// Handle randomisation toggles
		if (!CoreStruct->dRandomiseKeyItems) { //User preference "RandomiseKeys"
			if (IsGameProgressionItem(dItemID)) return;
		};
		if (!CoreStruct->dDisallowPlusRings) { //User preference "DisallowPlusRings"
			if (IsPlusRing(dItemID)) return;
		};
		if (!CoreStruct->dRandomiseEstusShards) { //User preference "RandomiseEstusShards"
			if ((dItemID == ITEM_ESTUS_SHARD)) return;
		};
		if (!CoreStruct->dRandomiseBoneShards) { //User preference "RandomiseBoneShards"
			if ((dItemID == ITEM_UNDEAD_BONE_SHARD)) return;
		};

		if (CoreStruct->pItemArray[0] < dOffsetMax) {
			dItemID = CoreStruct->pItemArray[pOffsetArray[CoreStruct->pItemArray[0]]]; //Grab new item
			pOffsetArray[CoreStruct->pItemArray[0]] = 0;
		} 
		else {
			dItemID = CoreStruct->pItemArray[RandomiseNumber(1, dOffsetMax)]; //Default to random item list
		};

		CoreStruct->pItemArray[0]++;

		SortNewItem(&dItemID, &dItemQuantity);

		// Replace Estus Shard / Bone Shard if randomisation is allowed
		if ((dItemID == ITEM_ESTUS_SHARD) || (dItemID == ITEM_UNDEAD_BONE_SHARD)) {
			if (!CoreStruct->dRandomiseEstusShards) dItemID = ITEM_EMBER;
			if (!CoreStruct->dRandomiseBoneShards) dItemID = ITEM_EMBER;
		};

		// Replace key items if randomisation is allowed
		if (!CoreStruct->dRandomiseKeyItems) {
			if (IsGameProgressionItem(dItemID)) dItemID = ITEM_EMBER;
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
		*dItem = ITEM_EMBER;
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

		if (*dItem == 0x000A87500) return; //Dark Hand

		pPlayerPointer = *(UINT_PTR*)(CoreStruct->qLocalPlayer);
		pPlayerPointer = *(UINT_PTR*)(pPlayerPointer + 0x10);
		bPlayerUpgradeLevel = *(BYTE*)(pPlayerPointer + 0xB3);

		if (!bPlayerUpgradeLevel) return;

		if (IsWeaponSpecialType(*dItem)) {
			bPlayerUpgradeLevel >>= 1;
			*dItem += RandomiseNumber(0, bPlayerUpgradeLevel);
			return;
		};

		*dItem += RandomiseNumber(0, bPlayerUpgradeLevel);
		*dItem += (RandomiseNumber(0, 14) * 100);
	
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
		if (IsRestrictedGoods(*dItem)) return;
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

BOOL CItemRandomiser::IsWeaponSpecialType(DWORD dItemID) {

	int i = 0;

	while (pSpecialWeapons[i]) {
		if (dItemID == pSpecialWeapons[i]) return true;
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

extern DWORD pSpecialWeapons[300] = {
	0x000FB770,
	0x001053B0,
	0x00107AC0,
	0x00111700,
	0x00203230,
	0x0020A760,
	0x0020CE70,
	0x0020F580,
	0x002143A0,
	0x002206F0,
	0x0022A330,
	0x002E3BF0,
	0x002E6300,
	0x002E8A10,
	0x002EB120,
	0x003E4180,
	0x003E6890,
	0x003E8FA0,
	0x004C9960,
	0x004CC070,
	0x004CE780,
	0x004D0E90,
	0x005BB490,
	0x005C77E0,
	0x005D8950,
	0x005E2590,
	0x005E4CA0,
	0x005E9AC0,
	0x005F0FF0,
	0x005F3700,
	0x005F5E10,
	0x005F8520,
	0x005FAC30,
	0x005FD340,
	0x005FFA50,
	0x00602160,
	0x00604870,
	0x00606F80,
	0x00609690,
	0x0060BDA0,
	0x0060E4B0,
	0x00610BC0,
	0x006132D0,
	0x006B6C00,
	0x006BE130,
	0x006C0840,
	0x006C7D70,
	0x006CA480,
	0x006CCB90,
	0x007CAA10,
	0x007CD120,
	0x007CF830,
	0x007D4650,
	0x007D6D60,
	0x007D9470,
	0x007E09A0,
	0x007E30B0,
	0x007E57C0,
	0x007E7ED0,
	0x008B01F0,
	0x008B5010,
	0x008B7720,
	0x008BC540,
	0x008BEC50,
	0x008C1360,
	0x008C3A70,
	0x008CAFA0,
	0x009959D0,
	0x0099A7F0,
	0x009A1D20,
	0x009A4430,
	0x009A6B40,
	0x009AE070,
	0x009B0780,
	0x00A826E0,
	0x00A84DF0,
	0x00A87500,
	0x00B7B740,
	0x00B916D0,
	0x00B93DE0,
	0x00B964F0,
	0x00B98C00,
	0x00B9B310,
	0x00B9DA20,
	0x00BA0130,
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
	0x00F4C040,
	0x00F55C80,
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
	0x01C9EA90,
	0x01CA11A0,
	0x01CA38B0,
	0x01CA5FC0,
	0x01CA86D0,
	0x01CAADE0,
	0x01CAD4F0,
	0x01CB2310,
	0x01CB4A20,
	0x01CB7130,
	0x01CBBF50,
	0x01CBE660,
	0x01CC5B90,
	0x01CC82A0,
	0x01CCD0C0,
	0x01CD1EE0,
	0x01CDBB20,
	0x01CE5760,
	0x01CE7E70,
	0x01CEA580,
	0x01CECC90,
	0x01CF41C0,
	0x01CFB6F0,
	0x01CFDE00,
	0x01D02C20,
	0x01D07A40,
	0x01D0A150,
	0x01D0EF70,
	0x01D164A0,
	0x01D18BB0,
	0x01D1B2C0,
	0x01D200E0,
	0x01D227F0,
	0x01D24F00,
	0x01D27610,
	0x01D29D20,
	0x01D2C430,
	0x01D31250,
	0x01D33960,
	0x01D36070,
	0x01D38780,
	0x01D3AE90,
	0x01D3FCB0,
	0x01D423C0,
	0x01D44AD0,
	0x01D471E0,
	0x01D498F0,
	0x01D4C000,
	0x01D50E20,
	0x01D55C40,
	0x01D58350,
	0x01D5AA60,
	0x01D5D170,
	0x01D5F880,
	0x01D646A0,
	0x01D694C0,
	0x01D6BBD0,
	0x01D6E2E0,
	0x01D73100,
	0x01D75810,
	0x01D7F450,
	0x01D84270,
	0x01D86980,
	0x01D89090,
	0x01D8B7A0,
	0x01D92CD0,
	0x01D953E0,
	0x01D97AF0,
	0x01D9A200,
	0x01D9C910,
	0x01D9F020,
	0x01DA1730,
	0x01DA6550,
	0x01DA8C60,
	0x01DAB370,
	0x01DB0190,
	0x01DB28A0,
	0x01DB4FB0,
	0x01DB76C0,
	0x01DB9DD0,
	0x01DBC4E0,
	0x01DD99A0,
	0x01DDC0B0,
	0x01DDE7C0,
	0x01DE0ED0,
	0x01DE35E0,
	0x01DE5CF0,
	0x01DE8400,
	0x01DEAB10,
	0x01DED220,
	0x01DEF930,
	0x01DF2040,
	0x00000000
};

extern DWORD pPlusRings[150] = {
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