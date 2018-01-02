#ifndef ANALYZE_INST_H
#define ANALYZE_INST_H

#include <stdbool.h>

#include "io.h"

typedef struct _Registers {
	char *reg_name;
	double V;
	//char *Q;
	int Q; // ADD_SUB_RESORVATION_STATION 1, MUL_RESORVATION_STATION 2, DIV_RESORVATION_STATION 3
	int station_offset;
} Registers;

typedef struct _InstQueue {
	bool is_busy;
	int op;
	int dst;
	int src0;
	int src1;
	int imm;
	int original_inst;
} InstQueue;

void DecodeInst(int inst, int *op, int *dst, int *src0, int *src1, int *imm);
void InitRegistersStruct();
int Fetch(int last, int *PC, int *inst_queue_size);
void Issue(CfgParameters *cfg_parameters, int *inst_queue_size, int *add_sub_res_stations_size,
	int *mul_res_stations_size, int *divide_res_stations_size, int *load_res_stations_size,
	int *store_res_stations_size, int cycle);
#endif