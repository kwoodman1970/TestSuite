#ifndef TESTSUITE_H
#define TESTSUITE_H

// ============================================================================================
//
// HEADER FILE:  testsuite.h
//
// ============================================================================================

// ============================================================================================
// INCLUDE FILES
// ============================================================================================

#include <iostream.h>
#include <stdarg.h>
#include <assert.h>

#ifdef FAT_FILENAMES
  #include <strstrea.h>
#else
  #include <strstream.h>
#endif

#include <platform.h>

// ============================================================================================
// MACRO DEFINITIONS
// ============================================================================================

#define TEST(testName)                                                                        \
  class TestSuite_Test_##testName##:                                                          \
    public TestSuite::Test                                                                    \
  {                                                                                           \
    public:                                                                                   \
      virtual const char *const name() const                                                  \
                                  {return #testName;}                                         \
      virtual const TestResult  testMethod();                                                 \
  };                                                                                          \
                                                                                              \
  TestSuite_Test_##testName testName;                                                         \
                                                                                              \
  const TestSuite::Test::TestResult TestSuite_Test_##testName##::testMethod()                 \

// ============================================================================================
// TESTSUITE CLASS DECLARATION
// ============================================================================================

class TestSuite
{
  public:

    // ----------------------------------------------------------------------------------------

    class TestDataRaw
    {
      public:
                                TestDataRaw(istream&);

        const char *const       readLine();
        const unsigned long int lineCounter() const
                                  {return _lineCounter;}

      private:
        friend class TestSuite;

        istream *const    _dataStream;
        unsigned long int _lineCounter;

        void reset();
    };

    // ----------------------------------------------------------------------------------------

    class TestData:
      public TestDataRaw
    {
      public:
                          TestData(istream&);
                          ~TestData();

        const char *const readTestName();
        const char *const readTestCase();

      private:
        const char* _lastLineRead;       // the last line of text that was read from readLine()
    };

    // ----------------------------------------------------------------------------------------

    class TestCase
    {
      public:
                           TestCase(const unsigned int, const unsigned int, const char *const);
                           ~TestCase()
                             {delete[] (char*)_dataAsText; return;}

        const unsigned int number() const
                             {return _number;}
        const unsigned int lineCounter() const
                             {return _lineCounter;}
        istream&           data()
                             {return _data;}

      private:
        const unsigned int _number;       // which test case this is (in order, starting at 1)
        const unsigned int _lineCounter;  // the line in the data stream where it was found
        const char *const  _dataAsText;   // the entire test case information as a line of text
        istrstream         _data;         // the entire test case information as an istream
    };

    // ----------------------------------------------------------------------------------------

    class Test
    {
      public:
        enum TestResult             // result codes to be returned by user-written test methods
        {
          pass,           // the test passed
          fail,           // the test failed
          abortThisTest,  // the test failed, and the remaining test cases should be skipped
          abortAllTests   // the test failed, and testing should cease (a catastrophe occurred)
        };

                                  Test();
        virtual const char *const name() const = 0;

	    protected:
	      TestSuite::TestCase&      testCase()
	                                  {return *_testCase;}
	      TestSuite::TestDataRaw&   testData()
	                                  {return *_testData;}
	      ostream&                  log()
	                                  {return *_log;}

      private:
        friend class TestSuite;

        TestSuite::TestCase*    _testCase;
        TestSuite::TestDataRaw* _testData;
        ostream*                _log;

        void                     setData(TestSuite::TestCase&, TestSuite::TestDataRaw,
                                   ostream);
        #ifndef NDEBUG
          void                   assertReady() const;
        #endif
        virtual const TestResult testMethod() const = 0;
    };

    // ----------------------------------------------------------------------------------------

    static void registerTest(const Test *const);

                TestSuite(istream&, ostream&);
    void        one(const char *const);
    void        group(const char *const, ...);
    void        group(const unsigned int, const char *const *const);
    void        all();
    ostream&    log() const
                  {assert(_log != NULL); return *_log;}

  protected:
    virtual void logHeader() const
                   {return;}
    virtual void logTestHeader(const Test&) const;
    virtual void logUnknownTestName(const char *const) const;
    virtual void logTestCasePassed(const Test&, const TestCase&) const
                   {return;}
    virtual void logTestCaseFailed(const Test&, const TestCase&) const;
    virtual void logTestAborted(const Test&) const;
    virtual void logAllTestsAborted() const;
    virtual void logTestFooter(const Test&, const unsigned int, const unsigned int) const;
    virtual void logFooter() const
                   {return;}

  private:
    class ListNode
    {
      public:
                          ListNode(const Test *const, ListNode *const = NULL);
        ListNode *const   next() const
                            {return _next;}
        void              setNext(ListNode *const newNext)
                            {_next = newNext; return;}
        const Test *const test() const
                            {assert(_test != NULL); return _test;}

      private:
        const Test *const _test;                         // points to a test object
        ListNode*         _next;                         // points to the next node in the list
    };

    // ----------------------------------------------------------------------------------------

    static ListNode*   _tests;                  // list of tests
    static bool        _atExitRegistered;       // has the atExit() method been registered yet?

    TestData           _testData;               // source stream of test data
    ostream *const     _log;                    // where all test results are logged
    unsigned int       _totalTestCases;         // total no. of test cases applied
    unsigned int       _totalFailedTestCases;   // total no. of failed test cases

    static const Test *const getTest(const char *const, const ListNode *const);
    static void              deleteList(const ListNode *const);
    static void              atExit();

    void                     prepareForTesting();
    const ListNode *const    getTests(const char *const, va_list&) const;
    const ListNode *const    getTests(const unsigned int, const char *const *const) const;
    void                     runTests(const ListNode *const);
    const bool               runTest(Test&);

    void                     assertInvariants() const;
};

#endif
