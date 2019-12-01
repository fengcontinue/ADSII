#ifndef __MAIN_H
#define __MAIN_H

#include "includes.h"
#include "data.h"
#include "tcp_client.h"
#include "udp_client.h"	 	 
#include "gps.h"
#include "spi.h"
#include "iwdg.h"
#include "adc.h"
#include "gpio.h"


//看门狗任务
#define IWDG_TASK_PRIO		6
#define IWDG_STK_SIZE			128
void iwdg_task(void *pdata);

//接收前向指令任务
#define FWRECV_TASK_PRIO		11
#define FWRECV_STK_SIZE			128
void fw_recv_task(void *pdata);

//GPS数据采集任务
#define GPS_TASK_PRIO			12
#define GPS_STK_SIZE			128
void gps_task(void *pdata);  

//姿控状态接收任务
#define ATTITUDE_TASK_PRIO		13
#define ATTITUDE_STK_SIZE			128
void attitude_task(void *pdata); 

//压力数据采集任务
#define PRESSURE_TASK_PRIO		14
#define PRESSURE_STK_SIZE			128
void pressure_task(void *pdata);

//电压值采集任务
#define VOLTAGE_TASK_PRIO		15
#define VOLTAGE_STK_SIZE			128
void voltage_task(void *pdata);

//状态更新任务
#define UPDATE_TASK_PRIO		16
#define UPDATE_STK_SIZE			128
void update_task(void *pdata);

//开始任务
#define START_TASK_PRIO			17
#define START_STK_SIZE			128
void start_task(void *pdata); 

//惯导数据任务
#define IMU300_TASK_PRIO  18
#define IMU300_STK_SIZE  128
void IMU300_task(void *pdata);

extern OS_FLAG_GRP *flags;
extern FlagStatus flag_e1;
extern FlagStatus flag_e3;
extern FlagStatus flag_e4;
extern FlagStatus flag_e5;
extern FlagStatus flag_e7;

#define FLAG_TCP (0x01<<0)
#define FLAG_UDP (0x01<<1)

extern u16 pressure;
extern u16 voltage;
extern double gpitch,groll;
#endif

