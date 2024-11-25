# /bin/make must be run from $REPO_HOME/developer

RESOURCE:=$(REPO_HOME)/tool_shared/third_party/resource/release
include $(RESOURCE)/make/environment_RT_0

# override defaults
EXECDIR=mockup/subu_server_home
LIBDIR=scratchpad

# assign a compiler to use
C=gcc

# _GNU_SOURCE is needed to get ucred, the credentials of the pid that connects to a socket
CFLAGS= -Werror -ggdb -D_GNU_SOURCE -DDEBUG -DDEBUGDB -include "$(RESOURCE)/cc/RT_0.h" -I$(INCDIR) 

LINKFLAGS=-L$(LIBDIR) -L/usr/lib64 -L/usr/lib -l$(PROJECT)

# Import the rules. The dash prefix means to ignore include errors. This is
# required because the DEPFILE might not exist yet.
include $(RESOURCE)/make/targets
-include $(DEPFILE)
