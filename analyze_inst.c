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
extern Station *add_sub_res_stations, *mul_res_stations, *divide_res_stations,
    *load_res_stations, *store_res_stations;
extern InstQueue inst_queue[INST_QUEUE_SIZE];
extern Registers registers[NUM_OF_REGISTERS];

void EnterToInstQueue(int inst, int op, int dst, int src0, int src1, int imm,
                      int inst_queue_size);
void UpdateRegisters(int station_name, int dst, int relevent_size);
void PopInstQueue(int *inst_queue_size);
int SearchFirstNonBusy(Station *res_station, int size);

// gets the value between lsd and msb bits of the number x.
int sbs(int x, int msb, int lsb) {
  if (msb == 31 && lsb == 0)
    return x;
  return (x >> lsb) & ((1 << (msb - lsb + 1)) - 1);
}
// the function returns all the fields of the instruction
void DecodeInst(int inst, int *op, int *dst, int *src0, int *src1, int *imm) {
  *op = sbs(inst, 27, 24);
  *dst = sbs(inst, 23, 20);
  *src0 = sbs(inst, 19, 16);
  *src1 = sbs(inst, 15, 12);
  *imm = sbs(inst, 11, 0);
}

// sets the register file struct to zeros (initializing it).
void InitRegistersStruct() {
  int i = 0;
  for (i = 0; i < NUM_OF_REGISTERS; i++) {
    registers[i].reg_name = reg_name[i];
    registers[i].V = (float)i;
    registers[i].Q = 0;
    registers[i].station_offset = 0;
  }
}

// Does the Fatch progress: Enters new instructions to inst queue (if enough
// place). if the instruction is halt, don't enter to inst queue.
//
// int last - the last inst address in memin.
// int *PC (output) - the PC of the current inst.
// int *inst_queue_size (output) - the current size of the inst queue.
// bool *is_halt (output) - will be 'true' if the current inst is HALT.
// otherwise, 'false'.
//
// return the number of new instruction entered to instruction queue.
int Fetch(int last, int *PC, int *inst_queue_size, bool *is_halt) {
  int op, dst, src0, src1, imm, inst, new_insts = 0;
  if (*is_halt == true) {
    return 0;
  }
  if (*inst_queue_size < INST_QUEUE_SIZE) {
    inst = mem[*PC];
    (*PC)++;
    new_insts++;
    DecodeInst(inst, &op, &dst, &src0, &src1, &imm);
    EnterToInstQueue(inst, op, dst, src0, src1, imm, *inst_queue_size);
    (*inst_queue_size)++;
    if (op == 6) {
      *is_halt = true;
    }
  }
  return new_insts;
}

// enter the instruction to instruction queue.
//
// arguments:
// int inst - the original instruction (8 HEX digits).
// int op - the op code of the instruction.
// int dst - the dst register number
// int src0 - the src0 register number
// int src1 - the src1 register number
// int imm - the imm value.
// int inst_queue_size - the index where to put the inst in the inst queue.
//
void EnterToInstQueue(int inst, int op, int dst, int src0, int src1, int imm,
                      int inst_queue_size) {
  inst_queue[inst_queue_size].original_inst = inst;
  inst_queue[inst_queue_size].op = op;
  inst_queue[inst_queue_size].dst = dst;
  inst_queue[inst_queue_size].src0 = src0;
  inst_queue[inst_queue_size].src1 = src1;
  inst_queue[inst_queue_size].imm = imm;
  inst_queue[inst_queue_size].is_busy = true;
}

