#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<windows.h>
#include<winuser.h>
#include<TlHelp32.h>
#include"fmod/inc/fmod.h"
#include"fmod/inc/fmod_errors.h"
#include<unordered_map>
#include"sharepool.h"


using namespace std;
FMOD_SYSTEM *fmodSystem;

struct sharepool *MyPool;

inline int readNUM() {
	int x=0,f=1;
	char ch=getchar();
	while(ch<'0'||ch>'9') {
		if(ch=='-')f=-1;
		ch=getchar();
	}
	while(ch>='0'&&ch<='9') {
		x=x*10+ch-'0';
		ch=getchar();
	}
	return x*f;
}
bool Work=0;
FMOD_SOUND *KeySound;
double CPUclock(){
	LARGE_INTEGER nFreq;
	LARGE_INTEGER t1;
	double dt;
 	QueryPerformanceFrequency(&nFreq);
 	QueryPerformanceCounter(&t1);
  	dt=(t1.QuadPart)/(double)nFreq.QuadPart;
  	return(dt*1000);
}
void mainloop(){
	static unordered_map<HSAMPLE,FMOD_SOUND*> sample_maping;
	while(Work){
		while(MyPool->Load.head!=MyPool->Load.tail){
			//printf("[Load] %d -> %d\n",MyPool->Load.head,MyPool->Load.tail);
			HSAMPLE hSample=MyPool->Load.pool[MyPool->Load.head];
			MyPool->Load.head=(MyPool->Load.head+1)%LoadPoolSize;
			char name[256];
			int i;
			for(i=0; MyPool->Load.pool[MyPool->Load.head]!=0; MyPool->Load.head=(MyPool->Load.head+1)%LoadPoolSize, i++)name[i]=MyPool->Load.pool[MyPool->Load.head];
			name[i]=0;MyPool->Load.head=(MyPool->Load.head+1)%LoadPoolSize;
			printf("Load Name: %s\nhSample: %u\n",name,hSample);
			FMOD_RESULT err;
			if (FMOD_OK == (err=FMOD_System_CreateSound(fmodSystem, name, FMOD_LOOP_OFF|FMOD_NONBLOCKING|FMOD_LOWMEM|FMOD_MPEGSEARCH|FMOD_CREATESAMPLE|FMOD_IGNORETAGS, 0, &KeySound))) {
				printf("[FMOD] Loaded Sample (%s)\n", name);
				sample_maping.insert(pair<HSAMPLE,FMOD_SOUND*>(hSample,KeySound));
			}else printf("[FMOD] %s\n",FMOD_ErrorString(err));
			FMOD_System_Update(fmodSystem);
		}
		while(MyPool->Play.head!=MyPool->Play.tail){
			if(DETAILOUTPUT)printf("[Play] %d -> %d\n",MyPool->Play.head,MyPool->Play.tail);
			double Time=MyPool->Play.pool[MyPool->Play.head].Time;
			HSAMPLE hSample=MyPool->Play.pool[MyPool->Play.head].handle;
			MyPool->Play.head=(MyPool->Play.head+1)%PlayPoolSize;
			auto iter=sample_maping.find(hSample);
			if(iter!=sample_maping.end()){
				FMOD_CHANNEL *Ch;
				FMOD_System_PlaySound(fmodSystem, iter->second, NULL, false, &Ch);
				if(DETAILOUTPUT){
					int x;
					FMOD_Channel_GetIndex(Ch,&x);
					printf("Index: %d\n",x);
					printf("Play\nLatency: %.4lfms\nhSample: %u\n",CPUclock()-Time,hSample);
				}
				FMOD_System_Update(fmodSystem);
			}
		}
		//Sleep(1);
	}
}
bool UpPrivilege(){   
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    bool result = OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken);   
    if(!result)return result;   
    result=LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tkp.Privileges[0].Luid);   
    if(!result)return result;   
    tkp.PrivilegeCount=1; 
    tkp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;   
    result=AdjustTokenPrivileges(hToken,FALSE,&tkp,sizeof(TOKEN_PRIVILEGES),(PTOKEN_PRIVILEGES)NULL,(PDWORD)NULL);   
    return result;   
}
HMODULE DllInject(HANDLE hProcess,const char *dllname){
	unsigned long  (__stdcall *faddr)(void*);
	size_t abc;
	HMODULE hdll;
	HANDLE ht;
	LPVOID paddr;
	unsigned long exitcode;
	int dllnamelen;
	hdll=GetModuleHandleA("kernel32.dll");
	if(hdll==0) return 0;
	faddr=(unsigned long (__stdcall *)(void*))GetProcAddress(hdll,"LoadLibraryA");
	if(faddr==0) return 0;
	dllnamelen=strlen(dllname)+1;
	paddr=VirtualAllocEx(hProcess,NULL,dllnamelen,MEM_COMMIT,PAGE_READWRITE);
	if(paddr==0) return 0;
	WriteProcessMemory(hProcess,paddr,(void*)dllname,strlen(dllname)+1,(SIZE_T*) &abc);
	ht=CreateRemoteThread(hProcess,NULL,0,faddr, paddr,0,NULL);
	if(ht==0){
		VirtualFreeEx(hProcess,paddr,dllnamelen,MEM_DECOMMIT);
		return 0;
	}
	WaitForSingleObject(ht,INFINITE);
	GetExitCodeThread(ht,&exitcode);
	CloseHandle(ht);
	VirtualFreeEx(hProcess,paddr,dllnamelen,MEM_DECOMMIT);
	return (HMODULE)exitcode;
}
DWORD getPID(LPCSTR ProcessName) {
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)return 0;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap,&pe32)) {
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return 0;
	}
	DWORD dwPid=0;
	do {
		if(!strcmp(ProcessName, pe32.szExeFile)) {
			dwPid=pe32.th32ProcessID;
			break;
		}
	} while(Process32Next(hProcessSnap,&pe32));
	CloseHandle(hProcessSnap);
	return dwPid;
}
int main(int argc, char* argv[]) {
	
	
	
	printf("FMOD Studio Low Level API (C) Firelight Technologies Pty Ltd.\n");
	printf("For custom key bindings, refer keycodes on site:\n");
	printf("    https://docs.microsoft.com/en-us/windows/desktop/inputdev/virtual-key-codes\n\n");

	FMOD_RESULT initRet = FMOD_System_Create(&fmodSystem);
	if (initRet != FMOD_OK) {
		printf("Create FMOD System Failed: %s\n", FMOD_ErrorString((FMOD_RESULT)initRet));
		system("pause");
		return 0;
	}
	HANDLE hMapFile;
	LPVOID lpBase;
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct sharepool), "ShareMemoryForOsuASIO4Play");
	lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if(hMapFile&&lpBase)puts("ShareMemory Success.");
	MyPool=(struct sharepool*)lpBase;
	memset(MyPool,0,sizeof(struct sharepool));
	printf("Pool Size: %dKB\n\n",sizeof(struct sharepool)/1024);
	
	if (argc >= 2) {
		int b = atoi(argv[1]);
		if (b == 0) b = 128;
		FMOD_System_SetDSPBufferSize(fmodSystem, b, 2);
	} else FMOD_System_SetDSPBufferSize(fmodSystem, 128, 2);

	FMOD_System_SetOutput(fmodSystem, FMOD_OUTPUTTYPE_ASIO);
	int driverId, driverNums;
	FMOD_System_GetNumDrivers(fmodSystem, &driverNums);
	char name[256];
	int systemRate;
	int speakerChannels;

	for (int i = 0; i < driverNums; i++) {
		FMOD_System_GetDriverInfo(fmodSystem, i, name, 255, 0, &systemRate, 0, &speakerChannels);
		printf("DeviceID: %-3d DeviceName: %s  Rate: %d  Channels: %d\n", i, name, systemRate, speakerChannels);
	}
	printf("Please select the DeviceID: ");
	driverId = readNUM();
	FMOD_System_SetDriver(fmodSystem, driverId);
	FMOD_System_GetDriver(fmodSystem, &driverId);
	FMOD_System_GetDriverInfo(fmodSystem, driverId, name, 255, 0, &systemRate, 0, &speakerChannels);
	unsigned bufLen;
	int bufNum;
	FMOD_System_GetDSPBufferSize(fmodSystem, &bufLen, &bufNum);

	initRet = FMOD_System_Init(fmodSystem, 32, FMOD_INIT_NORMAL, 0);
	if (initRet != FMOD_OK) {
		printf("FMOD System Initialize Failed: %s\n", FMOD_ErrorString((FMOD_RESULT)initRet));
		system("pause");
		return 0;
	}
	printf("FMOD System Initialize Finished.\n");
	printf("[FMOD] Device Name: %s\n", name);
	printf("[FMOD] Device Sample Rate: %d\n", systemRate);
	printf("[FMOD] Device Channels: %d\n", speakerChannels);
	printf("[FMOD] DSP buffer size: %d * %d\n", bufLen, bufNum);
	printf("[FMOD] Latency: %.10lfms\n", bufLen * bufNum * 1000.0 / systemRate);

