#ifndef MYTINYSTL_VECTOR_TEST_H_
#define MYTINYSTL_VECTOR_TEST_H_

// vector test : 测试 vector 的接口与 push_back 的性能

#include <vector>

#include "../include/vector.h"
#include "test.h"
//#include "rdma_connection.h"


#include<iostream>
#include<thread>
#include<mutex>
#include<condition_variable>


namespace mystl
{
namespace test
{
namespace aaa
{

void vector_latency_test()
{
  std::cout << "[===============================================================]\n";
  std::cout << "[----------------- Run container test : vector -----------------]\n";
  std::cout << "[-------------------------- API test ---------------------------]\n";
  client_connection* conn = new client_connection();
  int a[] = { 1,2,3,4,5 };
  int ele = 9;
  mystl::vector<int> v1(conn, 200);
  v1.push_back(ele);
  std::cout<<std::endl;
  std::cout<<v1[0]<<"--------------------konglx test-------------------------"<<std::endl;
  
  v1.push_back(250);
  std::cout<<std::endl;
  std::cout<<v1[0]<<"--------------------konglx test-------------------------"<<std::endl;
  std::cout<<v1[1]<<"--------------------konglx test-------------------------"<<std::endl;
  
  for(int i = 0;i < 1000;i++){
  	v1.push_back(i);
  } 
  
  auto start = rdtsc();
  
  for(int  i = 0;i < 100;i++){
  //	v1[rand()%1000]
  }

  auto end = rdtsc();
}

} // namespace vector_test
} // namespace test
} // namespace mystl
#endif // !MYTINYSTL_VECTOR_TEST_H_

