#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "io.h"
#include "stations.h"
#include "analyze_inst.h"

extern char op_name[][NUM_OF_OP_CODES];
extern char reg_name[][NUM_OF_REGISTERS];
extern double reg_values[NUM_OF_REGISTERS];
extern Station *add_sub_res_stations, *mul_res_stations, *divide_res_stations, *load_res_stations, *store_res_stations;
extern int mem[MEM_SIZE];
extern InstQueue inst_queue[INST_QUEUE_SIZE];
extern Registers registers[NUM_OF_REGISTERS];

void ClearResSlot(Station *res_station, int offset);
void SetReadyForExec(Station *res_station, int size, int cycle);
int GetNumberOfWorkingExecUnits(Station *res_station, int size);
void EnterToExec(Station *res_station, int res_size, int nr_of_exec_units, int nr_of_working_exec_units, int cycle, int cycles_in_exec);

void PrepareReservationStations(CfgParameters *cfg_parameters) {
	add_sub_res_stations = (Station *)calloc(cfg_parameters->add_nr_reservation, sizeof(Station));
	if (add_sub_res_stations == NULL) {
		exit(ERROR_CODE);
	}
	mul_res_stations = (Station *)calloc(cfg_parameters->mul_nr_reservation, sizeof(Station));
	if (mul_res_stations == NULL) {
		exit(ERROR_CODE);
	}
	divide_res_stations = (Station *)calloc(cfg_parameters->div_nr_reservation, sizeof(Station));
	if (divide_res_stations == NULL) {
		exit(ERROR_CODE);
	}
	load_res_stations = (Station *)calloc(cfg_parameters->mem_nr_load_buffers, sizeof(Station));
	if (load_res_stations == NULL) {
		exit(ERROR_CODE);
	}
	store_res_stations = (Station *)calloc(cfg_parameters->mem_nr_store_buffers, sizeof(Station));
	if (store_res_stations == NULL) {
		exit(ERROR_CODE);
	}
}

PutInReservationStation(Station *required_res_station, int inst_queue_size) {
	int op = inst_queue[inst_queue_size].op;
	required_res_station->is_busy = true;
}

void EnterToReservationStation(InstQueue inst, Station *res_station, int *size, int cycle, CfgParameters *cfg_parameters)
{
	switch (registers[inst.src0].Q) {
	case ADD_SUB_RESORVATION_STATION:
		res_station[*size].q_j = ADD_SUB_RESORVATION_STATION;
		res_station[*size].q_j_station_offset = registers[inst.src0].station_offset;
		break;
	case MULT_RESORVATION_STATION:
		res_station[*size].q_j = MULT_RESORVATION_STATION;
		res_station[*size].q_j_station_offset = registers[inst.src0].station_offset;
		break;
	case DIV_RESORVATION_STATION:
		res_station[*size].q_j = DIV_RESORVATION_STATION;
		res_station[*size].q_j_station_offset = registers[inst.src0].station_offset;
		break;
	case 0:
		res_station[*size].v_j = registers[inst.src0].V;
		res_station[*size].q_j = 0;
		break;
	}
	switch (registers[inst.src1].Q) {
	case ADD_SUB_RESORVATION_STATION:
		res_station[*size].q_k = ADD_SUB_RESORVATION_STATION;
		res_station[*size].q_k_station_offset = registers[inst.src1].station_offset;
		break;
	case MULT_RESORVATION_STATION:
		res_station[*size].q_k = MULT_RESORVATION_STATION;
		res_station[*size].q_k_station_offset = registers[inst.src1].station_offset;
		break;
	case DIV_RESORVATION_STATION:
		res_station[*size].q_k = DIV_RESORVATION_STATION;
		res_station[*size].q_k_station_offset = registers[inst.src1].station_offset;
		break;
	case 0:
		res_station[*size].v_k = registers[inst.src1].V;
		res_station[*size].q_k = 0;
		break;
	}
	res_station[*size].cycle_entered = cycle;

	switch (inst.op)
	{
	case OP_ADD:
		res_station[*size].is_add = true;
		break;
	case OP_SUB:
		res_station[*size].is_sub = true;
		break;
	case OP_LD:
		res_station[*size].is_ld = true;
		break;
	}
	res_station[*size].is_busy = true;
}

