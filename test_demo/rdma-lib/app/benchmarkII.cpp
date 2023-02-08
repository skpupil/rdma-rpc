#include "btHashMap.h"
#include "getCPUTime.h"
//#include "rdma_connection.h"

#include <ctime>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <unordered_map>
#include<iostream>

static inline uint64_t rdtsc(void)
{
        uint32_t a, d;
        asm volatile("rdtsc" : "=a" (a), "=d" (d));
        return ((uint64_t)a) | (((uint64_t)d) << 32);
}





class timer
{
 public:
         timer() { _start_time = getCPUTime(); } // postcondition: elapsed()==0
  void   restart() { _start_time = getCPUTime(); } // post: elapsed()==0
  double elapsed() const                  // return elapsed time in seconds
    { return  getCPUTime() - _start_time; }

 private:
  double _start_time;
}; // timer

double btBench(int seed, long tests, int num, client_connection* conn)
{
	btHashMap<btHashInt, btHashInt> dict(conn,num);
	
	srand(seed); // setup random seed.
	
	timer t;
	auto start  = rdtsc();
	for(long i = 0; i < tests; ++i) {
		int r = rand(); // generate random int.

		// lookup in the hash map.
		btHashInt* val = dict.find(btHashInt(r));
		
		if(val != NULL) { // found, update directly.
			val->setUid1(val->getUid1() + 1);
		}
		else { // not found, insert <key, 1>
			dict.insert(btHashInt(r), btHashInt(1));
		}
	}
	auto end = rdtsc();
	return end - start;
	//return t.elapsed();
}

double stdBench(int seed, long tests)
{
	std::unordered_map<int, int> dict;
	
	srand(seed); // setup random seed.
	
	timer t;

	auto start  = rdtsc();
	for(long i = 0; i < tests; ++i) {
		int r = rand(); // generate random int.
		
		auto it = dict.find(r); // lookup.
		
		if(it != dict.end()) { // found
			++it->second;
		}
		else { // not found
			dict.insert(std::make_pair(r, 1));
		}
	}
	auto end = rdtsc();
	return end - start;
	//return t.elapsed();
}

int main(int argc, char* argv[])
{
	/*
	if(argc < 2) {
		fprintf(stderr, "Usage: %s tests\n", argv[0]);
		return -1;
	}
	*/
	
	long tests = 1000;//atol(argv[1]);
	client_connection* conn = new client_connection();
	
	
	int seed = time(NULL);
	for(int i = 0; i <=1000;i+=100){
	
		
		double btTime = btBench(seed, tests, i, conn);
		double stdTime = stdBench(seed, tests);
		printf("test_res:%11ld\t% 5.3f\t% 5.3f\n", tests, stdTime/tests, btTime/tests);
	}	
	
	return 0;
}
