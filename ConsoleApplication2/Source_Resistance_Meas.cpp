// ConsoleApplication2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


//int _tmain(int argc, _TCHAR* argv[])
//{
//	return 0;
//}

/*
*******************************************************************************
* File:         sample.c
* Description:  Sample Program of E5270A TIS Library
* Revisions:    A.01.00   [ 12/1/2002 ]
* Language:     C
*
* (C) Copyright 2002, Agilent Technologies Company, all rights reserved.
*******************************************************************************
*/


#include <stdio.h>
#include <iostream>
#include <Windows.h>

#pragma warning (disable : 4996)

/*
*! TIS Bridge Library Header File. This include file must need to use the
*! porting library.
*! When you will port to 4070 TIS, it's should change to
*! "/opt/hp4070/include/tis.h"
*/
#include "E5270_TIS.h"  


void MeasureIVTriangle(double Vbase, double Vmax, double Vdstep, int halfperiods, int negative, double Hold, double Delay, int pins[], const char *fname, double vrange, double irange) {

	/*negative=  0 -> positive and negative sweeps will follow each other if halfperiods is greater than 1
	negative=  1 -> inverted sweeps i.e. -Vmax will be sent
	negative= -1 -> original sweeps s i.e. Vmax will be sent*/

	printf(fname);

	int const N = 1001;
	int const Nmeas = 30;
	int    points = (int)(abs(Vmax - Vbase) / Vdstep + 1.5);
	int	   port[1];

	double measure[Nmeas][N]; //sweep values with type set by set_iv
	double source[Nmeas][N];
	double timestamp[Nmeas][N];
	int    status[Nmeas][N];

	//double vrange = 2.0;
	//double irange = 100e-3;

	double voltages[5];
	voltages[0] = Vbase;
	voltages[1] = Vmax;
	voltages[2] = Vbase;
	voltages[3] = -Vmax;
	voltages[4] = Vbase;



#ifdef E5270_TIS_H
	/*
	*! Don't use the SMU port number directly. You should use a variable which
	*! have a SMU port number. It's useful for porting to 4070 system which
	*! is diffrent configuration with E5270. [Recommendation]
	*/
	int smuD = SMU1;
	int smuW = SMU2;

#else
	int smuD = PORT(0, 1);

#endif

	int Drive = pins[0];
	int Wire = pins[1];
	port[0] = Drive;


	/* This function measures Vth using sweep measurement */
	connect_pin(smuD, Drive);
	connect_pin(smuW, Wire);

	//	double time_stamp, ira, voltage;

	FILE   *pFile;
	pFile = fopen(fname, "w");

	set_timestamp(1);
	reset_timestamp();

	force_v(Wire, Vbase, vrange, REMAIN);

	printf("\n");
	for (int i = 0; i<halfperiods; i++) {
		printf("Current halfperiod: %d\n", i);

		//force_v(Drive, Vbase, vrange, REMAIN);

		set_iv(Drive, LINEAR_V_DBL, vrange, Vbase, Vmax*((negative == 0) ? pow(-1, i) : negative), points, Hold, Delay, REMAIN, 0.0, COMP_CONT);

		sweep_iv(Drive, I_MEAS, irange, measure[i], source[i], NULL);

		status_miv(1, port, status[i], timestamp[i]);

	}
	//force_v(Drive, 2.0, 20.0, REMAIN);

	printf("Total points: %d\n", points);
	printf("Drive(V) Current(mA) Status Time(s)\n");

	//for(int u = 0; u<500; u++){
	//	//force_v(Drive, (double)0.002*u-2.0, 20.0, REMAIN);
	//	measure_it(Drive,&current,20e-3,&time_stamp);
	//	measure_vt(Drive,&voltage,20.0,&time_stamp);
	//	printf( "%d V = %e, I = %e, t=%e\n ", u, voltage, current, time_stamp);
	//	fprintf(pFile,"%d\t%e\t%e\t%4.3f\n",u,voltage,current,time_stamp);
	//	//Sleep(10);
	//}


	fprintf(pFile, "Drive(V),Current(mA),Status,Time(s)\n");
	for (int j = 0; j < halfperiods; j++) {
		for (int i = 0; i < points * 2; i++) {
			printf("%4.2f,%7.8f,%d,%6.4f\n", source[j][i], measure[j][i] * 1000, status[j][i], timestamp[j][i]);
			fprintf(pFile, "%4.2f,%7.8f,%d,%6.4f\n", source[j][i], measure[j][i] * 1000, status[j][i], timestamp[j][i]);
		}
	}

	fclose(pFile);

	disable_port(Drive);

	disconnect_all();

}

void MeasureIR(int samples, int pins[], double vset, double vrange, double irange, const char *fname) {

	int	   port[1];


#ifdef E5270_TIS_H
	/*
	*! Don't use the SMU port number directly. You should use a variable which
	*! have a SMU port number. It's useful for porting to 4070 system which
	*! is diffrent configuration with E5270. [Recommendation]
	*/
	int smuD = SMU1;
	int smuW = SMU2;

#else
	int smuD = PORT(0, 1);

#endif

	int Drive = pins[0];
	int Wire = pins[1];
	port[0] = Drive;

	double current, voltage, time_stamp;

	/* This function measures Vth using sweep measurement */
	connect_pin(smuD, Drive);
	connect_pin(smuW, Wire);

	FILE   *pFile;
	pFile = fopen(fname, "w");

	set_timestamp(1);
	reset_timestamp();

	//force_v(Wire, 0.0, vrange, REMAIN);
	//force_v(Drive, vset, vrange, REMAIN);


	fprintf(pFile, "Drive(V) Current(mA) Status Time(s)\n");
	printf("Drive(V) Current(mA) Status Time(s)\n");
	for (int u = 0; u<400; u++) {
		//force_v(Drive, (double)0.002*u-2.0, 20.0, REMAIN);
		measure_it(Drive, &current, irange, &time_stamp);
		measure_vt(Drive, &voltage, vrange, &time_stamp);

		printf("%4.2f\t%7.8f\t%d\t%6.4f\n", voltage, current * 1000, 0, time_stamp);

		fprintf(pFile, "%4.2f\t%7.8f\t%d\t%6.4f\n", voltage, current * 1000, 0, time_stamp);

		fflush(stdout);
		//fprintf(pFile,"%d\t%e\t%e\t%4.3f\n",u,voltage,current,time_stamp);
		//Sleep(10);
	}

	fclose(pFile);

	disable_port(Drive);

	disconnect_all();
}

