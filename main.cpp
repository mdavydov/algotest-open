#include "algotest_tests.h"
#include "algotest_c.h"
#include "algotest_tensor.h"

using namespace algotest;

DECLARE_TEST(myTest)
{
    tensor<int> a({2,3}, initializer(1) );
    auto b = tensor<int>::zeros({ 2,3 });

    int i = 0;
    b.apply([&i](int& a) {a = i; i += 5; });

    LOGI_(b);
 //   STOP_TESTS();
}



int main()
{
    tensor<int> a = tensor<int>::arange(8).reshape({2,2,2});
    LOGI_(a);
    
    LOGI_(a.sum_last_axes(2));
    
    LOGI_(a.sum(2));
    
    
    algotest::TestBase::runAllTests();
}


