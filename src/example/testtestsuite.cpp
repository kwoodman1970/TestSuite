// ============================================================================
//
// SOURCE FILE:  testtestsuite.cpp
//
// ============================================================================

// ============================================================================
// DESCRIPTION
// ============================================================================

/*
This source file serves two purposes:  it provides examples of how to use the
"TestSuite" class and it tests the "TestSuite" class to ensure that it works
correctly.

This source file uses only ANSI C/C++ routines and therefore should work with
any ANSI-complient C++ compiler.
*/

// ============================================================================
// INCLUDE FILES
// ============================================================================

#include <fstream.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "testsuite.h"

#include <platform.h>
#include <shiftstringin.h>

//#if ((PF_COMPILER == PF_BORLAND) && (PF_COMPILER_VER > 0x0520))
//  #include <vcl\condefs.h>
//  #pragma hdrstop
//#endif

// ============================================================================
// BORLAND C++ BUILDER DECLARATIONS
// ============================================================================

#if ((PF_COMPILER == PF_BORLAND) && (PF_COMPILER_VER > 0x0520))
  USEUNIT("\Projects\Common\TestSuite\testsuite.cpp");
#endif

// ============================================================================
// GLOBAL CONSTANTS & VARIABLES
// ============================================================================

static const char testDataFileName[] = "testData.txt";    // test data filename

// ============================================================================
// TEST OBJECTS
// ============================================================================

/*****************************************************************************/

TEST(basicRead)

/*
This test object test's a "TestSuite" object's ability to actually read test
cases from a test data file.  It returns "abortAllTests" if the test fails.

Test case format:

<unsigned int first> <unsigned int second>

where "first" and "second" are two equal numbers.
*/

 {
  /*
  The idea here is to initialize two unsigned integer variables with unequal
  values, then parse two unsigned integers from the test case.  If the two
  variables are then equal then the parse is considered to be successful;
  otherwise, it isn't.
  */

  unsigned int first =  1U;             // the first unsigned integer variable
  unsigned int second = 2U;             // the second unsigned integer variable

 testCase.data() >> first >> second;

  if (first == second)
    return pass;
  else
   {
    log << "  " << first << " != " << second << endl;
    return abortAllTests;
   }
 }

/*****************************************************************************/

TEST(testTestName)

/*
This test object tests a "TestSuite" object's "testName()" method.

Test case format:

"testTestName"

Only one test case is needed.
*/

 {
  /*
  The idea here is to parse a test name from the test case and test to see if
  it's equal to what the "testName()" method returns.
  */

  const size_t testNameSize = 81U;           // max. size of a test name string
  char         testName[testNameSize] = "";  // the parsed test name

  testCase.data() >> ShiftASCIIStringIn(testName, testNameSize);

  if (strcmp(testName, name()) != 0)
  {
    log << "  Expected \"" << name() << "\" but got \"" << testName << "\"." << endl;
    return fail;
  }
  else
    return pass;
 }

/*****************************************************************************/

TEST(testTestCaseNum)

/*
This test object tests a "TestSuite" object's "testCaseNum()" method.

Test case format:

<unsigned int testCaeNum>

where "testCaseNum" is the test case number, starting at 0.
*/

 {
  /*
  The idea here is to parse the test case number from the test case and test to
  see if it's equal to what "testCaseNum()" returns.
  */

  unsigned int caseNum = UINT_MAX;               // the parsed test case number

  testCase.data() >> caseNum;

  if (caseNum != testCase.number())
  {
    log << "  Expected " << testCase.number() << ", but got " << caseNum << "." << endl;
    return fail;
  }
  else
    return pass;
 }

/*****************************************************************************/

TEST(testTestResult)

