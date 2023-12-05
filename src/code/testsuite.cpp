// ============================================================================================
//
// SOURCE FILE:  testsuite.cpp
//
// ============================================================================================

// ============================================================================================
// DESCRIPTION
// ============================================================================================

/*
This file implements a simple test management class.  It was originally intended to manage
black-box test functions that test other user-written routines and functions for correctness,
but it can be used for any kind of black-box testing.

Here are some terms that are used throughout this file:

USER        -- A programmer who uses the "TestSuite" class.
TEST        -- An algorithm for determining if something is correct or not.
TEST NAME   -- A unique name for a test.
TEST OBJECT -- A C++ object that encapsulates a test name and a test method.  The object's
               class is a descendent of the "TestSuite::Test" nested class.
TEST METHOD -- A user-written C++ method that performs a test by applying a test case to
               it.  The method is responsible for determining whether or not a test case passes
               or fails the test, and whether there's any point in applying further test cases
               (or even performing further tests).
TEST CASE   -- A single case (typically a set of inputs and expected outputs) that's applied to
               a test method.

Generally speaking, a series of test names and their associated test cases are read from a
stream (typically an ASCII text file).  The test cases are then individually applied to their
respective test methods in the order in which they were read.

To use "TestSuite", the user must do the following:

1.  Create a series of uniquely-named test objects in C++ source files.
2.  Instanciate a "TestSuite" object, then have that object perform whichever tests are
    desired.
3.  Create test cases for the tests and place them in a test data stream (typically a text
    file).

These steps are explained more fully in the following sections.

After that, the source files must be compiled and linked with the test suite object or library
file (plus whatever other object or library files are used by the test functions) to create an
executable.  Command-line executables are recommended because they're simpler to create and
easier to log.

This source file uses only ANSI C/C++ routines and therefore should work with any ANSI-complient
C++ compiler.
*/

// ============================================================================================
// CREATING TEST OBJECTS
// ============================================================================================

/*
Test objects are defined with the "TEST()" macro in a C++ source file.  The macro takes a
single argument:  a test name.  The test name must be a valid C++ variable name, but otherwise
is left up to the user.

After a test object has been defined with the "TEST()" macro, the body of the test method must
be coded immediately after it -- like this:

  TEST(anyOldName)
  {
    // body of test method goes here.
    return result;
  }

The test method has three hidden arguments:  "TestSuite::TestCase& testCase",
"TestSuite::TestDataRaw& testData" and "ostream& log".  "testCase" contains the test case
information to be applied to the test, which can be shited out piece by piece with the ">>"
operator.  If a large block of data is required to complete the test (such as an ASCII-encoded
bitmap) then "testData.readLine()" can be called to read in additional lines from the test data
stream.  Human-readable test results (or any useful information) can be shifted out into "log".

The test method's return type is "TestSuite::TestResult".
*/

// ============================================================================================
// FORMAT OF THE TEST DATA STREAM
// ============================================================================================

/*
The test data stream can be any type of "istream" -- a file stream (i.e. an ASCII text file), a
large string stream, or even "cin" (possibly with some limitations).

The test data stream is read line by line.  It MUST adhere to the following format:

-----------------------------------------------------------------------------------------------

// This is a comment.  The two slashes MUST be the first two non-whitespace characters on the
// line.

:<test name>
<test case>
<test case>
<test case>

:<test name>
<test case>
<test case>
[extra information required by the above test case]
[extra information required by the above test case]
<test case>
<test case>

-----------------------------------------------------------------------------------------------

A line starting with two slashes (//) denotes a comment and will be ignored.

Leading whitespace is skipped, and blank lines are ignored.

A colon in the first column of a line indicates that the rest of the line is a test name, and
the test cases that follow are all to be applied to that test's object.  Test names can occur
in more than one place in the test data stream, if desired.

Otherwise, a non-blank, non-comment line is considered to be a single test case.  These lines
can be in whatever format is required by the test method associated with the last-read test
name.  Since test cases will be made available to test methods as streams, it is recommended
that the data in a test case be separated by whitespace so that the shift-in operator (">>")
can be used to easily parse the test case.

A test method can optionally read extra information on subsequent lines of the input stream.
This information can be in whatever format is required by the test method and be on as many
lines as needed.  The only restriction is that the first characters can't be two slashes or a
colon (for obvious reasons).  It's the user's responsiblity to ensure that test methods don't
inadvertently read a line that's a test case or a test name (the results of which would be
indeterminate).
*/

