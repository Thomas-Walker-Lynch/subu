

filename.tag.extension

extension: 
  .c for C source
  .cc for C++ source
  .h for C header file
  .hh for C++ header file
  .o an object file

tag:
  .lib. The resulting .o file to be placed in release library and is part of the
        programming interface.
  .aux. The resulting.o file not directly part of the programming interface, but
        it might be called by functions that are.
  .cli. The source file has a main call and is to be relased as part of the command line interface
  .loc. file has a main call to be made into a local uitlity function

We carry the source file tag and extension to the .o file.  We do not put tags
nor extensions on command line executables.

local_common.h should be included in all source files
