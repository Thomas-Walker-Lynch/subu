#ifndef ACC_LIB_H
#define ACC_LIB_H

typedef struct{
  Da outstanding_malloc;
  Da spurious_free;
} AccChecker;

extern Da *acc_live_checkers;

#define MALLOC acc_malloc
#define FREE acc_free
#define ACCOUNT acc_start()
#define BALANCE acc_result()
#define CLOSE_ACC da_na_free(&heap_acc); da_na_free(&extra_frees)


#endif
