#pragma once

#include "krnln.h"

//λ���� - λȡ��
/*
    ���ø�ʽ�� �������͡� λȡ�� �������� ��ȡ������ֵ�� - ϵͳ����֧�ֿ�->λ����
    Ӣ�����ƣ�bnot
    ��ָ����ֵÿһ������λ��ֵȡ���󷵻ء�������Ϊ�м����
    ����<1>������Ϊ����ȡ������ֵ��������Ϊ�������ͣ�int������
*/
void _cdecl krnln_bnot(PMDATA_INF pRetData, INT nArgCount, PMDATA_INF pArgInf) {
    pRetData->m_int = ~(pArgInf->m_int);
    pRetData->m_dtDataType = SDT_INT;
    return;
}
