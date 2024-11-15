/*
 * 作者: 66hh futz12
 * 许可证: GPLV3
 */

#include "krnln.h"

//位运算 - 位取反
/*
	调用格式： 〈整数型〉 位取反 （整数型 欲取反的数值） - 系统核心支持库->位运算
	英文名称：bnot
	将指定数值每一个比特位的值取反后返回。本命令为中级命令。
	参数<1>的名称为“欲取反的数值”，类型为“整数型（int）”。
*/
void _cdecl krnln_bnot(PMDATA_INF pRetData, INT nArgCount, PMDATA_INF pArgInf) {
	pRetData->m_int = ~(pArgInf[0].m_int);
	pRetData->m_dtDataType = SDT_INT;
	return;
}

REGISTER_FUNCTION(0xc0, krnln_bnot)