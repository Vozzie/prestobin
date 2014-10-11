
#include "prestobin.h"


int main(int argc, char * argv[])
{
	WORKINGSET ws;
	char * extension;
	int result;
	int i;
	INIT_RESOURCE();
	if(argc <= 1)
	{
		show_usage();
		return 0;
	}
	memset(&ws, 0, sizeof(WORKINGSET));
	for(i = 1; i < argc; i++)
	{		
		extension = strrchr(argv[i], '.');
		if(extension != NULL) 
		{
			if(stricmp(extension, ".h") == 0)  { ws.header_file = argv[i]; continue; }
			else if(stricmp(extension, ".rc") == 0) { ws.resource_file = argv[i]; continue; }
		}
		//else if(folder_exists(argv[i]))	{ ws.extra_dir = argv[i]; continue;	}
		print_error(IDS_INVALID_ARGUMENT, argv[i]);
		show_usage();
		return 0; 
	}
		
	if(ws.resource_file == NULL) 
	{ 
		print_error(IDS_MISSING_RESOURCE);
		show_usage();
		return 0; 
	}


	if(ws.header_file != NULL)
	{
		if(load_header_file(&ws))
		{
			cleanup_workingset(&ws);
			return 0;
		}
		if(get_folders_from_paths(ws.header_file, &ws.header_folder))
		{
			print_error(IDS_OUTOFMEMORY);
			cleanup_workingset(&ws);
			return 0;
		}
	}

	if(get_folders_from_paths(ws.resource_file , &ws.resource_folder))
	{
		print_error(IDS_OUTOFMEMORY);
		cleanup_workingset(&ws);
		return 0;
	}

	if(load_resource_file(&ws))
	{
		cleanup_workingset(&ws);
		return 0;	
	}

	result = generate_resource_bin(&ws);
	cleanup_workingset(&ws);
	return result == 0 ? 1 : 0;
}

PRESOURCELIST add_new_item(WORKINGSET *pws, int id, char* label)
{
	PRESOURCELIST list, item;
	item = malloc(sizeof(RESOURCELIST));
	if(item == NULL)
	{
		return NULL;
	}
	memset(item, 0, sizeof(RESOURCELIST));
	item->id = id;
	item->label = label;
	// Add to list (insert would be mor ez, but keep order...)
	if(pws->list == NULL)
	{
		pws->list = item;
	}
	else 
	{
		list = pws->list;
		while(list->next) list = list->next;
		list->next = item;
	}
	return item;
}

void cleanup_workingset(PWORKINGSET pws)
{
	PRESOURCELIST list;
	while(pws->list != NULL)
	{
		if(pws->list->data != NULL) free(pws->list->data);
		list = pws->list->next;
		free(pws->list);
		pws->list = list;
	}
	if(pws->header_folder != NULL) free(pws->header_folder);
	if(pws->header_data != NULL) free(pws->header_data);
	if(pws->resource_folder != NULL) free(pws->resource_folder);
	if(pws->resource_data != NULL) free(pws->resource_data);
}

/*int escape_sequence(char * sequence, size_t sequence_size, char * buffer, size_t buffer_size)
{
	unsigned int i;
	char c, * origin;
	origin = buffer;
	for( i = 0; i < sequence_size; i++ )
	{
		c = *(sequence + i);
		switch(c)
		{
		case 0x07: c = 'a'; break;
		case 0x08: c = 'b'; break;
		case 0x1B: c = 'e'; break;
		case 0x0C: c = 'f'; break;
		case 0x0A: c = 'n'; break;
		case 0x0D: c = 'r'; break;
		case 0x09: c = 't'; break;
		case 0x0B: c = 'v'; break;
		case 0x3F: c = '?'; break;
		case 0x5C: c = '\\'; break;
		case 0x27: c = '\''; break;
		case 0x22: c = '\"'; break;
		//case 'u':
		default:
			//if( c < 0x20 || c >= 0x7F)
			if( c < 0x20 || c == 0x7F || c == 0xFF )
			{
				if(buffer - origin + 5 > buffer_size) return ERANGE;

				sprintf(buffer, "\\x%02X", (int)c);
				buffer += 4;
			}
			else if(buffer - origin + 2 > buffer_size) return ERANGE;
			else *buffer++ = c;
			continue;
		}
		if(buffer - origin + 3 > buffer_size) return ERANGE;
		*buffer++ = '\\';
		*buffer++ = c;
	}
	*buffer = 0;
	return (buffer - origin) * -1;
}*/

