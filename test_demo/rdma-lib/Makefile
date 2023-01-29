CXX = g++
AR = ar
CXXFLAGS= -std=c++14 -ggdb 
LDLIBS=-lrdmacm -libverbs #-lpci -lpthread
INC = -I./include


all: bin/client bin/server bin/test_general

OBJS=src/rdma_common.o

libsrc = $(wildcard ./src/*.cpp)



bin/client: app/rdma_client.cpp $(libsrc)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDLIBS) $(INC)
bin/server: app/rdma_server.cpp $(libsrc)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDLIBS) $(INC)
bin/test_general: app/test_general.cpp $(libsrc)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDLIBS) $(INC)


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< $(INC) -o $@


#${OBJS} : rdma_conn.h

.PHONY: all

.PHONY: clean
clean:
	rm -rf  client server src/*.o bin/*
