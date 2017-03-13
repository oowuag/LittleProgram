##########################################################
PROJECT_TOP_DIR=$(abspath $(firstword $(MAKEFILE_LIST))) 

PROJECT_SRC_DIR=./
PROJECT_INC_DIR=./ -I/usr/local/include/ -I/usr/include/libxml2/
PROJECT_LIB_DIR=./ -lpthread -pthread -lrt -lbluetooth -lopenobex -lxml2 -Wl,-rpath,/usr/local/lib
#PROJECT_LIB_DIR=./ -lpthread -pthread -lrt -lbluetooth -lxml2 -Wl,-rpath,/usr/local/lib /usr/local/lib/libopenobex.a
PROJECT_OBJ_DIR=./objs
PROJECT_BIN_DIR=./bin
$(shell mkdir -p ${PROJECT_OBJ_DIR} $(PROJECT_BIN_DIR))

TARGETS = obex-test
SRCS   :=	obex_test.cpp \
			obex_test_client.cpp \
			sdp_browse.cpp \
			vcard_xml_parse.cpp

CC := g++
CFLAGS := -g -I$(PROJECT_INC_DIR) -DHAVE_BLUETOOTH
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

