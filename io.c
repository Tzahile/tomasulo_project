#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "io.h"
#include "common.h"
#include "stations.h"

extern int mem[MEM_SIZE], last;
extern IssueList *issue_list;
extern Registers registers[NUM_OF_REGISTERS];
extern Files files_struct;

bool OpenFiles(Files *files_struct, char *argv[]){
	 files_struct->cfg = fopen(argv[1], "r");
	 files_struct->memin = fopen(argv[2], "r");
	 files_struct->memout = fopen(argv[3], "w");
	 files_struct->regout = fopen(argv[4], "w");
	 files_struct->traceinst = fopen(argv[5], "w");
	 files_struct->tracedb = fopen(argv[6], "w");
	 if (files_struct->cfg == NULL || files_struct->memin == NULL || files_struct->memout == NULL ||
		 files_struct->regout == NULL || files_struct->traceinst == NULL || files_struct->tracedb == NULL) {
		return ERROR_CODE;
	 }
	 else {
		return SUCCESS;
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

int FindLastInstPC() {
	int last = 0;
	while (last <= (MEM_SIZE) - 1 && mem[last] != 0) {
	last++;
	}
	return last - 1;
}

int FindLastNotZeroAddress()
{
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

void PrintTo_traceinst_file(FILE *traceinst_file)
{
	int i;
	char temp[100];
	for (i = 0; i < last; i++)
	{
		sprintf(temp, "%08x", issue_list[i].original_inst);
		fprintf(traceinst_file, "%s ", temp);
		sprintf(temp, "%d", issue_list[i].PC);
		fprintf(traceinst_file, "%s ", temp);
		switch (issue_list[i].tag)
		{
		case ADD_SUB_RESORVATION_STATION:
			fprintf(traceinst_file, "ADD");
			break;
		case MULT_RESORVATION_STATION:
			fprintf(traceinst_file, "MUL");
			break;
		case DIV_RESORVATION_STATION:
			fprintf(traceinst_file, "DIV");
			break;
		case STORE_RESORVATION_STATION:
			fprintf(traceinst_file, "ST");
			break;
		case LOAD_RESORVATION_STATION:
			fprintf(traceinst_file, "LD");
			break;
		}
		fprintf(traceinst_file, "%d ", issue_list[i].offset);
		sprintf(temp, "%d ", issue_list[i].cycle_issued);
		fprintf(traceinst_file, "%s", temp);
		sprintf(temp, "%d ", issue_list[i].cycle_execute_start);
		fprintf(traceinst_file, "%s", temp);
		sprintf(temp, "%d ", issue_list[i].cycle_execute_end);
		fprintf(traceinst_file, "%s", temp);
		sprintf(temp, "%d", issue_list[i].cycle_write_cdb);
		fprintf(traceinst_file, "%s", temp);
		if (i != last - 1)
		{
			fprintf(traceinst_file, "\n");
		}
	}
}

void PrintTo_regout_file(FILE *traceinst_file)
{
	int i;
	for (i = 0; i < NUM_OF_REGISTERS; i++)
	{
		fprintf(traceinst_file, "%.6f", registers[i].V);
		if (i != NUM_OF_REGISTERS - 1)
		{
			fprintf(traceinst_file, "\n");
		}
	}
}

void PrintTo_memout_file(FILE *traceinst_file)
{
	int i;
	int last = FindLastNotZeroAddress();

	for (i = 0; i <= last; i++)
	{
		fprintf(traceinst_file, "%08x", mem[i]);
		if (i != last)
		{
			fprintf(traceinst_file, "\n");
		}
	}
}