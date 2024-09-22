
#include "EContext.h"

// ������־��
#include "logger/easylogging++.h"

EContext* AppContext;

void InitContext(PFN_NOTIFY_SYS NotifySys)
{
	AppContext = (EContext*)malloc(sizeof(EContext));
	AppContext->Heap = GetProcessHeap();
	AppContext->NotifySys = NotifySys;
    AppContext->IsErrorCallBack = false;

#ifdef _DEBUG
    LOG(INFO) << "AppContext ��ʼ�����";
    LOG(INFO) << "Heap: " << AppContext->Heap;
#endif

}

void FreeContext()
{

#ifdef _DEBUG
    LOG(INFO) << "�˳����򲢿�ʼ�ͷ���Դ";
#endif

	// �ͷ�dll��������
	if (AppContext->DllCmdHead != NULL) {
		free(AppContext->DllCmdHead);
	}

    if (AppContext->LibInfoHead != NULL) {
        PLIBINFO LibInfo = AppContext->LibInfoHead;
        for (UINT32 i = 1; i <= AppContext->LibCount; i++) {

#ifdef _DEBUG
            LOG(INFO) << "����ж��" << LibInfo->LibName;
#endif

            if (LibInfo->LibHandle == NULL ||
                LibInfo->LibInfo == NULL) {
                LibInfo++;
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