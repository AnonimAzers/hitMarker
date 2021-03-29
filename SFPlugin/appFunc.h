#pragma once
LPCSTR filename = "SAMPFUNCS\\HitMarker.ini";

struct scriptSettings {
	int timeDelay = 100;
	int markerSize = 32;
	int randRange = 10;
	bool status = false;
};

struct GiveAndTakeDMG {
	bool bGiveOrTake = false;
	UINT16 wPlayerID = 1001;
	UINT32 dWeaponID = 0, dBodypart = 0;
	float damage_amount = 0;
};
scriptSettings SS;

void getCrossHairPos(int &crosshairPosX, int &crosshairPosY)
{
	int iHeight, iWidth;
	SF->getGame()->getScreenResolution(&iWidth, &iHeight);
	float chOff1 = *(float*)0xB6EC10, chOff2 = *(float*)0xB6EC14;
	crosshairPosX = iWidth * chOff2; // Координаты прицела по оси Z
	crosshairPosY = iHeight * chOff1; // Координаты прицела по оси Y
}

double getDistanceBetweenCoords3d(float x1, float y1, float z1, float x2, float y2, float z2) {
	return hypot(hypot(x1 - x2, y1 - y2), z1 - z2);
}

bool FileIsExist(LPCSTR filename)
{
	bool isExist = false;
	std::ifstream iff(filename);

	if (iff.is_open())
		isExist = true;

	iff.close();
	return isExist;
}

void returnData(char *buf, LPCSTR key, bool isStatus, int Number) {
	if (isStatus) {
		GetPrivateProfileStringA("settings", key, "false", buf, 6, filename);
	}
	else {
		char second_buf[6];
		sprintf_s(second_buf, "%d", Number);
		GetPrivateProfileStringA("settings", key, second_buf, buf, 6, filename);
	}

}

void saveSettingsInt(LPCSTR section, LPCSTR keyName, int lpString) {
	char buf[5];
	sprintf_s(buf, "%d", lpString);
	WritePrivateProfileStringA(section, keyName, buf, filename);
}

void CALLBACK changeDelay(std::string param) {
	if (param.empty()) {
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), "[{ff6666}H{f2e9cd}Marker{ffffff}] Неверно указан параметр! /changedelay (millisec)");
		return;
	}
	int i = std::stoi(param);
	SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), "[{ff6666}H{f2e9cd}Marker{ffffff}] Cкорость хитмаркера изменена на: {ff6666}%d", i);
	SS.timeDelay = i;
	saveSettingsInt("settings", "Delay", i);
}

void CALLBACK changeSize(std::string param) {
	if (param.empty()) {
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), "[{ff6666}H{f2e9cd}Marker{ffffff}] Неверно указан параметр! /changesize (1 > inf)");
		return;
	}
	int i = std::stoi(param);
	if (i < 1) {
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), "[{ff6666}H{f2e9cd}Marker{ffffff}] Неверно указан параметр! /changesize (1 > inf)");
		return;
	}
	SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), "[{ff6666}H{f2e9cd}Marker{ffffff}] Размер хитмаркера изменен на: {ff6666}%d", i);
	SS.markerSize = i;
	saveSettingsInt("settings", "Size", i);

}

void CALLBACK changeRange(std::string param) {
	if (param.empty()) {
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), "[{ff6666}H{f2e9cd}Marker{ffffff}] Неверно указан параметр! /changerange (1 > inf)");
		return;
	}
	int i = std::stoi(param);
	if (i < 1) {
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), "[{ff6666}H{f2e9cd}Marker{ffffff}] Неверно указан параметр! /changerange (1 > inf)");
		return;
	}
	SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), "[{ff6666}H{f2e9cd}Marker{ffffff}] Разброс хитмаркера изменен на: {ff6666}%d", i);
	SS.randRange = i;
	saveSettingsInt("settings", "Range", i);
}

void CALLBACK changeStatus(std::string param) {
	char buf[6];
	if (SS.status) {
		SS.status = false;
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), "[{ff6666}H{f2e9cd}Marker{ffffff}] Плагин {ff0000}выключен{ffffff}!");
		sprintf_s(buf, "false");
	}
	else {
		SS.status = true;
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), "[{ff6666}H{f2e9cd}Marker{ffffff}] Плагин {00ff00}включен{ffffff}!");
		sprintf_s(buf, "true");
	}
	WritePrivateProfileStringA("settings", "Status", buf, filename);
}