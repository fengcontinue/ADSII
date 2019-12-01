#ifndef __USART_H
#define __USART_H

#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h"
#include "delay.h"
#include "timer.h"

#define UART4_RECV_LEN  			128
#define UART4_SEND_LEN				128
#define EN_UART4_RX 						1							//使能（1）/禁止（0）串口4接收
	  	
extern u8  UART4_RX_BUF[UART4_RECV_LEN]; 		//接收缓冲
extern u8  UART4_TX_BUF[UART4_SEND_LEN]; 		//发送缓冲
extern vu16 UART4_RX_STA;         						//接收状态标记	
void UART4_Init(u32 bound);

#define UART5_RECV_LEN  			800
#define UART5_SEND_LEN				800
#define EN_UART5_RX 						0							//使能（1）/禁止（0）串口5接收
	  	
extern u8  UART5_RX_BUF[UART5_RECV_LEN]; 			//接收缓冲，以0D 0A结束
extern u8  UART5_TX_BUF[UART5_SEND_LEN]; 			//发送缓冲
extern vu16 UART5_RX_STA;         						//接收状态标记	
void UART5_Init(u32 bound);

#define USART6_RECV_LEN  		  800
#define USART6_SEND_LEN				800
#define EN_USART6_RX 						1							//使能（1）/禁止（0）串口6接收
	  	
extern u8  USART6_RX_BUF[USART6_RECV_LEN]; 		//接收缓冲
extern u8  USART6_TX_BUF[USART6_SEND_LEN]; 		//发送缓冲
extern vu16 USART6_RX_STA;         						//接收状态标记	
void USART6_Init(u32 bound);

#define USART3_RECV_LEN  			512
#define USART3_SEND_LEN				512
#define EN_USART3_RX 						1							//使能（1）/禁止（0）串口3接收
	  	
extern u8  USART3_RX_BUF[USART3_RECV_LEN]; 		//接收缓冲
extern u8  USART3_TX_BUF[USART3_SEND_LEN]; 		//发送缓冲
extern vu16 USART3_RX_STA;         						//接收状态标记	
void USART3_Init(u32 bound);
void usart3_send(u8* buf, u16 len);

#define USART1_RECV_LEN  			128
#define USART1_SEND_LEN				128
#define EN_USART1_RX 						1							//使能（1）/禁止（0）串口1接收
	  	
extern u8  USART1_RX_BUF[USART1_RECV_LEN]; 		//接收缓冲
extern u8  USART1_TX_BUF[USART1_SEND_LEN]; 		//发送缓冲
extern vu16 USART1_RX_STA;         						//接收状态标记	
void USART1_Init(u32 bound);
void usart1_send(u8* buf, u16 len);

void USART2_Init(u32 bound);
void usart2_send(u8* buf, u16 len);

extern u8 RS485_RX_BUF[64]; 		//接收缓冲,最大64个字节
extern u8 RS485_RX_CNT;   			//接收到的数据长度

//模式控制
#define RS485_TX_EN		PGout(6)	//485模式控制.0,接收;1,发送.
//如果想串口中断接收，设置EN_USART2_RX为1，否则设置为0
#define EN_USART2_RX 	1			//0,不接收;1,接收.
														 
void RS485_Init(u32 bound);
void RS485_Send_Data(u8 *buf,u8 len);
void RS485_Receive_Data(u8 *buf,u8 *len);		 
#endif


