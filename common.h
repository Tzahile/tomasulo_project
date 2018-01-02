#ifndef COMMON_H
#define COMMON_H

#define ERROR_CODE 1
#define SECCESS 0
#define NUM_OF_OP_CODES 7
#define NUM_OF_REGISTERS 16
#define MEM_SIZE 1<<12
#define NUM_OF_CFG_PARAMETERS 12
#define OP_CODE_MAX_CHARS 5
#define INST_QUEUE_SIZE 16

#define OP_LD 0
#define OP_ST 1
#define OP_ADD 2
#define OP_SUB  3
#define OP_MULT 4
#define OP_DIV 5
#define OP_HALT 6

#define ADD_SUB_RESORVATION_STATION 1
#define MULT_RESORVATION_STATION 2
#define DIV_RESORVATION_STATION 3

#endif