#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "lwip_comm.h"
#include "dp83848.h"
#include "malloc.h"
#include "lwipopts.h"
#include "main.h"


OS_STK IWDG_TASK_STK[IWDG_STK_SIZE];
OS_STK FWRECV_TASK_STK[FWRECV_STK_SIZE];
OS_STK GPS_TASK_STK[GPS_STK_SIZE];
OS_STK ATTITUDE_TASK_STK[ATTITUDE_STK_SIZE];
OS_STK PRESSURE_TASK_STK[PRESSURE_STK_SIZE];
OS_STK VOLTAGE_TASK_STK[VOLTAGE_STK_SIZE];
OS_STK UPDATE_TASK_STK[UPDATE_STK_SIZE];
OS_STK START_TASK_STK[START_STK_SIZE];
OS_STK IMU300_TASK_STK[IMU300_STK_SIZE];

OS_FLAG_GRP *flags;
FlagStatus flag_e1 = RESET;
FlagStatus flag_e3 = RESET;
FlagStatus flag_e4 = RESET;
FlagStatus flag_e5 = RESET;
FlagStatus flag_e7 = RESET;

//定义几个全局变量
u16 pressure;
u16 voltage;
u8 leak;
double gpitch, groll;
double FangWei_ZhuanTai = 0.0, FuYang_ZhuanTai = 0.0;
uint16_t agc_threshold = 150, agc_now = 0;
gpsdata mubiao_gps = {110.5, 0, 36000.0 + 6370.0}, zhuantai_gps = {104.06, 30.67, 6370};



int main(void)
{
	u8 key=0xFF;
	
	delay_init(168);       	//延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断分组配置
	UART5_Init(115200);//rsz_uart5是AP与STM32407的接口
	USART6_Init(38400);//rsz_uart6是Ublox与STM32407的接口
	USART3_Init(115200);//rsz_uart3是FPGA与STM32407的接口
	USART1_Init(9600);//rsz_uart1是STM32407的输出接口，输出的PC104(浮标端)与雷赛开发板(船基)
	UART4_Init(9600); //惯导数据接收
	USART2_Init(9600);
	SPI2_Init();
	Adc_Init();
	Leak_Init();
	
	mymem_init(SRAMIN);  	//初始化内部内存池
	mymem_init(SRAMCCM); 	//初始化CCM内存池
	//配置Ublox
	if(Ublox_Cfg_Rate(1000,1)!=0)	//设置定位信息更新速度为10bb00ms,顺便判断GPS模块是否在位. 
	{
//   	printf("NEO Setting...");
		while((Ublox_Cfg_Rate(1000,1)!=0)&&key)	//持续判断,直到可以检查到NEO-6M,且数据保存成功
		{
			USART6_Init(9600);				//初始化串口3波特率为9600(EEPROM没有保存数据的时候,波特率为9600.)
	  	Ublox_Cfg_Prt(38400);			//重新设置模块的波特率为38400
			USART6_Init(38400);				//初始化串口3波特率为38400
			Ublox_Cfg_Tp(1000000,100000,1);	//设置PPS为1秒钟输出1次,脉冲宽度为100ms	    
			key=Ublox_Cfg_Cfg_Save();		//保存配置  
		}	  					 
//	  printf("NEO Set Done!!");
		delay_ms(500);
	}
	OSInit(); 					//UCOS初始化
//	while(lwip_comm_init()) 	//lwip初始化
//	{
////		printf("Lwip Init failed!"); 	//lwip初始化失败
//		delay_ms(1000);
//	}
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);
	OSStart(); //开启UCOS
}



//看门狗任务函数
void iwdg_task(void *pdata)
{
	IWDG_Init(5,2500);//溢出时间为Tout=((4*2^prer)*rlr)/32 (ms).
	while(1)
	{
		IWDG_Feed();
		delay_ms(1000);
	}
}



//开始任务函数
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr;
	u8 err;
	pdata = pdata ;

	flags=OSFlagCreate(0,&err);
	
	OSStatInit();  			//初始化统计任务
	OS_ENTER_CRITICAL();  	//关中断
#if LWIP_DHCP
	lwip_comm_dhcp_creat(); //创建DHCP任务
#endif
//	tcp_client_init();
//	udp_client_init();
 	err = OSTaskCreate(iwdg_task,(void*)0,(OS_STK*)&IWDG_TASK_STK[IWDG_STK_SIZE-1],IWDG_TASK_PRIO);
	err = OSTaskCreate(gps_task,(void*)0,(OS_STK*)&GPS_TASK_STK[GPS_STK_SIZE-1],GPS_TASK_PRIO);
//	err = OSTaskCreate(fw_recv_task,(void*)0,(OS_STK*)&FWRECV_TASK_STK[FWRECV_STK_SIZE-1],FWRECV_TASK_PRIO);
	err = OSTaskCreate(attitude_task,(void*)0,(OS_STK*)&ATTITUDE_TASK_STK[ATTITUDE_STK_SIZE-1],ATTITUDE_TASK_PRIO);
