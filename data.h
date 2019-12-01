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
}msg_e1;										//��ѧ���ݣ��ṹ���СΪ1043�ֽ�

__packed typedef struct
{
	u8 status[72];
	u8 heartbeat[72];
}msg_e2;										//ƽ̨״̬��Ϣ���ṹ���СΪ144�ֽ�

__packed typedef struct
{
	u8 cmd[30];
}msg_e3;										//ƽ̨����ָ��ṹ���СΪ30�ֽ�

__packed typedef struct
{
	nmea_utc_time utc;						//UTCʱ�䣬��ʽhhmmss
	s32 longitude;				//���ȣ�-180��180, ����ϵ��1E-7
	s32 latitude;					//γ�ȣ�-90��90, ����ϵ��1E-7
	s32 altitude;					//���Σ�����ϵ��0.1
	u16 voltage;					//��ѹ������ϵ��0.01
	u16 pressure;				//ѹ��������ϵ��0.001
	u16 reserve;			
}msg_e4;										//ͨ�Ÿ���״̬��Ϣ���ṹ���СΪ24�ֽ�

__packed typedef struct
{
	s16 cal_azimuth;			//��λ��-180��180,����0.01
	s16 cal_pitch;				//������-90��90,����0.01
	s16 course;						//���򣬱���0.01
	s16 pitch;						//��ҡ������0.01
	s16 roll;							//���������0.01
	u8 antenna;					//����״̬��0��ʼ����1Ѱ�ǣ�2��׽��3���٣�4������5ʧ��
	u8 AHRS;							//ARHS״̬��0��ʼ����1ƽ��궨��2����궨��3���ã�4�쳣
	u8 GPS;							//��λ״̬��0��ʼ����1�Ѷ�λ
	u8 ctrl;							//����ģʽ��0�Զ�ģʽ��0xAA�ֶ�ģʽ
	u8 AGC_threshold;		//AGC���ޣ�0��10������ϵ��0.1
	u8 AGC_max;					//AGC���ֵ��0��10������ϵ��0.1
	u8 AGC;							//AGC��ǰֵ��0��10������ϵ��0.1
	u8 reserve[7];
}msg_e10;										//��̬��Ϣ���ṹ���СΪ24�ֽ�

__packed typedef struct
{
	msg_e4 buoy_status;				//ͨ�Ÿ���״̬��Ϣ
	msg_e10 attitude;					//��̬��Ϣ
	u8 reserve1[10];
	u32 cnt;							//֡����
	u8 modulate_mode;		//����ģʽ��0x55   ����ģʽ��0xAA   ���ز�ģʽ
	u8 leak;							//©ˮ��0x00: ������0xFF: ©ˮ����
	u8 power;						//���߿��أ�0x00:����0xFF:��
	u8 reserve2[9];
}msg_e7;										//ͨ��ϵͳ״̬��Ϣ���ṹ���СΪ74�ֽ�

__packed typedef struct
{
	u8 header;
	u8 id;
	msg_e2 platform_status;		//ƽ̨״̬��Ϣ
	msg_e7 system_status;			//ͨ��ϵͳ״̬��Ϣ
	u8 check;
}msg_e5;										//ADSϵͳ״̬��Ϣ���ṹ���СΪ218�ֽ�

__packed typedef struct
{
	u8 modulate_mode;		//����ģʽ��0x55   ����ģʽ��0xAA   ���ز�ģʽ
	u8 satellite;				//ѡ�ǣ�0x00   ������0x11   ��ͨ
	u8 AGC_threshold;		//AGC���ޣ�����ֵ:0��10������ϵ��0.1
	u8 calibration;			//��̬У׼��0��У׼��1Ϊƽ��У׼��2Ϊƽ��У׼������3Ϊ����У׼
	u8 power;						//���߿��أ�0x00:����0xFF:��
	u8 attitude_mode;		//�˿�ģʽ��0�Զ���Ĭ�ϣ���0x55�ֶ���У׼������
	s16 azimuth;					//��λ��У׼�����ʹ�ã�����0.01
	s16 pitch;						//������У׼�����ʹ�ã�����0.01
	u32 memory;					//�洢����
	u8 reserve[6];
}msg_e8;										//ͨ��ϵͳ����ָ��ṹ���СΪ20�ֽ�

__packed typedef struct
{
	u8 header;
	u8 id;
	msg_e3 platform_cmd;			//ƽ̨����ָ��
	msg_e8 system_cmd;				//ͨ��ϵͳ����ָ��
	u8 check;
}msg_e6;										//ADSϵͳ���ƣ��ṹ���СΪ50�ֽ�

__packed typedef struct
{
	u8 header;
	u8 id;
	s32 longitude;				//���ȣ�-180��180, ����ϵ��1E-7
	s32 latitude;					//γ�ȣ�-90��90, ����ϵ��1E-7
	s32 altitude;					//���Σ�����ϵ��0.1
	u8 satellite;				//ѡ�ǣ�0������1��ͨ
	u8 calibration;			//��̬У׼��0��У׼��1Ϊƽ��У׼��2Ϊƽ��У׼������3Ϊ����У׼
	u8 AGC;							//AGC��ƽ��0��10������ϵ��0.1
	u8 AGC_threshold;		//AGC���ޣ�����ֵ:0��10������ϵ��0.1
	u8 attitude_mode;		//�˿�ģʽ��0�Զ���Ĭ�ϣ���0x55�ֶ���У׼������
	s16 azimuth;					//��λ��У׼�����ʹ�ã�����0.01
	s16 pitch;						//������У׼�����ʹ�ã�����0.01
	u8 reserve[3];
	u8 check;
}msg_e11;										//�˿ص�Ԫ����ָ��ṹ���СΪ24�ֽ�

__packed typedef struct
{
	u8 header;
	u8 id;
	u32 cnt;							//֡����
	u16 AGC;							//AGC��ƽ��0��10������ϵ��0.1
	u16 mode_toggle;		  //����ģʽ�л���55�ز�ģʽ��AA���ز�ģʽ
	u16 reserve;
	u8 check;
}msg_e12;										//������·״̬


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
}msg_e13;       //�ߵ�����

__packed typedef struct
{
	u8 header;
	u8 frameno;
	u8 fwdata[2];
	u8 gddata[2];
	u8 check;
}msg_e14; //�ŷ�����������

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
