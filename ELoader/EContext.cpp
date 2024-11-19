/*
 * 作者: 66hh futz12
 * 许可证: GPLV3
 */

#include "EContext.h"

// 引入日志库
#include "logger/easylogging++.h"

EContext* AppContext;

void InitContext(PFN_NOTIFY_SYS NotifySys)
{
	AppContext = (EContext*)malloc(sizeof(EContext));
	AppContext->Heap = GetProcessHeap();
	AppContext->NotifySys = NotifySys;
	AppContext->IsErrorCallBack = false;
	AppContext->ExitCallBack = 0;

#ifdef _DEBUG
	LOG(INFO) << "AppContext 初始化完毕";
	LOG(INFO) << "Heap: " << AppContext->Heap;
#endif

}

void FreeContext()
{

#ifdef _DEBUG
	LOG(INFO) << "退出程序并开始释放资源";
#endif

	// 释放dll引用数据
	if (AppContext->DllCmdHead != NULL) {
		free(AppContext->DllCmdHead);
	}

	if (AppContext->LibInfoHead != NULL) {
		PLIBINFO LibInfo = AppContext->LibInfoHead;
		for (UINT32 i = 1; i <= AppContext->LibCount; i++) {

#ifdef _DEBUG
			LOG(INFO) << "正在卸载" << LibInfo->LibName;
#endif

			if (LibInfo->LibHandle == NULL ||
				LibInfo->LibInfo == NULL) {
				LibInfo++;
				continue;
			}

			// 通知支持库将被卸载
			PFN_NOTIFY_LIB NotifyLib = LibInfo->LibInfo->m_pfnNotify;
			if (NotifyLib != NULL) {
				NotifyLib(NL_FREE_LIB_DATA, 0, 0);
			}

			FreeLibrary(LibInfo->LibHandle);
			LibInfo->LibHandle = NULL;
			LibInfo->LibInfo = NULL;

			LibInfo++;
		}
	}

}