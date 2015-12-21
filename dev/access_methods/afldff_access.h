#ifndef AFL_DFF_ACCESS
#define AFL_DFF_ACCESS


/*****************************************************
 *These are thread safe functions for accessing the  *
 *packet information.                                *
 *****************************************************/

//return test cases for an individual machine 
long long get_test_case(unsigned int id);

//return crash cases for an individual machine 
long long get_crash_case(unsigned int id);


//return the sum of all crash cases encountered so far.
long long get_all_test_cases();

//return the sum of all crash cases encountered so far.
long long get_all_crash_cases();

#endif
