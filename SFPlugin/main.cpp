#include <Windows.h>
#include <ctime>
#include "main.h"
#include <thread>
#include <fstream>
#include "appFunc.h"

SYSTEMTIME st;
SAMPFUNCS *SF = new SAMPFUNCS();
stFontInfo *pFont;
int iWidth, iHeight;
stTextureInfo *whiteHitMarker, *redHitMarker;
bool isDamaged = false;
int startDamage;
int randomNumberX = 0;
int randomNumberY = 0;
GiveAndTakeDMG GATDMG;

bool CALLBACK Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride,
	CONST RGNDATA *pDirtyRegion)
{
	if (SUCCEEDED(SF->getRender()->BeginRender())) // если рендерер готов к рисованию
	{
		GetLocalTime(&st);
		srand(st.wMilliseconds);
		int crosshairPosX, crosshairPosY;
		getCrossHairPos(crosshairPosX, crosshairPosY);
		srand(time(0));
		if (isDamaged) {

			if (clock() < startDamage + SS.timeDelay) {
				if (GATDMG.dBodypart == 9) {
					SF->getRender()->DrawTexture(redHitMarker, crosshairPosX - randomNumberX - redHitMarker->imgInfo.Width / (SS.markerSize*2), crosshairPosY - randomNumberY - redHitMarker->imgInfo.Height / (SS.markerSize*2),
						redHitMarker->imgInfo.Width / SS.markerSize, redHitMarker->imgInfo.Height / SS.markerSize, 0, -1);
				}
				else {
					SF->getRender()->DrawTexture(whiteHitMarker, crosshairPosX - randomNumberX - whiteHitMarker->imgInfo.Width / (SS.markerSize*2), crosshairPosY - randomNumberY -whiteHitMarker->imgInfo.Height / (SS.markerSize*2),
						whiteHitMarker->imgInfo.Width / SS.markerSize, whiteHitMarker->imgInfo.Height / SS.markerSize, 0, -1);
				}
			}
			else {
				isDamaged = false;
			}
		}
		
		//SF->getRender()->DrawPolygon(crosshairPosX, crosshairPosY, 5, 5, 0, 50, D3DCOLOR_ARGB(255, 255, 255, 0));
		SF->getRender()->EndRender(); // завершаем рисование
	};

	return true; // возвращаем положительный результат
};

bool CALLBACK outcomingData(stRakNetHookParams *params) // определение callback-функции, которая будет вызвана при отправке какого либо пакета
{
	if (params->packetId == 115) {

		params->bitStream->ResetReadPointer();
		params->bitStream->Read(GATDMG.bGiveOrTake);
		if (!GATDMG.bGiveOrTake && SS.status) {
			params->bitStream->Read(GATDMG.wPlayerID);
			params->bitStream->Read(GATDMG.damage_amount);
			params->bitStream->Read(GATDMG.dWeaponID);
			params->bitStream->Read(GATDMG.dBodypart);
			isDamaged = true;
			startDamage = clock();
			randomNumberX = (rand() % SS.randRange * 2 + 1) - SS.randRange;
			randomNumberY = (rand() % SS.randRange * 2 + 1) - SS.randRange;
		}
		params->bitStream->ResetReadPointer();
		
	}
	return true; // успешно завершаем отправку пакета
}; 

void __stdcall mainloop()
{
	static bool initialized = false;
	if (!initialized)
	{
		if (GAME && GAME->GetSystemState() == eSystemState::GS_PLAYING_GAME && SF->getSAMP()->IsInitialized())
		{
			initialized = true;
			SF->getRender()->registerD3DCallback(eDirect3DDeviceMethods::D3DMETHOD_PRESENT, Present);
			whiteHitMarker = SF->getRender()->LoadTextureFromFile("SAMPFUNCS\\hitMarkers\\standartHitMarker.png");
			redHitMarker = SF->getRender()->LoadTextureFromFile("SAMPFUNCS\\hitMarkers\\headHitMarker.png");
			
			SF->getRakNet()->registerRakNetCallback(RakNetScriptHookType::RAKHOOK_TYPE_OUTCOMING_RPC, outcomingData); // регистрируем RakNet callback
			SF->getRender()->registerD3DCallback(eDirect3DDeviceMethods::D3DMETHOD_PRESENT, Present); // регистрируем D3D hook

			// Регистрация всех команд
			SF->getSAMP()->registerChatCommand("changedelay", changeDelay); 
			SF->getSAMP()->registerChatCommand("changesize", changeSize);
			SF->getSAMP()->registerChatCommand("changerange", changeRange);
			SF->getSAMP()->registerChatCommand("enablemarker", changeStatus);


			SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), "[{ff6666}H{f2e9cd}Marker{ffffff}] Успешно загружен! Разработчик: {cdd6f2}ANZR{ffffff} | ВК: {cdd6f2}@pu1seanon");
			if (!FileIsExist(filename)) {
				SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), "[{ff6666}H{f2e9cd}Marker{ffffff}] Файлы конфигурации {b8fcff}не найдены{ffffff}! Загружаем стандартные настройки...");
			}
			else {
				char buf[6];
				returnData(buf, "Status", true, NULL);
				if (_strcmpi(buf, "true") == 0) {
					SS.status = true;
				}
				else {
					SS.status = false;
				}

				returnData(buf, "Delay", false, SS.timeDelay);
				SS.timeDelay = atoi(buf);
				returnData(buf, "Size", false, SS.markerSize);
				SS.markerSize = atoi(buf);
				returnData(buf, "Range", false, SS.randRange);
				SS.randRange = atoi(buf);

			}
		}
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReasonForCall, LPVOID lpReserved)
{
	if (dwReasonForCall == DLL_PROCESS_ATTACH)
		SF->initPlugin(mainloop, hModule);
	return TRUE;
}
