#include <iostream>

#include "windows.h"
#include "lib2.h"

int main()
{
    HINSTANCE hDll = LoadLibrary(L"krnln.fne");
    if(!hDll){
        printf("dll加载失败!\n");
    }

    PFN_GET_LIB_INFO GetNewInf = (PFN_GET_LIB_INFO)GetProcAddress(hDll, FUNCNAME_GET_LIB_INFO);

    PLIB_INFO info = GetNewInf();

    if (info->m_pBeginCmdInfo == NULL || info->m_pCmdsFunc == NULL)
    {
        printf("此支持库没有导出函数!\n");
    }

    int offset = 0;
    PCMD_INFO CmdInfo = info->m_pBeginCmdInfo;
    PFN_EXECUTE_CMD* CmdPtr = info->m_pCmdsFunc;

    printf("函数名 是否实现 偏移\n");

    for (size_t i = 0; i < info->m_nCmdCount; i++)
    {

        printf("%s %d %#x\n", CmdInfo->m_szName, *CmdPtr != 0, offset);

        offset += 4;
        CmdInfo += 1;
        CmdPtr += 1;
    }
}