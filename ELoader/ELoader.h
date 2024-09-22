#pragma once

#include "EContext.h"
#include "ENotifySys.h"

// �׸�ʽ��������
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

// �������׸�ʽ��ִ������ͷ��Ϣ
typedef struct
{
#define    NEW_E_APP_MARK  0x454E5457  // 'WTNE'
	DWORD m_dwMark;  // �����ǣ�Ӧ��Ϊ NEW_E_APP_MARK ��
#define    E_MARK_TEXT  " / MADE BY E COMPILER - WUTAO "
	char m_chMark[32];  // �������������Ե�˵���ı�E_MARK_TEXT��

	INT m_nHeaderSize;  // ��ͷ��Ϣ�ĳߴ磬Ϊ sizeof (APP_HEADER_INFO) + �������ݳߴ�

	INT m_nVersion;  // ����汾����1��ʼ��
	INT m_nType;  // �������ͣ�Ϊ PT_DEBUG_RUN_VER �� PT_RELEASE_RUN_VER ��
	DWORD m_dwState;  // �����״̬��־��

	DWORD m_Reserved;  // ����
	INT m_nDllCmdCount;  // �׳����ж����DllCmd��Ŀ��

	// ����������ڵ�Ļ�������ƫ�ƣ�����ڱ�ͷ��Ϣ�ף���
	INT m_nStartCodeOffset;

	///////////////////////////////////	����Ϣ

	// �����ָ����׼�εĶ�����
#define SN_CONST    _T("@const")
#define SN_FORM     _T("@form")
#define SN_HELPFUNC _T("@hlpfn")
#define SN_CODE     _T("@code")
#define SN_VAR      _T("@var")

// ��¼�������ٶ�λ�Ķ���Ϣλ�ã�����ƫ��λ�þ��������ڱ�ͷ��Ϣ�ף�
// ����öβ����ڣ���Ϊ -1 ��
	INT m_nConstSectionOffset;  // �������ݶ�λ�õ�ƫ����
	INT m_nWinFormSectionOffset;  // ����ģ�����ݶ�λ�õ�ƫ����
	/* �ӿ����ݶ�λ�õ�ƫ�������ӿ����ݶ������׳����ȡ����֧�ֿ��֧�� */
	INT m_nHelpFuncSectionOffset;
	INT m_nCodeSectionOffset;  // �������ݶ�λ�õ�ƫ����
	INT m_nVarSectionOffset;  // δ��ʼ��ȫ�ֱ������ݶ�λ�õ�ƫ����

	// ��¼���ж���Ϣ�����ס�
	// ����Ա�ṩ�׶ε� SECTION_INFO ��Ϣ����ڱ�ͷ��Ϣ�׵�ƫ�����������κζΣ�Ϊ-1��
	INT m_nBeginSectionOffset;

	// 1��INT m_nDllFileNameConstOffset [m_nDllCmdCount];   // Ϊ�ڳ������е�ƫ�ơ�
	// 2��INT m_nDllCmdNameConstOffset [m_nDllCmdCount];    // Ϊ�ڳ������е�ƫ�ơ�
	// 3��˳���ų��������б�ʹ��֧�ֿ��֧�ֿ�ָ�������Կ��ı���������
}
APP_HEADER_INFO, * PAPP_HEADER_INFO;


// ������¼�������е��ض�λ��Ϣ�
typedef struct
{
	// �ض�λ��Ϣ��ַ��𣨺�ֵ�����ٸı䣩��
#define    RT_HELP_FUNC     0  // ����ڽӿ����ݶζ����ݻ�ַ�ض�λ
#define    RT_CONST         1  // ����ڳ������ݶζ����ݻ�ַ�ض�λ
#define    RT_GLOBAL_VAR    2  // �����ȫ�ֱ������ݶζ����ݻ�ַ�ض�λ
#define    RT_CODE          3  // ����ڴ������ݶζ����ݻ�ַ�ض�λ
	unsigned m_btType : 3;
	// ָ�����ĳ�ζ������׵�һ��ƫ��INT��λ�ã���INT�ڵ�ֵ���ض�λʱ���������m_btType˵���Ļ�ַ��
	unsigned m_dwOffset : 29;
}
RELOCATION_INF, * PRELOCATION_INF;


// �������ݶ���Ϣ��
typedef struct
{
	INT m_nSectionSize;  // ������Ϣ�ĳߴ磬Ϊ sizeof (SECTION_INFO) + ���и������ݳߴ硣

	// ��¼��һ���ݶ���Գ���ͷ��Ϣ�׵�ƫ�������籾��Ϊ���һ�Σ��˳�ԱӦΪ-1��
	INT m_nNextSectionOffset;

#define    SCN_READ          (1 << 0)  // ���������ǿɶ��ġ�
#define    SCN_WRITE         (1 << 1)  // ���������ǿ�д�ġ�
#define    SCN_EXECUTE       (1 << 2)  // �������ݰ�����ִ�д��롣
#define    SCN_DISCARDABLE   (1 << 3)  // ���������ڳ���������Ϻ󼴿ɱ�������
#define    SCN_EXTEND        (1 << 4)  //   �������������ߴ罫�����䣨�����㷨Ϊ��
	// �ظ��ӱ���ʼ��Ϊ0�����ݿռ䣩��
	DWORD m_dwState; // ��¼���ε�״̬��־����SCN�ꡣ
#define    MAX_SECTION_NAME_LEN	20
	char m_szName[MAX_SECTION_NAME_LEN + 4];	// ������
	/* �����ݵ������ߴ磻
	   ��ξ���SCN_EXTEND��־���˳�Ա��¼�����ݽ������䵽�ĳߴ硣
	   �����ͬ��m_nRecordSize�� */
	INT m_nLoadedSize;
	/* �����ݵļ�¼�ߴ磨ʵ�ʼ�¼���ļ��е����ݳߴ磩���п���Ϊ0��
	   ��ξ���SCN_EXTEND��־���˳�Ա��¼�����ݱ�����ǰ�ĳߴ硣
	   �����ͬ�ڶ����ݵ������ߴ硣 */
	INT m_nRecordSize;
	// �����ݵ�ƫ��λ�ã�����ڳ���ͷ��Ϣ�ף������û�м�¼������(��m_nRecordSizeΪ0)����Ϊ-1��
	INT m_nRecordOffset;

	//////////////////////////

	INT m_nReLocationItemCount;  // ��������������Ҫ�ض�λ��ƫ��INT����Ŀ��
	INT m_nExportSymbolCount;  // ��������ķ�����Ŀ��

	/* ����˳��Ϊ��
	// ��¼����������ض�λ�
	RELOCATION_INF m_aryReLocationItem [m_nReLocationItemCount];
	// ��������ŵĶ�Ӧ���ݻ��ڶ������׵�λ��ƫ�ơ�
	INT m_arySymbolDataOffset [m_nExportSymbolCount];
	// ˳������������������ƻ��ڶ������׵�λ��ƫ�ƣ���m_arySymbolDataOffset���Ӧ��
	INT m_szarySymbolNameOffset [m_nExportSymbolCount];
	*/
}
SECTION_INFO, * PSECTION_INFO;