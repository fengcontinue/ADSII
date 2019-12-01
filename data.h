#ifndef __DATA_H
#define __DATA_H

#include "sys.h"
#include "gps.h"
#include "cord.h"

__packed typedef struct
{
	u8 header;
	u8 id;
	u8 data[1040];
	u8 check;
}msg_e1;										//声学数据，结构体大小为1043字节

__packed typedef struct
{
	u8 status[72];
	u8 heartbeat[72];
}msg_e2;										//平台状态信息，结构体大小为144字节

__packed typedef struct
{
	u8 cmd[30];
}msg_e3;										//平台控制指令，结构体大小为30字节

__packed typedef struct
{
	nmea_utc_time utc;						//UTC时间，格式hhmmss
	s32 longitude;				//经度，-180～180, 比例系数1E-7
	s32 latitude;					//纬度，-90～90, 比例系数1E-7
	s32 altitude;					//海拔，比例系数0.1
	u16 voltage;					//电压，比例系数0.01
	u16 pressure;				//压力，比例系数0.001
	u16 reserve;			
}msg_e4;										//通信浮标状态信息，结构体大小为24字节

__packed typedef struct
{
	s16 cal_azimuth;			//方位，-180～180,比例0.01
	s16 cal_pitch;				//俯仰，-90～90,比例0.01
	s16 course;						//航向，比例0.01
	s16 pitch;						//纵摇，比例0.01
	s16 roll;							//横滚，比例0.01
	u8 antenna;					//天线状态，0初始化；1寻星；2捕捉；3跟踪；4锁定；5失锁
	u8 AHRS;							//ARHS状态，0初始化；1平面标定；2多面标定；3可用；4异常
	u8 GPS;							//定位状态，0初始化；1已定位
	u8 ctrl;							//控制模式，0自动模式；0xAA手动模式
	u8 AGC_threshold;		//AGC门限，0～10，比例系数0.1
	u8 AGC_max;					//AGC最大值，0～10，比例系数0.1
	u8 AGC;							//AGC当前值，0～10，比例系数0.1
	u8 reserve[7];
}msg_e10;										//姿态信息，结构体大小为24字节

__packed typedef struct
{
	msg_e4 buoy_status;				//通信浮标状态信息
	msg_e10 attitude;					//姿态信息
	u8 reserve1[10];
	u32 cnt;							//帧节数
	u8 modulate_mode;		//调制模式，0x55   调制模式，0xAA   单载波模式
	u8 leak;							//漏水，0x00: 正常，0xFF: 漏水报警
	u8 power;						//天线开关，0x00:开，0xFF:关
	u8 reserve2[9];
}msg_e7;										//通信系统状态信息，结构体大小为74字节

__packed typedef struct
{
	u8 header;
	u8 id;
	msg_e2 platform_status;		//平台状态信息
	msg_e7 system_status;			//通信系统状态信息
	u8 check;
}msg_e5;										//ADS系统状态信息，结构体大小为218字节

__packed typedef struct
{
	u8 modulate_mode;		//调制模式，0x55   调制模式，0xAA   单载波模式
	u8 satellite;				//选星，0x00   天链，0x11   天通
	u8 AGC_threshold;		//AGC门限，门限值:0～10，比例系数0.1
	u8 calibration;			//姿态校准，0不校准，1为平面校准，2为平面校准结束，3为多面校准
	u8 power;						//天线开关，0x00:开，0xFF:关
	u8 attitude_mode;		//姿控模式，0自动（默认），0x55手动（校准天线阵）
	s16 azimuth;					//方位，校准相控阵使用，比例0.01
	s16 pitch;						//俯仰，校准相控阵使用，比例0.01
	u32 memory;					//存储控制
	u8 reserve[6];
}msg_e8;										//通信系统控制指令，结构体大小为20字节

__packed typedef struct
{
	u8 header;
	u8 id;
	msg_e3 platform_cmd;			//平台控制指令
	msg_e8 system_cmd;				//通信系统控制指令
	u8 check;
}msg_e6;										//ADS系统控制，结构体大小为50字节

__packed typedef struct
{
	u8 header;
	u8 id;
	s32 longitude;				//经度，-180～180, 比例系数1E-7
	s32 latitude;					//纬度，-90～90, 比例系数1E-7
	s32 altitude;					//海拔，比例系数0.1
	u8 satellite;				//选星，0天链，1天通
	u8 calibration;			//姿态校准，0不校准，1为平面校准，2为平面校准结束，3为多面校准
	u8 AGC;							//AGC电平，0～10，比例系数0.1
	u8 AGC_threshold;		//AGC门限，门限值:0～10，比例系数0.1
	u8 attitude_mode;		//姿控模式，0自动（默认），0x55手动（校准天线阵）
	s16 azimuth;					//方位，校准相控阵使用，比例0.01
	s16 pitch;						//俯仰，校准相控阵使用，比例0.01
	u8 reserve[3];
	u8 check;
}msg_e11;										//姿控单元控制指令，结构体大小为24字节

__packed typedef struct
{
	u8 header;
	u8 id;
	u32 cnt;							//帧节数
	u16 AGC;							//AGC电平，0～10，比例系数0.1
	u16 mode_toggle;		  //调制模式切换，55载波模式，AA单载波模式
	u16 reserve;
	u8 check;
}msg_e12;										//卫星链路状态


__packed typedef struct
{
	u8 header;
	u8 len;
	u8 addr;
	u8 cmd;
	u8 pitch[3];
	u8 roll[3];
	u8 reserve[3];
	u8 check;
}msg_e13;       //惯导数据

__packed typedef struct
{
	u8 header;
	u8 frameno;
	u8 fwdata[2];
	u8 gddata[2];
	u8 check;
}msg_e14; //伺服控制器数据

extern msg_e1 e1;
extern msg_e2 e2;
extern msg_e3 e3;
extern msg_e4 e4;
extern msg_e5 e5;
extern msg_e6 e6;
extern msg_e7 e7;
extern msg_e8 e8;
extern msg_e10 e10;
extern msg_e11 e11;
extern msg_e12 e12;
extern msg_e13 e13;
extern msg_e14 e14;

u8 check(u8* buf,u16 len);
double cal_angle ( uint8_t* data );
uint8_t bcd_decimal ( uint8_t bcd );
void JieSuanJieGuo(double ppitch, double proll, uint16_t AGC_threshold, uint16_t AGC_now, gpsdata mubiao_gps, gpsdata zhuantai_gps, double *fw_ZhuanTai, double *gd_ZhuanTai);
#endif
