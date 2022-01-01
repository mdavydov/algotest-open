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
#include "algotest_tests.h"
#include "algotest_c.h"
#include <stdlib.h>
#include <vector>

namespace algotest
{
    class StopTestException
    {
    public:
    };
    
    class UnsupportedTestException: public std::string
    {
    public:
        UnsupportedTestException(std::string s) : std::string(s) {}
    };
    
    std::vector<TestBase*>& getAllTests()
    {
        static std::vector<TestBase*> g_all_tests;
        return g_all_tests;
    }
    
    TestBase::TestBase(std::string name, void (*pF)()) : m_name(name), m_pF(pF)
    {
        getAllTests().push_back(this);
    }
    
    void TestBase::testFailed(std::string desc)
    {
#ifdef ALGOTEST_TERMINATE_APP_ON_FAILED_TEST
        const char * str = desc.c_str();
        LOGE("%s", str);
        abort();
#else
        throw desc;
#endif
    }
    
    void TestBase::stopAllTests()
    {
        throw StopTestException();
    }
    
    void TestBase::testUnsupported(std::string desc)
    {
        throw UnsupportedTestException(desc);
    }
    
    bool TestBase::runAllTests()
    {
        for(TestBase * p : getAllTests())
        {
            try
            {
                printf("Running test %s...", p->getName().c_str());
                p->run();
                printf("ok.\n");
            }
            catch (const UnsupportedTestException& err)
            {
                LOGE("not supported! %s\n", err.c_str());
            }
            catch (const StopTestException& )
            {
                LOGE("stopped!\n");
                break;
            }
            catch (const std::string& err)
            {
                LOGE("failed! %s\n",err.c_str());
            }
        }
        return true;
    }

    std::vector<TestResult> TestBase::runAllTestsAndGatherInfo()
    {
        std::vector<TestBase*>& all_tests = getAllTests();
        std::vector<TestResult> test_results;
        test_results.reserve(all_tests.size());
        
        for(TestBase * p : all_tests)
        {
            TestResult result;
            try
            {
                result.m_name = p->getName();
                p->run();
            }
            catch (const UnsupportedTestException& err)
            {
                result.m_info = err;
            }
            catch (const std::string& err)
            {
                result.m_info += "\n" + err;
                result.m_success = false;
            }
            test_results.push_back(result);
        }
        return test_results;
    }
    
    void TestBase::run(std::string test_name)
    {
        for(TestBase * p : getAllTests())
        {
            if (p->getName()!=test_name) continue;
            try
            {
                printf("Running test %s...", p->getName().c_str());
                fflush(stdout);
                p->run();
                printf("ok.\n");
                return;
            }
            catch (std::string err)
            {
                printf("failed! %s\n",err.c_str());
                abort();
            }
        }
        TEST_FAILED("Test " + test_name + " was not found");
    }
}
