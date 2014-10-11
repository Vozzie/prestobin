prestobin
=========

Convert Pelle's C resources to a binary file that can be used with other compilers.

Usage: prestobin myresource.rc resource.h

###Steps to use

* Include prestoapi.h in your project.
* Use the functions in prestoapi.h to access your resources.
* Add the offsets to a header file. See *note*
* Call INIT_RESOURCE() once in your project, before calling any of the resource functions. Best in main.
* Build a .bin file from your resources with prestobin.exe (or on linux with prestobin)
  `prestobin sample.rc sample.h`
* Build an object file from the .bin file with objcopy. See [makefile](https://github.com/Vozzie/prestobin/blob/master/makefile)
 
###Be aware

* The tool only converts the types RT_RCDATA and RT_STRING.
* The tool doesn't support unicode. (maybe later)
* Binary resources must be in the same folder of the .rc file.
* Instead of using the windows resource api there are 3 functions inside prestoapi.h
See [prestoapi.h](https://github.com/Vozzie/prestobin/blob/master/inc/prestoapi.h)

######Note: Add this code but change the `_binary_`___yourres_rc___`_bin_start` and `_binary_`___yourres_rc___`_end` See Resources(1) for info on how to obtain the names for `_binary_res_prestobin_rc_bin_start` and `_binary_res_prestobin_rc_bin_end` because these depend on your filename and/or objcopy version.

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

###Resources

1. http://www.linuxjournal.com/content/embedding-file-executable-aka-hello-world-version-5967

####Disclaimer

Feel free to use on your *own* responsibility.
