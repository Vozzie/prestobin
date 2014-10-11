prestobin
=========


Convert Pelle's C resources to a binary file that can be used with other compilers.

Usage: prestobin myresource.rc resource.h


###Be aware:

* The tool only converts the types RT_RCDATA and RT_STRING.

* Binary resources must be in the same folder of the .rc file.

* Instead of using the windows resource api there are 3 functions inside prestoapi.h
See [prestoapi.h](https://github.com/Vozzie/prestobin/blob/master/inc/prestoapi.h)


###Steps to use

1. Include prestoapi.h in your Pelle's c project.
2. Use the functions in prestoapi.h to access your resources.
3. Build a .bin file from your resources with prestobin.exe (or on linux with prestobin)
4. Add the offsets to a header file. See below (change names of the 2 extern char variables, see Resources)

```C
#ifdef PRESTOBINAPI
extern char _binary_res_prestobin_rc_bin_start;
extern char _binary_res_prestobin_rc_bin_end;
char * resource_offset;
#define INIT_RESOURCE() resource_offset = &_binary_res_prestobin_rc_bin_start;
#else
#define INIT_RESOURCE() 
#endif
```

5. Call INIT_RESOURCE() before in your project one time, before calling any of the resource functions.

#####Note: prestobin itself was made with these steps, see [makefile](https://github.com/Vozzie/prestobin/blob/master/makefile)


###Resources:

http://www.linuxjournal.com/content/embedding-file-executable-aka-hello-world-version-5967

