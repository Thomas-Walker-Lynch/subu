#ifndef SUBU_MK_0_H
#define SUBU_MK_0_H

#define ERR_ARG_CNT 1
#define ERR_SETUID_ROOT 2
#define ERR_BAD_MASTERU_HOME 3
#define ERR_NOT_EXIST_MASTERU_HOME 4
#define ERR_NOT_EXIST_SUBU_LAND 5
#define ERR_FAILED_MKDIR_SUBU 6
#define ERR_FAILED_RESTORATION 7
#define ERR_FAILED_USERADD 8


struct uid_gid subu_mk_0(char *subuname);

#endif
