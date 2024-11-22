# /bin/make must be run from $REPO_HOME/developer

RESOURCE := $(REPO_HOME)/tool_shared/third_party/resource/release/make
include $(RESOURCE)/makefile-environment

# overriding defaults
EXECDIR=mockup/subu_server_home
LIBDIR=scratchpad

# assign a compiler to use
C=gcc
CFLAGS= -include "$(RESOURCE)/environment.h" -std=gnu18 -Werror -O0 -ggdb -DDEBUG -DDEBUGDB -I$(INCDIR) 
LINKFLAGS=-L$(LIBDIR) -L/usr/lib -L/usr/lib64 -l$(PROJECT)

# Import the rules. The dash prefix means to ignore include errors. This is
# required because the DEPFILE might not exist yet.
-include $(DEPFILE)
include $(RESOURCE)/makefile
