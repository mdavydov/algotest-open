//
//  algotest_tensor_tests.cpp
//  Algotest
//
//  Created by Maksym Davydov on 8/21/17.
//  Copyright © 2017 AdvaSoft. All rights reserved.
//

#include "algotest_tests.h"
#include "algotest_timer.h"
#include "algotest_tensor.h"

using namespace algotest;

DECLARE_TEST(Tensor_crop_test)
{
    tensor<int> a = tensor<int>::arange(10);
    a = a.reshape( {2,5} );
    a.crop( {1,1}, {2, -1} ) = tensor<int>::scalar(0);
    TEST_ASSERT( a == tensor<int>::matrix( { { 0, 1, 2, 3, 4 },
                                             { 5, 0, 0, 0, 9 } } ) );
}

DECLARE_TEST(Tensor_index_select)
{
    tensor<int> a = tensor<int>::arange(12);
    a = a.reshape( {3,4} );
    
    TEST_ASSERT( a == tensor<int>::matrix( { { 0, 1,  2,  3 },
                                             { 4, 5,  6,  7 },
                                             { 8, 9, 10, 11 } } ) );
    
    TEST_ASSERT(a.index_select(0, {1,2}) == tensor<int>::matrix( { { 4, 5,  6,  7 },
                                                                   { 8, 9, 10, 11 } } ) );
    
    
    TEST_ASSERT(a.index_select(1, {1,-1}) ==
                tensor<int>::matrix( { { 1,  3 },
                                       { 5,  7 },
                                       { 9, 11 } } ) );

}

DECLARE_TEST(Tensor_split_axis)
{
    tensor<int> a = tensor<int>::arange(12);
    a = a.reshape( {3,4} );
    
    TEST_ASSERT( a == tensor<int>::matrix( { { 0, 1,  2,  3 },
                                             { 4, 5,  6,  7 },
                                             { 8, 9, 10, 11 } } ) );
    
    TEST_ASSERT(a.splitAxis(1, 2) ==
                tensor<int>::matrix3D( { { {0, 1},  {2,  3} },
                                         { {4, 5},  {6,  7} },
                                         { {8, 9}, {10, 11} } }) );
    
    tensor<int> b = tensor<int>::arange(12);
    b = b.reshape( {4,3} );
    TEST_ASSERT( b.splitAxis(0, 2)
                == tensor<int>::matrix3D( { {{0, 1, 2},
                                             {3, 4, 5}},
                                            {{6, 7, 8},
                                             {9, 10, 11}} } ) );
}

DECLARE_TEST(Tensor_interpolateByAxisNearest)
{
    tensor<int> a = tensor<int>::arange(12);
    a = a.reshape( {3,4} );
    
    TEST_ASSERT( a == tensor<int>::matrix( { { 0, 1,  2,  3 },
                                             { 4, 5,  6,  7 },
                                             { 8, 9, 10, 11 } } ) );
    
    TEST_ASSERT(a.interpolateByAxisNearest(0, 1) ==
                tensor<int>::matrix( { { 4, 5, 6, 7 } } ) );
    
    TEST_ASSERT(a.interpolateByAxisNearest(0, 3) == a );
    
    TEST_ASSERT(a.interpolateByAxisNearest(0, 4).shape == tensor_shape({4,4}));
    TEST_ASSERT(a.interpolateByAxisNearest(0, 5).shape == tensor_shape({5,4}));
    TEST_ASSERT(a.interpolateByAxisNearest(0, 6) ==
        tensor<int>::matrix( {  { 0, 1, 2, 3 },
                                { 0, 1, 2, 3 },
                                { 4, 5, 6, 7 },
                                { 4, 5, 6, 7 },
                                { 8, 9, 10, 11 },
                                { 8, 9, 10, 11 } } ) );
    
    TEST_ASSERT(a.interpolateByAxisNearest(1, 1).shape == tensor_shape({3,1}));
    TEST_ASSERT(a.interpolateByAxisNearest(1, 2).shape == tensor_shape({3,2}));
    TEST_ASSERT(a.interpolateByAxisNearest(1, 3).shape == tensor_shape({3,3}));
    TEST_ASSERT(a.interpolateByAxisNearest(1, 4) == a);
}