//	err = OSTaskCreate(pressure_task,(void*)0,(OS_STK*)&PRESSURE_TASK_STK[PRESSURE_STK_SIZE-1],PRESSURE_TASK_PRIO);
	err = OSTaskCreate(voltage_task,(void*)0,(OS_STK*)&VOLTAGE_TASK_STK[VOLTAGE_STK_SIZE-1],VOLTAGE_TASK_PRIO);
	err = OSTaskCreate(update_task,(void*)0,(OS_STK*)&UPDATE_TASK_STK[UPDATE_STK_SIZE-1],UPDATE_TASK_PRIO);
	err = OSTaskCreate(IMU300_task,(void*)0,(OS_STK*)&IMU300_TASK_STK[IMU300_STK_SIZE-1],IMU300_TASK_PRIO);
	OSTaskSuspend(OS_PRIO_SELF); //挂起start_task任务
	OS_EXIT_CRITICAL();  		//开中断
}




//接收前向指令任务函数
void fw_recv_task(void *pdata)
{
	u8 err;
	u16 rxlen;
	while(1)
	{ 
		if(USART3_RX_STA&(1<<15))
		{
			rxlen=USART3_RX_STA&0X7FFF;																	//得到数据长度
			USART3_RX_STA=0;																						//启动下一次接收
			
			if(rxlen >= 53)																							//E6帧长53字节
			{
				if(USART3_RX_BUF[0] == 0xAA && USART3_RX_BUF[1] == 0xF7)	//收到E6
				{
					if(USART3_RX_BUF[sizeof(e6)-1] == check(USART3_RX_BUF,sizeof(e6)-1))
					{
						//通知TCP发送平台控制指令
						OSFlagPend(flags,FLAG_TCP,OS_FLAG_WAIT_CLR_ALL,0,&err);//等待TCP空闲
						memcpy((u8*)(&e6),USART3_RX_BUF,sizeof(e6));
						e3 = e6.platform_cmd;
						flag_e3 = SET;
						OSFlagPost(flags,FLAG_TCP,OS_FLAG_SET,&err);						//TCP标志置位
						
						e8 = e6.system_cmd;
						USART3_TX_BUF[0] = 0xAA;
						USART3_TX_BUF[1] = 0xF9;
						memcpy(USART3_TX_BUF+2,(u8*)&e8,sizeof(e8));
						USART3_TX_BUF[sizeof(e8)+2] = check(USART3_TX_BUF,sizeof(e8)+2);
						usart3_send(USART3_TX_BUF,sizeof(e8)+3);
						
						e11.satellite = e8.satellite;
						e11.calibration = e8.calibration;
						e11.AGC_threshold = e8.AGC_threshold;
						e11.attitude_mode = e8.attitude_mode;
						e11.azimuth = e8.azimuth;
						e11.pitch = e8.pitch;
						e11.check = check((u8*)&e11,sizeof(e11)-1);
						usart1_send((u8*)&e11,sizeof(e11));
					}
				}
			}
			else if(rxlen >= 13)																				//E12帧长13字节
			{
				if(USART3_RX_BUF[0] == 0xAA && USART3_RX_BUF[1] == 0xFD)	//收到E12
				{					
					memcpy((u8*)&e12,USART3_RX_BUF,sizeof(e12));
				}
			}
		}
		delay_ms(5);
	}
}



//GPS数据采集任务
void gps_task(void *pdata)
{	
	while(1)
	{
		if(USART6_RX_STA&(1<<15))																			//接收到一次数据了
		{
			GPS_Analysis(&gpsx,USART6_RX_BUF);
			memset(USART6_RX_BUF,0,USART6_RECV_LEN);
 			USART6_RX_STA=0;		   																			//启动下一次接收
		}
		delay_ms(100);
 	}
}



/* 
** 姿控状态接收任务
** 姿控板发送状态信息频率为100ms一次
** ARM收到姿控单元发送的状态信息（E10），先进行数据校验，再更新E7，E5，通过串口3和UDP发送。
*/
void attitude_task(void *pdata)
{
	while(1)
	{
		if(USART1_RX_STA&(1<<15))	//接收到一次数据了
		{
			u16 rxlen = USART1_RX_STA & 0x7FFF;																	//得到数据长度
			USART1_RX_STA = 0;	//清除接收状态标志，开始下一次接收
			if(rxlen >= 27)
			{
				if(USART1_RX_BUF[0] == 0xAA && USART1_RX_BUF[1] == 0xFB)
				{
					memcpy((u8*)&e10, USART1_RX_BUF + 2, sizeof(e10));
				}
			}
		}
		delay_ms(10);
 	}
}


