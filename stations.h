#ifndef STATIONS_H
#define STATIONS_H

#include "analyze_inst.h"
#include "io.h"
#include <stdbool.h>

typedef struct _Station {
  bool is_busy;
  char *command;
  float v_j;
  float v_k;
  int q_j;
  int q_j_station_offset;
  int q_k;
  int q_k_station_offset;
  int addr;
  bool is_load;
  bool is_store;
  bool is_add;
  bool is_sub;
  bool is_mult;
  bool is_div;
  bool is_halt;
  int cycle_entered;
  bool is_ready_for_exec;
  bool is_in_exec;
  int cycle_to_finish_exec;
  int original_inst;
  int issue_num;
  int PC;
  int imm;
} Station;

typedef struct _CDB_status {
  bool is_ADD_SUB_CDB_used;
  bool is_MUL_CDB_used;
  bool is_DIV_CDB_used;
  bool is_MEM_CDB_used;
} CDB_status;

typedef struct _IssueList {
  int original_inst;
  int PC;
  int tag;
  int offset;
  int cycle_issued;
  int cycle_execute_start;
  int cycle_execute_end;
  int cycle_write_cdb;
  bool is_busy;
} IssueList;

void PrepareReservationStations(CfgParameters *cfg_parameters);
void EnterToReservationStation(InstQueue inst, Station *res_station, int size,
                               int cycle, CfgParameters *cfg_parameters);
void Exec(CfgParameters *cfg_parameters, int cycle);
int LastCDBCycle(CfgParameters *cfg_parameters);
bool isBusy(CfgParameters *cfg_parameters);
void EnterToIssueList(Station *res_station, int offset, int cycle);
void PrintTo_tracecdb_file(FILE *tracecdb_file, Station res_station, int offset,
                           int cycle, float data_on_cdb);

#endif
