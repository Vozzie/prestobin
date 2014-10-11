#pragma once

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#include "prestoapi.h"
#include "resource.h"

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








/*
	//PHEADERLIST first, last; //, *tok, *delims, *chr;
	char *data, *chrptr, *tok, *keystart, *keyend, *valstart, *valend, *delims; 
	int size;

	data = read_text_file(pws->resource_file);
	if(data == NULL)  return EIO;

	delims = "\r\n";
	tok = strtok(data, delims);
	while(tok != NULL)
	{
		chrptr = tok;
		while(*chrptr && (*chrptr == ' ' || *chrptr == '\t')) chrptr++;
		keystart = chrptr;
		if(*chrptr) chrptr++;
		while(*chrptr && !(*chrptr == ' ' || *chrptr == '\t')) chrptr++;
		if(*chrptr)
		{
			keyend = chrptr;
			*keyend = 0; 
			valstart = strchr(chrptr + 1, '\"');
			valend = strrchr(chrptr + 1, '\"');
			if(valstart != NULL && valend != NULL)
			{

			}
		}
		tok = strtok(NULL, delims);
	}
*/
/*
delims = "\r\n";
tok = strtok(data, delims);
while(tok != NULL)
{
	while(*tok && !(*tok == '#' || *tok == ' ' || *tok == '\t')) tok++;
	if(stricmp(tok, "#define"))
	{
		while(*tok && !(*tok == ' ' || *tok == '\t')) tok++;
		while(*tok && (*tok == ' ' || *tok == '\t')) tok++;
		if(*tok)
		{
			last->key = malloc(strlen(tok) + 1);
			strcpy(last->key, tok);
			chr = strchr(last->key, ' ');
			if(!chr) chr = strchr(last->key, '\t');
			while(*tok && !(*tok == ' ' || *tok == '\t')) tok++;
			while(*tok && (*tok == ' ' || *tok == '\t')) tok++;
			if(chr == NULL || !*tok)
			{
				free(last->key);
				last->key = NULL;
			}
			else
			{
				*chr = 0;
				last->value = atoi(tok);
				last->pNext = malloc(sizeof(HEADERLIST));
				memset(last->pNext, 0, sizeof(HEADERLIST));
				last = last->pNext;
			}
		}
	}
	tok = strtok(NULL, delims);
}
*/
////// UNESCAPE OCTET \???
//if(*(copy + 1) && *(copy + 2))
//{// triplet octal sequence (000-777)?
//	for(a = 0, c = 0; c < 3; c++)
//	{
//		if(copy[c] < 0x30 || copy[c] > 0x37) break;
//		a <<= 3;
//		a |= (int)(copy[c] - 0x30);
//	}
//	if(c == 3)
//	{// Whatever it was, it's broken down to a byte 0-255
//		copy += 2;
//		*sequence++ = a & 0xFF;
//		break;
//	}
//}
//////
////// UNESCAPE HEX \x??
//for(a = 0, c = 1; c < 3; c++)
//{// hex
//	a <<= 4;
//	if(copy[c] > 0x29 && copy[c] < 0x40) a |= copy[c] - 0x30;
//	else if(copy[c] > 0x40 && copy[c] < 0x5B) a |= copy[c] - 0x34;
//	else if(copy[c] > 0x60 && copy[c] < 0x7B) a |= copy[c] - 0x54;
//	else break;
//}
//if(c == 3) { copy+= 2; *sequence++ = a & 0xFF; break; }
//////
// fall thru
//free(origin);
//return EINVAL;