DECLARE_TEST(Tensor_slice_test)
{
    tensor<int> a = tensor<int>::arange(10);
    
    TEST_ASSERT( a.slice({{7, .n=2}}) == tensor<int>::array({7,8}) );
    TEST_ASSERT( a.slice({{7, .n=3, .step=2}}) == tensor<int>::array({7,9}) );
    TEST_ASSERT( a.slice({{7, .n=2, .step=-1}}) == tensor<int>::array({7,6}) );
    TEST_ASSERT( a.slice({{7, .n=10, .step=-2}}) == tensor<int>::array({7,5,3,1}) );
    
    TEST_ASSERT( a.slice({{.i=7}}) == tensor<int>::array({7}) );
    TEST_ASSERT( a.slice({{.i=-1}}) == tensor<int>::array({9}) );
    TEST_ASSERT( a.slice({{.i=-10}}) == tensor<int>::array({0}) );
    
    a = a.reshape( {2,5} );
    a.slice({ {1}, {1,-1} }) = 0;
    TEST_ASSERT( a == tensor<int>::matrix( { { 0, 1, 2, 3, 4 },
                                             { 5, 0, 0, 0, 9 } } ) );
    
    tensor<int> b = tensor<int>::arange(20).reshape( {4,5} );
    b.slice({ {0,-1}, {.step=2} }) = 0;
    TEST_ASSERT( b == tensor<int>::matrix({{ 0, 1, 0, 3, 0 },
                                           { 0, 6, 0, 8, 0 },
                                           { 0, 11, 0, 13, 0 },
                                           { 15, 16, 17, 18, 19 }}));
    
    tensor<int> c = tensor<int>::arange(20).reshape( {4,5} );
    c.slice({ {0,-1}, {4,.step=-2} }) = 0;
    TEST_ASSERT( c == tensor<int>::matrix({{ 0, 1, 0, 3, 0 },
                                           { 0, 6, 0, 8, 0 },
                                           { 0, 11, 0, 13, 0 },
                                           { 15, 16, 17, 18, 19 }}));
}

DECLARE_TEST(Tensor_reshape)
{
    TEST_ASSERT( tensor<float>({1,1,81}).canReshapeTo({9,9}) );
    TEST_ASSERT( tensor<float>({1,1,1, 81, 1}).canReshapeTo({9,9}) );
    TEST_ASSERT( tensor<float>({1,81, 1, 1, 9}).canReshapeTo({9,9,9}) );
    TEST_ASSERT( tensor<float>({1,81, 1, 1, 9}).canReshapeTo({9,3,3,9}) );
    TEST_ASSERT( tensor<float>({1,81, 1, 1, 9}).canReshapeTo({9,3,27}) );
}

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
    
    {
        START_TIMER("Tensor speed test");
        
        tensor<float> tfbig1( {1000,100,100,5,2}, initializer(2.0f) );
        NOTIFY_TIME("Init");
        
        auto res = tfbig1 + tfbig1;
        NOTIFY_TIME("Sum");

        tensor<float> tfbig3( {1000,100,100,5,2} );
        int n = tfbig3.numElements();
        float* data = tfbig3.data();
        for( int i = 0; i<n;++i) data[i] = float(i);
        NOTIFY_TIME("Linear cycle");
        
        for( int i = 0; i<n;++i) data[i] = 2.0f;
        NOTIFY_TIME("Linear initialization");
    }
    
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
      
    auto t8 = t7.reshape({5,3,2});

    LOGI_(t8.trim({3,2,1}));
    LOGI_(t8.trim({3,2,-1}));
    LOGI_(t8.trim({3,-1,-1}));
    
    //LOGI_(t7[{0, 4}]);
    //t6[ tensor::slice(0)(2,5)()(3,4) ];
    //t6[{2,3}][2][{}] // t6[2:3, 2,

    //copy tensor()
}

DECLARE_TEST(Tensor_destroyAxis)
{
    auto t = tensor<float>::matrix( { {0,1,2},
                                      {3,4,5} } );
    TEST_ASSERT( t.destroyAxis(0,0) ==  tensor<float>::array( {0,1,2} ));
    TEST_ASSERT( t.destroyAxis(1,1) ==  tensor<float>::array( {1,4} ));
}

