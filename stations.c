#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "io.h"
#include "stations.h"
#include "analyze_inst.h"
#define LOAD_NR_UNITS 1
#define STORE_NR_UNITS 1

extern char op_name[][NUM_OF_OP_CODES];
extern char reg_name[][NUM_OF_REGISTERS];
extern double reg_values[NUM_OF_REGISTERS];
extern Station *add_sub_res_stations, *mul_res_stations, *divide_res_stations, *load_res_stations, *store_res_stations;
extern int mem[MEM_SIZE], last;
extern InstQueue inst_queue[INST_QUEUE_SIZE];
extern Registers registers[NUM_OF_REGISTERS];
extern CDB_status CDB_status_var;
extern IssueList *issue_list;
extern Files files_struct;

void ClearResSlot(Station *res_station, int offset);
void SetReadyForExec(Station *res_station, int res_size, int cycle);
int GetNumberOfWorkingExecUnits(Station *res_station, int res_size);
bool EnterToExec(Station *res_station, int res_size, int nr_of_exec_units, int nr_of_working_exec_units, int cycle, int cycles_in_exec);
void RemoveLabel(Station *res_station, int type, int offset, int res_size, float dst_result);
float DoArithmeticCalc(Station *inst);
void PutInReservationStation(Station *required_res_station, int inst_queue_size);
int getReservationType(Station *res_station);
void DealWithCDB(CfgParameters *cfg_parameters, int cycle);
bool CDB(CfgParameters *cfg_parameters, Station *res_station, int size, int cycle);
int FindLastExecCycle(Station *res_station, int size);
int getMax(int num[], int size);
int getNumOfBusy(Station *res_station, int size);
int DoMemCalc(Station *inst);

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

void PutInReservationStation(Station *required_res_station, int inst_queue_size) {
	int op = inst_queue[inst_queue_size].op;
	required_res_station->is_busy = true;
}

void EnterToReservationStation(InstQueue inst, Station *res_station, int size, int cycle, CfgParameters *cfg_parameters)
{
	switch (registers[inst.src0].Q) {
	case ADD_SUB_RESORVATION_STATION:
		res_station[size].q_j = ADD_SUB_RESORVATION_STATION;
		res_station[size].q_j_station_offset = registers[inst.src0].station_offset;
		break;
	case MULT_RESORVATION_STATION:
		res_station[size].q_j = MULT_RESORVATION_STATION;
		res_station[size].q_j_station_offset = registers[inst.src0].station_offset;
		break;
	case DIV_RESORVATION_STATION:
		res_station[size].q_j = DIV_RESORVATION_STATION;
		res_station[size].q_j_station_offset = registers[inst.src0].station_offset;
		break;
	case 0:
		res_station[size].v_j = registers[inst.src0].V;
		res_station[size].q_j = 0;
		break;
	}
	switch (registers[inst.src1].Q) {
	case ADD_SUB_RESORVATION_STATION:
		res_station[size].q_k = ADD_SUB_RESORVATION_STATION;
		res_station[size].q_k_station_offset = registers[inst.src1].station_offset;
		break;
	case MULT_RESORVATION_STATION:
		res_station[size].q_k = MULT_RESORVATION_STATION;
		res_station[size].q_k_station_offset = registers[inst.src1].station_offset;
		break;
	case DIV_RESORVATION_STATION:
		res_station[size].q_k = DIV_RESORVATION_STATION;
		res_station[size].q_k_station_offset = registers[inst.src1].station_offset;
		break;
	case 0:
		res_station[size].v_k = registers[inst.src1].V;
		res_station[size].q_k = 0;
		break;
	}
	res_station[size].cycle_entered = cycle;

	switch (inst.op)
	{
	case OP_ADD:
		res_station[size].is_add = true;
		break;
	case OP_SUB:
		res_station[size].is_sub = true;
		break;
	case OP_MULT :
		res_station[size].is_mult = true;
		break;
	case OP_DIV:
		res_station[size].is_div = true;
		break;
	case OP_STORE:
		res_station[size].is_store = true;
		break;
	case OP_LOAD:
		res_station[size].is_load = true;
		break;
	case OP_HALT:
		res_station[size].is_halt = true;
		break;
	}
	res_station[size].original_inst = inst.original_inst;
	res_station[size].is_busy = true;
	res_station[size].imm = inst.imm;

	EnterToIssueList(res_station, size, cycle);
}

