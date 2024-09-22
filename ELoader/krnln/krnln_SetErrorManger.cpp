#include "krnln.h"

extern EContext* AppContext;

//其他 - 置错误提示管理
/*
	调用格式： 〈无返回值〉 置错误提示管理 （［子程序指针 用作进行错误提示的子程序］） - 系统核心支持库->其他
	英文名称：SetErrorManger
	本命令用来设置当运行时如果产生了导致程序崩溃的严重错误时用来对该错误进行详细提示的子程序地址，如果未进行此项设置，发生严重错误时运行时环境将自动提示该错误的详细信息并直接退出。本命令为高级命令。
	参数<1>的名称为“用作进行错误提示的子程序”，类型为“子程序指针（SubPtr）”，可以被省略。注意该子程序必须接收两个参数，第一个参数为整数型，用作接收错误代码，
	第二个参数为文本型，用作接收详细错误文本。同时该子程序必须返回一个逻辑值，返回真表示已经自行处理完毕，系统将不再显示该错误信息，返回假表示由系统来继续显示该错误信息。
	以上设置必须完全正确，否则结果不能预测，另外，还请注意以下几点：
	1、该错误提示子程序不要再引发新的错误；
	2、在进入该错误提示子程序后，系统将自动关闭事件消息通知处理机制，也就是说任何事件将无法得到响应；
	3、无论如何，当该子程序调用退出后，系统将自动将整个应用程序关闭。  如果省略本参数，系统将恢复错误提示的默认处理方式。
*/
void _cdecl krnln_SetErrorManger(PMDATA_INF pRetData, INT nArgCount, PMDATA_INF pArgInf) {
	if (pArgInf->m_dtDataType == SDT_SUB_PTR && pArgInf[0].m_pdwSubCodeAdr)
		AppContext->ErrorCallBack = (ERRORCALLBACK)pArgInf[0].m_pdwSubCodeAdr;
	else
		AppContext->ErrorCallBack = NULL;
}

REGISTER_FUNCTION(0xa08, krnln_SetErrorManger)