int extract_rcdata(WORKINGSET *pws, char *chrptr, char *rcdata)
{
	PRESOURCELIST item;
	char *label, *origin, *data;
	unsigned int id, length;
	label = origin = chrptr;
	chrptr = skip_label(chrptr);
	*chrptr = 0;
	rcdata = skip_label(rcdata);
	rcdata = skip_whitespace(rcdata);
	chrptr = strrchr(rcdata, '\"');
	if(chrptr == NULL || *rcdata != '\"' || *chrptr != '\"' || chrptr == rcdata)
	{
		print_error(IDS_INVALID_RCDATAENTRY, origin);
		return EINVAL;
	}
	*chrptr = 0;
	rcdata++;
	if(strtouint(label, &id)) item = label_to_resource(pws, label);
	else item = add_new_item(pws, id, label);
	if( item == NULL )
	{
		print_error(IDS_INVALID_RCDATAENTRY, origin);
		return ENOMEM;
	}
	data = load_rcdata(pws, rcdata, &length);
	if(data == NULL)
	{
		print_error(IDS_LOAD_RCDATA_ERROR, origin);
		return EINVAL;
	}
	item->type = (unsigned int)PREST_RCDATA;
	item->data = data;
	item->length = length;
	return 0;
}

int extract_string(PWORKINGSET pws, char * chrptr)
{
	char *label, *value, *origin, *copy;
	PRESOURCELIST item;
	unsigned int id;
	// Get id or label
	origin = label = chrptr;
	chrptr = skip_label(chrptr);
	if(!*chrptr)
	{
		print_error(IDS_INVALID_STRINGENTRY, origin);
		return EINVAL;
	}
	*chrptr++ = 0;
	// Get the value
	if((value = strchr(chrptr, '\"')) == NULL
	||(chrptr = strrchr(value, '\"')) == NULL
	|| chrptr == value)
	{
		print_error(IDS_INVALID_STRINGENTRY, origin);
		return EINVAL;
	}
	value++;
	*chrptr = 0;
	// Cast to uint, on invalid number so search in list
	if(strtouint(label, &id)) item = label_to_resource(pws, label);
	else item = add_new_item(pws, id, label);
	copy = malloc(strlen(value) + 1);
	// Item found or created?
	if( item == NULL || copy == NULL)
	{
		print_error(IDS_OUTOFMEMORY);
		return ENOMEM;
	}
	if(0 < unescape_sequence(value, copy))
	{
		print_error(IDS_EXCEPTION_STRINGENTRY, origin);
		return EINVAL;
	}
	item->type = (unsigned int)PREST_STRING;
	item->data = copy;
	item->length = strlen(copy) + 1; // include null char?
	return 0;
}
/*
int folder_exists(char * folder)
{
	struct _stat s;
	int err;
	err = _stat(folder, &s);
	if(err) return 0;
	if(S_ISDIR(s.st_mode)) return 1;
	return 0;
}

void free_headerlist(PWORKINGSET pws)
{
	PRESOURCELIST temp;
	while(pws->list != NULL)
	{
		temp = pws->list->next;
		free(pws->list);
		pws->list = temp;
	}
}
*/

