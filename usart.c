#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos 使用	  
#endif


//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
	int handle;
};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit ( int x )
{
	x = x;
}
//重定义fputc函数
int fputc ( int ch, FILE* f )
{
	while ( ( UART5->SR & 0X40 ) == 0 ); //循环发送,直到发送完毕

	UART5->DR = ( u8 ) ch;
	return ch;
}
#endif






/*************************************************************************
**	UART5（调试）
**************************************************************************/

u8 UART5_TX_BUF[UART5_SEND_LEN];
u8 UART5_RX_BUF[UART5_RECV_LEN];
#if EN_UART5_RX
vu16 UART5_RX_STA = 0;

//UART5中断服务函数
void UART5_IRQHandler ( void )
{
	u8 Res;
#if SYSTEM_SUPPORT_UCOS
	OSIntEnter();
#endif

	if ( USART_GetITStatus ( UART5, USART_IT_RXNE ) != RESET )
	{
		Res = USART_ReceiveData ( UART5 );

		if ( ( UART5_RX_STA & 0x8000 ) == 0 )
		{
			if ( UART5_RX_STA & 0x4000 )
			{
				if ( Res != 0x0a ) {
					UART5_RX_STA = 0;
				}
				else {
					UART5_RX_STA |= 0x8000;
				}
			}
			else
			{
				if ( Res == 0x0d ) {
					UART5_RX_STA |= 0x4000;
				}
				else
				{
					UART5_RX_BUF[UART5_RX_STA & 0X3FFF] = Res ;
					UART5_RX_STA++;

					if ( UART5_RX_STA > ( UART5_RECV_LEN - 1 ) ) {
						UART5_RX_STA = 0;
					}
				}
			}
		}
	}

#if SYSTEM_SUPPORT_UCOS
	OSIntExit();
#endif
}
#endif

//UART5初始化函数
void UART5_Init ( u32 bound )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE );
	RCC_APB1PeriphClockCmd ( RCC_APB1Periph_UART5, ENABLE );

	GPIO_PinAFConfig ( GPIOC, GPIO_PinSource12, GPIO_AF_UART5 );
	GPIO_PinAFConfig ( GPIOD, GPIO_PinSource2, GPIO_AF_UART5 );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init ( GPIOC, &GPIO_InitStructure );
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init ( GPIOD, &GPIO_InitStructure );

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init ( UART5, &USART_InitStructure );

	USART_Cmd ( UART5, ENABLE );

	USART_ClearFlag ( UART5, USART_FLAG_TC );

#if EN_UART5_RX
	USART_ITConfig ( UART5, USART_IT_RXNE, ENABLE );

	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init ( &NVIC_InitStructure );

#endif

}






/*************************************************************************
**	USART6（GPS），使用TIM7接收
**************************************************************************/

u8 USART6_TX_BUF[USART6_SEND_LEN];
#ifdef EN_USART6_RX
u8 USART6_RX_BUF[USART6_RECV_LEN];
vu16 USART6_RX_STA = 0;

//USART6中断服务函数
void USART6_IRQHandler ( void )
{
#if SYSTEM_SUPPORT_UCOS
	OSIntEnter();
#endif

	if ( USART_GetITStatus ( USART6, USART_IT_RXNE ) != RESET )
	{
		u8 res;
		res = USART_ReceiveData ( USART6 );

		if ( ( USART6_RX_STA & ( 1 << 15 ) ) == 0 )
		{
			if ( USART6_RX_STA < USART6_RECV_LEN )
			{
				TIM_SetCounter ( TIM7, 0 );

				if ( USART6_RX_STA == 0 ) {
					TIM_Cmd ( TIM7, ENABLE );
				}

				USART6_RX_BUF[USART6_RX_STA++] = res;
			}
			else
			{
				TIM_Cmd ( TIM7, DISABLE );
				USART6_RX_STA |= 1 << 15;
			}
		}
	}

#if SYSTEM_SUPPORT_UCOS
	OSIntExit();
#endif
}
#endif