DECLARE_TEST(Tensor_patial_product_sum)
{
    auto t1 = tensor<float>::matrix({   {1,0,0},
                                        {0,1,0},
                                        {0,0,1},
                                        {1,1,1} } );
    
    auto t2 = tensor<float>::matrix({   {2,0,0},
                                        {0,1,0},
                                        {0,0,2},
                                        {1,2,1} } );
    auto res = tensor<float>::matrix({ { 2, 0, 0, 2 },
                                       { 0, 1, 0, 1 },
                                       { 0, 0, 2, 2 },
                                       { 1, 2, 1, 4 } });

    
    ASSERT( t1.insertAxis(0, 4).partial_product_sum(t2.insertAxis(1, 4), 1)==res);
}

DECLARE_TEST(Tensor_matmul)
{
    auto t1 = tensor<float>::matrix({   {1,0,0},
                                        {0,1,3} } );
    
    auto t2 = tensor<float>::matrix({   {2,0,0,1},
                                        {0,1,0,1},
                                        {0,0,2,0} } );
    
    auto res = tensor<float>::matrix({ { 2, 0, 0, 1 },
                                       { 0, 1, 6, 1 } });

    
    ASSERT( t1.matmul(t2)==res);
}

DECLARE_TEST(Tensor_crop)
{
    tensor<float> t = tensor<float>::arange(30).reshape({5,6});
    TEST_ASSERT(t.crop({0,0}, {3,2}) == tensor<float>::matrix( {{ 0, 1},
                                                                { 6, 7},
                                                                {12,13} }));
    
    TEST_ASSERT(t.crop({2,2}, t.shape) == tensor<float>::matrix( {{14,15,16,17},
                                                                  {20,21,22,23},
                                                                  {26,27,28,29} }));
}

DECLARE_TEST(Tensor_axis_revert)
{
    tensor<float> t = tensor<float>::matrix( {{ 0, 1},
                                              { 6, 7},
                                              {12,13} });
    TEST_ASSERT(t.flip(0) == tensor<float>::matrix( {{12,13},
                                                     { 6, 7},
                                                     { 0, 1} }) );
    TEST_ASSERT(t.flip(1) == tensor<float>::matrix( {{ 1, 0},
                                                     { 7, 6},
                                                     {13,12} }) );

}

DECLARE_TEST(Tensor_axis_extension)
{
    auto a = tensor<float>::arange(4).reshape({1,4});
    auto b = tensor<float>::arange(4).reshape({4,1});
    TEST_ASSERT( ((a*b)[{3,3}] == 9) );
    TEST_ASSERT( ((a/(b+1))[{1,2}] == 1) );
    TEST_ASSERT( ((a+b)[{3,2}]==5) );
    TEST_ASSERT( ((a-b)[{3,2}]==-1) );
}

DECLARE_TEST(Tensor_max)
{
    tensor<float> test = tensor<float>::matrix({ {3, 2, 1},
                                                 {4, 7, 5} } );
    
    TEST_ASSERT(test.max(1) == tensor<float>::array({3,7}) );
    TEST_ASSERT(test.min(1) == tensor<float>::array({1,4}) );
    
    TEST_ASSERT(test.max(0) == tensor<float>::array({4,7,5}));
    TEST_ASSERT(test.min(0) == tensor<float>::array({3,2,1}));
    
    TEST_ASSERT(test.max(1).max(0) == tensor<float>::scalar(7));
    TEST_ASSERT(test.min(1).min(0) == tensor<float>::scalar(1));
    TEST_ASSERT(test.max()==7);
    TEST_ASSERT(test.min()==1);
}

DECLARE_TEST(Tensor_softmax)
{
    tensor<float> test = tensor<float>::arange(30).reshape({2,3,5});
    
    TEST_ASSERT(test.softmax(0).sum(0).allclose( tensor<float>::scalar(1) ) );
    TEST_ASSERT(test.softmax(1).sum(1).allclose( tensor<float>::scalar(1) ) );
    TEST_ASSERT(test.softmax(2).sum(2).allclose( tensor<float>::scalar(1) ) );
}