// Doing the issue progress.
//
// arguments:
// CfgParameters *cfg_parameters - the struct conatining all parameters in the
// parameters file. int *inst_queue_size - the first usused alot in the inst
// queue. int cycle - current cycle
//
void Issue(CfgParameters *cfg_parameters, int *inst_queue_size, int cycle) {
  int op, dst, non_busy_offset = 0;
  if (inst_queue_size == 0) {
    return;
  }
  op = inst_queue[0].op;
  dst = inst_queue[0].dst;
  switch (op) {
  case OP_ADD:
  case OP_SUB:
    non_busy_offset = SearchFirstNonBusy(add_sub_res_stations,
                                         cfg_parameters->add_nr_reservation);
    if (non_busy_offset == RES_STAT_FULL) {
      return;
    }
    EnterToReservationStation(inst_queue[0], add_sub_res_stations,
                              non_busy_offset, cycle, cfg_parameters);
    UpdateRegisters(ADD_SUB_RESORVATION_STATION, dst, non_busy_offset);
    PopInstQueue(inst_queue_size);
    break;
  case OP_MULT:
    non_busy_offset = SearchFirstNonBusy(mul_res_stations,
                                         cfg_parameters->mul_nr_reservation);
    if (non_busy_offset == RES_STAT_FULL) {
      return;
    }
    EnterToReservationStation(inst_queue[0], mul_res_stations, non_busy_offset,
                              cycle, cfg_parameters);
    UpdateRegisters(MULT_RESORVATION_STATION, dst, non_busy_offset);
    PopInstQueue(inst_queue_size);
    break;
  case OP_DIV:
    non_busy_offset = SearchFirstNonBusy(divide_res_stations,
                                         cfg_parameters->div_nr_reservation);
    if (non_busy_offset == RES_STAT_FULL) {
      return;
    }
    EnterToReservationStation(inst_queue[0], divide_res_stations,
                              non_busy_offset, cycle, cfg_parameters);
    UpdateRegisters(DIV_RESORVATION_STATION, dst, non_busy_offset);
    PopInstQueue(inst_queue_size);
    break;
  case OP_LOAD:
    non_busy_offset = SearchFirstNonBusy(load_res_stations,
                                         cfg_parameters->mem_nr_load_buffers);
    if (non_busy_offset == RES_STAT_FULL) {
      return;
    }
    EnterToReservationStation(inst_queue[0], load_res_stations, non_busy_offset,
                              cycle, cfg_parameters);
    UpdateRegisters(LOAD_RESORVATION_STATION, dst, non_busy_offset);
    PopInstQueue(inst_queue_size);
    break;
  case OP_STORE:
    non_busy_offset = SearchFirstNonBusy(store_res_stations,
                                         cfg_parameters->mem_nr_store_buffers);
    if (non_busy_offset == RES_STAT_FULL) {
      return;
    }
    EnterToReservationStation(inst_queue[0], store_res_stations,
                              non_busy_offset, cycle, cfg_parameters);
    PopInstQueue(inst_queue_size);
    break;
  case OP_HALT:
    break;
  }
}

// finds the first unused (not busy) slot in a reservation station.
//
// arguments:
// Station *res_station - the reservation station we want to find the first not
// busy slot. int size - the maximum size of this reservation station (as in
// parameters file).
//
// returns the first free slot in the reservation station
int SearchFirstNonBusy(Station *res_station, int size) {
  int offset = 0;
  for (offset = 0; offset < size; offset++) {
    if (res_station[offset].is_busy == false)
      return offset;
  }
  return RES_STAT_FULL;
}

// sets the Q and station_offset in the register struct array
//
// arguments:
// int station_name - the Q (TAG)
// int dst - the register we want to change
// relevent_size - the offset in the TAG (ADD1 vs ADD2 for example).
//
void UpdateRegisters(int station_name, int dst, int relevent_size) {
  registers[dst].Q = station_name;
  registers[dst].station_offset = relevent_size;
}

// Pops the first inst in the inst queue, once this inst was issued.
//
// arguments:
// int *inst_queue_size - the current size of the inst queue.
//
void PopInstQueue(int *inst_queue_size) {
  int i;
  for (i = 1; i < INST_QUEUE_SIZE; i++) {
    inst_queue[i - 1] = inst_queue[i];
  }
  (*inst_queue_size)--;
}