// ============================================================================================
// EXAMPLE
// ============================================================================================

/*
See "testtestsuite.cpp" and "testdata.txt" for a good example of how to use "TestSuite" -- they
test "TestSuite" to make sure that it works properly!
*/

// ============================================================================================
// INCLUDE FILES
// ============================================================================================

#ifdef FAT_FILENAMES
  #include <strstrea.h>
#else
  #include <strstream.h>
#endif

#include <iomanip.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#ifdef FAT_FILENAMES
  #include "testsuit.h"
#else
  #include "testsuite.h"
#endif

#include <platform.h>

// ============================================================================================
// STATIC MEMBER INITIALIZATIONS FOR TESTSUITE CLASS
// ============================================================================================

TestSuite::ListNode* TestSuite::_tests            = NULL;
bool                 TestSuite::_atExitRegistered = false;

// ============================================================================================
// PUBLIC METHOD DEFINITIONS FOR TESTSUITE CLASS
// ============================================================================================

/*********************************************************************************************/

TestSuite::TestSuite
(
  istream& testData,                   // source of test data
  ostream& log                         // test results and other information is to be sent here
):

/*
This is the constructor for class "TestSuite".

"testData" is the input stream from which test cases are pulled.  "log" is the output stream to
which all test results, etc., are written to.

PRECONDITIONS:
"testData" and "log" must be open streams.

POSTCONDITIONS:
A valid "TestSuite" object is created and ready to test the test objects.
*/

  _testData(testData),
  _log(&log),
  _totalTestCases(0U),
  _totalFailedTestCases(0U)

{
  assertInvariants();
  return;
}

/*********************************************************************************************/

void TestSuite::one
(
  const char *const testName                                 // the name of the test to perform
)

/*
This method performs a single test (whose name is given in "testName") by applying all of its
test cases.

Tests are performed in the order in which they appear in the test data stream.

PRECONDITIONS:
"testName" can't be NULL.

POSTCONDITIONS:
All test cases for "testName" in the test data stream (if any) will have been applied to the
specified test object.
*/

{
  assertInvariants();
  assert(testName != NULL);

  prepareForTesting();
  logHeader();

  const ListNode *const tests = getTests(1U, &testName);    // list of (1) test to be performed

  runTests(tests);
  deleteList(tests);
  logFooter();

  assertInvariants();
  return;
}

/*********************************************************************************************/

void TestSuite::group
(
  const char *const firstTestName,                    // the name of the first test to perform
  ...
)

/*
This method performs all the tests given in the arguments by applying their respective test
cases to them.  All arguments MUST be of type "const char *const", and the last argument must
be NULL.

Tests are performed in the order in which they appear in the test data stream.

PRECONDITIONS:
"firstTestName" can't be NULL.

POSTCONDITIONS:
All test cases in the test data stream (if any) will have been applied to the test objects
specified in the argument list.
*/

{
  assertInvariants();
  assert(firstTestName != NULL);

  prepareForTesting();
  logHeader();

  va_list argList;                                          // the list of remaining test names

  va_start(argList, firstTestName);

  const ListNode *const tests = getTests(firstTestName, argList);   // list of tests to perform

  va_end(argList);

  runTests(tests);
  deleteList(tests);
  logFooter();

  assertInvariants();
  return;
}

/*********************************************************************************************/

void TestSuite::group
(
  const unsigned int       numTestNames,     // the number of test names in the array
  const char *const *const testNames         // an array of test names of tests to be performed
)

/*
This method performs all the tests in "testNames" by applying their respective test cases to
them.

Tests are performed in the order in which they appear in the test data stream.

PRECONDITIONS:
"numTests" can't be 0U and "testNames" can't be NULL.  No element in "testNames" can be NULL,
either.

POSTCONDITIONS:
All test cases in the test data stream (if any) will have been applied to the test objects
specified in "testNames".
*/

{
  assertInvariants();
  assert(numTestNames > 0U);
  assert(testNames != NULL);

  prepareForTesting();
  logHeader();

  const ListNode *const tests = getTests(numTestNames, testNames);  // list of tests to perform

  runTests(tests);
  deleteList(tests);
  logFooter();

  assertInvariants();
  return;
}

