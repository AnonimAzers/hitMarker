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

bool CALLBACK Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride,
	CONST RGNDATA *pDirtyRegion)
{
	if (SUCCEEDED(SF->getRender()->BeginRender())) // åñëè ðåíäåðåð ãîòîâ ê ðèñîâàíèþ
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
		SF->getRender()->EndRender(); // çàâåðøàåì ðèñîâàíèå
	};

	return true; // âîçâðàùàåì ïîëîæèòåëüíûé ðåçóëüòàò
};

bool CALLBACK outcomingData(stRakNetHookParams *params) // îïðåäåëåíèå callback-ôóíêöèè, êîòîðàÿ áóäåò âûçâàíà ïðè îòïðàâêå êàêîãî ëèáî ïàêåòà
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
	return true; // óñïåøíî çàâåðøàåì îòïðàâêó ïàêåòà
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
			
			SF->getRakNet()->registerRakNetCallback(RakNetScriptHookType::RAKHOOK_TYPE_OUTCOMING_RPC, outcomingData); // ðåãèñòðèðóåì RakNet callback
			SF->getRender()->registerD3DCallback(eDirect3DDeviceMethods::D3DMETHOD_PRESENT, Present); // ðåãèñòðèðóåì D3D hook

			// Ðåãèñòðàöèÿ âñåõ êîìàíä
			SF->getSAMP()->registerChatCommand("changedelay", changeDelay); 
			SF->getSAMP()->registerChatCommand("changesize", changeSize);
			SF->getSAMP()->registerChatCommand("changerange", changeRange);
			SF->getSAMP()->registerChatCommand("enablemarker", changeStatus);


			SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), "[{ff6666}H{f2e9cd}Marker{ffffff}] Óñïåøíî çàãðóæåí! Ðàçðàáîò÷èê: {cdd6f2}ANZR{ffffff} | ÂÊ: {cdd6f2}@pu1seanon");
			if (!FileIsExist(filename)) {
				SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), "[{ff6666}H{f2e9cd}Marker{ffffff}] Ôàéëû êîíôèãóðàöèè {b8fcff}íå íàéäåíû{ffffff}! Çàãðóæàåì ñòàíäàðòíûå íàñòðîéêè...");
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
