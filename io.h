#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdbool.h>

//#include "stations.h"


typedef struct _Files{
	FILE *cfg;
	FILE *memin;
	FILE *memout;
	FILE *regout;
	FILE *traceinst;
	FILE *tracedb;
} Files;

typedef struct _CfgParameters {
	int add_nr_units, mul_nr_units, div_nr_units;
	int add_nr_reservation, mul_nr_reservation, div_nr_reservation;
	int add_delay, mul_delay, div_delay;
	int mem_delay;
	int mem_nr_load_buffers, mem_nr_store_buffers;
} CfgParameters;

bool OpenFiles(Files *files_struct, char *argv[]);
void ReadMem(Files *memin_file);
int FindLastInstPC();
int FindLastNotZeroAddress();
void SetCfgParameters(FILE *cfg_file, CfgParameters *cfg_parameters);
void PrintTo_traceinst_file(FILE *traceinst_file);
void PrintTo_regout_file(FILE *regout_file);
void PrintTo_memout_file(FILE *memout_file);
float GetSinglePrecisionFormat(int sign, int exponent, int fraction);
int GetFloatToBin(float number);

#endif