int generate_resource_bin(PWORKINGSET pws)
{
	PRESOURCEHEADER header;
	PRESOURCELIST list;
	RESOURCE resource;
	struct stat st;
	char *filename, *chrptr;
	FILE *file;
	int length;
	unsigned int offset;
	length = strlen(pws->resource_file);
	filename = malloc(length + 4);
	if( filename == NULL)
	{
		print_error(IDS_OUTOFMEMORY);
		return ENOMEM;
	}
	strcpy(filename, pws->resource_file);
	chrptr = filename + length - 1;
	while(*chrptr && *chrptr != '\\'&& *chrptr != '/') { if(*chrptr == '.') *chrptr = '_'; chrptr--; }
	strcat(filename, ".bin");
	if(stat(filename, &st))
	{
		// exists
	}
	file = fopen(filename, "wb");
	if(file == NULL)
	{
		print_error(IDS_CREATEFILE_ERROR, filename);
		// could not create file
		return EINVAL;
	}
	header = malloc(sizeof(RESOURCEHEADER));
	if(header == NULL) 
	{
		print_error(IDS_OUTOFMEMORY);
		return ENOMEM;
	}
	memset(header, 0, sizeof(RESOURCEHEADER));
	header->version = 0;
	header->signature = MAKESIGNATURE('S', 'o', 'V', '!');
	header->count = 0;
	list = pws->list;
	while(list)
	{ 
		if(list->data != NULL) 
		{
			header->count++;
		}
		else
		{
			print_error(IDS_ENTRY_NODATA, list->id, list->label);
		}
		list = list->next; 
	}
	fwrite(header, sizeof(RESOURCEHEADER), 1, file);
	list = pws->list;
	offset = sizeof(RESOURCEHEADER) + sizeof(RESOURCE) * header->count;
	while(list)
	{
		if(list->data != NULL)
		{
			resource.id = list->id;
			resource.type = list->type;
			resource.offset = offset;
			offset += resource.length = list->length;
			fwrite(&resource, sizeof(RESOURCE), 1, file);
		}
		list = list->next;
	}
	list = pws->list;
	while(list)
	{
		if(list->data != NULL)
		{
			switch(list->type){
			case PREST_RCDATA: print_output(pws, IDS_WRITING_RCDATAENTRY, list->id, list->label); break;
			case PREST_STRING: print_output(pws, IDS_WRITING_STRINGENTRY, list->id, list->label); break;
			}
			fwrite(list->data, 1, list->length, file);
		}
		list = list->next;
	}
	fclose(file);
	return 0;
}

int	get_folders_from_paths(char * psource, char ** pptarget)
{
	char *chrptr, oldchr;
	chrptr = strrchr(psource, '\\');
	if(!chrptr) chrptr = strrchr(psource, '/');
	if(chrptr)
	{
		*pptarget = malloc((++chrptr - psource) + 1);
		if(*pptarget == NULL) return ENOMEM;
		oldchr = *chrptr;
		*chrptr = 0;
		strcpy(*pptarget, psource);
		*chrptr = oldchr;
	}
	return 0;
}

PRESOURCELIST label_to_resource(PWORKINGSET pws, char * label)
{
	PRESOURCELIST list;
	list = pws->list;
	while(list)
	{
		if(strcmp(list->label, label) == 0)
		{
			return list;
		}
		list = list->next;
	}
	return NULL;
}

char* load_rcdata(WORKINGSET *pws, char *filename, unsigned int *size)
{
	char *path, *data;
	if(pws->resource_folder == NULL)
	{
		path = malloc(strlen(filename) + 1);
		strcpy(path, filename);
	}
	else
	{
		path = malloc(strlen(pws->resource_folder) + strlen(filename) + 1);
		strcpy(path, pws->resource_folder);
		strcat(path, filename);
	}
	if(path == NULL)
	{
		// err
	}
	data = read_binary_file(path, size);
	free(path);
	return data;
}

int load_header_file(PWORKINGSET pws)
{
	char *data, *chrptr, *tok, *label, *value, *delims; 
	PRESOURCELIST item;
	unsigned int id;
	pws->header_data = data = read_text_file(pws->header_file);
	if(data == NULL)
	{
		print_error(IDS_READFILE_ERROR, pws->header_file);
		return EIO;
	}
	delims = "\r\n";
	tok = strtok(data, delims);
	while(tok != NULL)
	{
		chrptr = tok;
		chrptr = strstr(chrptr, "#define");
		if(chrptr)
		{
			chrptr = skip_whitespace(chrptr + 7);
			label = chrptr;
			chrptr = skip_label(chrptr);
			if(*chrptr)
			{	
				*chrptr = 0;		
				value = chrptr = skip_whitespace(chrptr + 1);
				while(*chrptr && !strchr("\t /;", *chrptr)) chrptr++;
				if(*value && chrptr > value)
				{
					*chrptr = 0;
					if(!strtouint(value, &id))
					{
						item = add_new_item(pws, id, label);
					}
				}
			}
		}
		tok = strtok(NULL, delims);
	}
	return 0;
}

