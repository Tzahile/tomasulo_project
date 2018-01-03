#include <stdlib.h>
#include <string.h>

#include "analyze_inst.h"
#include "common.h"
#include "io.h"
#include "stations.h"

extern char op_name[][NUM_OF_OP_CODES];
extern char reg_name[][NUM_OF_REGISTERS];
extern double reg_values[NUM_OF_REGISTERS];
extern char reg_name[][NUM_OF_REGISTERS];
extern int mem[MEM_SIZE];
extern Station *add_sub_res_stations, *mul_res_stations, *divide_res_stations, *load_res_stations, *store_res_stations;
//extern int current_inst_queue_size, current_add_sub_res_stations_size, current_mul_res_stations_size,
//current_divide_res_stations_size, current_load_res_stations_size, current_store_res_stations_size;
extern InstQueue inst_queue[INST_QUEUE_SIZE];
extern Registers registers[NUM_OF_REGISTERS];

void EnterToInstQueue(int inst, int op, int dst, int src0, int src1, int imm, int inst_queue_size);
void UpdateRegisters(int station_name, int dst, int *relevent_size);
void PopInstQueue();


int sbs(int x, int msb, int lsb);

int sbs(int x, int msb, int lsb)
{
	if (msb == 31 && lsb == 0)
		return x;
	return (x >> lsb) & ((1 << (msb - lsb + 1)) - 1);
}

void DecodeInst(int inst, int *op, int *dst, int *src0, int *src1, int *imm) {
	*op = sbs(inst, 27, 24);
	*dst = sbs(inst, 23, 20);
	*src0 = sbs(inst, 19, 16);
	*src1 = sbs(inst, 15, 12);
	*imm = sbs(inst, 11, 0);
}

void InitRegistersStruct() {
	int i = 0;
	for (i = 0; i < NUM_OF_REGISTERS; i++) {
		registers[i].reg_name = reg_name[i];
		registers[i].V = (float)i;
		registers[i].Q = 0;
		registers[i].station_offset = 0;
	}
}

// return the number of new instruction entered to instruction queue.
int Fetch(int last, int *PC, int *inst_queue_size) {
	int op, dst, src0, src1, imm, inst, new_insts = 0;
	if (*inst_queue_size < INST_QUEUE_SIZE) {
		inst = mem[*PC];
		(*PC)++;
		new_insts++;
		DecodeInst(inst, &op, &dst, &src0, &src1, &imm);
		EnterToInstQueue(inst, op, dst, src0, src1, imm, *inst_queue_size);
		(*inst_queue_size)++;
	}
	return new_insts;
}

void EnterToInstQueue(int inst, int op, int dst, int src0, int src1, int imm, int inst_queue_size) {
	inst_queue[inst_queue_size].original_inst = inst;
	inst_queue[inst_queue_size].op = op;
	inst_queue[inst_queue_size].dst = dst;
	inst_queue[inst_queue_size].src0 = src0;
	inst_queue[inst_queue_size].src1 = src1;
	inst_queue[inst_queue_size].imm = imm;
	inst_queue[inst_queue_size].is_busy = true;
}

void Issue(CfgParameters *cfg_parameters, int *inst_queue_size, int *add_sub_res_stations_size,
	int *mul_res_stations_size, int *divide_res_stations_size, int *load_res_stations_size,
	int *store_res_stations_size, int cycle)
{
	int op, dst;
	if (inst_queue_size == 0)
	{
		return;
	}
		op = inst_queue[0].op;
		dst = inst_queue[0].dst;
		switch (op)
		{
			case OP_ADD:
			case OP_SUB:
				if (*add_sub_res_stations_size == cfg_parameters->add_nr_reservation)
				{ 
					return;
				}
				EnterToReservationStation(inst_queue[0], add_sub_res_stations, add_sub_res_stations_size, cycle, cfg_parameters);
				UpdateRegisters(ADD_SUB_RESORVATION_STATION, dst, add_sub_res_stations_size);
				(*add_sub_res_stations_size)++;
				PopInstQueue(inst_queue_size);
				break;
			case OP_MULT:
				if (*mul_res_stations_size == cfg_parameters->mul_nr_reservation)
				{
					return;
				}
				EnterToReservationStation(inst_queue[0], mul_res_stations, mul_res_stations_size, cycle, cfg_parameters);
				UpdateRegisters(MULT_RESORVATION_STATION, dst, mul_res_stations_size);
				(*mul_res_stations_size)++;
				PopInstQueue(inst_queue_size);
				break;
			case OP_DIV:
				if (*divide_res_stations_size == cfg_parameters->div_nr_reservation)
				{
					return;
				}
				(*divide_res_stations_size)++;
				UpdateRegisters(DIV_RESORVATION_STATION, dst, divide_res_stations_size);
				break;
		}


}

void UpdateRegisters(int station_name, int dst, int *relevent_size)
{
	registers[dst].Q = station_name;
	registers[dst].station_offset = *relevent_size;
}

void PopInstQueue(int *inst_queue_size)
{
	int i;
	for (i = 1; i < INST_QUEUE_SIZE; i++)
	{
		inst_queue[i - 1] = inst_queue[i];
	}
	(*inst_queue_size)--;
}
