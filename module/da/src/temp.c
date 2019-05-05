#include <stdio.h>


typedef struct {
  int a;
  int b;
}pair;


int main(){
  pair s0, s1, s2;
  s0.a = 10;
  s0.b = 11;
  s1.a = 20;
  s1.b = 21;
  s2.a = 10;
  s2.b = 11;

  if( s0 == s1 ) {
    printf("s0 s1 are equal!\n");
  }else{
    printf("s0 s1  are not equal!\n");
  }
  if( s0 == s2 ) {
    printf("s0 s2 are equal!\n");
  }else{
    printf("s0 s2 are not equal!\n");
  }

    
  return 0;
}


  
