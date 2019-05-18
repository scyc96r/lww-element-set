// LWW-Element-Set by Stephen Chan, 18th May 2019
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include "lww_element_set.h"

void sleep(unsigned int ms);
void print( const std::string& txt );
bool test1();
bool test2();
bool test3();
bool test4();
bool test5();
bool test6();

int main( int argc, char *argv[] )
{
  print("");
  print("This code tests the LWW-Element-Set implementation");
  print("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  print("");
  
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();

  return 0;
}

void sleep(unsigned int ms = 1)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void print( const std::string& txt )
{
  std::cout << txt << std::endl;
}

bool test1()
{
  print("Test: See if set can handle integer elements");

  LwwElementSet< int > set_a;

  // need sleep for more consistent testing as timestamps may be identical depending on speed
  set_a.add(1); sleep();
  set_a.add(2); sleep();
  set_a.add(3); sleep();
  set_a.add(5); sleep();
  set_a.add(7); sleep();
  set_a.add(11); sleep();

  bool pass = set_a.lookup(1) && set_a.lookup(2) && set_a.lookup(3) && \
              set_a.lookup(5) && set_a.lookup(7) && set_a.lookup(11); 

  if( pass ){ print("pass"); } else { print("fail"); }

  return pass;
}


bool test2()
{
  print("Test: See if set can handle string elements");

  LwwElementSet< std::string > set_a;

  set_a.add("good"); sleep();
  set_a.add("notes"); sleep();
  set_a.add("lww-crdt"); sleep();

  bool pass = set_a.lookup("good") && set_a.lookup("notes") && set_a.lookup("lww-crdt"); 

  if( pass ){ print("pass"); } else { print("fail"); }

  return pass;
}

bool test3()
{
  print("Test: Test the add, remove, lookup functionality");

  LwwElementSet< int > set_a;

  set_a.add(1); sleep();
  set_a.add(2); sleep();
  set_a.add(3); sleep();
  set_a.remove(2); sleep();
  bool pass = set_a.lookup(1) && set_a.lookup(3);
  set_a.add(2); sleep(); 
  pass = pass & set_a.lookup(1) && set_a.lookup(2) && set_a.lookup(3);

  // deliberate
  set_a.remove(4); sleep(); 
  pass = pass & set_a.lookup(1) && set_a.lookup(2) && set_a.lookup(3) && !set_a.lookup(4);
  set_a.add(4); sleep();
  pass = pass & set_a.lookup(1) && set_a.lookup(2) && set_a.lookup(3) && set_a.lookup(4);
  
  if( pass ){ print("pass"); } else { print("fail"); }

  return pass;
}


bool test4()
{
  print("Test: Test if merges are idempotent");

  LwwElementSet< int > set_a;
  LwwElementSet< int > set_b;

  set_a.add(1); sleep();
  set_a.add(2); sleep();
  set_a.add(3); sleep();

  set_b.merge( set_a );
  bool pass = set_b.lookup(1) && set_b.lookup(2) && set_b.lookup(3);
  set_b.merge( set_a );
  pass = pass && set_b.lookup(1) && set_b.lookup(2) && set_b.lookup(3);
  set_b.merge( set_b );
  pass = pass && set_b.lookup(1) && set_b.lookup(2) && set_b.lookup(3);
  
  std::set<int> output_set;
  set_b.get_set( output_set );
  pass = pass && output_set.size() == 3;
  pass = pass && *(output_set.find(1)) == 1 && *(output_set.find(2)) == 2 && *(output_set.find(3)) == 3;
  if( pass ){ print("pass"); } else { print("fail"); }

  return pass;
}

bool test5()
{
  print("Test: Test merge function");

  LwwElementSet< int > set_a;
  LwwElementSet< int > set_b;

  set_a.add(1); sleep();
  set_a.add(2); sleep();
  set_a.add(3); sleep();

  set_a.remove(2); sleep();
  set_b.add(2); sleep();
  
  set_b.merge(set_a);
  bool pass = set_b.lookup(1) && set_b.lookup(2) && set_b.lookup(3);
 
  set_a.add(4); sleep();
  set_b.remove(4); sleep();

  set_a.add(5); sleep();
  set_b.add(6); sleep();

  set_b.merge(set_a);
  set_a.merge(set_b);
 
  pass = pass && set_a.lookup(1) && set_a.lookup(2) && set_a.lookup(3) && !set_a.lookup(4) && set_a.lookup(5) && set_a.lookup(6);
  pass = pass && set_b.lookup(1) && set_b.lookup(2) && set_b.lookup(3) && !set_b.lookup(4) && set_b.lookup(5) && set_b.lookup(6);

  if( pass ){ print("pass"); } else { print("fail"); }

  return pass;
}

void thread_1( LwwElementSet< int >& myset )
{
  std::stringstream ss;
  ss << std::this_thread::get_id();
  print( "In Thread ID : " + ss.str() );

  myset.add(1); sleep(1000);
  myset.add(2); sleep(1000);
  myset.add(3); sleep(1000);
  myset.remove(8); sleep(1000);
} 

void thread_2( LwwElementSet< int >& myset )
{
  std::stringstream ss;
  ss << std::this_thread::get_id();
  print( "In Thread ID : " + ss.str() );

  myset.add(6); sleep(500);
  myset.remove(1); sleep(1000);
  myset.remove(5); sleep(1000);
  myset.add(4); sleep(1000);
  myset.remove(3); sleep(1000);

} 

void thread_3( LwwElementSet< int >& myset )
{
  std::stringstream ss;
  ss << std::this_thread::get_id();
  print( "In Thread ID : " + ss.str() );

  myset.add(5); sleep(1000);
  myset.remove(6); sleep(1000);
  myset.add(7); sleep(1000);
} 

bool test6()
{
  print("Test: Test set in threaded environment simulated distributed environment");

  // simulate the following threads (thread 4 is the main thread):
  // 1: ..add(1)...........add(2)..........add(3).........del(8)....................................-> time
  // 2: ..add(6)...del(1)..........del(5)..........add(4).........del(3)............................-> time
  // 3: ..add(5)...........del(6)..........add(7)...................................................-> time  
  // 4: ..add(8)..........................................................add(9)......merge(a,b,c). -> time
  LwwElementSet< int > set_a;
  LwwElementSet< int > set_b;
  LwwElementSet< int > set_c;
  LwwElementSet< int > set_d;
  
  set_d.add(8);

  std::thread t1(thread_1, std::ref(set_a));
  std::thread t2(thread_2, std::ref(set_b));
  std::thread t3(thread_3, std::ref(set_c));

  t1.join();
  t2.join();
  t3.join();

  set_d.add(9);
  set_d.merge(set_a);
  set_d.merge(set_b);
  set_d.merge(set_c);

  bool pass = !set_d.lookup(1) && set_d.lookup(2) && !set_d.lookup(3) && set_d.lookup(4) && \
              !set_d.lookup(5) && !set_d.lookup(6) && set_d.lookup(7) && !set_d.lookup(8) && \
              set_d.lookup(9);

  if( pass ){ print("pass"); } else { print("fail"); }

  return pass;
}