void EnterToIssueList(Station *res_station, int offset, int cycle)
{
	int i;
	for (i = 0; i < last; i++)
	{
		if (issue_list[i].is_busy == false)
		{
			issue_list[i].is_busy = true;
			issue_list[i].original_inst = res_station->original_inst;
			issue_list[i].cycle_issued = cycle;
			issue_list[i].PC = i;
			res_station[offset].PC = i;
			issue_list[i].tag = getReservationType(&res_station[offset]);
			issue_list[i].offset = offset;
			break;
		}
	}
}

void Exec(CfgParameters *cfg_parameters, int cycle)
{
	int current_add_sub_in_exec = 0, working_units = 0, mem_working_units = 0;
	bool is_add_sub_exec_busy = false, is_mul_exec_busy = false, is_divide_res_stations_busy = false;
	bool is_load_just_entered = false;

	// Add-SUB
	working_units = GetNumberOfWorkingExecUnits(add_sub_res_stations, cfg_parameters->add_nr_reservation);
	SetReadyForExec(add_sub_res_stations, cfg_parameters->add_nr_reservation, cycle);
	EnterToExec(add_sub_res_stations, cfg_parameters->add_nr_reservation, cfg_parameters->add_nr_units, working_units, cycle, cfg_parameters->add_delay);

	// MUL
	working_units = GetNumberOfWorkingExecUnits(mul_res_stations, cfg_parameters->mul_nr_reservation);
	SetReadyForExec(mul_res_stations, cfg_parameters->mul_nr_reservation, cycle);
	EnterToExec(mul_res_stations, cfg_parameters->mul_nr_reservation, cfg_parameters->mul_nr_units, working_units, cycle, cfg_parameters->mul_delay);

	// DIV
	working_units = GetNumberOfWorkingExecUnits(divide_res_stations, cfg_parameters->div_nr_reservation);
	SetReadyForExec(divide_res_stations, cfg_parameters->div_nr_reservation, cycle);
	EnterToExec(divide_res_stations, cfg_parameters->div_nr_reservation, cfg_parameters->div_nr_units, working_units, cycle, cfg_parameters->div_delay);

	// LOAD
	working_units = GetNumberOfWorkingExecUnits(load_res_stations, cfg_parameters->mem_nr_load_buffers);
	SetReadyForExec(load_res_stations, cfg_parameters->mem_nr_load_buffers, cycle);
	is_load_just_entered = EnterToExec(load_res_stations, cfg_parameters->mem_nr_load_buffers, LOAD_NR_UNITS, working_units, cycle, cfg_parameters->mem_delay);

	// STORE
	if (is_load_just_entered == false)
	{
		working_units = GetNumberOfWorkingExecUnits(store_res_stations, cfg_parameters->mem_nr_store_buffers);
		//mem_working_units = working_units + GetNumberOfWorkingExecUnits(load_res_stations, cfg_parameters->mem_nr_load_buffers);
		SetReadyForExec(store_res_stations, cfg_parameters->mem_nr_store_buffers, cycle);
		EnterToExec(store_res_stations, cfg_parameters->mem_nr_store_buffers, STORE_NR_UNITS, 0, cycle, cfg_parameters->mem_delay);
	}
	// CDBs

	DealWithCDB(cfg_parameters, cycle);
}

void DealWithCDB(CfgParameters *cfg_parameters, int cycle)
{
	bool is_load_in_CDB = false;
	CDB(cfg_parameters, add_sub_res_stations, cfg_parameters->add_nr_reservation, cycle);
	CDB(cfg_parameters, mul_res_stations, cfg_parameters->mul_nr_reservation, cycle);
	CDB(cfg_parameters, divide_res_stations, cfg_parameters->div_nr_reservation, cycle);
	is_load_in_CDB = CDB(cfg_parameters, load_res_stations, cfg_parameters->mem_nr_load_buffers, cycle);
	if (is_load_in_CDB == false) {
		CDB(cfg_parameters, store_res_stations, cfg_parameters->mem_nr_store_buffers, cycle);
	}
}

// clears resorvation station slot to its defaults (0, NULL and false).
// this is done once the slot is needed to be removed from a resorvation station.
//
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
	res_station[offset].is_load = false;
	res_station[offset].is_mult = false;
	res_station[offset].is_ready_for_exec = false;
	res_station[offset].is_store = false;
	res_station[offset].is_sub = false;
	res_station[offset].q_j = 0;
	res_station[offset].q_j_station_offset = 0;
	res_station[offset].q_k = 0;
	res_station[offset].q_k_station_offset = 0;
	res_station[offset].v_j = 0;
	res_station[offset].v_k = 0;
	res_station[offset].original_inst = 0;
	res_station[offset].PC = 0;
	res_station[offset].imm = 0;
}

