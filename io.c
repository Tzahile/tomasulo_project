#define _CRT_SECURE_NO_WARNINGS
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "io.h"
#include "stations.h"

extern int mem[MEM_SIZE], last;
extern IssueList *issue_list;
extern Registers registers[NUM_OF_REGISTERS];
extern Files files_struct;

// opens all input and output files.
//
// arguments:
// Files *files_struct - struct containing all the files.
// char *argv[] - array of all files names to be opened.
//
// returns 0 (SUCCESS) if finished successfuly. otherwise, returns -1
// (ERROR_CODE).
bool OpenFiles(Files *files_struct, char *argv[])
{
	files_struct->cfg = fopen(argv[1], "r");
	files_struct->memin = fopen(argv[2], "r");
	files_struct->memout = fopen(argv[3], "w");
	files_struct->regout = fopen(argv[4], "w");
	files_struct->traceinst = fopen(argv[5], "w");
	files_struct->tracedb = fopen(argv[6], "w");
	if (files_struct->cfg == NULL || files_struct->memin == NULL || files_struct->memout == NULL || files_struct->regout == NULL || files_struct->traceinst == NULL || files_struct->tracedb == NULL)
	{
		return ERROR_CODE;
	}
	else
	{
		return SUCCESS;
	}
}

// reads the content of memin.txt and enters it to mem variable.
//
// arguments:
// Files *files - struct containing all the files (we need memin.txt).
//
void ReadMem(Files *files)
{
	int i = 0;
	while (!feof(files->memin))
	{
		if (fscanf(files->memin, "%08X\n", &mem[i]) != 1)
			break;
		i++;
	}
}

// finds the PC of HALT command and returns it.
//
int FindLastInstPC()
{
	int last = 0;
	while (last <= (MEM_SIZE)-1 && mem[last] != 0x06000000)
	{
		last++;
	}
	return last;
}

// returns the PC of last non zero memory
//
int FindLastNotZeroAddress()
{
	int last = (MEM_SIZE)-1;
	while (last >= 0 && mem[last] == 0)
	{
		last--;
	}
	return last;
}

// reads cfg.txt and sets all the parameters in a CfgParameters struct
//
// arguments:
// FILE *cfg_file - struct with all the opened files.
// CfgParameters *cfg_parameters (output) - the struct we put all the read
// parameters
//
void SetCfgParameters(FILE *cfg_file, CfgParameters *cfg_parameters)
{
	char temp1[100];
	int val;
	int i = 0;
	for (i = 0; i < NUM_OF_CFG_PARAMETERS; i++)
	{
		fscanf(cfg_file, "%s = %d\n", temp1, &val);
		if (strcmp(temp1, "add_nr_units") == 0)
		{
			cfg_parameters->add_nr_units = val;
		}
		else if (strcmp(temp1, "mul_nr_units") == 0)
		{
			cfg_parameters->mul_nr_units = val;
		}
		else if (strcmp(temp1, "div_nr_units") == 0)
		{
			cfg_parameters->div_nr_units = val;
		}
		else if (strcmp(temp1, "add_nr_reservation") == 0)
		{
			cfg_parameters->add_nr_reservation = val;
		}
		else if (strcmp(temp1, "mul_nr_reservation") == 0)
		{
			cfg_parameters->mul_nr_reservation = val;
		}
		else if (strcmp(temp1, "div_nr_reservation") == 0)
		{
			cfg_parameters->div_nr_reservation = val;
		}
		else if (strcmp(temp1, "add_delay") == 0)
		{
			cfg_parameters->add_delay = val;
		}
		else if (strcmp(temp1, "mul_delay") == 0)
		{
			cfg_parameters->mul_delay = val;
		}
		else if (strcmp(temp1, "div_delay") == 0)
		{
			cfg_parameters->div_delay = val;
		}
		else if (strcmp(temp1, "mem_delay") == 0)
		{
			cfg_parameters->mem_delay = val;
		}
		else if (strcmp(temp1, "mem_nr_load_buffers") == 0)
		{
			cfg_parameters->mem_nr_load_buffers = val;
		}
		else if (strcmp(temp1, "mem_nr_store_buffers") == 0)
		{
			cfg_parameters->mem_nr_store_buffers = val;
		}
	}

	// fscanf(cfg_file, "%s = %d\n", temp1, &(cfg_parameters->add_nr_units));
	// fscanf(cfg_file, "%s = %d\n", temp1, &(cfg_parameters->mul_nr_units));
	// fscanf(cfg_file, "%s = %d\n", temp1, &(cfg_parameters->div_nr_units));
	// fscanf(cfg_file, "%s = %d\n", temp1,
	// &(cfg_parameters->add_nr_reservation));  fscanf(cfg_file, "%s = %d\n",
	// temp1,
	// &(cfg_parameters->mul_nr_reservation));  fscanf(cfg_file, "%s = %d\n",
	// temp1,
	// &(cfg_parameters->div_nr_reservation));  fscanf(cfg_file, "%s = %d\n",
	// temp1,
	// &(cfg_parameters->add_delay));  fscanf(cfg_file, "%s = %d\n", temp1,
	// &(cfg_parameters->mul_delay));  fscanf(cfg_file, "%s = %d\n", temp1,
	// &(cfg_parameters->div_delay));  fscanf(cfg_file, "%s = %d\n", temp1,
	// &(cfg_parameters->mem_delay));  fscanf(cfg_file, "%s = %d\n", temp1,
	// &(cfg_parameters->mem_nr_load_buffers));  fscanf(cfg_file, "%s = %d\n",
	// temp1, &(cfg_parameters->mem_nr_store_buffers));
}