DECLARE_TEST(Tensor_window)
{
    tensor<int> i = tensor<int>::arange(7);
    TEST_ASSERT(i == tensor<int>::array({0,1,2,3,4,5,6}));
    TEST_ASSERT(i.window(0, 2, 2) == tensor<int>::matrix({ { 0, 1 }, { 2, 3 }, { 4, 5 } }) );
    TEST_ASSERT(i.window(0, 1, 3) == tensor<int>::matrix({{ 0, 1, 2 },
                                                          { 1, 2, 3 },
                                                          { 2, 3, 4 },
                                                          { 3, 4, 5 },
                                                          { 4, 5, 6 }}) );
    
    tensor<int> m = tensor<int>::arange(9).reshape({3,3});
    TEST_ASSERT(m == tensor<int>::matrix({{ 0, 1, 2 },
                                          { 3, 4, 5 },
                                          { 6, 7, 8 }}));
                
    tensor<int> mw = m.window(0,1,2).window(1,1,2);
                
    TEST_ASSERT(mw.shape == tensor_shape({2,2,2,2}));
    
    TEST_ASSERT(mw.subtensor({0,0}) == tensor<int>::matrix({{ 0, 1 },
                                                            { 3, 4 }}) );
    TEST_ASSERT(mw.subtensor({0,1}) == tensor<int>::matrix({{ 1, 2 },
                                                            { 4, 5 }}) );
    TEST_ASSERT(mw.subtensor({1,0}) == tensor<int>::matrix({{ 3, 4 },
                                                            { 6, 7 }}) );
    TEST_ASSERT(mw.subtensor({1,1}) == tensor<int>::matrix({{ 4, 5 },
                                                            { 7, 8 }}) );
}

#if 0
DECLARE_TEST(Tensor_some_test)
{
    tensor<float> i = tensor<float>::arange(30).reshape({5,6});
    LOGI_(i);
    
    tensor<float> coords = tensor<float>({3,2});
    coords[{0,0}] = 0.9; coords[{0,1}] = 0.8;
    coords[{1,0}] = -0.5; coords[{1,1}] = 2.0;
    coords[{2,0}] = 4.5; coords[{2,1}] = 10;
    LOGI_(coords);
    
    LOGI_(i.bilinear_sample(coords));
    
    NOTIFY_TIME("FNet correlation");
    
    LOGI_(i.window(0,2,2).window(1,2,2));
    auto r = i.window(0,2,2).window(1,2,2).sum_last_axes(2)/4.0f;
    LOGI_(r);
    LOGI_(i.avg_pool({0,1}, {2,2}));
    
    LOGI_(i.sum());
    LOGI_(i.sum_last_axes(1));
    
    LOGI_(tensor<float>::arange(7).window(0, 2, 3));
    LOGI_(tensor<float>::arange(7).window(0, 2, 3).sum_last_axes(1));
}
#endif


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


DECLARE_TEST(Minus_1Dim_2Dim)
{
    tensor<int> a = tensor<int>::zeros({5,2});
    tensor<int> b = tensor<int>::arange(5);
    tensor<int> c = (b-a);
    ASSERT(c.ndim() == 2);
    ASSERT(c.shape[0] == a.shape[0]);
    ASSERT(c.shape[1] == a.shape[1]);

    for(int i = 0; i < 5; ++i)
        for (int j = 0; j < 2; ++j)
        {
            ASSERT( (c[{i,j}]) == ( (a[{i,j}]) - (b[{i}])) * -1);
        }
}

DECLARE_TEST(Multiple_1Dim_1Dim)
{
    tensor<int> a = tensor<int>::arange(10);
    tensor<int> b = tensor<int>::arange(10);
    tensor<int> c = a*b;
    ASSERT(c.ndim() == 1);
    ASSERT(c.shape[0] == a.shape[0]);
    
    for(auto i : c.shape.indices())
    {
        ASSERT(c[i] == a[i] * b[i]);
    }
}

DECLARE_TEST(Multiple_2Dim_1Dim)
{
    tensor<int> a = tensor<int>::random({10}, -50, 50).reshape({5,2});
    tensor<int> b = tensor<int>::random({5}, -50, 50);
    tensor<int> c = a*b;
    ASSERT(c.ndim() == 2);
    ASSERT(c.shape[0] == a.shape[0]);
    ASSERT(c.shape[1] == a.shape[1]);
    
    for(int i = 0; i < 5; ++i)
        for (int j = 0; j < 2; ++j)
        {
            ASSERT( (c[{i,j}]) == (a[{i,j}]) * (b[{i}]));
        }
}