//	if (FMOD_OK == FMOD_System_CreateSound(fmodSystem, "Key_Default.wav", FMOD_UNIQUE, 0, &KeySound)) {
//		printf("[FMOD] Loaded Sample (%s)\n", "Key_Default.wav");
//	}
//	FMOD_System_PlaySound(fmodSystem, KeySound, 0, false, 0);
	
	UpPrivilege();
	DWORD PID=0;
	while(!PID){
		PID=getPID("osu!.exe");
		Sleep(50);
	}
	Sleep(1000);
	/*
	PID=getPID("osu!.exe");
	if(!PID){
		puts("Please open osu first.");
		system("pause");
		return 0;
	}*/
	HANDLE hProcess;
	if(!(hProcess=OpenProcess(PROCESS_ALL_ACCESS,0,PID))){
		puts("Opening Failed");
		system("pause");
		return 0;
	}
	{
		char ch0[512];
		strcpy(ch0,_pgmptr);
		{
			char* p=ch0;
			while(strchr(p,'\\')) {
				p = strchr(p,'\\');
				p++;
			}
			*p = '\0';
		}
		strcat(ch0,"osu!asio_sound");
		DllInject(hProcess,ch0);
	}
	Work=1;
	HANDLE h1=CreateThread(0,0,(LPTHREAD_START_ROUTINE)mainloop,0,0,0);CloseHandle(h1);
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	Work=0;
	if (initRet == FMOD_OK && fmodSystem != nullptr) {
		FMOD_System_Release(fmodSystem);
		printf("FMOD System released.\n");
	}
	UnmapViewOfFile(lpBase);
	CloseHandle(hMapFile);
	return 0;
}
