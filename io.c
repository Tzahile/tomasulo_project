#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>

#include "io.h"
#include "common.h"

extern int mem[MEM_SIZE];

bool OpenFiles(Files *files_struct, char *argv[]){
	 files_struct->cfg = fopen(argv[1], "r");
	 files_struct->memin = fopen(argv[2], "r");
	 files_struct->memout = fopen(argv[3], "w");
	 files_struct->regout = fopen(argv[4], "w");
	 files_struct->traceinst = fopen(argv[5], "w");
	 files_struct->tracedb = fopen(argv[5], "w");
	 if (files_struct->cfg == NULL || files_struct->memin == NULL || files_struct->memout == NULL ||
		 files_struct->regout == NULL || files_struct->traceinst == NULL || files_struct->tracedb == NULL) {
		return ERROR_CODE;
	 }
	 else {
		return SECCESS;
	}
}

void ReadMem(Files *files) {
	int i = 0;
	while (!feof(files->memin)) {
		if (fscanf(files->memin, "%08X\n", &mem[i]) != 1)
			break;
		i++;
	}
}

int FindLastNotZeroAddress() {
	int last = (MEM_SIZE) - 1;
	while (last >= 0 && mem[last] == 0) {
		last--;
	}
	return last;
}

void SetCfgParameters(FILE *cfg_file, CfgParameters *cfg_parameters) {
	char temp1[100];
	int i = 0;
	fscanf(cfg_file, "%s = %d\n", temp1, &(cfg_parameters->add_nr_units));
	fscanf(cfg_file, "%s = %d\n", temp1, &(cfg_parameters->mul_nr_units));
	fscanf(cfg_file, "%s = %d\n", temp1, &(cfg_parameters->div_nr_units));
	fscanf(cfg_file, "%s = %d\n", temp1, &(cfg_parameters->add_nr_reservation));
	fscanf(cfg_file, "%s = %d\n", temp1, &(cfg_parameters->mul_nr_reservation));
	fscanf(cfg_file, "%s = %d\n", temp1, &(cfg_parameters->div_nr_reservation));
	fscanf(cfg_file, "%s = %d\n", temp1, &(cfg_parameters->add_delay));
	fscanf(cfg_file, "%s = %d\n", temp1, &(cfg_parameters->mul_delay));
	fscanf(cfg_file, "%s = %d\n", temp1, &(cfg_parameters->div_delay));
	fscanf(cfg_file, "%s = %d\n", temp1, &(cfg_parameters->mem_delay));
	fscanf(cfg_file, "%s = %d\n", temp1, &(cfg_parameters->mem_nr_load_buffers));
	fscanf(cfg_file, "%s = %d\n", temp1, &(cfg_parameters->mem_nr_store_buffers));

}