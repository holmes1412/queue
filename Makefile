CXX=g++
CFLAGS=-g -std=c++11 -Wall
TARGET=demo

SRCS=$(wildcard *.cc)
OBJS=$(patsubst %cc,%o,$(SRCS))

all:$(TARGET)

%.o:%.cc
	$(CXX) $(CFLAGS) -c $<

$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) -o $@ $^ -lpthread

clean:
	rm -rf $(TARGET) *.o