DECLARE_TEST(Multiple_1Dim_2Dim)
{
    tensor<int> a = tensor<int>::random({10}, -50, 50).reshape({5,2});
    tensor<int> b = tensor<int>::random({5}, -50, 50);
    tensor<int> c = b*a;
    ASSERT(c.ndim() == 2);
    ASSERT(c.shape[0] == a.shape[0]);
    ASSERT(c.shape[1] == a.shape[1]);

    for(int i = 0; i < 5; ++i)
        for (int j = 0; j < 2; ++j)
        {
            ASSERT( (c[{i,j}]) == (b[{i}]) * (a[{i,j}]));
        }
}

DECLARE_TEST(Devide_1Dim_1Dim)
{
    tensor<int> a = tensor<int>::random({10}, -50, 50);
    tensor<int> b = tensor<int>::random({10}, -50, 50);
    tensor<int> c = a/b;
    ASSERT(c.ndim() == 1);
    ASSERT(c.shape[0] == a.shape[0]);

    for(auto i : c.shape.indices())
    {
        ASSERT( c[i] == a[i] / b[i] );
    }
}

DECLARE_TEST(Devide_2Dim_1Dim)
{
    tensor<int> a = tensor<int>::random({10}, -50, 50).reshape({5,2});;
    tensor<int> b = tensor<int>::random({5}, -50, 50);
    tensor<int> c = a/b;
    ASSERT(c.ndim() == 2);
    ASSERT(c.shape[0] == a.shape[0]);
    ASSERT(c.shape[1] == a.shape[1]);

    for(int i = 0; i < 5; ++i)
        for (int j = 0; j < 2; ++j)
        {
            ASSERT( (c[{i,j}]) == (a[{i,j}]) / (b[{i}]));
        }
}

DECLARE_TEST(Devide_1Dim_2Dim)
{
    tensor<int> a = tensor<int>::random({10}, -50, 50).reshape({5,2});
    tensor<int> b = tensor<int>::random({5}, -50, 50);
    tensor<int> c = b/a;
    ASSERT(c.ndim() == 2);
    ASSERT(c.shape[0] == a.shape[0]);
    ASSERT(c.shape[1] == a.shape[1]);

    for(int i = 0; i < 5; ++i)
        for (int j = 0; j < 2; ++j)
        {
            ASSERT( (c[{i,j}]) == (b[{i}]) / (a[{i,j}]));
        }
}

DECLARE_TEST(PlusEqual_tensors)
{
    tensor<int> a = tensor<int>::random({10}, -50, 50).reshape({5,2});
    tensor<int> a_copy = a.copy();
    tensor<int> b = tensor<int>::random({10}, -50, 50).reshape({5,2});
    a+=b;
    
    
    ASSERT(a.ndim() == 2);
    ASSERT(a.shape[0] == b.shape[0]);
    ASSERT(a.shape[1] == b.shape[1]);
    for(auto i : a.shape.indices())
    {
        ASSERT ( a[i] == a_copy[i] + b[i] );
    }
}

DECLARE_TEST(PlusEqual_tensor_int)
{
    tensor<int> a = tensor<int>::random({10}, -50, 50).reshape({5,2});
    tensor<int> a_copy = a.copy();

    a+=5;
    
    ASSERT(a.ndim() == 2);
    ASSERT(a.shape[0] == a_copy.shape[0]);
    ASSERT(a.shape[1] == a_copy.shape[1]);
    for(auto i : a.shape.indices())
    {
        ASSERT ( a[i] == a_copy[i] + 5 );
    }
}

DECLARE_TEST(MinusEqual_tensors)
{
    tensor<int> a = tensor<int>::random({10}, -50, 50).reshape({5,2});
    tensor<int> a_copy = a.copy();
    tensor<int> b = tensor<int>::random({10}, -50, 50).reshape({5,2});
    a-=b;
    ASSERT(a.ndim() == 2);
    ASSERT (a.shape[0] == a_copy.shape[0]);
    ASSERT(a.shape[1] == a_copy.shape[1]);
    
    for (auto i : a.shape.indices())
    {
        ASSERT ( a[i] == a_copy[i] - b[i] );
    }
}

DECLARE_TEST(MinusEqual_Tensor_int)
{
    tensor<int> a = tensor<int>::random({10}, -50, 50).reshape({5,2});
    tensor<int> a_copy = a.copy();
    a-=5;
    ASSERT(a.ndim() == 2);
    ASSERT(a.shape[0] == a_copy.shape[0]);
    ASSERT(a.shape[1] == a_copy.shape[1]);
    
    for(auto i : a.shape.indices())
    {
        ASSERT ( a[i] == a_copy[i] - 5 );
    }
}

