##########################################################
PROJECT_TOP_DIR=$(abspath $(firstword $(MAKEFILE_LIST))) 

PROJECT_SRC_DIR=./
PROJECT_INC_DIR=./
PROJECT_LIB_DIR=./ -lpthread -pthread -lrt
PROJECT_OBJ_DIR=./objs
PROJECT_BIN_DIR=./bin
$(shell mkdir -p ${PROJECT_OBJ_DIR} $(PROJECT_BIN_DIR))

TARGETS = cardparser
SRCS   :=	cardparser.cpp \
			test.cpp \

CC := g++
CFLAGS := -g -I$(PROJECT_INC_DIR) -fpermissive
LDFLAG := -L$(PROJECT_LIB_DIR)

PROJECT_OBJ := $(patsubst %.cpp,%.o,$(SRCS))  
	
PROJECT_ALL_OBJS := $(addprefix $(PROJECT_OBJ_DIR)/, $(PROJECT_OBJ))
	
all:$(PROJECT_SRC_DIR) $(PROJECT_ALL_OBJS)
	$(CC) $(CFLAGS) $(PROJECT_ALL_OBJS) -o $(PROJECT_BIN_DIR)/$(TARGETS) $(LDFLAG)

$(PROJECT_OBJ_DIR)/%.o : $(PROJECT_SRC_DIR)%.cpp
	$(CC) -c $(CFLAGS) $(LDFLAG) $< -o $@

clean:
	rm -fr $(PROJECT_OBJ_DIR)
	rm -fr $(PROJECT_BIN_DIR)/$(TARGETS)

