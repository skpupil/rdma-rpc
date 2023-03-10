
#ifdef _MSC_VER
#define _SCL_SECURE_NO_WARNINGS
#endif

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC 
#include <stdlib.h>
#include <crtdbg.h>
#endif // check memory leaks

#include "vector_test.h"
//#include "local_vector.h"
//#include "vector_latency_test.h"
#include "aaa.h"
//#include "string_test.h"
#include <cstdint>
//#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))
//#define ACCESS_ONCE(x) (*static_cast<decltype(x) volatile *>(&(x)))
#define ACCESS_ONCE(x) (*static_cast<std::remove_reference<decltype(x)>::type volatile *>(&(x)))

#include<iostream>
#include<thread>
#include<mutex>
#include<condition_variable>



//#include "sb.h"
client_connection* conn = nullptr;

static inline uint64_t rdtsc(void)
{
	uint32_t a, d;
	asm volatile("rdtsc" : "=a" (a), "=d" (d));
	return ((uint64_t)a) | (((uint64_t)d) << 32);
}

void vector_latency_test(client_connection* conn, uint32_t local_num)
{
  std::cout << "[===============================================================]\n";
  std::cout << "[----------------- Run container test : vector -----------------]\n";
  std::cout << "[-------------------------- API test ---------------------------]\n";
  std::cout << "[-------------------------- LRoll "<<local_num/1000.0*100.0<<"% ---------------------------]\n";
  //client_connection* conn = new client_connection();
  int a[] = { 1,2,3,4,5 };
  int ele = 9;
  mystl::vector<int> v1(conn, local_num);
  //vector_local<int> v4;
  std::array<int,1010> v2;
  std::vector<int > v3;

  for(int i = 0;i < 1000;i++){
        v1.push_back(i);
	//std::cout<<i<<std::endl;
	v2[i] = i;
	v3.push_back(i);
  }

  auto start = rdtsc();

  for(int  i = 0;i < 100;i++){
      //v1.get(rand()%1000);
      ACCESS_ONCE(v1[rand()%1000]);
  }

  auto end = rdtsc();

  std::cout<<(end - start)/100.0<<std::endl;

  auto start2 = rdtsc();

  for(int  i = 0;i < 100;i++){
      ACCESS_ONCE(v2[rand()%1000]);
  }

  auto end2 = rdtsc();

  std::cout<<(end2 - start2)/100.0<<std::endl;
  
  auto start3 = rdtsc();

  for(int  i = 0;i < 100;i++){
      ACCESS_ONCE(v3[rand()%1000]);
  }

  auto end3 = rdtsc();

  std::cout<<(end3 - start3)/100.0<<std::endl;
}


void vector_latency_test_eve(client_connection* conn)
{
  std::cout << "[===============================================================]\n";
  std::cout << "[----------------- Run container test : vector -----------------]\n";
  std::cout << "[-------------------------- API test ---------------------------]\n";
  //client_connection* conn = new client_connection();
  int a[] = { 1,2,3,4,5 };
  int ele = 9;
  mystl::vector<int> v1(conn, 5);
  std::array<int,1100> v2;
  for(int i = 0;i < 10;i++){
        v1.push_back(i);
        v2[i] = i;
  }

  auto start = rdtsc();

  for(int  i = 0;i < 10;i++){
      //v1.get(rand()%1000);
      ACCESS_ONCE(v1[rand()%10]);
  }

  auto end = rdtsc();

  std::cout<<(end - start)<<std::endl;

  auto start2 = rdtsc();

  for(int  i = 0;i < 10;i++){
      ACCESS_ONCE(v2[rand()%10]);
  }

  auto end2 = rdtsc();

  std::cout<<(end2 - start2)<<std::endl;
}

void vector_throughput_test_eve(client_connection* conn)
{
  std::cout << "[===============================================================]\n";
  std::cout << "[----------------- Run container test : vector -----------------]\n";
  std::cout << "[-------------------------- API test ---------------------------]\n";
  //client_connection* conn = new client_connection();
  int a[] = { 1,2,3,4,5 };
  int ele = 9;
  mystl::vector<int> v1(conn, 5);
  std::array<int,1100> v2;
  for(int i = 0;i < 10;i++){
        v1.push_back(i);
        v2[i] = i;
  }
  
  std::thread threads[100];
  auto start = rdtsc();

  for (int i = 0; i < 100; ++i)
  {
        threads[i] = std::thread([&](mystl::vector<int> v1) {
			for(int i = 0;i<100000;i++)
      				ACCESS_ONCE(v1[rand()%10]); 
			},v1
	);
  }
  for (int i = 0; i < 100; ++i)
  {
        threads[i].join();
  }
  /*
  for(int  i = 0;i < 10;i++){
      //v1.get(rand()%1000);
      ACCESS_ONCE(v1[rand()%10]);
  }
  */

  auto end = rdtsc();

  std::cout<<(end - start)<<std::endl;

  auto start2 = rdtsc();

  for(int  i = 0;i < 10;i++){
      ACCESS_ONCE(v2[rand()%10]);
  }

  auto end2 = rdtsc();

  std::cout<<(end2 - start2)<<std::endl;
}
#define barrier() __asm__ __volatile__("": : :"memory") 

