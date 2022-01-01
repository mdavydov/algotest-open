/*  The Algotest library for cross-platform testing of algorithms
 Copyright (C) 2014-2015 Maksym Davydov, ADVA Soft
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; version 3
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; If not, see <http://www.gnu.org/licenses/>.
 
 */


#ifndef algotest_tests_included
#define algotest_tests_included

#include "algotest_c.h"
#include <string>
#include <vector>

namespace algotest
{
    struct TestResult
    {
        std::string m_name;
        std::string m_info;
        bool m_success = true;
    };

    class TestBase
	{
		std::string m_name;
        void (*m_pF)();

	public:
        TestBase(std::string name, void (*pF)());
        const std::string& getName() { return m_name; }
        void run()
        {
            m_pF();
        }
        /*
         * Add ALGOTEST_TERMINATE_APP_ON_FAILED_TEST to preprocessor definitions if you want to terminate application if any of the tests failed.
           If ALGOTEST_TERMINATE_APP_ON_FAILED_TEST is not defined, a string exception with "desc" is thrown.
         */
        static void stopAllTests();
        static void testFailed(std::string desc);
        static void testUnsupported(std::string desc);
        static bool runAllTests();
        static std::vector<TestResult> runAllTestsAndGatherInfo();
        static void run(std::string test_name);
	};
}

#define STOP_TESTS() algotest::TestBase::stopAllTests();
#define TEST_FAILED(a) algotest::TestBase::testFailed(a);
#define TEST_ASSERT(a) if (!(a)) TEST_FAILED("TEST_ASSERT(" #a ")");
#define TEST_ASSERT_EQ(a,b) if (!((a)==(b))) TEST_FAILED("TEST_ASSERT_EQ(" #a "==" #b ")");
#define TEST_ASSERT_NOT_EQ(a,b) if (((a)==(b))) TEST_FAILED("TEST_ASSERT_NOT_EQ(" #a "!=" #b ")");
#define TEST_ASSERT_FLOAT_EQ(a,b,max_error) if (fabs(double((a)-(b)))>(max_error)) \
    TEST_FAILED("TEST_ASSERT_FLOAT_EQ(" #a " == " #b " +-" #max_error ")");
#define TEST_ASSERT_FLOAT_NOT_EQ(a,b,max_error) if (fabs(double((a)-(b)))<=(max_error)) \
    TEST_FAILED("TEST_ASSERT_FLOAT_NOT_EQ(" #a " != " #b " +-" #max_error ")");
#define TEST_ASSERT_FVECT_EQ(a,b,max_error) TEST_ASSERT_FLOAT_EQ((a).distance(b), 0, max_error)
#define TEST_UNSUPPORTED(a) algotest::TestBase::testUnsupported(a);


#define DECLARE_TEST_COMPILE(a) \
    void Test_##a(); \
    static algotest::TestBase Register_Test_##a( #a, &Test_##a ); \
    void Test_##a()

#define DECLARE_TEST_SKIP(a) \
template<class TestBase952583570352783> void Test_##a()


#ifdef ALGOTEST
    #define COMPILE_TESTS_ALGOTEST
#endif

#ifdef COMPILE_TESTS_ALGOTEST
    #define DECLARE_TEST DECLARE_TEST_COMPILE
#else
    #define DECLARE_TEST DECLARE_TEST_SKIP
#endif

// use DECLARE_TEST_MAC for declaration of Mac-only tests
#if TARGET_OS_OSX==1
    #define DECLARE_TEST_MAC DECLARE_TEST
#else
    #define DECLARE_TEST_MAC DECLARE_TEST_SKIP
#endif

#endif // algotest_tests_included
