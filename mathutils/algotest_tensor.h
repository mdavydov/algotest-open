/*  The Mathutil library
 Copyright (C) 2007-2021 Maksym Davydov
 
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

#ifndef algotest_tensor_included
#define algotest_tensor_included

#include <algorithm>
#include "algotest_tensor_strided_shape.h"
#include "algotest_memory.h"
#include "cnpy.h"

namespace algotest
{
    // vtensor represents tensor with "value" semantics were assignment operator copies values, not a reference
    // use vtensor<const T> for vtensor of constants
    template<class T>
    class [[nodiscard]] vtensor : protected tensor_impl
    {
    public:
        typedef T value_type;
        typedef T& reference;
        typedef const T& const_reference;
        
    private:
        /// Shared data
        std::shared_ptr<AbstractData> m_data_holder;
        T * m_data;
    public:
        // this field is protected from modifications by "const" semantics of it's public methods
        const_tensor_strided_shape shape;   // m_ is omitted because shape is a standard field in NumPy
        
    private:
        strided_array_ptr<T> strided_ptr() const
        {
            ASSERT(m_data!=0);
            return strided_array_ptr<T>(m_data,
                                        shape.shape_ptr(),
                                        shape.stride_ptr(),
                                        shape.ndim() );
        }
        
    protected:
        void copyRepresentationFrom(const vtensor& a)
        {
            m_data = a.m_data;
            m_data_holder = a.m_data_holder;
            shape = a.shape;
        }

    public:
        vtensor() : m_data(0) {}
        vtensor(const vtensor&) = default;
        vtensor(vtensor&&) = default;
        vtensor(const tensor_shape& shape) : shape(shape)
        {
            m_data = new T[shape.numElements()];
            m_data_holder = abstractDataHolder(ArrayPtr<T>(m_data));
        }
        vtensor(const tensor_strided_shape& shape, T* data, std::shared_ptr<AbstractData> data_holder)
            : shape(shape), m_data(data), m_data_holder(data_holder)
        {
        }
        vtensor(const tensor_shape& shape, const initializer_t<T>& init)
            : vtensor(shape)
        {
            T init_v = init;
            this->init(init_v);
        }
        
        vtensor& operator=(const vtensor& r)
        {
            copyValuesFrom(r);
            return *this;
        }
        
        vtensor& operator=(const T& v)
        {
            init(v);
            return *this;
        }
        
        operator vtensor<const T>() const
        {
            return vtensor<const T>(shape, m_data, m_data_holder);
        }
        
        T& operator[](const tensor_index& index) { return m_data[shape.getDisplace(index)]; }
        const T& operator[](const tensor_index& index) const { return m_data[shape.getDisplace(index)]; }
        
        // Use ref for fast indexing (though indexing is not so fast as othe access methods)
        template<size_t N>
        T& ref(const std::array<index_type, N>& index) { return m_data[shape.getDisplaceC(index)]; }

        // Use ref for fast indexing (though indexing is not so fast as othe access methods)
        template<size_t N>
        const T& ref(const std::array<index_type, N>& index) const { return m_data[shape.getDisplaceC(index)]; }
        
        // Use ref for fast indexing (though indexing is not so fast as othe access methods)
        template<tensor_index_type_class... C>
        const T& ref(C... index) const
        {
            std::array<index_type, sizeof...(index) > arr{ index... };
            return m_data[shape.getDisplaceC(arr)];
        }
        
        T* data() const { return m_data; }
        std::shared_ptr<AbstractData> dataHolder() const { return m_data_holder; }
        int numElements() const { return shape.numElements(); }
        int ndim() const { return shape.ndim(); }
        bool isSequential() const { return shape.isSequential(); }
        bool empty() const { return m_data==0; }
        index_type stride(int axis_index) const { return shape.stride(axis_index); }
        
        index_type getDisplace(const tensor_index& index) const
        {
            return shape.getDisplace(index);
        }
        
        /// @brief use this function for debugging purposes only.
        /// It is too slow for production.
        tensor_index referenceToIndex(const T& ref) const
        {
            return shape.displaceToIndex( index_type(&ref - m_data) );
        }
        
        bool canReshapeTo(const tensor_shape& other_shape) const
        {
            return shape.canReshapeTo(other_shape);
        }
        vtensor reshape(const tensor_shape& other_shape) const
        {
            return vtensor(shape.copy().reshape(other_shape), m_data, m_data_holder);
        }
        
        vtensor upshape(const tensor_shape& other_shape) const
        {
            if (other_shape.isPrefixOf(shape)) return *this;
            return vtensor(shape.copy().upshape(other_shape), m_data, m_data_holder);
        }
        
        template<tensor_index_type_class... C>
        vtensor view(C... index) const
        {
            return reshape( {index...} );
        }

        
        /** @brief destroyAxis removes one axis of the tensor (makes this axis size-1 and removes from the shape)
           [ [ 0 1 2 ]
             [ 2 3 5 ] ]
         destroyAxis(1, 1) = [ 1 3 ]
         destroyAxis(0, 0) = [ 0 1 2 ]
        */
        vtensor destroyAxis(int axis, index_type select_index = 0) const
        {
            return vtensor(shape.copy().destroyAxis(axis), m_data + stride(axis)*select_index, m_data_holder);
        }
        vtensor subtensor(const tensor_index& index) const
        {
            ASSERT(index.ndim() <= ndim());
            return vtensor(shape.stridedTail(ndim() - index.ndim()), m_data + getDisplace(index), m_data_holder);
        }
        vtensor trim(const tensor_index& index) const
        {
            tensor_strided_shape ss = shape;
            ss.trim(index);
            return vtensor(ss, m_data, m_data_holder);
        }
        vtensor trimTail(const tensor_index& index) const
        {
            tensor_strided_shape ss = shape;
            ss.trimTail(index);
            return vtensor(ss, m_data, m_data_holder);
        }
        vtensor trimStart(const tensor_index& index) const
        {
            tensor_strided_shape ss = shape;
            ss.trimTail(index);
            return vtensor(ss, m_data+getDisplace(index), m_data_holder);
        }
        vtensor crop(const tensor_index& begin, const tensor_index& end) const
        {
            tensor_strided_shape ss = shape;
            ss.crop(begin, end);
            return vtensor(ss, m_data+getDisplace(begin), m_data_holder);
        }
        vtensor crop_size(const tensor_index& begin, const tensor_index& size) const
        {
            tensor_strided_shape ss = shape;
            ss.crop_size(begin, size);
            return vtensor(ss, m_data+getDisplace(begin), m_data_holder);
        }
        
        vtensor slice( const std::initializer_list<index_slice>& s ) const
        {
            tensor_strided_shape ss = shape;
            index_type d = 0;
            ss.slice(s, d);
            return vtensor(ss, m_data+d, m_data_holder);
        }
        
        // convert axis into 2 axes
        vtensor splitAxis(int axis, int num_parts) const
        {
            tensor_strided_shape ss = shape;
            ss.splitAxis(axis, num_parts);
            return vtensor(ss, m_data, m_data_holder);
        }
        
        vtensor sliceAxis(int axis, index_type begin, index_type end, index_type step) const
        {
            tensor_strided_shape ss = shape;
            ss.sliceAxis(axis, begin, end, step);
            return vtensor(ss, m_data+shape.getDisplaceByAxis(axis, begin), m_data_holder);
        }
        vtensor cropAxis(int axis, index_type begin, index_type end) const
        {
            return sliceAxis(axis, begin, end, 1);
        }
        
        // permute first len(i) channels with indices \in {0,1, len(i)-1}
        // leave all other axes untouched
        vtensor permute(const std::vector<int>& axes) const
        {
            return vtensor(shape.copy().permuteAxes(axes), m_data, m_data_holder);
        }
        
        template<tensor_index_type_class... C>
        vtensor permute(C... index) const
        {
            return permute( {index...} );
        }
        
        vtensor swapAxes(int axis1, int axis2) const
        {
            return vtensor(shape.copy().swapAxes(axis1, axis2), m_data, m_data_holder);
        }
        
        // matrix transposition
        vtensor transpose() const
        {
            ASSERT(ndim()==2);
            return swapAxes(0, 1);
        }
        
        // matrix transposition
        vtensor transpose(int axis1, int axis2) const
        {
            return swapAxes(axis1, axis2);
        }
        
        // select some axes and destroy other
        vtensor selectAxes(const std::vector<int>& axes) const
        {
            return vtensor(shape.copy().selectAxes(axes), m_data, m_data_holder);
        }
        
        /// make some axis reverted
        vtensor flip(int axis) const
        {
            return vtensor(shape.copy().flip(axis), m_data + stride(axis)*(shape[axis]-1), m_data_holder);
        }
        
        vtensor<T> index_select(int axis, const tensor_index& indices) const
        {
            ASSERT(axis >= 0 && axis < ndim());
            ASSERT(indices.ndim()>0);
            
            index_type axis_size = shape[axis];
            
            tensor_shape res_shape = shape.copy();
            res_shape[axis] = index_type( indices.ndim() );
            vtensor<T> res( res_shape );
            for(auto [i, p] : enumerate( index_type(0), indices) )
            {
                index_type pi = p >= 0 ? p : axis_size+p;
                ASSERT( pi >= 0 && pi < axis_size);
                res.cropAxis(axis, i, i+1) = cropAxis(axis, pi, pi+1);
            }
            return res;
        }
        
        // add repeated dimensions to the end of a vtensor
        // The data is not copied, replicated values reference the same memory
        vtensor<T> replicateValues(const tensor_shape& s) const
        {
            return vtensor( shape.copy().insertAxes(shape.ndim(), s), m_data, m_data_holder);
        }
        /// insert several axes with 0-stride (it means that pointed data is repeated )
        vtensor<T> insertAxes(int axis_index, const tensor_shape& s) const
        {
            return vtensor( shape.copy().insertAxes(axis_index, s), m_data, m_data_holder);
        }
        /// insert one axis with 0-stride (it means that pointed data is repeated along this axis)
        vtensor<T> insertAxis(int axis_index, index_type count) const
        {
            return vtensor( shape.copy().insertAxes(axis_index, {count}), m_data, m_data_holder);
        }

        void init(const T& val) { strided_ptr().init(val); }
        
        template<class OP> void apply(OP&& op) const { strided_ptr().apply(op); }
        
        template<class U, class OP2>
        void apply(const vtensor<U>& a, OP2&& op) const
        {
            strided_ptr().apply(a.strided_ptr(), op);
        }
        
        template<class U, class V, class OP3>
        void apply(const vtensor<U>& a, const vtensor<V>& b, OP3&& op) const
        {
            strided_ptr().apply(a.strided_ptr(), b.strided_ptr(), op);
        }
        
        template<class OP> void apply_parallel(OP&& op) const { strided_ptr().apply_parallel(op); }
        
        template<class U, class OP2>
        void apply_parallel(const vtensor<U>& a, OP2&& op) const
        {
            strided_ptr().apply_parallel(a.strided_ptr(), op);
        }
        
        template<class U, class V, class OP3>
        void apply_parallel(const vtensor<U>& a, const vtensor<V>& b, OP3&& op) const
        {
            strided_ptr().apply_parallel(a.strided_ptr(), b.strided_ptr(), op);
        }
        
        friend std::ostream& operator<<(std::ostream& os, const vtensor& a)
        {
            a.strided_ptr().print(os);
            return os;
        }
        
        template<class U>
        void masked_fill(const vtensor<U>& condition, const T& val)
        {
            apply_parallel(condition.upshape(shape), [val](T& v, const U& c) { if (c) v = val; } );
        }

        
        template<class U>
        void copyValuesFrom(const vtensor<U>& a)
        {
            apply(a.upshape(shape), [](T& r, const U& a) {r = a;});
        }
        
        vtensor<T> copy() const
        {
            vtensor<T> res(shape);
            res.copyValuesFrom(*this);
            return res;
        }
        
        /// makes a sequential copy of tensor if the tensor is not sequential
        vtensor<T> sequential() const
        {
            if (isSequential()) return *this; else return copy();
        }
        
        vtensor<T> contiguous() const { return sequential(); }
        
        template<class U>
        vtensor<U> astype() const
        {
            vtensor<U> res(shape);
            res.copyValuesFrom(*this);
            return res;
        }
        
        template<> vtensor astype<T>() const
        {
            return *this;
        }
        
    public: // vtensor operations
        #define TENSOR_TENSOR_BINARY_OP(_op_) \
            template<class U> \
            auto operator _op_(const vtensor<U>& a) const \
            { \
                typedef decltype(std::declval<T>() _op_ std::declval<U>()) ResType; \
                vtensor< ResType > res( shape.copy().upshape(a.shape) ); \
                res.apply( upshape(a.shape), a.upshape(shape), [](ResType& r, const T& a, const U& b) {r = a _op_ b;} ); \
                return res; \
            }

        TENSOR_TENSOR_BINARY_OP(+);
        TENSOR_TENSOR_BINARY_OP(-);
        TENSOR_TENSOR_BINARY_OP(*);
        TENSOR_TENSOR_BINARY_OP(/);
        TENSOR_TENSOR_BINARY_OP(<);
        TENSOR_TENSOR_BINARY_OP(<=);
        TENSOR_TENSOR_BINARY_OP(>);
        TENSOR_TENSOR_BINARY_OP(>=);
    public: // vtensor/scalar operations
        #define TENSOR_SCALAR_BINARY_OP(_op_) \
            template<scalar_type U> \
            auto operator _op_(const U& rb) const \
            { \
                typedef decltype(std::declval<T>() _op_ std::declval<U>()) ResType; \
                vtensor< ResType > res( shape ); \
                U b = rb; \
                res.apply( *this, [b](ResType& r, const T& a) {r = a _op_ b;} ); \
                return res; \
            }
        
        TENSOR_SCALAR_BINARY_OP(+);
        TENSOR_SCALAR_BINARY_OP(-);
        TENSOR_SCALAR_BINARY_OP(*);
        TENSOR_SCALAR_BINARY_OP(/);
        TENSOR_SCALAR_BINARY_OP(==);
        TENSOR_SCALAR_BINARY_OP(!=);
        TENSOR_SCALAR_BINARY_OP(<);
        TENSOR_SCALAR_BINARY_OP(<=);
        TENSOR_SCALAR_BINARY_OP(>);
        TENSOR_SCALAR_BINARY_OP(>=);
        
        #define TENSOR_TENSOR_EQ_OP(_op_) \
            template<class U> \
            const vtensor& operator _op_(const vtensor<U>& a) \
            { \
                apply(a.upshape(shape), [](T& r, const T& a) {r _op_ a;} ); \
                return *this; \
            }
        
        TENSOR_TENSOR_EQ_OP(+=);
        TENSOR_TENSOR_EQ_OP(-=);
        TENSOR_TENSOR_EQ_OP(*=);
        TENSOR_TENSOR_EQ_OP(/=);

    public: // vtensor/scalar operations
        template<scalar_type U> const vtensor& operator+=(const U& b) { apply( [b](T& a) {a += b;} ); return *this; }
        template<scalar_type U> const vtensor& operator-=(const U& b) { apply( [b](T& a) {a -= b;} ); return *this; }
        template<scalar_type U> const vtensor& operator*=(const U& b) { apply( [b](T& a) {a *= b;} ); return *this; }
        template<scalar_type U> const vtensor& operator/=(const U& b) { apply( [b](T& a) {a /= b;} ); return *this; }

    public:
        
        void makeAxisIndexPositive(int& axis) const
        {
            if (axis < 0) axis = ndim()+axis;
            ASSERT(axis>=0 && axis < ndim());
        }
        
    public:
        template<class U=T>
        U sum() const
        {
            U sum = 0;
            apply( [&sum](const T& a) {sum += a;} );
            return sum;
        }
        
        template<class U=T>
        vtensor<U> sum(int axis) const
        {
            makeAxisIndexPositive(axis);
            if (axis == ndim()-1) return sum_last_axes<U>(1);
            else return swapAxes(axis, ndim()-1).template sum_last_axes<U>(1).swapAxes(axis, ndim()-2);
        }
        
        template<class U=T>
        vtensor<U> mean(int axis) const
        {
            makeAxisIndexPositive(axis);
            vtensor<U> res = sum<U>(axis);
            res/=U(shape[axis]);
            return res;
        }
        
        template<class U=T>
        vtensor<U> sum_last_axes(int num_last_dims) const
        {
            ASSERT(num_last_dims>=0 && num_last_dims<=ndim());
            
            vtensor<U> res( shape.first(ndim()-num_last_dims), initializer<U>(0.0) );

            if (num_last_dims<ndim())
            {
                apply_parallel( res.replicateValues( shape.last(num_last_dims) ), [](const T& a, U& sum) {sum += a;} );
            }
            else
            {
                apply( res.replicateValues( shape.last(num_last_dims) ), [](const T& a, U& sum) {sum += a;} );
            }
            return res;
        }
        
        /// find maximum value in a tensor
        const T& max() const
        {
            const T * p_res = m_data;
            apply( [&p_res](const T& a) { if (*p_res<a) p_res=&a; } );
            return *p_res;
        }
        
        // find max along the given axis and destroys this axis
        vtensor max(int axis) const
        {
            makeAxisIndexPositive(axis);
            
            vtensor res = destroyAxis(axis).copy().insertAxis(axis, shape[axis]);
            apply( res, [](const T& a, T& m) { m = std::max(m,a); } );
            return res.destroyAxis(axis);
        }
        
        const T& min() const
        {
            const T * p_res = m_data;
            apply( [&p_res](const T& a) { if (*p_res>a) p_res=&a; } );
            return *p_res;
        }
        
        // find min along the given axis and destroys this axis
        vtensor min(int axis) const
        {
            makeAxisIndexPositive(axis);
            
            vtensor res = destroyAxis(axis).copy().insertAxis(axis, shape[axis]);
            apply( res, [](const T& a, T& m) { m = std::min(m,a); } );
            return res.destroyAxis(axis);
        }
        
        // find softmax along the given axis
        template<class U=T>
        vtensor softmax(int axis) const
        {
            makeAxisIndexPositive(axis);
            
            vtensor maxValues = max(axis);
            vtensor<U> expSum(maxValues.shape, initializer<U>(0.0));
            
            apply(    expSum.insertAxis(axis, shape[axis]),
                   maxValues.insertAxis(axis, shape[axis]),
                   [](const T& a, U& exps, T& maxv) { exps += exp(U(a-maxv)); } );
            
            vtensor res = astype<U>();
            
            res.apply( expSum.insertAxis(axis, shape[axis]),
                    maxValues.insertAxis(axis, shape[axis]),
                   [](U& res, const U& exps, const T& maxv) { res = exp(U(res-maxv))/exps; } );
            
            return res;
        }
        
        template<class U=T>
        vtensor<U> partial_product_sum(const vtensor<T>& other, int num_last_dims) const
        {
            // apply_parallel requires num_last_dims<ndim()
            ASSERT(num_last_dims>=0 && num_last_dims<ndim());
            ASSERT(other.shape == shape);
            vtensor<U> res( shape.first(ndim()-num_last_dims), initializer<U>(0.0) );
            
            res.replicateValues( shape.last(num_last_dims) )
               .apply_parallel( *this, other,
                                [](U& sum, const T& a, const T& b) {sum += a*b;} );
            return res;
        }
        
        template<class U=T>
        vtensor<U> matmul(const vtensor<T>& other) const
        {
            ASSERT(ndim()==2 && other.ndim()==2);
            return insertAxis(1, other.shape[1]).template partial_product_sum<U>(
                       other.transpose().insertAxis(0, shape[0]), 1);
        }
        
        vtensor window(int axis, int step, int size) const
        {
            return vtensor( shape.copy().window(axis, step, size), m_data, m_data_holder);
        }
        
        vtensor pad(const std::vector<int>& axes, int pad_before, int pad_after, const T& value = T(0))
        {
            tensor_shape res_shape = shape;
            res_shape.extend(axes, pad_before + pad_after);
            vtensor res(res_shape);
            res.init(value);
            tensor_index beg( ndim(), 0 );
            tensor_index end = shape;
            for(int ax:axes)
            {
                beg[ax]+=pad_before;
                end[ax]+=pad_before;
            }
            res.crop(beg, end) = *this;
            return res;
        }
        
        vtensor pad(const std::vector<int>& axes, int pad_from_each_size, const T& value = T(0))
        {
            return pad(axes, pad_from_each_size, pad_from_each_size, value);
        }
        
        vtensor avg_pool(const tensor_index& indices,
                        const tensor_shape& step,
                        const tensor_shape& window_size);

        vtensor avg_pool(const tensor_index& indices,
                        const tensor_shape& step)
        {
            return avg_pool(indices, step, step);
        }
        
        vtensor interpolateByAxisNearest(int axis, index_type new_size) const
        {
            ASSERT(axis >= 0 && axis < ndim());
            ASSERT(new_size >= 1);
            
            if (shape[axis]%new_size == 0)
            {
                int step = shape[axis]/new_size;
                return sliceAxis(axis, step/2, step/2 + step*new_size, step);
            }
            
            vtensor me = cropAxis(axis, 0, 1);
            tensor_shape res_shape = shape;
            res_shape[axis] = new_size;
            vtensor res(res_shape);
            index_type old_size = shape[axis];
            index_type old_stride = stride(axis);
            index_type new_stride = res.stride(axis);
            
            me.apply(res.cropAxis(axis, 0, 1), [old_size, old_stride, new_size, new_stride](const T& x, T& xnew)
                {
                    for(index_type i=0;i<new_size;++i)
                    {
                        index_type old_i = (2*i + 1)*old_size/new_size/2;
                        ASSERT(old_i<old_size);
                        (&xnew)[new_stride*i] = (&x)[old_stride*old_i];
                    }
                });
            
            return res;
        }
        
        vtensor interpolateNearest(const tensor_shape& new_shape ) const;
        
        template<class U>
        vtensor bilinear_sample(const vtensor<U>& coords)
        {
            // this = [a,b,c, c0,c1], coords = [ a,b,c, NumCoords, (c0,c1) ]
            ASSERT( ndim()>=2 && ndim() == coords.ndim() );
            ASSERT( shape.first(ndim()-2)==coords.shape.first(ndim()-2) );
            
            index_type stc = coords.stride(coords.ndim()-1);  // stride between c0 and c1
            
            index_type num_coords = coords.shape[coords.ndim()-2];
            vtensor res(shape.first(ndim()-2).appendAxis(num_coords));
            index_type sh0 = shape[ndim()-2] - 1;
            index_type sh1 = shape[ndim()-1] - 1;
            index_type st0 = stride(ndim()-2);
            index_type st1 = stride(ndim()-1);
            
            vtensor<T> this_repl = insertAxis(ndim()-2, num_coords);
            
            res.apply_parallel(this_repl, coords,
                [stc, sh0, sh1, st0, st1](T& r, const T& m, const U& c)
                {
                    const U* pc = &c;
                    const T* pm = &m;
                    U x = pc[0];
                    U y = pc[stc];
                    U ix = floor(x);
                    U iy = floor(y);
                
                    index_type i0 = clamp<index_type>( ix,   0, sh0 );
                    index_type i1 = clamp<index_type>( ix+1, 0, sh0 );
                    
                    index_type j0 = clamp<index_type>( iy,   0, sh1 );
                    index_type j1 = clamp<index_type>( iy+1, 0, sh1 );
                    
                    float ai = x - ix, aj = y-iy;
                    
                    r = T( pm[i0*st0 + j0*st1] * (1.0-ai)*(1.0-aj) +
                           pm[i1*st0 + j0*st1] *       ai*(1.0-aj) +
                           pm[i0*st0 + j1*st1] * (1.0-ai)*aj       +
                           pm[i1*st0 + j1*st1] *       ai*aj       );
                });
            
            return res;
        }
        
        template<class U>
        vtensor bilinear_sample_zero_pad(const vtensor<U>& coords)
        {
            // this = [a,b,c, c0,c1], coords = [ a,b,c, NumCoords, (c0,c1) ]
            ASSERT( ndim()>=2 && ndim() == coords.ndim() );
            ASSERT( shape.first(ndim()-2)==coords.shape.first(ndim()-2) );
            
            index_type stc = coords.stride(coords.ndim()-1);  // stride between c0 and c1
            
            index_type num_coords = coords.shape[coords.ndim()-2];
            vtensor res(shape.first(ndim()-2).appendAxis(num_coords));
            index_type sh0 = shape[ndim()-2];
            index_type sh1 = shape[ndim()-1];
            index_type st0 = stride(ndim()-2);
            index_type st1 = stride(ndim()-1);
            
            vtensor<T> this_repl = insertAxis(ndim()-2, num_coords);
            
            res.apply_parallel(this_repl, coords,
                [stc, sh0, sh1, st0, st1](T& r, const T& m, const U& c)
                {
                    const U* pc = &c;
                    const T* pm = &m;
                    U x = pc[0];
                    U y = pc[stc];
                    U ix = floor(x);
                    U iy = floor(y);
                
                    index_type i0 = ix;
                    index_type i1 = ix+1;
                    index_type j0 = iy;
                    index_type j1 = iy+1;
                
                    bool mi0 = 0<=i0 && i0<sh0;
                    bool mi1 = 0<=i1 && i1<sh0;
                    bool mj0 = 0<=j0 && j0<sh1;
                    bool mj1 = 0<=j1 && j1<sh1;

                    T v00 = mi0 && mj0 ? pm[i0*st0 + j0*st1] : T(0.0);
                    T v10 = mi1 && mj0 ? pm[i1*st0 + j0*st1] : T(0.0);
                    T v01 = mi0 && mj1 ? pm[i0*st0 + j1*st1] : T(0.0);
                    T v11 = mi1 && mj1 ? pm[i1*st0 + j1*st1] : T(0.0);
                    
                    float ai = x - ix, aj = y-iy;
                    
                    r = T( v00 * (1.0-ai)*(1.0-aj) +
                           v10 *       ai*(1.0-aj) +
                           v01 * (1.0-ai)*aj       +
                           v11 *       ai*aj       );
                });
            
            return res;
        }
        
    public: // other operations
        template<class U> friend class vtensor;
        
        // returns vtensor with 1 more channel representing index of the cell
        // i.e. index_grid({2,2}) will be
        // [ [ [0,0], [0,1] ],
        //   [ [1,0], [1,1] ] ]
        static vtensor<T> index_grid(const tensor_shape& s)
        {
            index_type nc = index_type(s.ndim());
            
            tensor_shape res_shape( s, tensor_shape({nc}) );
            std::vector<index_type> cc = res_shape.cumulative_count();
            
            const index_type * shape_ptr = s.shape_ptr();
            const index_type * cc_ptr = std::data(cc)+1;
            
            vtensor<T> res( res_shape );
            index_type i = 0;
            res.apply( [&i, nc, shape_ptr, cc_ptr](T& t)
            {
                index_type c = i%nc;
                t = T( (i/cc_ptr[c]) % shape_ptr[c] );
                ++i;
            } );
            return res;
        }
        
        static vtensor arange(index_type size)
        {
            vtensor res({size});
            T i = 0;
            res.apply( [&i](T& t) {t = i++;} );
            return res;
        }
        
        static vtensor random(index_type size, T min = 0, T max = 1)
                {
                    vtensor res({size});
                    res.apply( [&min, &max](T& t) {t = ::random(min, max);} );
                    return res;
                }
        
        static vtensor linspace(const T& min_ref, const T& max_ref, index_type size)
        {
            vtensor res({size});
            T i = 0, min = min_ref, max = max_ref;
            res.apply( [&i, min, max, size](T& t) {t = min + (max-min)*T(i++)/(size-1);} );
            return res;
        }

        static vtensor zeros(const tensor_shape& s)
        {
            return vtensor(s, initializer(T(0)));
        }

        static vtensor ones(const tensor_shape& s)
        {
            return vtensor(s, initializer(T(1)));
        }

        vtensor meshgrid(const vtensor& other)
        {
            ASSERT(ndim()==1 && other.ndim()==1);
            
            vtensor res({shape[0], other.shape[0], 2});
            int i=0;
            int s1 = shape[0];
            res.apply( [&i, this, &other, s1](T& t)
                {
                    t = (i&1)? other[{(i/2)%s1}]:(*this)[{(i/2/s1)}];
                    ++i;
                } );
            return res;
        }
    public: // MARK: comparison
        template<class U>
        bool operator==(const vtensor<U>& other) const
        {
            if (other.shape != shape) return false;
            int i=0;
            apply( other, [&i](const T& t, const U& u)
                {
                    i += t!=u ? 1 : 0;
                } );
            return i==0;
        }
        
        template<class U>
        bool allclose(const vtensor<U>& other, const T& epsilon = 1e-6) const
        {
            if (ndim() > other.ndim()) return allclose(other.replicateValues(shape.last(ndim() - other.ndim())));
            if (other.shape != shape) return false;
            
            int i=0;
            T eps_val = epsilon;
            apply( other, [&i, eps_val](const T& t, const U& u)
                {
                    i += fabs(t-u)>eps_val ? 1 : 0;
                } );
            return i==0;
        }
        
        template<class U>
        bool allclose_dump(const vtensor<U>& other, const T& epsilon = 1e-6, int output_count = 5) const
        {
            if (ndim() > other.ndim()) return allclose(other.replicateValues(shape.last(ndim() - other.ndim())));
            if (shape != other.shape)
            {
                LOGI_("Shape missmatch " << shape << " and " << other.shape);
                return false;
            }
            
            int mismatch_count = 0;
            T eps_val = epsilon;
            float max_deviation = 0;
            apply( other, [&mismatch_count, &max_deviation, output_count, eps_val, this](const T& t, const U& u)
                {
                    float d = fabs(t-u);
                    max_deviation = std::max(max_deviation, d);
                    if (d>eps_val && ++mismatch_count <= output_count)
                    {
                        tensor_index i = referenceToIndex(t);
                        LOGI_("Mismatch at " << referenceToIndex(t) << " " << t << " != " << u);
                    }
                } );
            if (mismatch_count!=0) LOGI_("Total " << mismatch_count << " mismatches");
            LOGI_("Max deviation " << max_deviation);
            return mismatch_count==0;
        }
        
        bool allclose(const T& value_ref, const T& epsilon = 1e-6) const
        {
            int i=0;
            T eps_val = epsilon;
            T value = value_ref;
            apply( [&i, value, eps_val](const T& t)
                {
                    i += fabs(t-value)>eps_val ? 1 : 0;
                } );
            return i==0;
        }
        
        static vtensor indices(const tensor_shape& s)
        {
            index_type nc = index_type(s.ndim());
            tensor_shape res_shape( tensor_shape{nc}, s );
            std::vector<index_type> cc = s.cumulative_count();
            cc.push_back(1);
            
            const index_type num_elements = s.numElements();
            const index_type * shape_ptr = s.shape_ptr();
            const index_type * cc_ptr = std::data(cc)+1;
            
            vtensor<T> res( res_shape );
            index_type i = 0;
            res.apply( [&i, num_elements, shape_ptr, cc_ptr](T& t)
            {
                index_type c = i/num_elements;
                t = T( (i/cc_ptr[c]) % shape_ptr[c] );
                ++i;
            } );
            return res;
        }
        std::string summary() const
        {
            std::ostringstream os;
            if (m_data)
            {
                strided_ptr().print(os, 0, ' ', 6);
            }
            else os << "<empty>";
            
            return os.str();
        }
    };
    
    // tensor represents tensor with "reference" semantics (i.e. like python variables)
    // were assignment operator copies references, not values
    template<class T>
    class [[nodiscard]] tensor : public vtensor<T>
    {
        typedef vtensor<T> Base;
    public:
        tensor() {Base::summary();}
        tensor(const tensor_shape& shape) : Base(shape) {}
        tensor(const tensor_strided_shape& shape, T* data, std::shared_ptr<AbstractData> data_holder)
            : Base(shape, data, data_holder) {}
        tensor(const tensor_shape& shape, const initializer_t<T>& init)
            : Base(shape, init) {}
        
        tensor(const vtensor<T>& r) : Base(r) {}
        
        tensor(const tensor_shape& shape, const std::initializer_list<T>& values) : Base(shape)
        {
            ASSERT( Base::numElements()==values.size() );
            std::copy( values.begin(), values.end(), Base::data());
        }
        
        static tensor matrix3D( const std::initializer_list< std::initializer_list< std::initializer_list<T> > >& values )
        {
            ASSERT(values.size()>0);
            tensor res({values.size(), values.begin()->size(), values.begin()->begin()->size()});
            int i=0;
            int stride1 = res.stride(1);
            for(const auto& row : values)
            {
                ASSERT(row.size()==res.shape[1]);
                for(const auto& subrow : row)
                {
                    std::copy( subrow.begin(), subrow.end(), res.data() + i);
                    i+=stride1;
                }
            }
            return res;
        }
        
        static tensor matrix( const std::initializer_list< std::initializer_list<T> >& values )
        {
            ASSERT(values.size()>0);
            tensor res({values.size(), values.begin()->size()});
            int i=0;
            for(const std::initializer_list<T>& row : values)
            {
                ASSERT(row.size()==res.shape[1]);
                std::copy( row.begin(), row.end(), res.data() + i);
                i+=res.stride(0);
            }
            return res;
        }
        static tensor array(const std::initializer_list<T>& values)
        {
            tensor res({values.size()});
            std::copy( values.begin(), values.end(), res.data());
            return res;
        }
        static tensor scalar(const T& value)
        {
            tensor res{tensor_shape()};
            *res.data() = value;
            return res;
        }
        
        static tensor cat( const std::vector< tensor >& tensors, int axis )
        {
            ASSERT(!tensors.empty());
            tensors.front().makeAxisIndexPositive(axis);
            tensor_shape compare_shape = tensors.front().shape.copyShape().removeAxis(axis);
            
            tensor_impl::index_type sum = 0;
            for(const auto& t : tensors)
            {
                ASSERT( t.shape.copyShape().removeAxis(axis) == compare_shape );
                sum += t.shape[axis];
            }
            
            tensor_shape res_shape = compare_shape;
            res_shape.insertAxis(axis, sum);
            tensor res(res_shape);
            
            int i=0;
            for(const auto& t : tensors)
            {
                res.cropAxis(axis, i, i+t.shape[axis]) = t;
                i+= t.shape[axis];
            }
            return res;
        }

        
        tensor& operator=(const vtensor<T>& r)
        {
            Base::copyRepresentationFrom(r);
            return *this;
        }
        tensor& operator=(const tensor<T>& r)
        {
            Base::copyRepresentationFrom(r);
            return *this;
        }
    };
    
    
    template<class T>
    vtensor<T> vtensor<T>::avg_pool(const tensor_index& indices,
                                    const tensor_shape& step,
                                    const tensor_shape& window_size)
    {
        ASSERT(window_size.ndim() == indices.size());
        ASSERT(window_size.ndim() == step.ndim());
        ASSERT(!indices.empty() && indices.size() <= ndim());
        tensor w = *this;
        for(int i=0; i < int(indices.size());++i)
        {
            ASSERT(indices[i] >=0 && indices[i] < ndim());
            for(int j=i+1; j < int(indices.size());++j) ASSERT(indices[i]!=indices[j]);
            w = w.window(indices[i], step[i], window_size[i]);
        }
        //LOGI_(w.crop({0,0,0,0}, {1,1,4,4}));
        w = w.sum_last_axes( int(indices.size()) );
        w /= T(window_size.numElements());
        return std::move(w);
    }
    
    template<class T>
    vtensor<T> vtensor<T>::interpolateNearest(const tensor_shape& new_shape ) const
    {
        ASSERT(ndim() == new_shape.ndim());
        
        tensor res = *this;
        for(int i=0;i<ndim();++i)
        {
            if (shape[i] != new_shape[i])
            {
                res = res.interpolateByAxisNearest(i, new_shape[i]);
            }
        }
        return vtensor<T>(res);
    }
    
    template<class T>
    tensor<T> npy_load_tensor(std::string file_path)
    {
        cnpy::NpyArray it = cnpy::npy_load(file_path);
        std::vector<size_t> shape = it.shape;
        ASSERT(it.word_size == sizeof(T));
        ASSERT(it.fortran_order == false);
        return tensor<T>{tensor_shape(it.shape), it.data<T>(), abstractDataHolder(std::move(it.data_holder))};
    }
}

#endif // algotest_tensor_included
