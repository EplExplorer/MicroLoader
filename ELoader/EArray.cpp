
#include "EArray.h"

void FreeAryElement(void* pAryData)//对于文本和字节集数组有效
{
	DWORD AryElementCount = 0;
	LPINT* pArryPtr = (LPINT*)GetAryElementInf(pAryData, AryElementCount);

	for (INT i = 0; i < (INT)AryElementCount; i++)
	{
		void* pElementData = *pArryPtr;
		if (pElementData)
		{
			krnl_MFree(pElementData);
			*pArryPtr = NULL;
		}
		pArryPtr++;
	}

	krnl_MFree(pAryData);
}

void* GetAryElementInf(void* pAryData, DWORD& AryElementCount)
{
	LPINT pnData;
	INT nArys;
	UINT nElementCount;

	pnData = (LPINT)pAryData;

	nArys = *pnData; //取得维数。
	pnData += 1;

	// 计算成员数目。
	nElementCount = 1;
	while (nArys > 0)
	{
		nElementCount *= pnData[0];
		pnData++;
		nArys--;
	}

	AryElementCount = nElementCount;
	return pnData;
}