DECLARE_TEST(MultipleEqual_1Dim_1Dim)
{
    tensor<int> a = tensor<int>::random({5}, -50, 50);
    tensor<int> a_copy = a.copy();
    tensor<int> b = tensor<int>::random({5}, -50, 50);
    a*=b;
    ASSERT(a.ndim() == 1);
    ASSERT(a.shape[0] == a_copy.shape[0]);
    for(auto i : a.shape.indices())
    {
        ASSERT(a[i] == a_copy[i] * b[i]);
    }
}

DECLARE_TEST(MultipleEqual_2Dim_1Dim)
{
    tensor<int> a = tensor<int>::random({10}, -50, 50).reshape({5,2});
    tensor<int> a_copy = a.copy();
    tensor<int> b = tensor<int>::random({5}, -50, 50);
    a*=b;
    ASSERT(a.ndim() == 2);
    ASSERT(a.shape[0] == a_copy.shape[0]);
    ASSERT(a.shape[1] == a_copy.shape[1]);
    
    for(int i = 0; i < 5; ++i)
        for(int j = 0; j < 2; ++j)
        {
            ASSERT( (a[{i,j}]) == (a_copy[{i,j}]) * (b[{i}]) );
        }
}

DECLARE_TEST(MultipleEqual_1Dim_int)
{
    tensor<int> a = tensor<int>::random({5}, -50, 50);
    tensor<int> a_copy = a.copy();
    a*=5;
    ASSERT(a.ndim() == 1);
    ASSERT(a.shape[0] == a_copy.shape[0]);
    for(auto i : a.shape.indices())
    {
        ASSERT(a[i] == a_copy[i] * 5);
    }
}

DECLARE_TEST(MultipleEqual_2Dim_int)
{
    tensor<int> a = tensor<int>::random({10}, -50, 50).reshape({5,2});
    tensor<int> a_copy = a.copy();
    a*=5;
    ASSERT(a.ndim() == 2);
    ASSERT(a.shape[0] == a_copy.shape[0]);
    ASSERT(a.shape[1] == a_copy.shape[1]);
    
    for(int i = 0; i < 5; ++i)
        for(int j = 0; j < 2; ++j)
        {
            ASSERT( (a[{i,j}]) == (a_copy[{i,j}]) * 5 );
        }
}

DECLARE_TEST(DevideEqual_1Dim_1Dim)
{
    tensor<int> a = tensor<int>::random({5}, -50, 50);
    tensor<int> a_copy = a.copy();
    tensor<int> b = tensor<int>::random({5}, -50, 50);
    a/=b;
    ASSERT(a.ndim() == 1);
    ASSERT(a.shape[0] == a_copy.shape[0]);
    for(auto i : a.shape.indices())
    {
        ASSERT(a[i] == a_copy[i] / b[i]);
    }
}

DECLARE_TEST(DevideEqual_2Dim_1Dim)
{
    tensor<int> a = tensor<int>::random({10}, -50, 50).reshape({5,2});
    tensor<int> a_copy = a.copy();
    tensor<int> b = tensor<int>::random({5}, -50, 50);
    a/=b;
    ASSERT(a.ndim() == 2);
    ASSERT(a.shape[0] == a_copy.shape[0]);
    ASSERT(a.shape[1] == a_copy.shape[1]);
    
    for(int i = 0; i < 5; ++i)
        for(int j = 0; j < 2; ++j)
        {
            ASSERT( (a[{i,j}]) == (a_copy[{i,j}]) / (b[{i}]) );
        }
}

DECLARE_TEST(DevideEqual_1Dim_int)
{
    tensor<int> a = tensor<int>::random({5}, -50, 50);
    tensor<int> a_copy = a.copy();
    a/=5;
    ASSERT(a.ndim() == 1);
    ASSERT(a.shape[0] == a_copy.shape[0]);
    for(auto i : a.shape.indices())
    {
        ASSERT(a[i] == a_copy[i] / 5);
    }
}