int load_resource_file(PWORKINGSET pws)
{
	char *data, *token, *delim, *chrptr;
	int status;
	pws->resource_data = data = read_text_file(pws->resource_file);
	if( data == NULL)
	{
		print_error(IDS_READFILE_ERROR, pws->resource_file);
		return EIO;
	}
	delim = "\r\n";
	status = 0; // 0 = normal; 1 = processing string table
	token = strtok(data, delim);
	while(token != NULL)
	{
		token = skip_whitespace(token);
		if(*token)
		{
			if(status == 1)
			{
				if(*token == '}')
				{
					status = 0;
				}
				else if(*token != '{')
				{
					if(extract_string(pws, token)) return EINVAL;
				}
			}
			else if(strncmp(token, "STRINGTABLE", 11) == 0)
			{
				status = 1;
			}
			else if((chrptr = strstr(token, "RCDATA")) != NULL && chrptr > token)
			{
				if(extract_rcdata(pws, token, chrptr)) return EINVAL;
			}
		}
		token = strtok(NULL, delim);
	}
	return 0;
}

void print_output(WORKINGSET *pws, unsigned int id, ...)
{
	va_list args;
	char buffer[PRINTBUFFSIZE];
	int length;
	length = load_string(id, buffer, PRINTBUFFSIZE);
	if(length != 0 && length < (PRINTBUFFSIZE - 1))
	{
		va_start (args, id);
		vfprintf(stdout, buffer, args);
		va_end (args);
	}
	else
	{		
		fprintf(stderr, "Can't load resource string in print_error().\n");
		exit(-1);
	}
}

void print_error(unsigned int id, ...)
{
	va_list args;
	char buffer[PRINTBUFFSIZE];
	int length;
	length = load_string(id, buffer, PRINTBUFFSIZE);
	if(length != 0 && length < (PRINTBUFFSIZE - 1))
	{
		va_start (args, id);
		vfprintf(stderr, buffer, args);
		va_end (args);
	}
	else
	{		
		fprintf(stderr, "Can't load resource string in print_error().");
		exit(-1);
	}
}

char * read_binary_file(char * filename, unsigned int * length)
{
	return read_file(filename, "rb", length, 0);
}

char * read_file(char * filename, char * options, unsigned int *length, int add_zero)
{
	FILE * file;
	char * result;
	result = NULL;
	file = fopen(filename, options);
	if(file == NULL) return NULL;
	fseek(file, 0, SEEK_END);
	*length = ftell(file);
	fseek(file, 0, SEEK_SET);
	if(*length > 0)
	{
		result = malloc(*length + (add_zero ? 1 : 0));
		if(result != NULL)
		{
			if(fread(result, 1, *length, file) <= 0)
			{
				free(result);
				result = NULL;
			}else if(add_zero) result[*length] = 0;
		}
	}
	fclose(file);
	if( result == NULL) *length = 0;
	return result;
}

char * read_text_file(char * filename)
{
	unsigned int length;
	return read_file(filename, "rb", &length, 1);
}

void show_usage(void)
{
	unsigned int size;
	char * buffer;
	size = get_resource_size(IDD_USAGE, PREST_RCDATA);
	if(size == 0) return;
	buffer = malloc(size + 1);
	if(buffer == NULL) return;
	if(get_resource_value(IDD_USAGE, PREST_RCDATA, buffer, size))
	{
		buffer[size] = 0;
		printf("%s", buffer);
	}
	free(buffer);
}

char * skip_label(char *s)
{
	if(s == NULL) return NULL;
	while(*s && (isalpha(*s) || isdigit(*s) || *s == '_')) s++;
	return s;
}

char * skip_whitespace(char *s)
{
	if(s == NULL) return NULL;
	while(*s && (*s == ' ' || *s == '\t')) s++;
	return s;
}

