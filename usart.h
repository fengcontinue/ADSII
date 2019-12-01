#ifndef __USART_H
#define __USART_H

#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h"
#include "delay.h"
#include "timer.h"

#define UART4_RECV_LEN  			128
#define UART4_SEND_LEN				128
#define EN_UART4_RX 						1							//ʹ�ܣ�1��/��ֹ��0������4����
	  	
extern u8  UART4_RX_BUF[UART4_RECV_LEN]; 		//���ջ���
extern u8  UART4_TX_BUF[UART4_SEND_LEN]; 		//���ͻ���
extern vu16 UART4_RX_STA;         						//����״̬���	
void UART4_Init(u32 bound);

#define UART5_RECV_LEN  			800
#define UART5_SEND_LEN				800
#define EN_UART5_RX 						0							//ʹ�ܣ�1��/��ֹ��0������5����
	  	
extern u8  UART5_RX_BUF[UART5_RECV_LEN]; 			//���ջ��壬��0D 0A����
extern u8  UART5_TX_BUF[UART5_SEND_LEN]; 			//���ͻ���
extern vu16 UART5_RX_STA;         						//����״̬���	
void UART5_Init(u32 bound);

#define USART6_RECV_LEN  		  800
#define USART6_SEND_LEN				800
#define EN_USART6_RX 						1							//ʹ�ܣ�1��/��ֹ��0������6����
	  	
extern u8  USART6_RX_BUF[USART6_RECV_LEN]; 		//���ջ���
extern u8  USART6_TX_BUF[USART6_SEND_LEN]; 		//���ͻ���
extern vu16 USART6_RX_STA;         						//����״̬���	
void USART6_Init(u32 bound);

#define USART3_RECV_LEN  			512
#define USART3_SEND_LEN				512
#define EN_USART3_RX 						1							//ʹ�ܣ�1��/��ֹ��0������3����
	  	
extern u8  USART3_RX_BUF[USART3_RECV_LEN]; 		//���ջ���
extern u8  USART3_TX_BUF[USART3_SEND_LEN]; 		//���ͻ���
extern vu16 USART3_RX_STA;         						//����״̬���	
void USART3_Init(u32 bound);
void usart3_send(u8* buf, u16 len);

#define USART1_RECV_LEN  			128
#define USART1_SEND_LEN				128
#define EN_USART1_RX 						1							//ʹ�ܣ�1��/��ֹ��0������1����
	  	
extern u8  USART1_RX_BUF[USART1_RECV_LEN]; 		//���ջ���
extern u8  USART1_TX_BUF[USART1_SEND_LEN]; 		//���ͻ���
extern vu16 USART1_RX_STA;         						//����״̬���	
void USART1_Init(u32 bound);
void usart1_send(u8* buf, u16 len);

void USART2_Init(u32 bound);
void usart2_send(u8* buf, u16 len);

extern u8 RS485_RX_BUF[64]; 		//���ջ���,���64���ֽ�
extern u8 RS485_RX_CNT;   			//���յ������ݳ���

//ģʽ����
#define RS485_TX_EN		PGout(6)	//485ģʽ����.0,����;1,����.
//����봮���жϽ��գ�����EN_USART2_RXΪ1����������Ϊ0
#define EN_USART2_RX 	1			//0,������;1,����.
														 
void RS485_Init(u32 bound);
void RS485_Send_Data(u8 *buf,u8 len);
void RS485_Receive_Data(u8 *buf,u8 *len);		 
#endif


