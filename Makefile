SRCS = main.c cmd_list.c cmd_get.c cmd_post.c help.c cmd_repeat.c cmd_delete.c
OBJS = main.o cmd_list.o cmd_get.o cmd_post.o help.o cmd_repeat.o cmd_delete.o

BIN =smcpctl

LIB =-lsmcp

INCLUDE_DIR = /usr/local/include

CROSS_COMPILE :=
 
#GFLAGS = -pedantic -Wall -Werror -fPIC -g -std=c99 -DDEBUG=1
GFLAGS = -g -O2

GCC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

%.o : %.c
	$(GCC) $(GFLAGS) -c -o $@ $<

all : bin

bin : $(OBJS)
	$(GCC) -o $(BIN) $(GFLAGS) -g $(OBJS) $(LIB)

clean :
	rm -rf $(OBJS) $(BIN)