// printing to traceinst.txt file
//
// arguments:
// FILE *traceinst_file - pointer to traceinst.txt
//
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

// printing to regout.txt file
//
// arguments:
// FILE *regout_file - pointer to regout.txt
//
void PrintTo_regout_file(FILE *regout_file)
{
	int i, hex_value = 0;
	for (i = 0; i < NUM_OF_REGISTERS; i++)
	{
		fprintf(regout_file, "%.6f", registers[i].V);
		if (i != NUM_OF_REGISTERS - 1)
		{
			fprintf(regout_file, "\n");
		}
	}
}

// printing to memout.txt file
//
// arguments:
// FILE *memout_file - pointer to memout.txt
//
void PrintTo_memout_file(FILE *memout_file)
{
	int i;
	int last = FindLastNotZeroAddress();

	for (i = 0; i <= last; i++)
	{
		fprintf(memout_file, "%08x", mem[i]);
		if (i != last)
		{
			fprintf(memout_file, "\n");
		}
	}
}
// converts an single precision format to decimal number (4.5689 for example)
//
// arguments:
// int sign - the sign bit as presented in the single precision number.
// int exponent - the exponent part of the number.
// int fraction - the fraction part of the number.
//
// the function returns the decimal representation of the number.
//
float GetSinglePrecisionFormat(int sign, int exponent, int fraction)
{
	float result = 0;
	int i;
	for (i = 0; i < 23; i++)
	{
		if ((fraction & (1 << (22 - i))) > 0)
		{
			result += (float)pow(2, -(i + 1));
		}
	}
	return (float)(pow(-1, sign) * (1 + result) * pow(2, exponent - BIAS));
}

// converts an desimal number to its single precision format.
//
// arguments:
// float number - the decimal number we want to convert
//
// the function return 8 HEX digits, representation the number in single
// precision format.
int GetFloatToBin(float number)
{
	int sign = 0, i = 0, result = 0;
	int exponent = 0;
	float fraction = 0;
	if (number < 0)
	{
		sign = 1;
	}
	exponent = BIAS + (int)floor(log2(fabsf(number)));
	float absRes = fabsf(number);
	float powRes = powf(2.0, (float)(exponent - BIAS));
	fraction = (float)(absRes / powRes - 1.0);
	for (i = 1; i <= 23; i++)
	{
		if (fraction - powf(2.0, (float)-i) >= 0.0)
		{
			result = result | (1 << (22 - (i - 1)));
			fraction -= powf(2.0, (float)-i);
		}
	}
	exponent <<= 23;
	sign <<= 31;
	return (sign | exponent | result);
}