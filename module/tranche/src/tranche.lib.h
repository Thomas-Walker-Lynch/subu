#ifndef TRANCHE_LIB_H
#define TRANCHE_LIB_H

#define TRANCHE_ERR_ARGC 1
#define TRANCHE_ERR_SRC_OPEN 2
#define TRANCHE_ERR_DEP_OPEN 4

int tranche_send(FILE *src, Da *arg_fds);
int tranche_targets(FILE *src, Da *targets);


#endif
