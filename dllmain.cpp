#include<cstdio>
#include<cstring>
#include<windows.h>
#include<algorithm>
#include"MinHook.h"
#include"sharepool.h"

using namespace std;

typedef HSAMPLE (WINAPI *fpBASS_SampleLoad)(BOOL mem, const void *file, QWORD offset, DWORD length, DWORD max, DWORD flags);
typedef HCHANNEL (WINAPI *fpBASS_SampleGetChannel)(HSAMPLE handle, BOOL onlynew);

HSAMPLE WINAPI MyBASS_SampleLoad(BOOL mem, const void *file, QWORD offset, DWORD length, DWORD max, DWORD flags);
HCHANNEL WINAPI MyBASS_SampleGetChannel(HSAMPLE handle, BOOL onlynew);

fpBASS_SampleLoad pOrigBASS_SampleLoad = NULL,pBASS_SampleLoad = NULL;
fpBASS_SampleGetChannel pOrigBASS_SampleGetChannel = NULL,pBASS_SampleGetChannel = NULL;

struct sharepool *MyPool;
HANDLE hMapFile;
LPVOID lpBase;

void readSettings() {
	
}

extern "C" __declspec(dllexport) void initDLL(){
	Sleep(1000);
	MH_Initialize();
	pBASS_SampleLoad = (fpBASS_SampleLoad) GetProcAddress(GetModuleHandle("bass.dll"), "BASS_SampleLoad");
	if(MH_CreateHook((LPVOID)pBASS_SampleLoad,(LPVOID)MyBASS_SampleLoad,(PVOID*)&pOrigBASS_SampleLoad)){
		MessageBoxA(NULL,"Hook Failed: BASS_SampleLoad()", "Info", MB_ICONEXCLAMATION);
		exit(0);
	}
	pBASS_SampleGetChannel = (fpBASS_SampleGetChannel) GetProcAddress(GetModuleHandle("bass.dll"), "BASS_SampleGetChannel");
	if(MH_CreateHook((LPVOID)pBASS_SampleGetChannel,(LPVOID)MyBASS_SampleGetChannel,(PVOID*)&pOrigBASS_SampleGetChannel)){
		MessageBoxA(NULL,"Hook Failed: BASS_SampleGetChannel()", "Info", MB_ICONEXCLAMATION);
		exit(0);
	}
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct sharepool), "ShareMemoryForOsuASIO4Play");
	lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if(!(hMapFile&&lpBase)){
		MessageBoxA(NULL,"ShareMemory Failed", "Info", MB_ICONEXCLAMATION);
		exit(0);
	}
	MyPool=(struct sharepool*)lpBase;
	
	MH_EnableHook(MH_ALL_HOOKS);
}
INT APIENTRY DllMain(HMODULE hDLL,DWORD Reason,LPVOID Reserved) {
	switch(Reason){
		case DLL_PROCESS_ATTACH:{
			HANDLE h1=CreateThread(0,0,(LPTHREAD_START_ROUTINE)initDLL,0,0,0);CloseHandle(h1);
			break;
		}
		case DLL_PROCESS_DETACH:
			MH_DisableHook(MH_ALL_HOOKS);
			MH_RemoveHook((LPVOID)pOrigBASS_SampleLoad);
			MH_RemoveHook((LPVOID)pOrigBASS_SampleGetChannel);
			MH_Uninitialize();
			UnmapViewOfFile(lpBase);
			CloseHandle(hMapFile);
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}
HSAMPLE WINAPI MyBASS_SampleLoad(BOOL mem, const void *file, QWORD offset, DWORD length, DWORD max, DWORD flags){
	if(mem)return pOrigBASS_SampleLoad(mem,file,offset,length,max,flags);
	char name[256];
	HSAMPLE hSample = pOrigBASS_SampleLoad(mem,file,offset,length,max,flags);
	if(hSample==0)return hSample;
	DWORD NameL=WideCharToMultiByte(0, 0, (LPCWSTR)file, (DWORD)-1, 0, 0, 0, 0);
	WideCharToMultiByte(0, 0, (LPCWSTR)file, (DWORD)-1, name, NameL, 0, 0);
	name[NameL]=0;
	
	int p=MyPool->Load.tail;
	while((p+1)%LoadPoolSize==MyPool->Load.head);
	MyPool->Load.pool[p++]=hSample;
	p%=LoadPoolSize;
	for(int i=0;name[i]!=0;i++){
		while((p+1)%LoadPoolSize==MyPool->Load.head);
		MyPool->Load.pool[p++]=name[i];
		p%=LoadPoolSize;
	}
	while((p+1)%LoadPoolSize==MyPool->Load.head);
	MyPool->Load.pool[p++]=0;
	p%=LoadPoolSize;
	MyPool->Load.tail=p;
	return hSample;
}
HCHANNEL WINAPI MyBASS_SampleGetChannel(HSAMPLE handle, BOOL onlynew){
	
	int p=MyPool->Play.tail;
	while((p+1)%PlayPoolSize==MyPool->Play.head);
	//MyPool->Play.pool[p].Time=GetTickCount();
	MyPool->Play.pool[p].handle=handle;
	p=(p+1)%PlayPoolSize;
	MyPool->Play.tail=p;
	return pOrigBASS_SampleGetChannel(handle,onlynew);
}