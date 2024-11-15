/*
 * 作者: 66hh futz12
 * 许可证: GPLV3
 */

#include "krnln.h"

//位运算 - 位与
/*
    调用格式： 〈整数型〉 位与 （整数型 位运算数值一，整数型 位运算数值二，... ） - 系统核心支持库->位运算
    英文名称：band
    如两个数值有某共同比特位为1，则返回值的对应位也为1，否则为0。返回计算后的结果。本命令为中级命令。命令参数表中最后一个参数可以被重复添加。
    参数<1>的名称为“位运算数值一”，类型为“整数型（int）”。
    参数<2>的名称为“位运算数值二”，类型为“整数型（int）”。
*/
void _cdecl krnln_band(PMDATA_INF pRetData, INT nArgCount, PMDATA_INF pArgInf) {
    INT result = 0;
    result = pArgInf -> m_int;
    for (int i = 1; i <= (nArgCount - 1); i++) {
        result = result & (pArgInf + i) -> m_int;
    }
    pRetData -> m_int = result;
    pRetData -> m_dtDataType = SDT_INT;
    return;
}

REGISTER_FUNCTION(0xc4, krnln_band)