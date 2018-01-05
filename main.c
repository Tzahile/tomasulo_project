#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

#include "common.h"
#include "io.h"
#include "analyze_inst.h"
#include "stations.h"

char op_name[][NUM_OF_OP_CODES] = { "LD", "ST", "ADD", "SUB", "MULT", "DIV", "HALT" };
char reg_name[][NUM_OF_REGISTERS] = { "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
									  "F13", "F14", "F15" };
float reg_values[NUM_OF_REGISTERS] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
int mem[MEM_SIZE] = { 0 }, last = 0;
Station *add_sub_res_stations = NULL, *mul_res_stations = NULL, *divide_res_stations = NULL, *load_res_stations = NULL,
		*store_res_stations = NULL;
InstQueue inst_queue[INST_QUEUE_SIZE] = { 0 };
Registers registers[NUM_OF_REGISTERS] = { 0 };
CDB_status CDB_status_var = { false };
IssueList *issue_list = { 0 };
Files files_struct = { NULL };



int main(int argc, char *argv[]) {
	//Files files_struct;
	CfgParameters cfg_parameters;
	bool is_add_exec_occupied = false, is_mult_exec_occupied = false, is_div_exec_occupied = false;
	int nr_instrs_read = 0;
	int err_code = SUCCESS; //last = 0;
	int cycle = 0;
	int PC = 0;
	int inst_queue_size = 0, add_sub_res_stations_size = 0, mul_res_stations_size = 0,
		divide_res_stations_size = 0, load_res_stations_size = 0, store_res_stations_size = 0;
	err_code = OpenFiles(&files_struct, argv);
	if (err_code == ERROR_CODE) {
		return ERROR_CODE;
	}
	SetCfgParameters(files_struct.cfg, &cfg_parameters);
	fclose(files_struct.cfg);
	PrepareReservationStations(&cfg_parameters);
	InitRegistersStruct();
	ReadMem(&files_struct);
	last = FindLastInstPC();
	issue_list = (IssueList *)calloc(last +	1, sizeof(IssueList));
	PC = 0;

	// cycle 0
	nr_instrs_read = Fetch(last, &PC, &inst_queue_size);
	nr_instrs_read += Fetch(last, &PC, &inst_queue_size);
	cycle++;
	// end of cycle 0
	while (isBusy(&cfg_parameters) || cycle == 1) {
		// cycle 1, 2, 3, 4, ...
		Issue(&cfg_parameters, &inst_queue_size, cycle);
		Issue(&cfg_parameters, &inst_queue_size, cycle);
		Exec(&cfg_parameters, cycle);
		nr_instrs_read += Fetch(last, &PC, &inst_queue_size);
		nr_instrs_read += Fetch(last, &PC, &inst_queue_size);
		// end of cycle 1, 2, 3, 4, ...
		cycle++;
	}
	PrintTo_traceinst_file(files_struct.traceinst);
	PrintTo_regout_file(files_struct.regout);
	PrintTo_memout_file(files_struct.memout);
	fclose(files_struct.traceinst);
	fclose(files_struct.regout);
	fclose(files_struct.memout);
	fclose(files_struct.tracedb);
	return SUCCESS;
}