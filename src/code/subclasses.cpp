// ============================================================================================
//
// testdata.cpp
//
// ============================================================================================

// ============================================================================================
// DESCRIPTION
// ============================================================================================

/*
*/

// ============================================================================================
// FORMAT OF THE TEST DATA STREAM
// ============================================================================================

/*
The test data stream can be any type of "istream" -- a file stream (i.e. an ASCII text file), a
large string stream, or even "cin" (possibly with some limitations).

The test data stream is read line by line.  It MUST adhere to the following format:

-----------------------------------------------------------------------------------------------

// This is a comment.  The two slashes MUST be the first two characters on the line.

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

Whitespace is not skipped; blank lines are ignored, though.

A colon in the first column of a line indicates that the rest of the line is a test name, and
the test cases that follow are all to be applied to that test's object.  Test names can occur
in more than one place in the test data stream, if desired.

A non-blank line that's neither a comment nor a test name is considered to be a single test
case.  These lines can be in whatever format is required by the test function associated with
the last-read test name -- except that the first characters can't be two slashes or a colon
(for obvious reasons).  Since test cases will be made available to test functions as streams,
it is recommended that the data in a test case be separated by whitespace so that the shift-in
operator (">>") can be used to easily parse the test case.

A test function can optionally read extra information on subsequent lines of the input stream.
This information can be in whatever format is required by the test method and be on as many
lines as needed.  The only restriction is that the first characters can't be two slashes or a
colon (for obvious reasons).  It's the user's responsiblity to ensure that test methods don't
inadvertently read a line that's a test case or a test name (the results of which would be
indeterminate).
*/

// ============================================================================================
// INCLUDE FILES
// ============================================================================================

#ifdef FAT_FILENAMES
  #include <strstrea.h>
#else
  #include <strstream.h>
#endif

#include <string.h>
#include <ctype.h>

#ifdef FAT_FILENAMES
  #include "testsuit.h"
#else
  #include "testsuite.h"
#endif

// ============================================================================================
// STATIC FUNCTION DECLARATIONS
// ============================================================================================

static char *const        newString(const char *const);
static const char *const startOfData(const char *const);
static const bool         isTestName(const char *const);
static const char *const extractTestName(const char *const);
static const bool         isComment(const char *const);

// ============================================================================================
// METHOD DEFINITIONS FOR TESTSUITE::TESTDATARAW
// ============================================================================================

/*********************************************************************************************/

TestSuite::TestDataRaw::TestDataRaw
(
  istream& dataStream
):

  _dataStream(&dataStream),
  _lineCounter(0UL)

{
  assert(_dataStream != NULL);

  _dataStream->seekg(0);
  return;
}

/*********************************************************************************************/

void TestSuite::TestDataRaw::reset()
{
  assert(_dataStream != NULL);

  _dataStream->clear();
  _dataStream->seekg(0);
  _lineCounter = 0UL;

  return;
}

/*********************************************************************************************/

const char *const TestSuite::TestDataRaw::readLine()
{
  assert(_dataStream != NULL);

  char* line = NULL;

  if (_dataStream->good())
  {
    char inputChar;

    _dataStream->get(inputChar);
    if (_dataStream->good())
    {
      ostrstream lineAsStream;

      while (_dataStream->good() && ((inputChar != '\n')))
      {
        lineAsStream.put(inputChar);
        _dataStream->get(inputChar);
      }

      lineAsStream.put('\0');
      ++_lineCounter;
      line = lineAsStream.str();

      assert(line != NULL);
    }
  }

  return line;
}

// ============================================================================================
// METHOD DEFINITIONS FOR TESTSUITE::TESTDATA
// ============================================================================================

/*********************************************************************************************/

TestSuite::TestData::TestData
(
  istream& dataStream
):

  TestDataRaw(dataStream),
  _lastLineRead(NULL)

{
  return;
}

/*********************************************************************************************/

TestSuite::TestData::~TestData()

{
  if (_lastLineRead != NULL)
    delete[] (char*)_lastLineRead;

  return;
}

