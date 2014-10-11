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

* Include prestoapi.h in your Pelle's c project.
* Use the functions in prestoapi.h to access your resources.
* Build a .bin file from your resources with prestobin.exe (or on linux with prestobin)
* Add the offsets to a header file.

######Note: See Resources(1) for info on how to obtain the names for `_binary_res_prestobin_rc_bin_start` and `_binary_res_prestobin_rc_bin_end` because these depend on your filename and/or objcopy version.

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

* Call INIT_RESOURCE() once in your project, before calling any of the resource functions. Best in main.

######Note: prestobin itself was made with these steps, see [makefile](https://github.com/Vozzie/prestobin/blob/master/makefile)


###Resources:

1. http://www.linuxjournal.com/content/embedding-file-executable-aka-hello-world-version-5967

