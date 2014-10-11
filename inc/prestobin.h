#pragma once

#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#include "prestoapi.h"
#include "resource.h"

#ifdef PRESTOBINAPI
extern char _binary_res_prestobin_rc_bin_start;
extern char _binary_res_prestobin_rc_bin_end;
char * resource_offset;
#define INIT_RESOURCE() resource_offset = &_binary_res_prestobin_rc_bin_start;
#else
#define INIT_RESOURCE() 
#endif

#ifdef __LINUX
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#define MAKESIGNATURE(ch0, ch1, ch2, ch3)        \
	((unsigned int)(unsigned char)(ch0)        | \
	((unsigned int)(unsigned char)(ch1) << 8)  | \
	((unsigned int)(unsigned char)(ch2) << 16) | \
	((unsigned int)(unsigned char)(ch3) << 24))

#define PRINTBUFFSIZE 1024
#define INT_MAGIC 0x3B9ACA00
#define DIGIT_OFFSET 0x30

typedef struct RESOURCELIST_ {
	struct RESOURCELIST_ * next;
	unsigned int type;
	char * label;
	unsigned int id;
	char * data;
	unsigned int length;
} RESOURCELIST, *PRESOURCELIST, *LPRESOURCELIST;

typedef struct {
	PRESOURCELIST list;
	char * header_file;
	char * header_folder;
	char * header_data;
	char * resource_file;
	char * resource_folder;
	char * resource_data;
} WORKINGSET, *PWORKINGSET, *LPWORKINGSET;

PRESOURCELIST add_new_item(PWORKINGSET, int, char*);
void cleanup_workingset(PWORKINGSET);
//int		escape_sequence(char*, size_t, char*, size_t)
int  extract_rcdata(PWORKINGSET, char*, char*);
int  extract_string(PWORKINGSET, char*);
//int  folder_exists(char*);
//void free_headerlist(PWORKINGSET);
int  generate_resource_bin(PWORKINGSET);
int  get_folders_from_paths(char*, char**);
PRESOURCELIST label_to_resource(PWORKINGSET, char*);
char* load_rcdata(PWORKINGSET, char *, unsigned int *);
int   load_header_file(PWORKINGSET);
int   load_resource_file(PWORKINGSET);
void  print_error(unsigned int, ...);
void  print_output(PWORKINGSET, unsigned int, ...);
char* read_binary_file(char*, unsigned int*);
char* read_file(char*, char*, unsigned int*, int);
char* read_text_file(char*);
void  show_usage(void);
char* skip_label(char * s);
char* skip_whitespace(char * s);
int   strtouint(char*, unsigned int*);
int   unescape_sequence(char*, char*);

void test(void)
{
#if defined(PRESTOBINAPI) && !defined(__LINUX)
	char buffer[1024];
	int length;
	FILE * file;
	long size;
	file = fopen("res\\prestobin_rc.bin", "rb");
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);
	resource_offset = malloc(size);
	fread(resource_offset, 1, size, file);
	fclose(file);
	length = get_resource_size(IDD_USAGE, PREST_RCDATA);
	length = get_resource_value(IDD_USAGE, PREST_RCDATA, buffer, length);
	length = load_string(IDS_INVALID_ARGUMENT, buffer, 5);
	length = load_string(IDS_INVALID_ARGUMENT, buffer, 1024);
#endif
}