void Measvth(double Vdstart, double Vdstop, double Vdstep, double Hold, double delay, int pins[])
{

	int const N = 1001;
	int    points = (int)(abs(Vdstop - Vdstart) / Vdstep + 1.5);
	int	   port[1];

	double measure[N]; //sweep values with type set by set_iv
	double source[N];
	double timestamp[N];
	int    status[N];

	double vrange = 2.0;
	double irange = 100e-3;



#ifdef E5270_TIS_H
	/*
	*! Don't use the SMU port number directly. You should use a variable which
	*! have a SMU port number. It's useful for porting to 4070 system which
	*! is diffrent configuration with E5270. [Recommendation]
	*/
	int smuD = SMU1;
	int smuW = SMU2;

#else
	int smuD = PORT(0, 1);

#endif

	int Drive = pins[0];
	int Wire = pins[1];
	port[0] = Drive;


	/* This function measures Vth using sweep measurement */
	connect_pin(smuD, Drive);
	connect_pin(smuW, Wire);

	set_timestamp(1);
	reset_timestamp();

	force_v(Drive, Vdstart, vrange, REMAIN);
	force_v(Wire, Vdstart, vrange, REMAIN);

	set_iv(Drive, LINEAR_V_DBL, vrange, Vdstart, Vdstop, points, Hold, delay, REMAIN, 0.0, COMP_STOP);

	sweep_iv(Drive, I_MEAS, irange, measure, source, NULL);

	status_miv(1, port, status, timestamp);

	//force_v(Drive, 2.0, 20.0, REMAIN);

	FILE   *pFile;
	pFile = fopen("TTOT_wire_170kOhm_11.txt", "w");

	printf("Total points: %d\n", points);
	printf("Drive(V) Current(mA) Status Time(s)\n");

	//for(int u = 0; u<500; u++){
	//	//force_v(Drive, (double)0.002*u-2.0, 20.0, REMAIN);
	//	measure_it(Drive,&current,20e-3,&time_stamp);
	//	measure_vt(Drive,&voltage,20.0,&time_stamp);
	//	printf( "%d V = %e, I = %e, t=%e\n ", u, voltage, current, time_stamp);
	//	fprintf(pFile,"%d\t%e\t%e\t%4.3f\n",u,voltage,current,time_stamp);
	//	//Sleep(10);
	//}


	fprintf(pFile, "Drive(V) Current(mA) Status Time(s)\n");
	for (int i = 0; i < points; i++) {
		printf("%4.2f\t%7.8f\t%d\t%6.4f\n", source[i], measure[i] * 1000, status[i], timestamp[i]);
		fprintf(pFile, "%4.2f\t%7.8f\t%d\t%6.4f\n", source[i], measure[i] * 1000, status[i], timestamp[i]);
	}

	fclose(pFile);

	disable_port(Drive);

	disconnect_all();

	///* Vth extraction */
	//for ( i = 0 ; i < point ; i++ ) {
	//     printf( "Measured I%d = %e\n ", i, measure[i] );
	//}

}

int main()
{
	int    ret;
	int    pins[4];

#ifdef E5270_TIS_H
	/*
	* Non compatible function. This function must be called before
	* use TIS porting library functions.
	*/
	ret = open_E5270("GPIB0::16::INSTR", ERR_DETECT_ON, NULL);
	if (ret != 0) {
		printf("E5270 device open failed. Exit program.\n");
		return(-1);
	}
#endif

	/* Initialize */
	init_system();

	pins[0] = 1;
	pins[1] = 2;
	//pins[2] = 3;
	//pins[3] = 4;

	//Measvth( 1.2, 0, 5e-3, 0.0, 0.0, pins);

	//|sample id|_|composition|_|pins|_|measurement_number|.txt
	//no4_agar04_12mwnt_6cb_4wo3_pin3_4_meas32.txt

	const char fname[] = "Glass_Au-AgZnONi_02062019_560_2.txt";
	//const char fname[] = "AgZnOAg_01312019_519.txt";
	const double vrange = 0;
	const double irange = 0;
	const int vdirection = 1;

	//(double Vbase, double Vmax, double Vdstep, int halfperiods, int negative, double Hold, double Delay, int pins[], const char *fname, double vrange, double irange) {
	MeasureIVTriangle(0, 35, 8e-1, 10, vdirection, 0.000, 0.00, pins, fname, vrange, irange);


	//MeasureIR(1000,pins,0,vrange,irange,fname);
	//MeasureIR(pins);

	//printf( "Measured Vth = %e\n", vth );

#ifdef E5270_TIS_H
	close_E5270();  /*
					*! Instrument will be implicitly closed, when program
					*! completed. But recommend to call close_E5270() before
					*! exit program.
					*/
#endif


	return(0);
}