void func(mystl::vector<int>& v1){

  std::thread threads[100];
 
  auto start = rdtsc();

  for (int i = 0; i < 1; ++i)
  {
        threads[i] = std::thread([&](mystl::vector<int> v1) {
			for(int i = 0;i<10;i++){
				int inx = rand()%3;
				std::cout<<"acces: "<<inx<<std::endl;
      				ACCESS_ONCE(v1[inx]); 
			}
	}
			,v1
	);
  }
  for (int i = 0; i < 1; ++i)
  {
        threads[i].join();
  }
  
  auto end = rdtsc();

  std::cout<<"klxthought: "<<10000.0*(end - start)/100*100000<<std::endl;
}



void vector_throught_test(client_connection* conn, uint32_t local_num)
{
  std::cout << "[===============================================================]\n";
  std::cout << "[----------------- Run container test : vector -----------------]\n";
  std::cout << "[-------------------------- API test ---------------------------]\n";
  std::cout << "[-------------------------- TRoll "<<local_num/1000.0*100<<"% ---------------------------]\n";
  //client_connection* conn = new client_connection();
  int a[] = { 1,2,3,4,5 };
  int ele = 9;
  mystl::vector<int> v1(conn, local_num);
  std::array<int,1010> v2;
  std::vector<int > v3;

  for(int i = 0;i <= 1000;i++){
        v1.push_back(i);
        v2[i] = i;
	v3.push_back(i);
  }
  /*
  for(int i = 0;i<100000;i++){
  	ACCESS_ONCE(v1[rand()%3]); 
  }
  */
  //sleep(10);
  barrier();
  asm volatile("mfence" ::: "memory");
  
  const int try_num = 10000000;

  auto start1 = rdtsc();
  for(int i = 0;i<try_num;i++){
    int inx = rand()%1000;
    //std::cout<<"acces: "<<inx<<std::endl;
    ACCESS_ONCE(v1[rand()%1000]);
  }

  auto end1 = rdtsc();
  std::cout<<1000.0*try_num/(end1 - start1)<<std::endl;
  
  auto start2 = rdtsc();
  for(int i = 0;i<try_num;i++){
    int inx = rand()%1000;
    //std::cout<<"acces: "<<inx<<std::endl;
    ACCESS_ONCE(v2[rand()%1000]);
  }

  auto end2 = rdtsc();
  std::cout<<1000.0*try_num/(end2 - start2)<<std::endl;
  
  auto start3 = rdtsc();
  for(int i = 0;i<try_num;i++){
    int inx = rand()%1000;
    //std::cout<<"acces: "<<inx<<std::endl;
    ACCESS_ONCE(v3[rand()%1000]);
  }

  auto end3 = rdtsc();
  std::cout<<1000.0*try_num/(end3 - start3)<<std::endl;
}

int main()
{
  using namespace mystl::test;

  std::cout.sync_with_stdio(false);
  
  
  client_connection* conn = new client_connection();
  //vector_test::vector_test();
  //vector_latency_test_eve(conn);
 
  //latency test
  
  for(int i = 0;i <= 1000;i += 100){
  	vector_latency_test(conn, i);
  }


  sleep(1);
  //throuthg test
  
  for(int i = 0;i <= 1000;i += 100){
  	vector_throught_test(conn, i);
  }


  //mystls::vector_local<int > lv;
  //local_vector<int> lv;
  //mystls::vector_local<int > lv2;

#if defined(_MSC_VER) && defined(_DEBUG)
  _CrtDumpMemoryLeaks();
#endif // check memory leaks

}


  
  /*
  RUN_ALL_TESTS();
  algorithm_performance_test::algorithm_performance_test();
  vector_test::vector_test();
  list_test::list_test();
  deque_test::deque_test();
  queue_test::queue_test();
  queue_test::priority_test();
  stack_test::stack_test();
  map_test::map_test();
  map_test::multimap_test();
  set_test::set_test();
  set_test::multiset_test();
  unordered_map_test::unordered_map_test();
  unordered_map_test::unordered_multimap_test();
  unordered_set_test::unordered_set_test();
  unordered_set_test::unordered_multiset_test();
  string_test::string_test();
*/