DECLARE_TEST(DevideEqual_2Dim_int)
{
    tensor<int> a = tensor<int>::random({10}, -50, 50).reshape({5,2});
    tensor<int> a_copy = a.copy();
    a/=5;
    ASSERT(a.ndim() == 2);
    ASSERT(a.shape[0] == a_copy.shape[0]);
    ASSERT(a.shape[1] == a_copy.shape[1]);
    
    for(int i = 0; i < 5; ++i)
        for(int j = 0; j < 2; ++j)
        {
            ASSERT( (a[{i,j}]) == (a_copy[{i,j}]) / 5 );
        }
}

DECLARE_TEST(Lesser_1Dim_1Dim)
{
    tensor<int> a = tensor<int>::random({10});
    tensor<int> b = tensor<int>::random({10});
    tensor<bool> c = (a<b);
    
    ASSERT(a.ndim() == 1);
    ASSERT(c.shape[0] == b.shape[0]);
    for (auto i : c.shape.indices())
    {
        ASSERT( c[i] == a[i] < b[i] );
    }
}

DECLARE_TEST(Lesser_2Dim_1Dim)
{
    tensor<int> a = tensor<int>::random({10}).reshape({5,2});
    tensor<int> b = tensor<int>::random({5});
    tensor<bool> c = (a<b);
    
    ASSERT(c.ndim() == 2);
    ASSERT(c.shape[0] == a.shape[0]);
    ASSERT(c.shape[1] == a.shape[1]);
    
    for(int i = 0; i < 5; ++i)
        for(int j = 0; j < 2; ++j)
        {
            ASSERT((c[{i,j}]) == (a[{i,j}]) < (b[{i}]));
        }
}

DECLARE_TEST(Lesser_1Dim_int)
{
    tensor<int> a = tensor<int>::random({5});

    tensor<bool> c = (a<1);
    
    ASSERT(c.ndim() == 1);
    ASSERT(c.shape[0] == a.shape[0]);
    
        for(int j = 0; j < 5; ++j)
        {
            ASSERT((c[{j}]) == (a[{j}]) < 1 );
        }
}

DECLARE_TEST(Lesser_2Dim_int)
{
    tensor<int> a = tensor<int>::random({10}).reshape({5,2});

    tensor<bool> c = (a<1);
    
    ASSERT(c.ndim() == 2);
    ASSERT(c.shape[0] == a.shape[0]);
    ASSERT(c.shape[1] == a.shape[1]);
    
    for(int i = 0; i < 5; ++i)
        for(int j = 0; j < 2; ++j)
        {
            ASSERT((c[{i,j}]) == (a[{i,j}]) < 1 );
        }
}

DECLARE_TEST(LesserEqual_1Dim_1Dim)
{
    tensor<int> a = tensor<int>::random({10});
    tensor<int> b = tensor<int>::random({10});
    tensor<bool> c = (a<=b);
    
    ASSERT(a.ndim() == 1);
    ASSERT(c.shape[0] == b.shape[0]);
    for (auto i : c.shape.indices())
    {
        ASSERT( c[i] == a[i] <= b[i] );
    }
}

DECLARE_TEST(LesserEqual_2Dim_1Dim)
{
    tensor<int> a = tensor<int>::random({10}).reshape({5,2});
    tensor<int> b = tensor<int>::random({5});
    tensor<bool> c = (a<=b);
    
    ASSERT(c.ndim() == 2);
    ASSERT(c.shape[0] == a.shape[0]);
    ASSERT(c.shape[1] == a.shape[1]);
    
    for(int i = 0; i < 5; ++i)
        for(int j = 0; j < 2; ++j)
        {
            ASSERT((c[{i,j}]) == (a[{i,j}]) <= (b[{i}]));
        }
}

DECLARE_TEST(LesserEqual_1Dim_int)
{
    tensor<int> a = tensor<int>::random({5});

    tensor<bool> c = (a<=1);
    
    ASSERT(c.ndim() == 1);
    ASSERT(c.shape[0] == a.shape[0]);
    
        for(int j = 0; j < 5; ++j)
        {
            ASSERT((c[{j}]) == (a[{j}]) <= 1 );
        }
}

