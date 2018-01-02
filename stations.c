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
extern int mem[MEM_SIZE]; //PC, current_inst_queue_size;
extern InstQueue inst_queue[INST_QUEUE_SIZE];
extern Registers registers[NUM_OF_REGISTERS];

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
	//strcpy(required_res_station->command, op_name[0][op]);
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
		//res_station[*size].ready_in_cycle = cycle + cfg_parameters->add_delay;
		break;
	case OP_SUB:
		res_station[*size].is_sub = true;
		//res_station[*size].ready_in_cycle = cycle + cfg_parameters->add_delay;
		break;
	case OP_LD:
		res_station[*size].is_ld = true;
		//res_station[*size].ready_in_cycle = cycle + cfg_parameters->mem_delay;
		break;
	}
	res_station[*size].is_busy = true;
	//(*size)++;
	//if (inst.op == OP_ADD)
	//{

	//}
}

void Exec(CfgParameters *cfg_parameters, int cycle) {

}

void UpdateReservationStationsData(CfgParameters *cfg_parameters, int cycle, int add_sub_res_stations_size, int mul_res_stations_size,
	 int divide_res_stations_size, int load_res_stations_size, int store_res_stations_size)
{
	int i;
	bool is_add_sub_exec_busy = false, is_mul_exec_busy = false, is_divide_res_stations_busy = false;

	// checking if exec unit is busy
	for (i = 0; i < add_sub_res_stations_size; i++)
	{
		if (add_sub_res_stations[i].is_in_exec == true)
		{
			is_add_sub_exec_busy = true;
		}
	}

	for (i = 0; i < mul_res_stations_size; i++)
	{
		if (mul_res_stations[i].is_in_exec == true)
		{
			is_mul_exec_busy = true;
		}
	}

	for (i = 0; i < divide_res_stations_size; i++)
	{
		if (divide_res_stations[i].is_in_exec == true)
		{
			is_divide_res_stations_busy = true;
		}
	}

	for (i = 0; i < load_res_stations_size; i++)
	{
		if (load_res_stations[i].is_in_exec == true)
		{
			is_divide_res_stations_busy = true;
		}
	}
	//
	// enter to apropriate exec if the exec is not busy
	if (is_add_sub_exec_busy == false)
	{
		for (i = 0; i < add_sub_res_stations_size; i++)
		{
			if (add_sub_res_stations[i].is_ready_for_exec == cycle)
			{
				add_sub_res_stations[i].is_in_exec = true;
				break;
			}
		}
	}

	if (is_mul_exec_busy == false)
	{
		for (i = 0; i < mul_res_stations_size; i++)
		{
			if (mul_res_stations[i].is_ready_for_exec == cycle)
			{
				mul_res_stations[i].is_in_exec = true;
				break;
			}
		}
	}

	if (is_divide_res_stations_busy == false)
	{
		for (i = 0; i < divide_res_stations_size; i++)
		{
			if (divide_res_stations[i].is_ready_for_exec == cycle)
			{
				divide_res_stations[i].is_in_exec = true;
				break;
			}
		}
	}


		/*if (cycle = add_sub_res_stations[i].cycle_entered + 1)
		{
			add_sub_res_stations[i].is_ready_for_exec = true;
		}*/
}