/*********************************************************************************************/

void TestSuite::all()

/*
This method performs all the tests by applying their respective test cases to them.

Tests are performed in the order in which they appear in the test data stream.

PRECONDITIONS:
None.

POSTCONDITIONS:
All test cases in the test data stream (if any) will have been applied to all registered test
objects.
*/

{
  assertInvariants();

  prepareForTesting();
  logHeader();
  runTests(_tests);
  logFooter();

  assertInvariants();
  return;
}

// ============================================================================================
// PRIVATE METHOD DEFINITIONS FOR TESTSUITE CLASS
// ============================================================================================

/*********************************************************************************************/

void TestSuite::registerTest
(
  const Test *const test                                    // the test object to be registered
)

/*
This method registers tests objects and is called by the "TestSuite::Test" constructor.

At the same time, if it hasn't been done yet, the "atExit()" method is registered as a routine
to be called when the program terminates.

PRECONDITIONS:
"test" can't be NULL.

POSTCONDITIONS:
The test object "test" is registered and test cases can be applied to it.
*/

{
  assert(test != NULL);

  if (!_atExitRegistered)
  {
    assert(_tests == NULL);

    const int status = atexit(atExit);     // resulting status of atexit() registration attempt

    assert(status == 0);
    _atExitRegistered = (status == 0);
  }

  _tests = new ListNode(test, _tests);
  assert(_tests != NULL);

  return;
}

/*********************************************************************************************/

const TestSuite::Test *const TestSuite::getTest
(
  const char *const     testName,
  const ListNode *const tests
)

/*
This function finds the test object in "tests" that has the same name as "testName".

PRECONDITIONS:
"testName" can't be NULL.

POSTCONDITIONS:
Test object with the same name as "testName" is returned, or NULL is returned if no such object
exists in "tests".
*/

{
  assert(testName != NULL);

  const ListNode* current = tests;                                    // iterates through tests

  while ((current != NULL) && (strcmp(current->test()->name(), testName) != 0))
    current = current->next();

  return (current != NULL ? current->test() : NULL);
}

/*********************************************************************************************/

void TestSuite::deleteList
(
  const ListNode *const list                    // the list of nodes to de-allocate from memory
)

/*
This routine de-allocates all "ListNode's" in "list" from memory.

PRECONDITIONS:
None.

POSTCONDITIONS:
All "ListNode's" in "list" are de-allocated from memory.  It is an error to dereference "list"
after this routine exits.
*/

{
  const ListNode* current = list;                                      // iterates through list

  while (current != NULL)
  {
    const ListNode *const victim = current;  // ListNode for de-allocation in current iteration

    current = current->next();
    delete (ListNode*)victim;
  }

  return;
}

/*********************************************************************************************/

void TestSuite::atExit()

/*
This routine de-allocates all "ListNode's" in "_tests" from memory.  It should be called after
the program has terminated, which means that it must be registered with "atexit()".

PRECONDITIONS:
None.

POSTCONDITIONS:
All "ListNode's" in "_tests" are de-allocated from memory, thus preventing a memory leak when
the program terminates.
*/

{
  deleteList(_tests);
  return;
}

/*********************************************************************************************/

void TestSuite::prepareForTesting()

/*
This method prepares a "TestSuite" object to perform a series of tests by reseting a few member
variables and the test data stream.
*/

{
  assertInvariants();

  _totalTestCases       = 0U;
  _totalFailedTestCases = 0U;

  _testData.reset();

  assertInvariants();
  return;
}

/*********************************************************************************************/

const TestSuite::ListNode *const TestSuite::getTests
(
  const char *const firstTestName,                // the first test name to look up
  va_list&          argList                       // the remaining test names to look up
)
const

/*
This method returns a NULL-terminated array of pointers to the test objects specified by the
arguments.  It is the caller's responsibility to eventually de-allocate the array (but NOT the
test objects).

"firstTestName" and "argList" are test names for the test objects to be included in the
returned array ("argList" should refer to a series of "const char *const" types).  Only test
objects that match these test names will be included.  If no test object can be found then the
offending test name is logged.

PRECONDITIONS:
"firstTestName" can't be NULL and "numTestNames" must be greater than zero.  Also,
"firstTestName" and "argList" together must have as many test names as is specified in
"numTestNames".

POSTCONDITIONS:
A NULL-terminated array of pointers to test objects is returned.
*/

