CXX = g++
AR = ar
CXXFLAGS= -std=c++14 -ggdb 
LDLIBS=-lrdmacm -libverbs #-lpci -lpthread
INC = -I./include


all: bin/client bin/server

OBJS=src/rdma_common.o

#libsrc = $(wildcard $(cur_path)/src/*.h)



bin/client: src/rdma_client.cpp src/rdma_common.cpp
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDLIBS) $(INC)
bin/server: src/rdma_server.cpp src/rdma_common.cpp
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDLIBS) $(INC)


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< $(INC) -o $@


#${OBJS} : rdma_conn.h

.PHONY: all

.PHONY: clean
clean:
	rm -rf  client server src/*.o bin/*