int strtouint(char * s, unsigned int *puint)
{
	unsigned int v, m;
	char * p;
	// empty string, invalid pointer?
	if(s == NULL || puint == NULL || !*s) return EINVAL;
	// skip whitespace
	while(strchr("\t ", *s)) s++;
	// remove plus if any
	if(*s == '+') s++;
	// init
	*puint = 0;
	p = s;
	// remove trailing zeroes
	while(*p == '0') p++;
	// empty string left?
	if(*p == 0 && p - s > 0) return 0;
	else s = p;
	while(isdigit(*p)) p++;
	// no digits?
	if(s == p) return EINVAL; 
	// remove trailing zeroes
	while(*s == '0') s++;
	// empty string left?
	if(!*s) return EINVAL;
	// init
	*puint = 0;
	m = 1;
	// go to end of digits
	p = s;
	while(isdigit(*p)) p++;
	// no digits?
	if(p == s) return EINVAL;
	// convert chars
	while(s <= --p) 
	{
		// digit to value
		v = (*p - DIGIT_OFFSET) * m;
		// overflow?
		if(*puint > UINT_MAX - v
		|| (m == INT_MAGIC && p != s))
			return E2BIG;
		*puint += v;
		m *= 10;
	}
	return 0;
}

int unescape_sequence(char * sequence, char * buffer)
{
	char * origin;
	if(sequence == NULL || buffer == NULL) return EINVAL;
	for( origin = buffer ; *sequence ; sequence++)
	{
		switch(*sequence)
		{
		case '\\':
			switch(*++sequence)
			{
			case 'a':  *buffer++ = 0x07; break; // Alarm
			case 'b':  *buffer++ = 0x08; break; // Bakcspace
			case 'e':  *buffer++ = 0x1B; break; // Err
			case 'f':  *buffer++ = 0x0C; break; // Formfeed
			case 'n':  *buffer++ = 0x0A; break; // Newline
			case 'r':  *buffer++ = 0x0D; break; // Carriage Return
			case 't':  *buffer++ = 0x09; break; // HTab
			case 'v':  *buffer++ = 0x0B; break; // VTab
			case '?':  *buffer++ = 0x3F; break; // Question mark
			case '\\': *buffer++ = 0x5C; break; // Backslash
			case '\'': *buffer++ = 0x27; break; // Single Quote
			case '\"': *buffer++ = 0x22; break; // Double Quote
			case 'x': // HEX \xHH
				if(sequence[1] && sequence[2])
				{
					*buffer = 0;
					if(sequence[1] > 0x29 && sequence[1] < 0x40) *buffer |= sequence[1] - 0x30;
					else if(sequence[1] > 0x40 && sequence[1] < 0x5B) *buffer |= sequence[1] - 0x34;
					else if(sequence[1] > 0x60 && sequence[1] < 0x7B) *buffer |= sequence[1] - 0x54;
					else return EINVAL;
					*buffer <<= 4;
					if(sequence[2] > 0x29 && sequence[2] < 0x40) *buffer |= sequence[2] - 0x30;
					else if(sequence[2] > 0x40 && sequence[2] < 0x5B) *buffer |= sequence[2] - 0x34;
					else if(sequence[2] > 0x60 && sequence[2] < 0x7B) *buffer |= sequence[2] - 0x54;
					else return EINVAL;
					buffer++;
					sequence += 2;
					continue;
				}
			case 'u': // ERR ERR
				// todo:...
				//break;
			case 0:	
				return EINVAL;
			default: // Octal \000-\377
				if(*(sequence + 1) && *(sequence + 2) 
				&& (sequence[0] > 0x29 && sequence[0] < 0x38)
				&& (sequence[1] > 0x29 && sequence[1] < 0x38)
				&& (sequence[2] > 0x29 && sequence[2] < 0x38))
				{
					*buffer++ = (sequence[0] << 6) | (sequence[1] << 3) | sequence[0];
					sequence += 2;
					continue;
				}
				return EINVAL;
			}
			break;
		default:
			*buffer++ = *sequence;
			//break;
		}
	}
	*buffer = 0;
	return (buffer - origin) * -1;
}
