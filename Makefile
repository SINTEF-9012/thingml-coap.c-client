#SRCS = cmd_list.c cmd_get.c cmd_post.c help.c cmd_repeat.c cmd_delete.c thingml_coap_utility.c
#OBJS = cmd_list.o cmd_get.o cmd_post.o help.o cmd_repeat.o cmd_delete.o thingml_coap_utility.o
SRCS = cmd_get.c thingml_coap_utility.c
OBJS = cmd_get.o thingml_coap_utility.o

SRCS_EXAMPLE = ./examples/coap_get_test.c
OBJS_EXAMPLE = ./examples/coap_get_test.o
BIN_EXAMPLE =./examples/coap_get_test

SRC_OBS_EXAMPLE = ./examples/coap_observe_test.c
OBJ_OBS_EXAMPLE = ./examples/coap_observe_test.o
BIN_OBS_EXAMPLE =./examples/coap_observe_test


STATIC_LIB_LOCATION = libtmlcaopclient.a
DYNAMIC_LIB_LOCATION = libtmlcaopclient.so

LIB =-lsmcp

INSTALL_LIB_DIR = /usr/local/lib
INCLUDE_DIR = /usr/local/include
INCLUDE_COAP_DIR = thingmlcaop

CROSS_COMPILE :=
 
#GFLAGS = -pedantic -Wall -Werror -fPIC -g -std=c99 -DDEBUG=1
GFLAGS = -fPIC -g

GCC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

%.o : %.c
	$(GCC) $(GFLAGS) -c -o $@ $<

all : staticlib dynamiclib bin

staticlib : $(OBJS)
	$(AR) -rcs $(STATIC_LIB_LOCATION) $(OBJS)

dynamiclib : $(OBJS)
	$(GCC) -shared -rdynamic -o $(DYNAMIC_LIB_LOCATION) $(OBJS)
	
install: staticlib dynamiclib
	install -d $(INCLUDE_DIR)/$(INCLUDE_COAP_DIR)
	install $(STATIC_LIB_LOCATION) $(INSTALL_LIB_DIR)
	install $(DYNAMIC_LIB_LOCATION) $(INSTALL_LIB_DIR)
	cp -r ./*.h $(INCLUDE_DIR)/$(INCLUDE_COAP_DIR)
	ldconfig

uninstall:
	rm -rf $(INCLUDE_DIR)/$(INCLUDE_COAP_DIR)
	rm -rf $(INSTALL_LIB_DIR)/$(STATIC_LIB_LOCATION)
	rm -rf $(INSTALL_LIB_DIR)/$(DYNAMIC_LIB_LOCATION)

bin : bin_get bin_obs

bin_obs : $(OBJ_OBS_EXAMPLE)
	$(GCC) -o $(BIN_OBS_EXAMPLE) $(GFLAGS) $(OBJ_OBS_EXAMPLE) $(STATIC_LIB_LOCATION) $(LIB)

bin_get : $(OBJS_EXAMPLE)
	$(GCC) -o $(BIN_EXAMPLE) $(GFLAGS) $(OBJS_EXAMPLE) $(STATIC_LIB_LOCATION) $(LIB)

clean :
	rm -rf $(OBJS) $(OBJS_EXAMPLE) $(BIN_EXAMPLE) $(OBJ_OBS_EXAMPLE) $(BIN_OBS_EXAMPLE) $(STATIC_LIB_LOCATION) $(DYNAMIC_LIB_LOCATION)