#pragma once

#define PREST_STRING  6
#define PREST_RCDATA  10

typedef struct {
	unsigned int version; // 0
	unsigned int signature;
	unsigned int count;
	//RESOURCE resources[1];
} RESOURCEHEADER, *PRESOURCEHEADER, *LPRESOURCEHEADER;

typedef struct RESOURCE_ {
	unsigned int type;
	unsigned int id;
	unsigned int offset;
	unsigned int length;
} RESOURCE, *PRESOURCE, *LPRESOURCE;

#ifdef PRESTOBINAPI

extern char * resource_offset;

int load_string(unsigned int id, char * buffer, unsigned int length)
{
	PRESOURCEHEADER header;
	PRESOURCE resource;
	unsigned int count;
	if(buffer == NULL || length == 0) return 0;
	header = (PRESOURCEHEADER) resource_offset;
	//if(header->signature != ) return 0;
	resource = (PRESOURCE)(header+1);
	for(count = 0; count < header->count; count++)
	{
		if(resource->type == PREST_STRING && resource->id == id)
		{
			length = (length > resource->length ? resource->length : length - 1);
			strncpy(buffer, resource_offset + resource->offset, length);
			buffer[length] = 0;
			return length;
		}
		resource++;
	}
	return 0;
}

int get_resource_size(unsigned int id, unsigned int type)
{
	PRESOURCEHEADER header;
	PRESOURCE resource;
	unsigned int count;
	header = (PRESOURCEHEADER) resource_offset;
	//if(header->signature != ) return 0;
	resource = (PRESOURCE)(header + 1);
	for(count = 0; count < header->count; count++)
	{
		if(resource->type == type && resource->id == id)
		{
			return resource->length;
		}
		resource++;
	}
	return 0;
}

int get_resource_value(unsigned int id, unsigned int type, char * buffer, unsigned int length)
{
	PRESOURCEHEADER header;
	PRESOURCE resource;
	unsigned int count;
	if(buffer == NULL || length == 0) return 0;
	header = (PRESOURCEHEADER) resource_offset;
	//if(header->signature != ) return 0;
	resource = (PRESOURCE)(header + 1);
	for(count = 0; count < header->count; count++)
	{
		if(resource->type == type && resource->id == id)
		{
			if(length < resource->length) return 0;
			memcpy(buffer, resource_offset + resource->offset, length);
			return resource->length;
		}
		resource++;
	}
	return 0;
}

#else

#include <windows.h>

int load_string(unsigned int id, char * buffer, unsigned int length)
{
	return LoadString(GetModuleHandle(NULL), id, buffer, length);
}

int get_resource_size(unsigned int id, unsigned int type)
{
	HMODULE hModule;
	HRSRC hResource;
	hModule = GetModuleHandle(NULL);
	hResource = FindResource(hModule, MAKEINTRESOURCE(id), MAKEINTRESOURCE(type)); 
	if(hResource == NULL) return -1;
	return SizeofResource(hModule, hResource);
}

int get_resource_value(unsigned int id, unsigned int type, char * buffer, unsigned int size)
{
	HMODULE hModule;
	HRSRC hResource;
	HGLOBAL hGlobal;
	LPVOID lpLock;
	unsigned int total;
	total = get_resource_size(id, type);
	if(total == -1 || size < total) return EINVAL;
	hModule = GetModuleHandle(NULL);
	hResource = FindResource(hModule, MAKEINTRESOURCE(id), MAKEINTRESOURCE(type));
	if(hResource != NULL)
	{
		hGlobal = LoadResource(hModule, hResource);
		if(hGlobal != NULL)
		{
			lpLock = LockResource(hGlobal);
			if(lpLock != NULL && buffer != NULL)
			{
				memcpy(buffer, lpLock, size);
				return 0;
			}
			if(lpLock != NULL) UnlockResource(lpLock);
		}
		if(hGlobal != NULL) FreeResource(hGlobal);
	}
	return EINVAL;
}

#endif