//USART6初始化函数
void USART6_Init ( u32 bound )
{

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;


	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOC, ENABLE );
	RCC_APB2PeriphClockCmd ( RCC_APB2Periph_USART6, ENABLE );

	USART_DeInit ( USART6 );

	GPIO_PinAFConfig ( GPIOC, GPIO_PinSource6, GPIO_AF_USART6 );
	GPIO_PinAFConfig ( GPIOC, GPIO_PinSource7, GPIO_AF_USART6 );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init ( GPIOC, &GPIO_InitStructure );

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init ( USART6, &USART_InitStructure );

	USART_Cmd ( USART6, ENABLE );

	USART_ClearFlag ( USART6, USART_FLAG_TC );

#if EN_USART6_RX
	USART_ITConfig ( USART6, USART_IT_RXNE, ENABLE );

	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init ( &NVIC_InitStructure );

	TIM7_Int_Init ( 100 - 1, 8400 - 1 );
#endif
}




/*************************************************************************
**	USART3（FPGA），使用TIM4接收
**************************************************************************/

u8 USART3_TX_BUF[USART3_SEND_LEN];
#ifdef EN_USART3_RX
u8 USART3_RX_BUF[USART3_RECV_LEN];
vu16 USART3_RX_STA = 0;

//USART3中断服务函数
void USART3_IRQHandler ( void )
{
#if SYSTEM_SUPPORT_UCOS
	OSIntEnter();
#endif

	if ( USART_GetITStatus ( USART3, USART_IT_RXNE ) != RESET )
	{
		u8 res;
		res = USART_ReceiveData ( USART3 );

		if ( ( USART3_RX_STA & ( 1 << 15 ) ) == 0 )
		{
			if ( USART3_RX_STA < USART3_RECV_LEN )
			{
				TIM_SetCounter ( TIM4, 0 );

				if ( USART3_RX_STA == 0 ) {
					TIM_Cmd ( TIM4, ENABLE );
				}

				USART3_RX_BUF[USART3_RX_STA++] = res;
			}
			else
			{
				TIM_Cmd ( TIM4, DISABLE );
				USART3_RX_STA |= 1 << 15;
			}
		}
	}

#if SYSTEM_SUPPORT_UCOS
	OSIntExit();
#endif
}
#endif

//USART3初始化函数
void USART3_Init ( u32 bound )
{

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOD, ENABLE );
	RCC_APB1PeriphClockCmd ( RCC_APB1Periph_USART3, ENABLE );

	USART_DeInit ( USART3 );

	GPIO_PinAFConfig ( GPIOD, GPIO_PinSource8, GPIO_AF_USART3 );
	GPIO_PinAFConfig ( GPIOD, GPIO_PinSource9, GPIO_AF_USART3 );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init ( GPIOD, &GPIO_InitStructure );

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init ( USART3, &USART_InitStructure );

	USART_Cmd ( USART3, ENABLE );

	USART_ClearFlag ( USART3, USART_FLAG_TC );

#if EN_USART3_RX
	USART_ITConfig ( USART3, USART_IT_RXNE, ENABLE );

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init ( &NVIC_InitStructure );

	TIM4_Int_Init ( 100 - 1, 8400 - 1 );
#endif
}

//USART3发送
void usart3_send ( u8* buf, u16 len )
{
	u16 i;

	for ( i = 0; i < len; i++ )
	{
		while ( ( USART3->SR & 0X40 ) == 0 );

		USART3->DR = buf[i];
	}
}





/*************************************************************************
**	USART1（姿控板），使用TIM5接收
**************************************************************************/

u8 USART1_TX_BUF[USART1_SEND_LEN];
#ifdef EN_USART1_RX
u8 USART1_RX_BUF[USART1_RECV_LEN];
vu16 USART1_RX_STA = 0;

