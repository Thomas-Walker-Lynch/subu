#ifndef TRANCHE_LIB_H
#define TRANCHE_LIB_H

#define TRANCHE_ERR_ARG_PARSE 1
#define TRANCHE_ERR_SRC_OPEN 2
#define TRANCHE_ERR_DST_OPEN 4
#define TRANCHE_ERR_FCLOSE 8

char *path_chop(char *path);
int tranche_send(FILE *src, Da *arg_fds);
int tranche_target(FILE *src, Da *targets);
void tranche_make(FILE *src_file, char *src_file_name, int mfile_fd, char *sdir, char *tdir);

#endif
