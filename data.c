#include "data.h"

int32_t t_conusscan;    
double fw_now_DaDi;
uint16_t AGC_max;

#define CHECK_MAX_AGC(n,m) (n>m?n:m)
#define NULL 0

msg_e1 e1 = {
	.header = 0xAA,
	.id = 0xF1
};
msg_e2 e2;
msg_e3 e3;
msg_e4 e4;

msg_e5 e5 = {
	.header = 0xAA,
	.id = 0xF6
};
msg_e6 e6 = {
	.header = 0xAA,
	.id = 0xF7
};
msg_e7 e7;
msg_e8 e8;
msg_e10 e10;
msg_e11 e11 = {
	.header = 0xAA,
	.id = 0xFC
};
msg_e12 e12 = {
	.header = 0xAA,
	.id = 0xFD
};

msg_e13 e13 = {
	.header = 0x77,
	.len = 0x0D,
	.addr = 00,
	.cmd = 0x84
};

msg_e14 e14 = {
	.header = 0xAA,
	.frameno = 0xFB
};


u8 check ( u8* buf, u16 len )
{
	u8 val = 0;
	u16 i = 0;

	for ( i = 0; i < len; i++ )	{
		val += buf[i];
	}

	return val;
}

double cal_angle ( uint8_t* data )
{
	int8_t sign = 1;
	double result_data = 0;

	if ( *data >= 0x10 )
	{
		sign = -1;
		*data -= 0x10;
	}

	result_data = ( double ) ( sign * ( bcd_decimal ( *data ) * 100 + bcd_decimal ( * ( data + 1 ) ) + ( double ) bcd_decimal ( * ( data + 2 ) ) / 100.0 ) );

	return result_data;
}

uint8_t bcd_decimal ( uint8_t bcd )
{
	return bcd - ( bcd >> 4 ) * 6;
}


void JieSuanJieGuo ( double ppitch, double proll, uint16_t AGC_threshold, uint16_t AGC_now, gpsdata mubiao_gps, gpsdata zhuantai_gps, double* fw_ZhuanTai, double* gd_ZhuanTai )
{

	double fw_ConusScan_DaDi = 0, gd_ConusScan_DaDi = 0;
	double fw_XunXing=0, gd_XunXing=0;
	double fw_now=0, gd_now=0;
	double PI = 3.1415926;

	if(fw_ZhuanTai == NULL || gd_ZhuanTai == NULL)
	{
		return;
	}
	
	cal_Antenna_axis ( fw_now_DaDi, ppitch, proll );
	cal_Antenna2Satellite_angle ( &fw_XunXing, &gd_XunXing, mubiao_gps, zhuantai_gps );

	if ( AGC_now > AGC_threshold )
	{

		if ( AGC_now < AGC_max )
		{
			ConusScan ( &fw_ConusScan_DaDi, &gd_ConusScan_DaDi, 120.0 / 180.0 * PI * t_conusscan * 0.02, 2 );  // 2¡ã×¶É¨
			t_conusscan++;		
		}
		else
		{
			AGC_max = CHECK_MAX_AGC ( AGC_now, AGC_max );		
		}
		fw_now = fw_XunXing + fw_ConusScan_DaDi;
		gd_now = gd_XunXing + gd_ConusScan_DaDi;	
	}
	else
	{
		t_conusscan = 0;
		fw_now_DaDi += 0.04;     //2¡ã/S

		if ( fw_now_DaDi >= 360.0 )
		{
			fw_now_DaDi -= 360.0;
		}

		fw_now = fw_XunXing;
		gd_now = gd_XunXing;		
	}
	
	if ( fw_now < 0 ) //×ª»»³É0-360
	{
		fw_now += 360.0;
	}
	else if ( fw_now >= 360.0 )
	{
		fw_now -= 360.0;
	}
	
	*fw_ZhuanTai = fw_now;
	*gd_ZhuanTai = gd_now;


}



