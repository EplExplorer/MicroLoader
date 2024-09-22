#include "krnln.h"

//位运算 - 左移
/*
    调用格式： 〈整数型〉 左移 （整数型 欲移动的整数，整数型 欲移动的位数） - 系统核心支持库->位运算
    英文名称：shl
    将某整数的数据位左移指定位数，返回移动后的结果。本命令为中级命令。
    参数<1>的名称为“欲移动的整数”，类型为“整数型（int）”。
    参数<2>的名称为“欲移动的位数”，类型为“整数型（int）”。
*/
void _cdecl krnln_shl(PMDATA_INF pRetData, INT nArgCount, PMDATA_INF pArgInf) {
    pRetData -> m_int = pArgInf -> m_int << (pArgInf + 1) -> m_int;
    pRetData -> m_dtDataType = SDT_INT;
    return;
}

REGISTER_FUNCTION(0x9f8, krnln_shl)