void Exec(CfgParameters *cfg_parameters, int cycle, int add_sub_res_stations_size, int mul_res_stations_size,
	 int divide_res_stations_size, int load_res_stations_size, int store_res_stations_size)
{
	int current_add_sub_in_exec = 0, working_units = 0;
	bool is_add_sub_exec_busy = false, is_mul_exec_busy = false, is_divide_res_stations_busy = false;
	working_units = GetNumberOfWorkingExecUnits(add_sub_res_stations, add_sub_res_stations_size);
	SetReadyForExec(add_sub_res_stations, add_sub_res_stations_size, cycle);
	EnterToExec(add_sub_res_stations, add_sub_res_stations_size, cfg_parameters->add_nr_units, working_units, cycle, cfg_parameters->add_delay);
	//CDB();
}

CheckIfInstFinishedExec(CfgParameters *cfg_parameters, int cycle, int add_sub_res_stations_size, int mul_res_stations_size,
	int divide_res_stations_size, int load_res_stations_size, int store_res_stations_size)
{
	int i;
	for (i = 0; i < add_sub_res_stations_size; i++)
	{
		if (cycle == add_sub_res_stations[i].cycle_to_finish_exec)
		{

			ClearResSlot(add_sub_res_stations, i);
			break;
		}
	}
}

void ClearResSlot(Station *res_station, int offset)
{
	res_station[offset].addr = 0;
	res_station[offset].command = NULL;
	res_station[offset].cycle_entered = 0;
	res_station[offset].cycle_to_finish_exec = 0;
	res_station[offset].is_add = false;
	res_station[offset].is_busy = false;
	res_station[offset].is_div = false;
	res_station[offset].is_halt = false;
	res_station[offset].is_in_exec = false;
	res_station[offset].is_ld = false;
	res_station[offset].is_mult = false;
	res_station[offset].is_ready_for_exec = false;
	res_station[offset].is_st = false;
	res_station[offset].is_sub = false;
	res_station[offset].q_j = 0;
	res_station[offset].q_j_station_offset = 0;
	res_station[offset].q_k = 0;
	res_station[offset].q_k_station_offset = 0;
	res_station[offset].v_j = 0;
	res_station[offset].v_k = 0;
}

void SetReadyForExec(Station *res_station, int size, int cycle)
{
	int i;
	for (i = 0; i < size; i++)
	{
		if ((cycle > res_station[i].cycle_entered) && res_station[i].is_in_exec == false && res_station[i].q_j == 0 && res_station[i].q_k == 0)
		{
			res_station[i].is_ready_for_exec = true;
		}
		else
		{
			res_station[i].is_ready_for_exec = false;
		}
	}
}

int GetNumberOfWorkingExecUnits(Station *res_station, int size)
{
	int i, counter = 0;
	for (i = 0; i < size; i++)
	{
		if (res_station[i].is_in_exec == true) {
			counter++;
		}
	}
	return counter;
}

void EnterToExec(Station *res_station, int res_size, int nr_of_exec_units, int nr_of_working_exec_units, int cycle, int cycles_in_exec)
{
	int i, nr_of_empty_exec_units = nr_of_exec_units - nr_of_working_exec_units;
	for (i = 0; i < res_size; i++)
	{
		if (res_station[i].is_ready_for_exec == true)
		{
			if (nr_of_empty_exec_units == 0) {
				break;
			}
			res_station[i].is_in_exec = true;
			res_station[i].cycle_to_finish_exec = cycle + cycles_in_exec;
			nr_of_empty_exec_units--;
		}
	}
}