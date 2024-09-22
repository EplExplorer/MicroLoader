#include <stdio.h>
#include <math.h>

#include <oleauto.h>
#pragma comment(lib, "OleAut32.lib")

void DateTimeFormat(LPSTR strValue, DATE dt, BOOL bOnlyDatePart = FALSE);

//��ʽ��ʱ���ı�
void DateTimeFormat(LPSTR strValue,DATE dtDt,BOOL bOnlyDatePart)
{
	SYSTEMTIME st = {0};
	VariantTimeToSystemTime(dtDt, &st);
	
	char strFormat [128];
	strcpy(strFormat,"%d��%d��%d��");


	//��ʽ��ʱ�䲿��
	char strFormatTime [128];
	
	if(st.wSecond)
		strcpy(strFormatTime,"%dʱ%d��%d��");
	else if(st.wMinute && st.wSecond ==0)
		strcpy(strFormatTime,"%dʱ%d��");
	else if(st.wHour && st.wMinute==0 && st.wSecond ==0)
		strcpy(strFormatTime,"%dʱ");
	else
		strFormatTime[0]=0;

	if(bOnlyDatePart==FALSE)
		strcat(strFormat,strFormatTime);

	sprintf(strValue, strFormat, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

}