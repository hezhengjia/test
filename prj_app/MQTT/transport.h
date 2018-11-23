#ifndef __TRANSPORT_H
#define __TRANSPORT_H
#include "stm32f4xx.h"


/************************************************************************
** ��������: transport_sendPacketBuffer									
** ��������: ��TCP��ʽ��������
** ��ڲ���: unsigned char* buf�����ݻ�����
**           int buflen�����ݳ���
** ���ڲ���: <0��������ʧ��							
************************************************************************/
s32 transport_sendPacketBuffer( u8* buf, s32 buflen);

/************************************************************************
** ��������: transport_getdata									
** ��������: �������ķ�ʽ����TCP����
** ��ڲ���: unsigned char* buf�����ݻ�����
**           int count�����ݳ���
** ���ڲ���: <=0��������ʧ��									
************************************************************************/
s32 transport_getdata(u8* buf, s32 count);

/************************************************************************
** ��������: transport_open									
** ��������: ��һ���ӿڣ����Һͷ����� ��������
** ��ڲ���: char* servip:����������
**           int   port:�˿ں�
** ���ڲ���: <0������ʧ��										
************************************************************************/
s32 transport_open(s8* servip, s32 port);

/************************************************************************
** ��������: transport_close									
** ��������: �ر��׽���
** ��ڲ���: unsigned char* buf�����ݻ�����
**           int buflen�����ݳ���
** ���ڲ���: <0��������ʧ��							
************************************************************************/
s32 transport_close(void);



#endif