{
  assertInvariants();

  ListNode*   tests    = NULL;
  const char* testName = firstTestName;                  // current test name to look up

  /*
  This is the main loop.  It iterates through all of the test names, finds their respective
  test objects (if any) and adds them (if any) to the array to be returned.
  */

  while (testName != NULL)
  {
    const Test *const test = getTest(testName, _tests);

    if (test == NULL)
      logUnknownTestName(testName);
    else
    {
      tests = new ListNode(test, tests);
      assert(tests != NULL);
    }

    testName = va_arg(argList, const char*);
  }

  return tests;
}

/*********************************************************************************************/

const TestSuite::ListNode *const TestSuite::getTests
(
  const unsigned int       numTestNames,                 // the number of test names to look up
  const char *const *const testNames                     // the test names to look up
)
const

/*
This method returns a NULL-terminated array of pointers to the test objects specified by the
"testNames".  It is the caller's responsibility to eventually de-allocate the array (but NOT
the test objects).

"testNames" is an array of test names for the test objects to be included in the returned
array.  Only test objects that match these test names will be included.  If no test object can
be found then the offending test name is logged.

PRECONDITIONS:
"testNames" can't be NULL and "numTestNames" must be greater than zero.  Also, there must be as
many test names in "testNames" as is specified in "numTestNames".

POSTCONDITIONS:
A NULL-terminated array of pointers to test objects is returned.
*/

{
  assertInvariants();
  assert((numTestNames == 0U) || (testNames != NULL));

  ListNode*    tests         = NULL;
  unsigned int testNameIndex = 0U;                         // for iterating through testNames

  /*
  This is the main loop.  It iterates through all of the test names, finds their respective
  test objects (if any) and adds them (if any) to the array to be returned.
  */

  while (testNameIndex < numTestNames)
  {
    const char *const testName = testNames[testNameIndex++];

    assert(testName != NULL);

    const Test *const test = getTest(testName, _tests);

    if (test == NULL)
      logUnknownTestName(testName);
    else
    {
      tests = new ListNode(test, tests);
      assert(tests != NULL);
    }
  }

  return tests;
}

/*********************************************************************************************/

void TestSuite::runTests
(
  const ListNode *const tests
)

/*
This method applies the test data in "_testData" to the tests in "tests".  Any tests that are
mentioned in "_testData" but haven't been registered will be logged.

PRECONDITIONS:
"tests" can't be NULL, and there must be a NULL sentinal in the array that "tests" points to.

POSTCONDITIONS:
All test cases in the test data stream (if any) will have been applied to the test objects
pointed to by "tests".
*/

{
  assertInvariants();

  if (tests == NULL)
    *_log << "*** No valid test names were provided! ***" << endl << endl;
  else
  {
    bool        abortAll = false;                           // should all testing be stopped?
    const char* testName = _testData.readTestName();        // last test name read from _testData

    /*
    This is the main loop.  During each iteration, a test name is sought in the test data stream
    and, if an associated test object appears in "tests", its test method is called.

    The loop terminates when either the test method requests that all testing be stopped or no
    test names can be retrieved from the test data stream.
    */

    while (!abortAll && (testName != NULL))
    {
      const Test *const test = getTest(testName, tests);

      if (test != NULL)
        abortAll = !runTest(*test);

      delete[] (char*)testName;
      if (!abortAll)
        testName = _testData.readTestName();
    }

    assertInvariants();
  }

  return;
}

/*********************************************************************************************/

const bool TestSuite::runTest
(
  TestSuite::Test& test
)

/*
This method performs a single test by applying all of its test cases to it.  It
stops when it detects either a new section of test cases or an error state
(including EOF) in "_testData".

"_testData" must be ready to read a test case.

"_testData" will be left ready to read the first test case for the next section
of test cases (if any).  "_testName" will contain the test function name for
the next section of test cases (if any).
*/

