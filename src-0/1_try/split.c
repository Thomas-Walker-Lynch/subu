/*
Using preprocessor to make header file?

gcc -E split.c -DPROTOTYPE

Outputs source code source comment lines starting with a hash. Resolves all macro commands,
hence the resulting header can not have macro commands.

 */

#if PROTOTYPE
##define GREATNESS 21
int f(int x);
#endif

#if IMPLEMENTATION
int f(int x){
  return x;
}
#endif
