/*
They say a cast is not required passing a typed pointer to a void * argument,
but What about void **?

gcc -std=gnu11 -o voidptr voidptr.c
voidptr.c: In function ‘main’:
voidptr.c:28:5: warning: passing argument 1 of ‘g’ from incompatible pointer type [-Wincompatible-pointer-types]
   g(&pt, y);
     ^~~
voidptr.c:13:15: note: expected ‘void **’ but argument is of type ‘int **’
 void g(void **pt0, void *pt1){
        ~~~~~~~^~~

*/
#include <stdio.h>

int f(void *pt){
  return *(int *)pt;
}

/* fails
void g(void **pt0, int *pt1){
  *pt0 = pt1;
}
*/

// passes
void g(void *pt0, int *pt1){
  *(int **)pt0 = pt1;
}

int main(){
  int x = 5;
  int *xp = &x;
  printf("%d\n",f(xp));

  int y[3];
  y[0] = 10;
  y[1] = 11;
  y[2] = 12;

  int *pt;
  g(&pt, y);
  printf("%d\n",*pt);

  printf("that's all folks\n");
  return 0;
}
