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

//���弸��ȫ�ֱ���
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
	
	delay_init(168);       	//��ʱ��ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//�жϷ�������
	UART5_Init(115200);//rsz_uart5��AP��STM32407�Ľӿ�
	USART6_Init(38400);//rsz_uart6��Ublox��STM32407�Ľӿ�
	USART3_Init(115200);//rsz_uart3��FPGA��STM32407�Ľӿ�
	USART1_Init(9600);//rsz_uart1��STM32407������ӿڣ������PC104(�����)������������(����)
	UART4_Init(9600); //�ߵ����ݽ���
	USART2_Init(9600);
	SPI2_Init();
	Adc_Init();
	Leak_Init();
	
	mymem_init(SRAMIN);  	//��ʼ���ڲ��ڴ��
	mymem_init(SRAMCCM); 	//��ʼ��CCM�ڴ��
	//����Ublox
	if(Ublox_Cfg_Rate(1000,1)!=0)	//���ö�λ��Ϣ�����ٶ�Ϊ10bb00ms,˳���ж�GPSģ���Ƿ���λ. 
	{
//   	printf("NEO Setting...");
		while((Ublox_Cfg_Rate(1000,1)!=0)&&key)	//�����ж�,ֱ�����Լ�鵽NEO-6M,�����ݱ���ɹ�
		{
			USART6_Init(9600);				//��ʼ������3������Ϊ9600(EEPROMû�б������ݵ�ʱ��,������Ϊ9600.)
	  	Ublox_Cfg_Prt(38400);			//��������ģ��Ĳ�����Ϊ38400
			USART6_Init(38400);				//��ʼ������3������Ϊ38400
			Ublox_Cfg_Tp(1000000,100000,1);	//����PPSΪ1�������1��,������Ϊ100ms	    
			key=Ublox_Cfg_Cfg_Save();		//��������  
		}	  					 
//	  printf("NEO Set Done!!");
		delay_ms(500);
	}
	OSInit(); 					//UCOS��ʼ��
//	while(lwip_comm_init()) 	//lwip��ʼ��
//	{
////		printf("Lwip Init failed!"); 	//lwip��ʼ��ʧ��
//		delay_ms(1000);
//	}
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);
	OSStart(); //����UCOS
}



//���Ź�������
void iwdg_task(void *pdata)
{
	IWDG_Init(5,2500);//���ʱ��ΪTout=((4*2^prer)*rlr)/32 (ms).
	while(1)
	{
		IWDG_Feed();
		delay_ms(1000);
	}
}



//��ʼ������
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr;
	u8 err;
	pdata = pdata ;

	flags=OSFlagCreate(0,&err);
	
	OSStatInit();  			//��ʼ��ͳ������
	OS_ENTER_CRITICAL();  	//���ж�
#if LWIP_DHCP
	lwip_comm_dhcp_creat(); //����DHCP����
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
	OSTaskSuspend(OS_PRIO_SELF); //����start_task����
	OS_EXIT_CRITICAL();  		//���ж�
}




//����ǰ��ָ��������
void fw_recv_task(void *pdata)
{
	u8 err;
	u16 rxlen;
	while(1)
	{ 
		if(USART3_RX_STA&(1<<15))
		{
			rxlen=USART3_RX_STA&0X7FFF;																	//�õ����ݳ���
			USART3_RX_STA=0;																						//������һ�ν���
			
			if(rxlen >= 53)																							//E6֡��53�ֽ�
			{
				if(USART3_RX_BUF[0] == 0xAA && USART3_RX_BUF[1] == 0xF7)	//�յ�E6
				{
					if(USART3_RX_BUF[sizeof(e6)-1] == check(USART3_RX_BUF,sizeof(e6)-1))
					{
						//֪ͨTCP����ƽ̨����ָ��
						OSFlagPend(flags,FLAG_TCP,OS_FLAG_WAIT_CLR_ALL,0,&err);//�ȴ�TCP����
						memcpy((u8*)(&e6),USART3_RX_BUF,sizeof(e6));
						e3 = e6.platform_cmd;
						flag_e3 = SET;
						OSFlagPost(flags,FLAG_TCP,OS_FLAG_SET,&err);						//TCP��־��λ
						
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
			else if(rxlen >= 13)																				//E12֡��13�ֽ�
			{
				if(USART3_RX_BUF[0] == 0xAA && USART3_RX_BUF[1] == 0xFD)	//�յ�E12
				{					
					memcpy((u8*)&e12,USART3_RX_BUF,sizeof(e12));
				}
			}
		}
		delay_ms(5);
	}
}



//GPS���ݲɼ�����
void gps_task(void *pdata)
{	
	while(1)
	{
		if(USART6_RX_STA&(1<<15))																			//���յ�һ��������
		{
			GPS_Analysis(&gpsx,USART6_RX_BUF);
			memset(USART6_RX_BUF,0,USART6_RECV_LEN);
 			USART6_RX_STA=0;		   																			//������һ�ν���
		}
		delay_ms(100);
 	}
}



/* 
** �˿�״̬��������
** �˿ذ巢��״̬��ϢƵ��Ϊ100msһ��
** ARM�յ��˿ص�Ԫ���͵�״̬��Ϣ��E10�����Ƚ�������У�飬�ٸ���E7��E5��ͨ������3��UDP���͡�
*/
void attitude_task(void *pdata)
{
	while(1)
	{
		if(USART1_RX_STA&(1<<15))	//���յ�һ��������
		{
			u16 rxlen = USART1_RX_STA & 0x7FFF;																	//�õ����ݳ���
			USART1_RX_STA = 0;	//�������״̬��־����ʼ��һ�ν���
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
**�ߵ����ݽ�������
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
** ѹ��ֵ�ɼ�����
** 100msһ��
*/

#if 0 
void pressure_task(void *pdata)
{
	u8 sendbuf[9] = {'$','0','0','R','P','0','3','2',0x0D};
	u8 recvbuf[20];
	u8 rxlen;
	while(1)
	{
		RS485_Send_Data(sendbuf,9);	//����9���ֽ� 									   
		RS485_Receive_Data(recvbuf,&rxlen);	//��10ms��ʱ
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
** ��ѹֵ�ɼ�����
** 100msһ��
*/
void voltage_task(void *pdata)
{
	while(1)
	{
		u16 tmp = Get_Adc_Average(ADC_Channel_0,20);//��ȡͨ��0��ת��ֵ��20��ȡƽ��
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
		//���¸���ѧ����������ݵ�
		OSFlagPend(flags,FLAG_TCP,OS_FLAG_WAIT_CLR_ALL,0,&err);		//�ȴ�TCP����
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
		OSFlagPost(flags,FLAG_TCP,OS_FLAG_SET,&err);								//TCP��־��λ
		
		//����710�Ϳռ�����״̬����
		OSFlagPend(flags,FLAG_UDP,OS_FLAG_WAIT_CLR_ALL,0,&err);		//�ȴ�UDP����
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
		OSFlagPost(flags,FLAG_UDP,OS_FLAG_SET,&err);								//UDP��־��λ
		
		//���¸�FPGA������
		usart3_send((u8*)&e5,sizeof(e5));
		
		//���¸��˿ص�����
		e11.longitude  = gpsx.longitude;
		e11.latitude = gpsx.latitude;
		e11.altitude = gpsx.altitude;
		e11.AGC = e12.AGC;
		e11.check = check((u8*)&e11,sizeof(e11)-1);
		usart1_send((u8*)&e11,sizeof(e11));
		
		delay_ms(1000 - (OSTimeGet() - t)*5);	//����1s
	}
}




