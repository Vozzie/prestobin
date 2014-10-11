////////////////////////////////////////////////////////////////////////
//
// file: prestoapi.h
//
#pragma once

#define PREST_STRING  6
#define PREST_RCDATA  10

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Function: load_string

  Summary:  Loads a resource string.

  Args:     unsigned int id
              Resource ID of the string.
            char * buffer
              Buffer to receive the string.
            unsigned int length
              The length of buffer.
  Returns:  int
              Zero on failure, the length of the string excluding the
              null char on success. The function terminates the 
              string with a null char.
--------------------------------------------------------------------*/
int load_string(unsigned int id, char * buffer, unsigned int length);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Function: get_resource_size

  Summary:  Gets the size of resource data.

  Args:     unsigned int id 
              Resource ID.
            unsigned int type
              Resource type (only PREST_RCDATA supported).

  Returns:  int
              Zero on failure, otherwise the length of the resource.
--------------------------------------------------------------------*/
int get_resource_size(unsigned int id, unsigned int type);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Function: get_resource_value

  Summary:  Gets the resource data.

  Args:     unsigned int id 
              Resource ID.
            unsigned int type
              Resource type (only PREST_RCDATA supported).
			char * buffer
              Buffer to receive the data.
            unsigned int length
              The length of buffer.

  Returns:  Zero on failure, the length of the resource on success.
--------------------------------------------------------------------*/
int get_resource_value(unsigned int id, unsigned int type, char * buffer, unsigned int length);

// Non user code below.
////////////////////////////////////////////////////////////////////////

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
	if(hResource == NULL) return 0;
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
	if(total == -1 || size < total) return 0;
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
				return total;
			}
			if(lpLock != NULL) UnlockResource(lpLock);
		}
		if(hGlobal != NULL) FreeResource(hGlobal);
	}
	return 0;
}

#endif
