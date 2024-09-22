#pragma once

#include <windows.h>

#include "ECommon.h"

// 环境上下文
typedef struct
{
	HANDLE Heap;
	PFN_NOTIFY_SYS NotifySys;

	// 将在退出之前被调用
	void* ExitCallBack;

	char EFileName[MAX_PATH];
	char EFilePath[MAX_PATH];

	// 错误回调
	void* ErrorCallBack;
	bool IsErrorCallBack;

	// 依赖信息
	PDLLCMD DllCmdHead;
	DWORD DllCount;

	PLIBINFO LibInfoHead;
	DWORD LibCount;
}
EContext;

// 初始化上下文
void InitContext(PFN_NOTIFY_SYS NotifySys);

// 释放上下文
void FreeContext();