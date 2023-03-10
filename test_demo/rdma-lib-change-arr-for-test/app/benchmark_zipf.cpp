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
#include <cmath>
#include "vector"
#include <random>

client_connection* conn = nullptr;//new client_connection();

/**
 * Example usage:
 *
 *    std::random_device rd;
 *    std::mt19937 gen(rd());
 *    zipf_table_distribution<> zipf(300);
 *
 *    for (int i = 0; i < 100; i++)
 *        printf("draw %d %d\n", i, zipf(gen));
 */
template<class IntType = unsigned long, class RealType = double>
class zipf_table_distribution
{
   public:
      typedef IntType result_type;

      static_assert(std::numeric_limits<IntType>::is_integer, "");
      static_assert(!std::numeric_limits<RealType>::is_integer, "");

      /// zipf_table_distribution(N, s)
      /// Zipf distribution for `N` items, in the range `[1,N]` inclusive.
      /// The distribution follows the power-law 1/n^s with exponent `s`.
      /// This uses a table-lookup, and thus provides values more
      /// quickly than zipf_distribution. However, the table can take
      /// up a considerable amount of RAM, and initializing this table
      /// can consume significant time.
      zipf_table_distribution(const IntType n,
                              const RealType q=1.0) :
         _n(init(n,q)),
         _q(q),
         _dist(_pdf.begin(), _pdf.end())
      {}
      void reset() {}

      IntType operator()(std::mt19937& rng)
      {
         return _dist(rng);
      }

      /// Returns the parameter the distribution was constructed with.
      RealType s() const { return _q; }
      /// Returns the minimum value potentially generated by the distribution.
      result_type min() const { return 1; }
      /// Returns the maximum value potentially generated by the distribution.
      result_type max() const { return _n; }

   private:
      std::vector<RealType>               _pdf;  ///< Prob. distribution
      IntType                             _n;    ///< Number of elements
      RealType                            _q;    ///< Exponent
      std::discrete_distribution<IntType> _dist; ///< Draw generator

      /** Initialize the probability mass function */
      IntType init(const IntType n, const RealType q)
      {
         _pdf.reserve(n+1);
         _pdf.emplace_back(0.0);
         for (IntType i=1; i<=n; i++)
            _pdf.emplace_back(std::pow((double) i, -q));
         return n;
      }
};


vector<int > pdf_;


static inline uint64_t rdtsc(void)
{
        uint32_t a, d;
        asm volatile("rdtsc" : "=a" (a), "=d" (d));
        return ((uint64_t)a) | (((uint64_t)d) << 32);
}

void generator(int n, double q ){
/*

	pdf_.reserve(n + 1);
  	pdf_.emplace_back(0.0);
  	for (int i = 1; i <= n; i++) {
		std::cout<<std::pow((double)i, -q)<<", ";
    		pdf_.emplace_back(std::pow((double)i, -q));
  	}


	std::random_device rd;
 	std::mt19937 gen(rd());
	std::discrete_distribution<int> dist_(pdf_.begin(), pdf_.end());
 	//zipf_table_distribution<> zipf(300);
 
 	for (int i = 0; i < 100; i++)
 		printf("draw %d %d\n", i, dist_(gen) - 1);


	*/
    	std::random_device rd;
     	std::mt19937 gen(rd());
     	zipf_table_distribution<> zipf(n, q );
 
     	for (int i = 0; i < 100; i++)
        	printf("draw %d %ld\n", i, zipf(gen));

	
	btHashMap<btHashInt, btHashInt> dict(conn, 200);
	
	for(int i = 0; i <= 1000;i++) {
		dict.insert(btHashInt(i), btHashInt(1));
		//dict.insert();
	}

	auto start  = rdtsc();
	for(int i = 0;i<10000;i++){
		dict.find(zipf(gen));	
	}
	auto end = rdtsc();
	printf("%d, klxzipf test %lf: %ld\n",n,q, end-start);
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
	/*	
	long tests = 1000;//atol(argv[1]);
	client_connection* conn = new client_connection();
	
	
	int seed = time(NULL);
	for(int i = 0; i <=1000;i+=100){
	
		
		double btTime = btBench(seed, tests, i, conn);
		double stdTime = stdBench(seed, tests);
		printf("test_res:%11ld\t% 5.3f\t% 5.3f\n", tests, stdTime/tests, btTime/tests);
	}	
	*/
	conn = new client_connection();
/*
	generator(12, 0.05);	
	generator(12, 0.15);	
	generator(12, 0.25);	
	generator(12, 0.35);	
	generator(12, 0.45);	
	generator(12, 0.55);	
	generator(12, 0.65);	
	generator(12, 0.75);	
	generator(12, 0.85);	
	generator(12, 0.95);	
*/
for(int i = 0;i<=1000;i+=100){
	generator(i, 0.85);	
}
	return 0;
}
