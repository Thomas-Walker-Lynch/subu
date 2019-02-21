#ifndef SUBU_MK_0_LIB_H
#define SUBU_MK_0_LIB_H

#define ERR_ARG_CNT 1
#define ERR_SETUID_ROOT 2
#define ERR_BAD_MASTERU_HOME 3
#define ERR_NOT_EXIST_MASTERU_HOME 4
#define ERR_NOT_EXIST_SUBULAND 5
#define ERR_FAILED_MKDIR_SUBU 6
#define ERR_FAILED_RESTORATION 7
#define ERR_FAILED_USERADD 8
#define ERR_SETFACL 9
#define ERR_MALLOC 10
#define ERR_BUG_SSS 11
#define ERR_MK_SUBUHOME 12

int subu_mk_0(char *subuname);

#endif
