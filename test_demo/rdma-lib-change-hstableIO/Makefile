CXX = g++
AR = ar
CXXFLAGS= -std=c++14 -ggdb 
LDLIBS=-lrdmacm -libverbs #-lpci -lpthread
INC = -I./include
INC += -I./Test

all: bin/client bin/server bin/test_general bin/test_stl bin/test_hash bin/test_rmhash bin/test_zipf_hash bin/test_zipf_hash_arr

OBJS=src/rdma_common.o

libsrc = $(wildcard ./src/*.cpp)



bin/client: app/rdma_client.cpp $(libsrc)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDLIBS) $(INC)
bin/server: app/rdma_server.cpp $(libsrc)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDLIBS) $(INC)
bin/test_general: app/test_general.cpp $(libsrc)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDLIBS) $(INC)
bin/test_stl: Test/test.cpp $(libsrc) 
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDLIBS) -lpthread $(INC)
bin/test_hash: app/warmUp.cpp $(libsrc)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDLIBS) $(INC)
bin/test_rmhash: app/benchmarkII.cpp $(libsrc)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDLIBS) $(INC)
bin/test_zipf_hash: app/benchmark_zipf.cpp $(libsrc)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDLIBS) $(INC)
bin/test_zipf_hash_arr: app/benchmark_zipf_arr.cpp $(libsrc)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDLIBS) $(INC)


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< $(INC) -o $@


#${OBJS} : rdma_conn.h

.PHONY: all

.PHONY: clean
clean:
	rm -rf  client server src/*.o bin/*
