#pragma once

#include "EContext.h"
#include "ENotifySys.h"

// 易格式数据区段
#define ESECTIONNAME ".ecode"

typedef void(__stdcall* ECODESTART)(void);

typedef struct
{
	void* pConstSectionOffset;
	void* pWinFormSectionOffset;
	void* pHelpFuncSectionOffset;
	void* pCodeSectionOffset;
	void* pVarSectionOffset;
}
ESections;

// 编译后的易格式可执行数据头信息
typedef struct
{
#define    NEW_E_APP_MARK  0x454E5457  // 'WTNE'
	DWORD m_dwMark;  // 程序标记，应该为 NEW_E_APP_MARK 。
#define    E_MARK_TEXT  " / MADE BY E COMPILER - WUTAO "
	char m_chMark[32];  // 用作放置易语言的说明文本E_MARK_TEXT。

	INT m_nHeaderSize;  // 本头信息的尺寸，为 sizeof (APP_HEADER_INFO) + 附加数据尺寸

	INT m_nVersion;  // 程序版本，从1开始。
	INT m_nType;  // 程序类型，为 PT_DEBUG_RUN_VER 或 PT_RELEASE_RUN_VER 。
	DWORD m_dwState;  // 程序的状态标志。

	DWORD m_Reserved;  // 保留
	INT m_nDllCmdCount;  // 易程序中定义的DllCmd数目。

	// 程序启动入口点的机器代码偏移（相对于本头信息首）。
	INT m_nStartCodeOffset;

	///////////////////////////////////	段信息

	// 下面宏指定标准段的段名。
#define SN_CONST    _T("@const")
#define SN_FORM     _T("@form")
#define SN_HELPFUNC _T("@hlpfn")
#define SN_CODE     _T("@code")
#define SN_VAR      _T("@var")

// 记录用作快速定位的段信息位置，所有偏移位置均相对相对于本头信息首，
// 如果该段不存在，则为 -1 。
	INT m_nConstSectionOffset;  // 常量数据段位置的偏移量
	INT m_nWinFormSectionOffset;  // 窗口模板数据段位置的偏移量
	/* 接口数据段位置的偏移量，接口数据段用作易程序获取来自支持库的支持 */
	INT m_nHelpFuncSectionOffset;
	INT m_nCodeSectionOffset;  // 代码数据段位置的偏移量
	INT m_nVarSectionOffset;  // 未初始化全局变量数据段位置的偏移量

	// 记录所有段信息的链首。
	// 本成员提供首段的 SECTION_INFO 信息相对于本头信息首的偏移量，如无任何段，为-1。
	INT m_nBeginSectionOffset;

	// 1、INT m_nDllFileNameConstOffset [m_nDllCmdCount];   // 为在常量段中的偏移。
	// 2、INT m_nDllCmdNameConstOffset [m_nDllCmdCount];    // 为在常量段中的偏移。
	// 3、顺序存放程序中所有被使用支持库的支持库指定串，以空文本串结束。
}
APP_HEADER_INFO, * PAPP_HEADER_INFO;


// 用作记录段数据中的重定位信息项。
typedef struct
{
	// 重定位信息基址类别（宏值不可再改变）。
#define    RT_HELP_FUNC     0  // 相对于接口数据段段数据基址重定位
#define    RT_CONST         1  // 相对于常量数据段段数据基址重定位
#define    RT_GLOBAL_VAR    2  // 相对于全局变量数据段段数据基址重定位
#define    RT_CODE          3  // 相对于代码数据段段数据基址重定位
	unsigned m_btType : 3;
	// 指定相对某段段数据首的一个偏移INT的位置，该INT内的值在重定位时必须加上由m_btType说明的基址。
	unsigned m_dwOffset : 29;
}
RELOCATION_INF, * PRELOCATION_INF;


// 程序数据段信息。
typedef struct
{
	INT m_nSectionSize;  // 本段信息的尺寸，为 sizeof (SECTION_INFO) + 所有附加数据尺寸。

	// 记录下一数据段相对程序头信息首的偏移量，如本段为最后一段，此成员应为-1。
	INT m_nNextSectionOffset;

#define    SCN_READ          (1 << 0)  // 本段数据是可读的。
#define    SCN_WRITE         (1 << 1)  // 本段数据是可写的。
#define    SCN_EXECUTE       (1 << 2)  // 本段数据包含可执行代码。
#define    SCN_DISCARDABLE   (1 << 3)  // 本段数据在程序载入完毕后即可被抛弃。
#define    SCN_EXTEND        (1 << 4)  //   本段数据载入后尺寸将被扩充（扩充算法为简单
	// 地附加被初始化为0的数据空间）。
	DWORD m_dwState; // 记录本段的状态标志，见SCN宏。
#define    MAX_SECTION_NAME_LEN	20
	char m_szName[MAX_SECTION_NAME_LEN + 4];	// 段名。
	/* 段数据的载入后尺寸；
	   如段具有SCN_EXTEND标志，此成员记录段数据将被扩充到的尺寸。
	   否则等同于m_nRecordSize； */
	INT m_nLoadedSize;
	/* 段数据的记录尺寸（实际记录在文件中的数据尺寸），有可能为0。
	   如段具有SCN_EXTEND标志，此成员记录段数据被扩充前的尺寸。
	   否则等同于段数据的载入后尺寸。 */
	INT m_nRecordSize;
	// 段数据的偏移位置（相对于程序头信息首），如果没有记录段数据(即m_nRecordSize为0)，则为-1。
	INT m_nRecordOffset;

	//////////////////////////

	INT m_nReLocationItemCount;  // 段数据内所有需要重定位的偏移INT的数目。
	INT m_nExportSymbolCount;  // 本段输出的符号数目。

	/* 后面顺序为：
	// 记录具体的所有重定位项。
	RELOCATION_INF m_aryReLocationItem [m_nReLocationItemCount];
	// 所输出符号的对应数据基于段数据首的位置偏移。
	INT m_arySymbolDataOffset [m_nExportSymbolCount];
	// 顺序存放所有输出符号名称基于段数据首的位置偏移，与m_arySymbolDataOffset相对应。
	INT m_szarySymbolNameOffset [m_nExportSymbolCount];
	*/
}
SECTION_INFO, * PSECTION_INFO;