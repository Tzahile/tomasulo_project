#ifndef STATIONS_H
#define STATIONS_H

#include "analyze_inst.h"

typedef struct _Station {
	bool is_busy;
	char *command;
	double v_j;
	double v_k;
	int q_j;
	int q_j_station_offset;
	int q_k;
	int q_k_station_offset;
	int addr;
	bool is_ld;
	bool is_st;
	bool is_add;
	bool is_sub;
	bool is_mult;
	bool is_div;
	bool is_halt;
	int cycle_entered;
	bool is_ready_for_exec;
	bool is_in_exec;

	int cycle_to_finish_exec;
}Station;

void PrepareReservationStations(CfgParameters *cfg_parameters);
void EnterToReservationStation(InstQueue inst, Station *res_station, int *size, int cycle, CfgParameters *cfg_parameters);
void UpdateReservationStationsData(CfgParameters *cfg_parameters, int cycle, int add_sub_res_stations_size, int mul_res_stations_size,
	int divide_res_stations_size, int load_res_stations_size, int store_res_stations_size);

#endif