{
  assertInvariants();

  unsigned int testCaseNum = 0U;

  bool         abortTest = false;        // should the current test be stopped?
  bool         abortAll  = false;
  unsigned int numFailedTestCases = 0U;  // total number of failed test cases
  const char*  testCaseData = _testData.readTestCase();

  logTestHeader(test);

  /*
  This is the main loop.  During each iteration, a test case is read from
  "_testData" and used to create "_testCase".  The appropriate test function
  is then called and its result code processed.

  The loop terminates when either a new test function name or an error state
  (including EOF) is detected in "_testData".
  */

  while (!abortTest && (testCaseData != NULL))
  {
    testCaseNum++;

    TestCase testCase(testCaseNum, _testData.lineCounter(), testCaseData);

    test.setData(testCase, _testData, *_log);

    const Test::TestResult testResult = test.testMethod();

    if (testResult == Test::pass)
      logTestCasePassed(test, testCase);
    else
    {
      numFailedTestCases++;
      logTestCaseFailed(test, testCase);

      if (testResult != Test::fail)
      {
        abortTest = true;

        if (testResult == Test::abortAllTests)
        {
          abortAll = true;
          logAllTestsAborted();
        }
        else
          logTestAborted(test);
      }
    }

    delete[] (char*)testCaseData;
    testCaseData = _testData.readTestCase();
  }

  delete[] (char*)testCaseData;
  logTestFooter(test, testCaseNum, numFailedTestCases);

  _totalTestCases       += testCaseNum;
  _totalFailedTestCases += numFailedTestCases;

  return !abortAll;
}

/*********************************************************************************************/

void TestSuite::logTestHeader
(
  const TestSuite::Test& test
)
const

/*
This method sends a start-of-test message to "report()".

It's called just before a series of test cases is applied to a test function.
*/

{
  assert(test.name() != NULL);

  log() << "-------------------------------------------------------------------------------" <<
    endl;
  log() << "Test name:  \"" << test.name() << "\"" << endl;
  log() << endl;
  return;
}

/*********************************************************************************************/

void TestSuite::logUnknownTestName
(
  const char *const testName    // name of the test that hasn't been registered
)
const

/*
This method sends an unknown-test-function message to "report()".
*/

{
  assert(testName != NULL);

  log() << "-------------------------------------------------------------------------------" <<
    endl;
  log() << "\"" << testName << "\" is not a registered test object." << endl;
  log() << endl;
  return;
}

/*********************************************************************************************/

void TestSuite::logTestCaseFailed
(
  const TestSuite::Test&     test,
  const TestSuite::TestCase& testCase
)
const

/*
This method sends a failed-test-case message to "report()".

It's called just after a test function reports that a test case has failed.
*/

{
  assert(test.name() != NULL);

  log() << endl;
  log() << "Test case failed -- \"" << test.name() << "\"[" << testCase.number() << "] (line "
    << testCase.lineCounter() << ")" << endl;
  log() << endl;
  return;
}

/*********************************************************************************************/

void TestSuite::logTestAborted
(
  const Test&
)
const

/*
This method sends an abort-test message to "report()".

It's called just after a test function reports that a test case has failed and
no further test cases should be applied.
*/

{
  log() << "*** The remaining test cases have been skipped. ***" << endl;
  log() << endl;
  return;
}

/*********************************************************************************************/

void TestSuite::logAllTestsAborted() const

/*
This method sends an abort-all-tests message to "report()".

It's called just after a test function reports that a test case has failed and
no further tests should be performed.
*/

{
  log() << "*** Testing has been aborted. ***" << endl;
  log() << endl;
  return;
}

/*********************************************************************************************/

void TestSuite::logTestFooter
(
  const Test&        test,
  const unsigned int numCases,
  const unsigned int numFailedCases   // number of test cases that failed
)
const

/*
This method sends a summary of test results to "report()".

It's called after the last test case has been applied to a test function.
*/

{
  assert(numCases >= numFailedCases);

  log() << numFailedCases << " of " << numCases << " test case" <<
    (numCases == 1 ? " that was" : "s that were") << " applied to test \"" << test.name() <<
     "\" failed." << endl;
  log() << endl;
  return;
}

/*********************************************************************************************/

void TestSuite::assertInvariants() const

{
  /*
  INVARIANTS:  "_testData", "_log" and "_tests" can't be NULL.
  */

  assert(_log != NULL);
  assert(_tests != NULL);

  /*
  INVARIANT:  The total number of test cases applied cannot be less than the total number that
  failed.
  */

  assert(_totalTestCases >= _totalFailedTestCases);

  return;
}
