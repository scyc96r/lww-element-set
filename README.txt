LWW-Element-Set 
~~~~~~~~~~~~~~~

Dev environment:
 - MAC iOS
 - C++11
 - CMake

Files list:
 - CMakeList.txt, cmake file
 - README.txt, this file
 - lww_element_set.h, the LWW Element Set implementation
 - main.cpp, test code

Compilation and run:
 1) cmake .
 2) make
 3) ./lwwtest

Test cases:
~~~~~~~~~~~

Test case 1: "See if set can handle integer elements"
 - Tests add and lookup functions

Test case 2: "See if set can handle string elements"
 - Tests add and lookup functions

Test case 3: "Test the add, remove, lookup functionality"
 - Tests add, remove, and lookup functions

Test case 4: "Test if merges are idempotent"
 - This is crucial for when set's converge to their most updated state.
 - This test also verifies if the converged set is correct.

Test case 5: "Test merge function"
 - Tests the non-commutative property of add(a) + remove(a) in set a
 - Tests the non-commutative property of remove(a) + add(a) in set b
 - Merges the two sets to confirm correctness of the merge

Test case 6: "Test set in threaded environment simulated distributed environment"
 - 3 threads are spawned to simulate concurrent systems and the main thread
 - add() and remove() operations are carefully timed so they avoid the non-deterministic condition of a perfect concurrent add() and remove()
 - 4 sets in total (one for each thread), are merged and tested for correctness.
 

