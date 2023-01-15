

CXXFLAGS += -O3 -std=c++0x 

# LDLIBS=-lrdmacm -libverbs -lpci -lpthread

all: a.out

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

a.out: server.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

# g++ -g route.h server.cpp -o a.out