/*
**惯导数据接收任务
**
*/
void IMU300_task(void *pdata)
{
//	double FangWei_ZhuanTai=0.0, FuYang_ZhuanTai=0.0;
//	uint8_t AGC_threshold=150, AGC_now=0;
//	gpsdata mubiao_gps={110.5, 0, 36000.0 + 6370.0}, zhuantai_gps={104.06, 30.67, 6370};
	
	
	while(1)
	{

		if ( UART4_RX_BUF[0] == e13.header && UART4_RX_BUF[1] == e13.len && UART4_RX_BUF[2] == e13.addr && UART4_RX_BUF[3] == e13.cmd )
		{
			memcpy ( ( u8* ) &e13, UART4_RX_BUF, sizeof ( e13 ) );
			gpitch = cal_angle ( e13.pitch );
			groll = cal_angle ( e13.roll );
			JieSuanJieGuo ( gpitch, groll, agc_threshold, agc_now, mubiao_gps, zhuantai_gps, &FangWei_ZhuanTai, &FuYang_ZhuanTai );

			e14.fwdata[0] = ( u8 ) ( ( u16 ) ( FangWei_ZhuanTai * 100 ) & 0xff );
			e14.fwdata[1] = ( u8 ) ( ( ( u16 ) ( FangWei_ZhuanTai * 100 ) & 0xff00 ) >> 8 );
			e14.gddata[0] = ( u8 ) ( ( u16 ) ( FuYang_ZhuanTai * 100 ) & 0xff );
			e14.gddata[1] = ( u8 ) ( ( ( u16 ) ( FuYang_ZhuanTai * 100 ) & 0xff00 ) >> 8 );
			e14.check = check ( ( u8* ) &e14, sizeof ( e14 ) - 1 );
			usart1_send ( ( u8* ) &e14, sizeof ( e14 ) );
		}


		if ( UART5_RX_BUF[0] == e15.header && UART5_RX_BUF[1] == e15.frameno )
		{
			memcpy ( ( u8* ) &e15, UART5_RX_BUF, sizeof ( e15 ) );
			agc_now = e15.AGC;
		}
		delay_ms(20);
 	}
}

/*
** 压力值采集任务
** 100ms一次
*/

#if 0 
void pressure_task(void *pdata)
{
	u8 sendbuf[9] = {'$','0','0','R','P','0','3','2',0x0D};
	u8 recvbuf[20];
	u8 rxlen;
	while(1)
	{
		RS485_Send_Data(sendbuf,9);	//发送9个字节 									   
		RS485_Receive_Data(recvbuf,&rxlen);	//有10ms延时
		if(rxlen >= 9)
		{
			if(recvbuf[0] == '*')
			{
				u16 tmp = 0;
				for(int i=4;i<9;i++)
				{
					if(recvbuf[i] != '.')
					{
						tmp *= 10;
						tmp += (recvbuf[i]-'0');
					}
				}
				pressure = tmp;
			}
		}
	}
}
#endif


/*
** 电压值采集任务
** 100ms一次
*/
void voltage_task(void *pdata)
{
	while(1)
	{
		u16 tmp = Get_Adc_Average(ADC_Channel_0,20);//获取通道0的转换值，20次取平均
		voltage = (float)tmp/4096*330;
		delay_ms(100);
	}
}


void update_task(void *pdata)
{
	u8 err;
	while(1)
	{
		u32 t = OSTimeGet();
		//更新给声学所的深度数据等
		OSFlagPend(flags,FLAG_TCP,OS_FLAG_WAIT_CLR_ALL,0,&err);		//等待TCP空闲
		e4.utc = gpsx.utc;
		if(gpsx.gpssta != 0)
		{
			e4.longitude = gpsx.longitude;
			e4.latitude = gpsx.latitude;
			e4.altitude = gpsx.altitude;
		}
		e4.voltage = voltage;
		e4.pressure = pressure;
		flag_e4 = SET;
		OSFlagPost(flags,FLAG_TCP,OS_FLAG_SET,&err);								//TCP标志置位
		
		//更新710和空间中心状态数据
		OSFlagPend(flags,FLAG_UDP,OS_FLAG_WAIT_CLR_ALL,0,&err);		//等待UDP空闲
		e7.buoy_status = e4;
		e7.attitude = e10;
		e7.cnt = e12.cnt;
		e7.modulate_mode = e12.mode_toggle;
		e7.leak = (LEAK ? 0 : 0xFF);
		e7.power = e8.power;
		flag_e7 = SET;

		e5.platform_status = e2;
		e5.system_status = e7;
		e5.check = check((u8*)&e5,sizeof(e5)-1);
		flag_e5 = SET;
		OSFlagPost(flags,FLAG_UDP,OS_FLAG_SET,&err);								//UDP标志置位
		
		//更新给FPGA的数据
		usart3_send((u8*)&e5,sizeof(e5));
		
		//更新给姿控的数据
		e11.longitude  = gpsx.longitude;
		e11.latitude = gpsx.latitude;
		e11.altitude = gpsx.altitude;
		e11.AGC = e12.AGC;
		e11.check = check((u8*)&e11,sizeof(e11)-1);
		usart1_send((u8*)&e11,sizeof(e11));
		
		delay_ms(1000 - (OSTimeGet() - t)*5);	//周期1s
	}
}