/*
This test object tests a "TestSuite" object's response to the various failure
values that could be returned by a test function.  The expected response is
sent to the report stream.

THE USER IS REQUIRED TO COMPARE THE EXPECTED RESPONSE TO THE ACTUAL RESPONSE
RECORDED IN THE REPORT STREAM.

Test case format:

<quoted testResult> <bool testCaseShouldBeApplied>

where "testResult" is one of "pass", "fail", "abortThisTest" and
"abortAllTests" and "testCaseShouldBeApplied" is one of 0 (false, meaning that
the test case shouldn't have been applied to this test function) or 1 (true,
meaning that the test case should be applied to this test function.
*/

 {
  const size_t testResultSize = 81U;
  char         testResult[testResultSize] = "";
  bool         testCaseShouldBeApplied = false;

  testCase.data() >> ShiftASCIIStringIn(testResult, testResultSize) >> testCaseShouldBeApplied;

  if (!testCaseShouldBeApplied)
   {
    log << "  Something went wrong -- test case " << testCase.number() << " shouldn't have "
      "been applied." << endl;
    return fail;
   }
  else if (strcmp(testResult, "fail") == 0)
   {
    log << "  Test case " << testCase.number() << " should fail..." << endl;
    return fail;
   }
  else if (strcmp(testResult, "abortThisTest") == 0)
   {
    log << "  Test case " << testCase.number() << " should fail and abort this test..." <<
      endl;
    return abortThisTest;
   }
  else if (strcmp(testResult, "abortAllTests") == 0)
   {
    log << "  Test case " << testCase.number() << " should fail and abort all testing..." <<
      endl;
    return abortAllTests;
   }
  else
   {
    log << "  Test case " << testCase.number() << " should pass..." << endl;
    return pass;
   }
 }

/*****************************************************************************/

TEST(stringPulling)

/*
This test function tests a "quoted" manipulator's ability to correctly extract
a quoted string from an input stream.

Test case format:

<unsigned int stringSelector> <quoted stringText>

where "stringSelector" is an index to an element in the "strings" array (which
is defined within this test functioN) and "stringText" is a quoted string that,
when parsed by the "quoted" manipulator, should be exactly the same as the
selected string in "strings".
*/

 {
  static const char *const strings[] =
   {
    "No escape characters.",
    "Escaped letters:  \a \b \f \n \r \t \v",
    "Hex chars:  \x05 \x65 \xBC",
    "Octal chars:  \007 \111 \247 \248",
    "Escaped symbols:  \' \" \\"
   };

  const size_t stringTextSize = 81U;
  char         stringText[stringTextSize] = "";
  unsigned int stringSelector = 0U;

  testCase.data() >> stringSelector >> ShiftCStringIn(stringText, stringTextSize);
  if (strcmp(stringText, strings[stringSelector]) == 0)
    return pass;
  else
   {
    log << "  Test case string = \"" << stringText << "\"; expected = \""
      << strings[stringSelector] << "\"" << endl;
    return fail;
   }
 }

/*****************************************************************************/

int main
 (
  const unsigned int argc,
  const char *const  argv[]
 )

 {
  ifstream testData(testDataFileName);

  assert(testData.good());

  TestSuite test(testData, cout);

  test.log() << "==========================================" << endl;
  test.log() << "Testing \"basicRead\"" << endl;
  test.log() << "==========================================" << endl;
  test.one("basicRead");

  test.log() << "==========================================" << endl;
  test.log() << "Testing \"stringPulling\" and \"testTestName\"" << endl;
  test.log() << "==========================================" << endl;
  test.group("stringPulling", "testTestName", NULL);

  test.log() << "==========================================" << endl;
  test.log() << "Tests from commmand-line arguments" << endl;
  test.log() << "==========================================" << endl;

  if (argc > 1U)
    test.group(argc - 1U, argv + 1);
  else
    test.log() << "No command-line arguments were found." << endl << endl;

  test.log() << "==========================================" << endl;
  test.log() << "Testing all" << endl;
  test.log() << "==========================================" << endl;
  test.all();

  return 0;
 }