// sets the "is_ready_for_exec" bit inside Station struct once the instruction is ready to enter the execution unit
//
// arguments:
// Station *res_station - one of the resorvation stations arrays
// int res_size - the size of the resorvation stations as in cfg.txt file.
// int cycle - the current cycle.
//
void SetReadyForExec(Station *res_station, int res_size, int cycle)
{
	int i;
	for (i = 0; i < res_size; i++)
	{
		if (res_station[i].is_busy == true && (cycle > res_station[i].cycle_entered) && res_station[i].is_in_exec == false && res_station[i].q_j == 0 && res_station[i].q_k == 0)
		{
			res_station[i].is_ready_for_exec = true;
		}
		else
		{
			res_station[i].is_ready_for_exec = false;
		}
	}
}

int GetNumberOfWorkingExecUnits(Station *res_station, int res_size)
{
	int i, counter = 0;
	for (i = 0; i < res_size; i++)
	{
		if (res_station[i].is_in_exec == true) {
			counter++;
		}
	}
	return counter;
}

bool EnterToExec(Station *res_station, int res_size, int nr_of_exec_units, int nr_of_working_exec_units, int cycle, int cycles_in_exec)
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
			res_station[i].cycle_to_finish_exec = cycle + cycles_in_exec - 1;

			issue_list[res_station[i].PC].cycle_execute_start = cycle;
			issue_list[res_station[i].PC].cycle_execute_end = res_station[i].cycle_to_finish_exec;

			nr_of_empty_exec_units--;
			return true;
		}
	}
	return false;
}

bool CDB(CfgParameters *cfg_parameters, Station *res_station, int size, int cycle)
{
	int i = 0;
	float dst_result = 0.0;
	int mem_read = 0;

	for (i = 0; i < size; i++)
	{
		if (cycle > res_station[i].cycle_to_finish_exec && res_station[i].is_in_exec == true)
		{
			res_station[i].is_in_exec = false;
			res_station[i].is_ready_for_exec = false;
			if (res_station[i].is_load == true || res_station[i].is_store == true)
			{
				dst_result = (float)DoMemCalc(&res_station[i]);
			}
			else
			{
				dst_result = DoArithmeticCalc(&res_station[i]);
			}
			if (res_station[i].is_store == true)
			{
				mem[(int)dst_result] = res_station[i].imm;
			}
			RemoveLabel(add_sub_res_stations, getReservationType(&res_station[i]), i, cfg_parameters->add_nr_reservation, dst_result);
			RemoveLabel(mul_res_stations, getReservationType(&res_station[i]), i, cfg_parameters->mul_nr_reservation, dst_result);
			RemoveLabel(divide_res_stations, getReservationType(&res_station[i]), i, cfg_parameters->div_nr_reservation, dst_result);
			RemoveLabel(load_res_stations, getReservationType(&res_station[i]), i, cfg_parameters->mem_nr_load_buffers, dst_result);
			RemoveLabel(store_res_stations, getReservationType(&res_station[i]), i, cfg_parameters->mem_nr_store_buffers, dst_result);

			issue_list[res_station[i].PC].cycle_write_cdb = cycle;

			PrintTo_tracecdb_file(files_struct.tracedb, res_station[i], i, cycle, dst_result);

			ClearResSlot(res_station, i);
			return true;
		}
	}
	return false;
}

float DoArithmeticCalc(Station *inst)
{
	float result = 0;
	if (inst->is_add == true)
	{
		return inst->v_j + inst->v_k;
	}
	else if (inst->is_sub == true)
	{
		return inst->v_j - inst->v_k;
	}
	else if (inst->is_mult == true)
	{
		return inst->v_j * inst->v_k;
	}
	return inst->v_j / inst->v_k;
}

int DoMemCalc(Station *inst)
{
	if (inst->is_load == true)
	{
		return mem[inst->imm];
	}
	return (int)registers[(int)inst->v_k].V;
}

void RemoveLabel(Station *res_station, int type, int offset, int res_size, float dst_result)
{
	int i;
	for (i = 0; i < res_size; i++)
	{
		if (res_station[i].q_j == type && res_station[i].q_j_station_offset == offset)
		{
			res_station[i].q_j = 0;
			res_station[i].q_j_station_offset = 0;
			res_station[i].v_j = dst_result;
		}
		if (res_station[i].q_k == type && res_station[i].q_k_station_offset == offset)
		{
			res_station[i].q_k = 0;
			res_station[i].q_k_station_offset = 0;
			res_station[i].v_k = dst_result;
		}
	}

	for (i = 0; i < NUM_OF_REGISTERS; i++)
	{
		if (registers[i].Q == type && registers[i].station_offset == offset)
		{
			registers[i].Q = 0;
			registers[i].station_offset = 0;
			registers[i].V = dst_result;
		}
	}
}

