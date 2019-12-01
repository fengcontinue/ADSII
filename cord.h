/*	   CORD.h
*/

#ifndef  _COORDINATETRANSFORM_H
#define  _COORDINATETRANSFORM_H


typedef struct
{
	double  longitude; //经度
	double  latitude;  //纬度
	double  high;
} gpsdata;

typedef struct
{
	double  x;    //
	double  y;    //
	double  z;
} D3vector;



D3vector vect_rot ( D3vector normal_vect ,  D3vector vect, double theta );
D3vector vect_cross ( D3vector v1, D3vector v2 );


D3vector ConvToCart ( gpsdata  gps00 );               //球坐标转换为直角坐标(地球坐标系)
D3vector ConvToGCord ( gpsdata  gps00, D3vector vect00 );    //地球坐标（直角）转换为地面坐标

double vect_dot ( D3vector v1, D3vector v2 ); //矢量点乘
double vect_len ( D3vector vect00 );        //矢量长度
D3vector vect_cross ( D3vector v1, D3vector v2 );

D3vector find_axisX ( gpsdata  gps00 );    //地面坐标系的X轴（东向矢量）在 地球直角坐标中的表示
D3vector find_axisY ( gpsdata  gps00 );    //地面坐标系的X轴（北向矢量）在 地球直角坐标中的表示
D3vector find_axisZ ( gpsdata  gps00 );    //地面坐标系的X轴（天向矢量）在 地球直角坐标中的表示

D3vector ProjectToVector ( D3vector normal_vect ,  D3vector vect ); //矢量到矢量的投影
D3vector ProjectToPlain ( D3vector normal_vect ,  D3vector vect ); //矢量对平面的投影

//void cal_Dir(gpsdata car_location0, gpsdata  satellite_location0,   float *fwangle0,  float *gdangle0);

double cal_Distance ( gpsdata location1, gpsdata  location2 );
void cal_Antenna_axis(double heading, double pitch, double roll);
void cal_Antenna2Satellite_angle ( double* Antenna_fw,  double* Antenna_gd ,gpsdata Satellite_gps, gpsdata Antenna_gps);
void transform2DaDiZuoBiaoXi (double pitch, double roll, double heading, double fw_deg, double gd_deg, double* a_deg, double* e_deg );
void transform2ZhuanTaiZuoBiaoXi ( double pitch, double roll, double heading, double fw_deg, double gd_deg, double* a_deg, double* e_deg );
void ConusScan ( double* fw, double* gd, double wt_rad, int A);
#endif
//-------------------------------------------------------
/*
	----------		end  file    ------------------
*/