DECLARE_TEST(LesserEqual_2Dim_int)
{
    tensor<int> a = tensor<int>::random({10}).reshape({5,2});

    tensor<bool> c = (a<=1);
    
    ASSERT(c.ndim() == 2);
    ASSERT(c.shape[0] == a.shape[0]);
    ASSERT(c.shape[1] == a.shape[1]);
    
    for(int i = 0; i < 5; ++i)
        for(int j = 0; j < 2; ++j)
        {
            ASSERT((c[{i,j}]) == (a[{i,j}]) <= 1 );
        }
}

DECLARE_TEST(Greater_1Dim_1Dim)
{
    tensor<int> a = tensor<int>::random({10});
    tensor<int> b = tensor<int>::random({10});
    tensor<bool> c = (a>b);
    
    ASSERT(c.shape == b.shape);
    for (auto i : c.shape.indices())
    {
        ASSERT( c[i] == a[i] > b[i] );
    }
}

DECLARE_TEST(Greater_2Dim_1Dim)
{
    tensor<int> a = tensor<int>::random({10}).reshape({5,2});
    tensor<int> b = tensor<int>::random({5});
    tensor<bool> c = (a>b);
    
    ASSERT(c.ndim() == 2);
    ASSERT(c.shape[0] == a.shape[0]);
    ASSERT(c.shape[1] == a.shape[1]);
    
    for(int i = 0; i < 5; ++i)
        for(int j = 0; j < 2; ++j)
        {
            ASSERT((c[{i,j}]) == (a[{i,j}]) > (b[{i}]));
        }
}

DECLARE_TEST(Greater_1Dim_int)
{
    tensor<int> a = tensor<int>::random({10});

    tensor<bool> c = (a>1);
    
    ASSERT(c.ndim() == 1);
    ASSERT(c.shape[0] == a.shape[0]);
    
        for(int j = 0; j < 5; ++j)
        {
            ASSERT((c[{j}]) == (a[{j}]) > 1 );
        }
}

DECLARE_TEST(Greater_2Dim_int)
{
    tensor<int> a = tensor<int>::random({10}).reshape({5,2});

    tensor<bool> c = (a>1);
    
    ASSERT(c.ndim() == 2);
    ASSERT(c.shape[0] == a.shape[0]);
    ASSERT(c.shape[1] == a.shape[1]);
    
    for(int i = 0; i < 5; ++i)
        for(int j = 0; j < 2; ++j)
        {
            ASSERT((c[{i,j}]) == (a[{i,j}]) > 1 );
        }
}

DECLARE_TEST(GreaterEqual_1Dim_1Dim)
{
    tensor<int> a = tensor<int>::random({10});
    tensor<int> b = tensor<int>::random({10});
    tensor<bool> c = (a>=b);
    
    ASSERT(c.shape == b.shape);
    for (auto i : c.shape.indices())
    {
        ASSERT( c[i] == a[i] >= b[i] );
    }
}

DECLARE_TEST(GreaterEqual_2Dim_1Dim)
{
    tensor<int> a = tensor<int>::random({10}).reshape({5,2});
    tensor<int> b = tensor<int>::ones({5});
    tensor<bool> c = (a>=b);
    
    ASSERT(c.ndim() == 2);
    ASSERT(c.shape[0] == a.shape[0]);
    ASSERT(c.shape[1] == a.shape[1]);
    
    for(int i = 0; i < 5; ++i)
        for(int j = 0; j < 2; ++j)
        {
            ASSERT((c[{i,j}]) == (a[{i,j}]) >= (b[{i}]));
        }
}

DECLARE_TEST(GreaterEqual_1Dim_int)
{
    tensor<int> a = tensor<int>::random({5});

    tensor<bool> c = (a>=1);
    
    ASSERT(c.ndim() == 1);
    ASSERT(c.shape[0] == a.shape[0]);
    
        for(int j = 0; j < 5; ++j)
        {
            ASSERT((c[{j}]) == (a[{j}]) >= 1 );
        }
}

DECLARE_TEST(GreaterEqual_2Dim_int)
{
    tensor<int> a = tensor<int>::random({10}).reshape({5,2});
    tensor<bool> c = (a>=1);
    
    ASSERT(c.ndim() == 2);
    ASSERT(c.shape[0] == a.shape[0]);
    ASSERT(c.shape[1] == a.shape[1]);
    
    for(int i = 0; i < 5; ++i)
        for(int j = 0; j < 2; ++j)
        {
            ASSERT((c[{i,j}]) == (a[{i,j}]) >= 1 );
        }
}
