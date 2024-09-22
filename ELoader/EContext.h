#pragma once

#include <windows.h>

#include "ECommon.h"

// ����������
typedef struct {
    HANDLE Heap;
    PFN_NOTIFY_SYS NotifySys;

    // �����˳�֮ǰ������
    void *ExitCallBack;

    char EFileName[MAX_PATH];
    char EFilePath[MAX_PATH];

    // ����ص�
    void *ErrorCallBack;
    bool IsErrorCallBack;

    // ������Ϣ
    PDLLCMD DllCmdHead;
    DWORD DllCount;

    PLIBINFO LibInfoHead;
    DWORD LibCount;
}
EContext;

// ��ʼ��������
void InitContext(PFN_NOTIFY_SYS NotifySys);

// �ͷ�������
void FreeContext();
