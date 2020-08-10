#ifdef _WIN32
#include <wtypes.h>
typedef unsigned __int64 QWORD;
#else
#include <stdint.h>
#define WINAPI
#define CALLBACK
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;
#ifndef __OBJC__
typedef int BOOL;
#endif
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#define LOBYTE(a) (BYTE)(a)
#define HIBYTE(a) (BYTE)((a)>>8)
#define LOWORD(a) (WORD)(a)
#define HIWORD(a) (WORD)((a)>>16)
#define MAKEWORD(a,b) (WORD)(((a)&0xff)|((b)<<8))
#define MAKELONG(a,b) (DWORD)(((a)&0xffff)|((b)<<16))
#endif

#define DETAILOUTPUT 0

typedef unsigned __int64 QWORD;
typedef DWORD HSAMPLE;		// sample handle
typedef DWORD HCHANNEL;		// playing sample's channel handle

const int LoadPoolSize=4096;
const int PlayPoolSize=512;
struct sharepool{
	struct LoadS{
		int head,tail;
		int pool[LoadPoolSize];
	}Load;
	struct PlayS{
		int head,tail;
		struct PlayP{
			double Time;
			HSAMPLE handle;
		}pool[PlayPoolSize];
	}Play;
};