//USART1中断服务函数
void USART1_IRQHandler ( void )
{
#if SYSTEM_SUPPORT_UCOS
	OSIntEnter();
#endif

	if ( USART_GetITStatus ( USART1, USART_IT_RXNE ) != RESET )
	{
		u8 res;
		res = USART_ReceiveData ( USART1 );

		if ( ( USART1_RX_STA & ( 1 << 15 ) ) == 0 )
		{
			if ( USART1_RX_STA < USART1_RECV_LEN )
			{
				TIM_SetCounter ( TIM5, 0 );

				if ( USART1_RX_STA == 0 ) {
					TIM_Cmd ( TIM5, ENABLE );
				}

				USART1_RX_BUF[USART1_RX_STA++] = res;
			}
			else
			{
				TIM_Cmd ( TIM5, DISABLE );
				USART1_RX_STA |= 1 << 15;
			}
		}
	}

#if SYSTEM_SUPPORT_UCOS
	OSIntExit();
#endif
}
#endif

//USART1初始化函数
void USART1_Init ( u32 bound )
{

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;


	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOA, ENABLE );
	RCC_APB2PeriphClockCmd ( RCC_APB2Periph_USART1, ENABLE );

	USART_DeInit ( USART1 );

	GPIO_PinAFConfig ( GPIOA, GPIO_PinSource9, GPIO_AF_USART1 );
	GPIO_PinAFConfig ( GPIOA, GPIO_PinSource10, GPIO_AF_USART1 );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init ( GPIOA, &GPIO_InitStructure );

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init ( USART1, &USART_InitStructure );

	USART_Cmd ( USART1, ENABLE );

	USART_ClearFlag ( USART1, USART_FLAG_TC );

#if EN_USART1_RX
	USART_ITConfig ( USART1, USART_IT_RXNE, ENABLE );

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init ( &NVIC_InitStructure );

	TIM5_Int_Init ( 100 - 1, 8400 - 1 );
#endif
}

//USART1发送
void usart1_send ( u8* buf, u16 len )
{
	u16 i;

	for ( i = 0; i < len; i++ )
	{
		while ( ( USART1->SR & 0X40 ) == 0 );

		USART1->DR = buf[i];
	}
}



//USART2初始化函数，伺服控制器232
void USART2_Init ( u32 bound )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOG, ENABLE );
	RCC_APB1PeriphClockCmd ( RCC_APB1Periph_USART2, ENABLE );

	GPIO_PinAFConfig ( GPIOD, GPIO_PinSource5, GPIO_AF_USART2 );
	GPIO_PinAFConfig ( GPIOD, GPIO_PinSource6, GPIO_AF_USART2 );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init ( GPIOD, &GPIO_InitStructure );

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init ( USART2, &USART_InitStructure );

	USART_Cmd ( USART2, ENABLE );

	USART_ClearFlag ( USART2, USART_FLAG_TC );

#if EN_USART2_RX
	USART_ITConfig ( USART2, USART_IT_RXNE, ENABLE );

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init ( &NVIC_InitStructure );

#endif
}

//USART1发送
void usart2_send ( u8* buf, u16 len )
{
	u16 i;

	for ( i = 0; i < len; i++ )
	{
		while ( ( USART2->SR & 0X40 ) == 0 );

		USART2->DR = buf[i];
	}
}



#if 0 //本程序未使用
/*************************************************************************
**	USART2（压力传感器）
**************************************************************************/

#if EN_USART2_RX
u8 RS485_RX_BUF[64];
u8 RS485_RX_CNT = 0;

//USART2中断服务函数
void USART2_IRQHandler ( void )
{
#if SYSTEM_SUPPORT_UCOS
	OSIntEnter();
#endif
	u8 res;

	if ( USART_GetITStatus ( USART2, USART_IT_RXNE ) != RESET )
	{
		res = USART_ReceiveData ( USART2 );

		if ( RS485_RX_CNT < 64 )
		{
			RS485_RX_BUF[RS485_RX_CNT] = res;
			RS485_RX_CNT++;
		}
	}

#if SYSTEM_SUPPORT_UCOS
	OSIntExit();
#endif
}
#endif

