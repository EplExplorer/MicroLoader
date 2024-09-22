#pragma once

#include <stdio.h>
#include <windows.h>

#include "ECommon.h"
#include "EContext.h"

// 核心服务数量
#define ESERVERCOUNT 13


typedef std::map<DWORD, PFN_EXECUTE_CMD> KernelCmd;

typedef void(__stdcall* UNKNOWFUN)(void);
typedef INT(__stdcall* ERRORCALLBACK)(int nCode, char* errText);

void _cdecl krnl_MFree(void* lpMem);

void* _cdecl krnl_MMalloc(DWORD dwSize);

void _cdecl krnl_MOtherHelp(DWORD lpCallBack);

void* _cdecl krnl_MRealloc(void* lpMem, DWORD dwSize);

void _cdecl krnl_MReportError(DWORD nMsg, DWORD dwMethodId, DWORD dwPos);

void _cdecl krnl_MReadProperty(void);

void _cdecl krnl_MWriteProperty(void);

void _cdecl krnl_MMessageLoop(UINT32 Param1);

void _cdecl krnl_MLoadBeginWin(void);

void _cdecl krnl_MExitProcess(DWORD uExitCode);

void _stdcall krnl_MCallDllCmd(void);

void InitServerPointTable(void);

void UpdataServerPointTable(void *pBase, DWORD nHelpOffset);

#define REGISTER_FUNCTION(offset,name) \
namespace { \
struct Register_##name { \
Register_##name() { \
KernelBaseCmd[offset] = name; \
} \
}; \
static Register_##name register_##name; \
} \

extern KernelCmd KernelBaseCmd;
