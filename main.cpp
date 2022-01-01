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
    STOP_TESTS();
}

int main()
{
    LOGI_("Hello");
    algotest::TestBase::runAllTests();
}