/*********************************************************************************************/

const char *const TestSuite::TestData::readTestName()
{
  const char* testName = NULL;
  const char* line;

  if (_lastLineRead != NULL)
  {
    line          = _lastLineRead;
    _lastLineRead = NULL;
  }
  else
    line = readLine();

  while ((line != NULL) && (testName == NULL))
  {
    const char* cookedLine = line;

    while ((*cookedLine != '\0') && isspace(*cookedLine))
      ++cookedLine;

    if (isTestName(cookedLine))
    {
      testName = extractTestName(cookedLine);
      assert(testName != NULL);

      delete[] (char*)line;
    }
    else
    {
      delete[] (char*)line;
      line = readLine();
    }
  }

  return testName;
}

/*********************************************************************************************/

const char *const TestSuite::TestData::readTestCase()
{
  const char* testCase = NULL;
  const char* line     = readLine();

  assert(_lastLineRead == NULL);

  while ((line != NULL) && (testCase == NULL) && (_lastLineRead == NULL))
  {
    const char *const data = startOfData(line);

    assert(data != NULL);

    if (isTestName(data))
      _lastLineRead = line;
    else if ((strlen(data) == 0U) || isComment(data))
    {
      delete[] (char*)line;
      line = readLine();
    }
    else
    {
      testCase = newString(data);
      assert(testCase != NULL);

      delete[] (char*)line;
    }
  }

  return testCase;
}

// ============================================================================================
// METHOD DEFINITIONS FOR TESTSUITE::TESTCASE CLASS
// ============================================================================================

/*********************************************************************************************/

TestSuite::TestCase::TestCase
(
  const unsigned int number,
  const unsigned int lineCounter,
  const char *const  dataAsText
):

  _number(number),
  _lineCounter(lineCounter),
  _dataAsText(newString((dataAsText == NULL) ? "" : dataAsText)),
  _data((char*)_dataAsText)

{
  assert(dataAsText != NULL);

  return;
}

// ============================================================================================
// METHOD DEFINITIONS FOR TESTSUITE::TEST CLASS
// ============================================================================================

/*********************************************************************************************/

TestSuite::Test::Test()
{
  TestSuite::registerTest(this);
  return;
}

// ============================================================================================
// METHOD DEFINITIONS FOR TESTSUITE::LISTNODE
// ============================================================================================

/*********************************************************************************************/

TestSuite::ListNode::ListNode
(
  const TestSuite::Test *const test,
  TestSuite::ListNode *const   nextNode
):

  _test(test),
  _next(nextNode)

{
  assert(_test != NULL);
  return;
}

// ============================================================================================
// STATIC FUNCTION DEFINITIONS
// ============================================================================================

/*********************************************************************************************/

static char *const newString
(
  const char *const source
)

{
  assert(source != NULL);

  char *const duplicateString = new char[strlen(source) + 1U];

  if (duplicateString != NULL)
    strcpy(duplicateString, source);

  return duplicateString;
}

/*********************************************************************************************/

static const char *const startOfData
(
  const char *const text
)

{
  assert(text != NULL);

  const char* startPoint = text;

  while ((*startPoint != '\0') && isspace(*startPoint))
    ++startPoint;

  return startPoint;
}

/*********************************************************************************************/

static const bool isTestName
(
  const char *const text
)

{
  assert(text != NULL);

  return (text[0] == ':');
}


/*********************************************************************************************/

static const char *const extractTestName
(
  const char *const text
)

{
  assert(text != NULL);
  assert(text[0] == ':');

  char* testName = newString(text + 1);

  assert(testName != NULL);

  size_t length = strlen(testName);

  while ((length > 0U) && isspace(testName[length - 1U]))
    testName[--length] = '\0';

  return testName;
}

/*********************************************************************************************/

static const bool isComment
(
  const char *const stringToCheck
)

{
  assert(stringToCheck != NULL);

  static const char   commentId[] = "//";
  static const size_t commentIdLength = 2U;

  return (strncmp(stringToCheck, commentId, commentIdLength) == 0);
}
