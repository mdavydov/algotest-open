//
//  algotest_tensor_tests.cpp
//  Algotest
//
//  Created by Maksym Davydov on 8/21/17.
//  Copyright © 2017 AdvaSoft. All rights reserved.
//

#include "algotest_tests.h"
//#include "algotest_timer.h"
#include "algotest_tensor.h"

using namespace algotest;


DECLARE_TEST(Tensor_InitTest)
{
    tensor<float> tf0({2,3,5,6});
    tensor<float> tf2( {2,3}, initializer(10.0f) );
    tensor<float> tf2_1( {2,3}, initializer(20.0f) );
    tensor<float> tf3({2,3,5});
    tensor<int> tf2i( {2,3}, initializer(3) );
                         
    LOGI_(tf2 + tf2i);
    
    TEST_ASSERT(tf0.isSequential());
    TEST_ASSERT(tf2.isSequential());
    TEST_ASSERT(tf3.isSequential());
    
    tf0.apply( [](float&f) {f=0;} );
    tf2.apply( [](float&f) {f=1;} );
    tf3.apply( [](float&f) {f=2;} );
    
    // Create tensor
    tensor<float> t1( {1000,100}, initializer(2.0f) );
    tensor<float> t2 = t1;  // reference
    tensor<float> t3 = t1.copy();

    // reshape tensor tests
    tensor<float> t5( {2, 5, 6, 7, 8} );
    TEST_ASSERT(t5.canReshapeTo({2, 5, 6, 7, 8}));
    TEST_ASSERT(t5.canReshapeTo({1, 2, 5, 6, 7, 8}));
    TEST_ASSERT(t5.canReshapeTo({2, 5, 1, 6, 7, 1, 8}));
    TEST_ASSERT(t5.canReshapeTo({10, 6, 7, 8}));
    TEST_ASSERT(t5.canReshapeTo({2, 5, 42, 8}));
    TEST_ASSERT(t5.canReshapeTo({2, 30, 7, 8}));
    TEST_ASSERT(t5.canReshapeTo({2, 5, 6, 56}));
    TEST_ASSERT(t5.canReshapeTo({2, 5, 6, 7, 8}));
    TEST_ASSERT(t5.canReshapeTo({2, 5, 6, 7, 2, 2, 2}));
    TEST_ASSERT(t5.canReshapeTo({2, 5, 6, 7, 8, 1, 1, 1}));
    
    TEST_ASSERT(!t5.canReshapeTo({2, 6, 6, 7, 8}));
    TEST_ASSERT(!t5.canReshapeTo({1, 1, 6, 6, 7, 8}));
    
    tensor<float> t6 = t5.reshape({10,42,2,4});
    TEST_ASSERT(t6.numElements()==t5.numElements());
    TEST_ASSERT(t6.isSequential());
    
    //tensor<float> t7 =
    //t6.slice( 2, 3, 4, std::initializer_list<int>{3,5} );
    
    auto t7 = tensor<int>::arange(30);
    LOGI_(t7);
    LOGI_(t7.trim({5}));
    LOGI_(t7.trimTail({5}));
    LOGI_(t7.trim({-5}));
    LOGI_(t7.trimStart({5}));
    LOGI_(t7.trimStart({-5}));
    
    auto t9 = t7.reshape({5,6});
    LOGI_(t9);
    LOGI_(t9.crop({1,1},{3,3}));
    t9.crop({1,1},{3,3}).init(0);
    t9.crop({3,3},{5,5}).copy().init(0);
    LOGI_(t9);
      
    STOP_TESTS();
    
    auto t8 = t7.reshape({5,3,2});

    LOGI_(t8.trim({3,2,1}));
    LOGI_(t8.trim({3,2,-1}));
    LOGI_(t8.trim({3,-1,-1}));
    
    //LOGI_(t7[{0, 4}]);
    //t6[ tensor::slice(0)(2,5)()(3,4) ];
    //t6[{2,3}][2][{}] // t6[2:3, 2,

    //copy tensor()
}

#if 0
DECLARE_TEST(Tensor_SliceTest)
{
    tensor<double> m23 = tensor<double>::random(2,3,0,1);
    tensor<double> m32 = tensor<double>::random(3,2,0,1);
    tensor<double> m11 = tensor<double>::random(1,1,0,1);
    
    tensor<double> m6 = tensor<double>(6,6, uninitialized() );
    slice(m6).from(0,0).withSize(2,3) = m23;
    slice(m6).from(0,3).withSize(3,2) = m32;
    slice(m6).from(2,0).withSize(3,2) = m32;
    slice(m6).from(3,2).withSize(2,3) = m23;
    slice(m6).from(2,2).withSize(1,1) = m11;
    
//    LOGI_( tensor<double>( slice(m6)(0,0,2,3) ));
//    LOGI_( m23);
    
    TEST_ASSERT( tensor<double>( slice(m6).from(0,0).withSize(2,3) )==m23 );
    TEST_ASSERT( tensor<double>( slice(m6).from(0,3).withSize(3,2) )==m32 );
    TEST_ASSERT( tensor<double>( slice(m6).from(2,0).withSize(3,2) )==m32 );
    TEST_ASSERT( tensor<double>( slice(m6).from(3,2).withSize(2,3) )==m23 );
    TEST_ASSERT( tensor<double>( slice(m6).from(2,2).withSize(1,1) )==m11 );
}

DECLARE_TEST(Tensor_Invert200x200_Random0_1)
{
    const int n = 200;
    auto m = tensor<double, n, n>::random(0,1);
    tensor<double, n, n> r;
    invert_with_LU_decomposition(r, m);
    
    auto mres = m*r;
    TEST_ASSERT(mres.isIdentity(0.001));
}

DECLARE_TEST(Tensor_FunctionalInitialization_DynamicStaticCast)
{
    tensor<double> q_1(40,40, [](int i, int j){return i==j?1:0;});
    ASSERT(q_1 == tensor<double>::identity(40,40));
    
    tensor<double,40,40> q_2([](int i, int j){return i==j?1:0;});
    ASSERT(q_2 == (tensor<double,40,40>::identity()));

    ASSERT(q_1 == tensor<double>(q_2));
}

DECLARE_TEST(Vector_InitializerListConstructor_LinearEquation)
{
    tensor<float> a = tensor<float>::random(14,14, -10,20);
    
    vect<float> v({1,7,2,8,0,6,4,1,7,2,8,0,6,4});
    vect<float> x = solve(a, v);
    
    vect<float> expected = a*x;
    TEST_ASSERT(sqrLength(expected-v)<0.001);
}

DECLARE_TEST(Tensor_InitializerListConstructor_FindingDeterminant)
{
    tensor<double, 3,3> m33 = {{1, 2, 4}, {3, 4, 1}, {1, -1, -1}};
    double det1 = det(m33);
    double determinant2 = determinant(m33);
    
    TEST_ASSERT_FLOAT_EQ(det1, determinant2, 0.001);
}

DECLARE_TEST(upper_multiple)
{
    TEST_ASSERT(upper_multiple(100,100)==100 );
    TEST_ASSERT(upper_multiple(100,50)==100 );
    TEST_ASSERT(upper_multiple(0,50)==0 );
    TEST_ASSERT(upper_multiple(51,50)==100 );
    TEST_ASSERT(upper_multiple(49,50)==50 );
}

#endif
