#include "krnln.h"

//文本操作 - 指针到文本
/*
    调用格式： 〈文本型〉 指针到文本 （整数型 内存文本指针） - 系统核心支持库->文本操作
    英文名称：pstr
    返回指定内存指针所指向地址处的文本，注意调用本命令前一定要确保所提供的内存指针真实有效，且指向一个以零字符结束的文本串。本命令的最佳使用场合就是在易语言回调子程序和易语言DLL公开子程序用作获取外部数据。本命令为高级命令。
    参数<1>的名称为“内存文本指针”，类型为“整数型（int）”。本参数提供指向一个以零字符结束的文本串内存指针值。
*/
void _cdecl krnln_pstr(PMDATA_INF pRetData, INT nArgCount, PMDATA_INF pArgInf) {
    char * ThisStr = NULL, * NewStr = NULL;
    UINT32 ThisStrLen = 0;

    ThisStr = (char * ) pArgInf -> m_int;

    try {
        ThisStrLen = strlen(ThisStr);
    } catch (...) {
        // 出现错误
        krnl_MExitProcess(0);
        return;
    }

    if (ThisStrLen != 0) {
        NewStr = (char * ) krnl_MMalloc(ThisStrLen + 1);

        memcpy(NewStr, ThisStr, ThisStrLen + 1);
    } else {
        NewStr = NULL;
    }

    pRetData -> m_pText = NewStr;
    pRetData -> m_dtDataType = SDT_TEXT;
    return;
}

REGISTER_FUNCTION(0x90c, krnln_pstr)