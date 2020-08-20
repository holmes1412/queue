CXX=g++
CXXFLAGS=-g -std=c++11 -Wall -O2
TARGET=demo

SRCS=$(wildcard *.cc)
OBJS=$(patsubst %cc,%o,$(SRCS))

GRPC_DIR=/search/ted/1412/01source/grpc/

INCLUDES = -I$(GRPC_DIR) -I$(GRPC_DIR)/third_party/abseil-cpp/ -I$(GRPC_DIR)/include
LIBS = -L$(GRPC_DIR)/libs/opt/

LIST_INCLUDES = -I/search/ted/subtask_branches/master/_include/workflow/

GRPC = -lgrpc_abseil -lgrpc

all:$(TARGET)

%.o:%.cc
	$(CXX) $(CXXFLAGS) -c $< $(INCLUDES)

$(TARGET):$(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDES) $(LIST_INCLUDES) $(LIBS) -lpthread -ljemalloc $(GRPC)

clean:
	rm -rf $(TARGET) *.o
