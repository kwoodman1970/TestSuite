# TestSuite (C/C++) &ndash; A Framework for Black-Box Testing C/C++ Code
**Status:**  Inactive<br />
*This project is not being actively developed at this time but has not been abandoned either.*

## About This Project

In the mid 1990's, I was doing my internship before my final year as a student at the [University of Alberta](https://www.ualberta.ca).  I was testing some code that I'd written one day and I began to think about how to automate this kind of testing.  Some companies were starting to sell software for black-box testing but the data types were limited to primitive C types and therefore would not be suitable for C++ objects.  I then thought, "What if C++ code could be used to test C++ code &ndash; including objects?"

That got me thinking even further.  The testing framework would need to be easy to code for and test cases would need to be easy to write.  Each tester would be its own object.  Each test case would be parsed from a line of text (additional test case data could be pulled from subsequent lines of text).  Each tester would report whether the subject of the test passed or failed a test case and indicate whether testing should continue or not.  It would be easy for tests and test cases to grow and change as the test subject grew and underwent maintenance.  There would be a feature for logging the results of the tests.

My employer at the time opted not to make use of what I'd proposed, so I continued to develop this project on my own.  This is the result.

*Personal self-agrandizing, ego-inflating thought:*  It is somewhat gratifying to note that [Jest](https://jestjs.io/) is based on a similar premise &ndash; using JavaScript code to test JavaScript code.  It seems that I was decades ahead of my time.

## How to Install

Use the green `<> Code` button to either clone this repository or download the zip file.

To install, first copy the contents of the `src/headers` directory to to a directory for your compiler's 3<sup>rd</sup>-party standard header files.

Next, compile and link the contents of the  `src/code` directory into `testsuite.lib` and place it in a directory for 3<sup>rd</sup>-party library files.

Do **not** place these files in the same directories as the files that shipped with your compiler &ndash; always use a separate directory (or directories) for 3<sup>rd</sup>-party files.

Each compiler is different &ndash; refer to your compiler's manual for more information.

## How to Use

*All source files are (almost) fully documented &ndash; what follows is a summary.*

Generally speaking, as per Best Practices, place the following directive in your source file(s) somewhere after all standard C/C++ library header files and before all project header files:

```c
#include <testsuite.h>
```

Once the test source files and test data have been created (see below), they can be compiled and linked (along with the test subject's object or library files) into an executable file and executed.

### Coding a Test Source File

A test source file could look like this:

```c
#include <fstream.h>

#include <testsuite.h>

#include "testsubject.h"  // header file for code to be tested

TEST(test01)
{
  /* Code to test first thing */
}

TEST(test02)
{
  /* Code to test second thing */
}

TEST(test03)
{
  /* Code to test third thing */
}

void main()
{
  ifstream  testData("testdata.txt");
  TestSuite test(testData, cout);

  test.all();

  return;
}
```

### Coding a Test Object Method

Test objects are defined with the `TEST()` macro.  The macro takes a single argument:  a test name.  The test name MUST be a valid C++ variable name, but otherwise is left up to the programmer.

After a test object has been defined with the `TEST()` macro, the body of the test method MUST be coded immediately after it &ndash; like this:

```c
  TEST(anyOldName)
  {
    // body of test method goes here.
    return result;
  }
```

The test method has three hidden arguments:  `TestSuite::TestCase& testCase`,
`TestSuite::TestDataRaw& testData` and `ostream& log`.  `testCase` contains the test case
information to be applied to the test, which can be shited out piece by piece with the `>>`
operator.  If a large block of data is required to complete the test (such as an ASCII-encoded
bitmap) then `testData.readLine()` can be called to read in additional lines from the test data
stream.  Human-readable test results (or any useful information) can be shifted out into `log`.

The test method's return type is `TestSuite::TestResult` and the value can be one of `pass`, `fail`, `abortThisTest` or `abortAllTests`.

### Writing Test Cases

Any `istream` will work but a text file is probably the most convenient place to store test case data.

The `istream` MUST be in the following format:

```
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
```

A leading colon denotes the start of a block of test cases and each `<test name>` corresponds to a test name passed to the `TEST()` macro (see above).  Each `<test case>` is a single line of test case data &ndash; everything the test method needs to carry out a test.  If one line isn't sufficient then additional data (e.g. text-encoded binary data) can be placed on subsequent lines.

### Example

`src/example/testtestsuite.cpp` will test TestSuite &ndash; how meta is that?

It requires my [Platform](https://github.com/kwoodman1970/Platform) library plus a special class that safely shifts strings from an `istream` into a `char[]` array which, unfortunately, has been lost to a file system failure (but may be in an old backup somewhere).  It's therefore not possible to compile & execute this program at this time.

## TODO

- Try to recover that lost shift-string-in class so that the example will compile
- Finish documenting the source files
- Create proper documentation (possibly using [Sphinx](https://www.sphinx-doc.org/)) instead of simply saying, "Look at the source files"

## How to Contribute

I'm not accepting contributions to this project at this time.