//RS485（USART2）初始化函数
void RS485_Init ( u32 bound )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOG, ENABLE );
	RCC_APB1PeriphClockCmd ( RCC_APB1Periph_USART2, ENABLE );

	GPIO_PinAFConfig ( GPIOD, GPIO_PinSource5, GPIO_AF_USART2 );
	GPIO_PinAFConfig ( GPIOD, GPIO_PinSource6, GPIO_AF_USART2 );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init ( GPIOD, &GPIO_InitStructure );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init ( GPIOG, &GPIO_InitStructure );

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init ( USART2, &USART_InitStructure );

	USART_Cmd ( USART2, ENABLE );

	USART_ClearFlag ( USART2, USART_FLAG_TC );

#if EN_USART2_RX
	USART_ITConfig ( USART2, USART_IT_RXNE, ENABLE );

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init ( &NVIC_InitStructure );

#endif

	RS485_TX_EN = 0;
}

//RS485（USART2）发送
void RS485_Send_Data ( u8* buf, u8 len )
{
	u8 t;
	RS485_TX_EN = 1;

	for ( t = 0; t < len; t++ )
	{
		while ( USART_GetFlagStatus ( USART2, USART_FLAG_TC ) == RESET ) {};

		USART_SendData ( USART2, buf[t] );
	}

	while ( USART_GetFlagStatus ( USART2, USART_FLAG_TC ) == RESET ) {};

	RS485_RX_CNT = 0;

	RS485_TX_EN = 0;
}


//RS485（USART2）接收
void RS485_Receive_Data ( u8* buf, u8* len )
{
	u8 rxlen;
	u8 i = 0;
	*len = 0;
	delay_ms ( 100 );
	rxlen = RS485_RX_CNT;

	for ( i = 0; i < rxlen; i++ )
	{
		buf[i] = RS485_RX_BUF[i];
	}

	*len = rxlen;
	RS485_RX_CNT = 0;
}
#endif

/*************************************************************************
**	UART4（惯导）
**************************************************************************/

#ifdef EN_UART4_RX
u8 UART4_RX_BUF[UART4_RECV_LEN];
vu16 UART4_RX_STA = 0;
//UART4初始化函数,惯导数据
void UART4_Init ( u32 bound )
{

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;


	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOC, ENABLE );
	RCC_APB1PeriphClockCmd ( RCC_APB1Periph_UART4, ENABLE );

	USART_DeInit ( UART4 );

	GPIO_PinAFConfig ( GPIOC, GPIO_PinSource10, GPIO_AF_UART4 );
	GPIO_PinAFConfig ( GPIOC, GPIO_PinSource11, GPIO_AF_UART4 );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init ( GPIOC, &GPIO_InitStructure );

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init ( UART4, &USART_InitStructure );

	USART_Cmd ( UART4, ENABLE );

	USART_ClearFlag ( UART4, USART_FLAG_TC );

#if EN_USART1_RX
	USART_ITConfig ( UART4, USART_IT_RXNE, ENABLE );

	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init ( &NVIC_InitStructure );

#endif
}

//UART4中断服务函数
void UART4_IRQHandler ( void )
{
#if SYSTEM_SUPPORT_UCOS
	OSIntEnter();
#endif

	if ( USART_GetITStatus ( UART4, USART_IT_RXNE ) != RESET )
	{
		u8 res;
		res = USART_ReceiveData ( UART4 );

		if ( ( UART4_RX_STA & ( 1 << 15 ) ) == 0 )
		{
			if ( UART4_RX_STA < UART4_RECV_LEN )
			{
				
				UART4_RX_BUF[UART4_RX_STA++] = res;
			}
			else
			{	
				UART4_RX_STA |= 1 << 15;
			}
		}
	}

#if SYSTEM_SUPPORT_UCOS
	OSIntExit();
#endif
}
#endif