int getReservationType(Station *res_station)
{
	if (res_station->is_add == true || res_station->is_sub == true)
	{
		return ADD_SUB_RESORVATION_STATION;
	}
	else if (res_station->is_mult == true)
	{
		return MULT_RESORVATION_STATION;
	}
	else if (res_station->is_div == true)
	{
		return DIV_RESORVATION_STATION;
	}
	else if (res_station->is_load == true)
	{
		return LOAD_RESORVATION_STATION;
	}
	return STORE_RESORVATION_STATION;
}

int LastCDBCycle(CfgParameters *cfg_parameters)
{
	int max_values[NUMBER_OF_RES_STATIONS];
	max_values[0] = FindLastExecCycle(add_sub_res_stations, cfg_parameters->add_nr_reservation);
	max_values[1] = FindLastExecCycle(mul_res_stations, cfg_parameters->mul_nr_reservation);
	max_values[2] = FindLastExecCycle(divide_res_stations, cfg_parameters->div_nr_reservation);
	max_values[3] = FindLastExecCycle(load_res_stations, cfg_parameters->mem_nr_load_buffers);
	max_values[4] = FindLastExecCycle(store_res_stations, cfg_parameters->mem_nr_store_buffers);
	return getMax(max_values, NUMBER_OF_RES_STATIONS) + 1;
}
int FindLastExecCycle(Station *res_station, int size) {
	int i = 0, max_cycle = 0;
	for (i = 0; i < size; i++)
	{
		if (res_station[i].cycle_to_finish_exec > max_cycle)
		{
			max_cycle = res_station[i].cycle_to_finish_exec;
		}
	}
	return max_cycle;
}
int getMax(int num[], int size) {
	int result = 0, i; 
	for (i = 0; i < size; i++)
	{
		if (num[i] > result)
		{
			result = num[i];
		}
	}
	return result;
}

bool isBusy(CfgParameters *cfg_parameters)
{
	int total_busy = 0;
	total_busy =  getNumOfBusy(add_sub_res_stations, cfg_parameters->add_nr_reservation);
	total_busy += getNumOfBusy(mul_res_stations, cfg_parameters->mul_nr_reservation);
	total_busy += getNumOfBusy(divide_res_stations, cfg_parameters->div_nr_reservation);
	total_busy += getNumOfBusy(load_res_stations, cfg_parameters->mem_nr_load_buffers);
	total_busy += getNumOfBusy(store_res_stations, cfg_parameters->mem_nr_store_buffers);
	if (total_busy > 0)
	{
		return true;
	}
	return false;
}

int getNumOfBusy(Station *res_station, int size)
{
	int i, num_of_busy = 0;
	for (i = 0; i < size; i++)
	{
		if (res_station[i].is_busy == true)
		{
			num_of_busy++;
		}
	}
	return num_of_busy;
}

void PrintTo_tracecdb_file(FILE *tracecdb_file, Station res_station, int offset, int cycle, float data_on_cdb)
{
	fprintf(tracecdb_file, "%d ", cycle);
	fprintf(tracecdb_file, "%d ", res_station.PC);
	if (res_station.is_add || res_station.is_sub)
	{
		fprintf(tracecdb_file, "ADD ");
	}
	else if (res_station.is_mult)
	{
		fprintf(tracecdb_file, "MUL ");
	}
	else if (res_station.is_div)
	{
		fprintf(tracecdb_file, "DIV ");
	}
	else if (res_station.is_load || res_station.is_store)
	{
		fprintf(tracecdb_file, "MEM ");
	}

	fprintf(tracecdb_file, "%08f ", data_on_cdb);

	if (res_station.is_add || res_station.is_sub)
	{
		fprintf(tracecdb_file, "ADD");
	}
	else if (res_station.is_mult)
	{
		fprintf(tracecdb_file, "MUL");
	}
	else if (res_station.is_div)
	{
		fprintf(tracecdb_file, "DIV");
	}
	else if (res_station.is_load || res_station.is_store)
	{
		fprintf(tracecdb_file, "MEM");
	}
	fprintf(tracecdb_file, "%d", offset);
	fprintf(tracecdb_file, "\n");
}