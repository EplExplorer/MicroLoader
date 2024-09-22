
#include "EContext.h"

EContext* AppContext;

void InitContext(PFN_NOTIFY_SYS NotifySys)
{
	AppContext = (EContext*)malloc(sizeof(EContext));
	AppContext->Heap = GetProcessHeap();
	AppContext->NotifySys = NotifySys;
    AppContext->IsErrorCallBack = false;
}

void FreeContext()
{

	// �ͷ�dll��������
	if (AppContext->DllCmdHead != NULL) {
		free(AppContext->DllCmdHead);
	}

    if (AppContext->LibInfoHead != NULL) {
        PLIBINFO LibInfo = AppContext->LibInfoHead;
        for (UINT32 i = 1; i <= AppContext->LibCount; i++) {
            if (LibInfo->LibHandle == NULL ||
                LibInfo->LibInfo == NULL) {
                continue;
            }

            // ֪֧ͨ�ֿ⽫��ж��
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