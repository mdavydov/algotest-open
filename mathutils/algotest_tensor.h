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
//#include <concepts>
#include "algotest_data_holder.h"
#include "algotest_tensor_impl.h"

namespace algotest
{
    typedef std::vector< tensor_settings::index_type > tensor_index;
    
    class tensor_shape;
    
//    template<typename T>
//    concept IsTensorShape = std::same_as<T, const tensor_shape&> ||
//                            std::same_as<T, tensor_shape> ||
//                            std::same_as<T, tensor_shape&& >;
    
    class tensor_shape : protected tensor_impl
    {
    protected:
        std::vector<index_type> m_shape;    // m_shape.size()>0
    public:
        tensor_shape(const std::initializer_list<index_type>& dimensions)
            : m_shape( dimensions )
        {
            ASSERT(m_shape.size()>0);
        }
        
        tensor_shape(const tensor_shape& shape)
            : m_shape( shape.m_shape )
        {
        }
        
        
        explicit tensor_shape(const index_type * ptr, int n)
            : m_shape( ptr, ptr+n )
        {
            
        }
        
        explicit tensor_shape(auto&& ... shape)
        {
            ( m_shape.insert(m_shape.end(), shape.begin(), shape.end()), ... );
        }
        
        tensor_shape first(int n) const
        {
            ASSERT(n>0 && n<=numDimensions());
            return tensor_shape( shape_ptr(), n);
        }
        tensor_shape last(int n) const
        {
            ASSERT(n>0 && n<=numDimensions());
            return tensor_shape( shape_ptr()+numDimensions()-n, n);
        }
        
        auto begin() const { return m_shape.begin(); }
        auto end() const { return m_shape.end(); }
        
        /// number of tensor dimensions
        index_type operator[](int i) const { return m_shape[i]; }
        
        const index_type* shape_ptr() const { return std::data(m_shape); }
        
        int numDimensions() const { return int(m_shape.size()); }
        /// total count of elements
        index_type numElements() const
        {
            index_type res = 1;
            for(int i=0, n = numDimensions(); i<n; ++i) res *= m_shape[i];
            return res;
        }
        
        std::vector<index_type> cumulative_count() const
        {
            std::vector<index_type> res(m_shape.size());
            index_type count = 1;
            for(int i=numDimensions()-1; i>=0; --i)
            {
                res[i] = (count*=m_shape[i]);
            }
            return res;
        }
        
        /// Trim shape to contain less data, but preserve number of dimensions
        void trim(const index_type * indices, int n)
        {
            ASSERT(n<=numDimensions());
            for(int i=0; i<n; ++i)
            {
                index_type trim_v = indices[i]>=0 ? indices[i] : m_shape[i]+indices[i];
                m_shape[i] = std::clamp(trim_v, 0, m_shape[i]);
            }
        }
        
        /// Trim shape to contain less data, but preserve number of dimensions
        void trimTail(const index_type * indices, int n)
        {
            ASSERT(n<=numDimensions());
            for(int i=0; i<n; ++i)
            {
                index_type trim_v = indices[i]>=0 ? m_shape[i]-indices[i] : -indices[i];
                m_shape[i] = std::clamp(trim_v, 0, m_shape[i]);
            }
        }
        
        void crop(const index_type * index_b, const index_type * index_e, int n)
        {
            ASSERT(n<=numDimensions());
            for(int i=0; i<n; ++i)
            {
                index_type trim1 = index_b[i]>=0 ? index_b[i] : m_shape[i]+index_b[i];
                index_type trim2 = index_e[i]>=0 ? index_e[i] : m_shape[i]+index_e[i];
                m_shape[i] = std::clamp(trim2-trim1, 0, m_shape[i]);
            }
        }
        
        void trim(const tensor_index& index)
        {
            trim(std::data(index), int(std::size(index)) );
        }
        void trimTail(const tensor_index& index)
        {
            trimTail(std::data(index), int(std::size(index)) );
        }
        void crop(const tensor_index& begin, const tensor_index& end)
        {
            ASSERT(begin.size()==end.size());
            crop(std::data(begin), std::data(end), int(std::size(begin)) );
        }
        
    protected:
        void append(index_type i) { m_shape.push_back(i); }
        void append(const tensor_shape& s) { m_shape.insert(m_shape.end(), s.m_shape.begin(), s.m_shape.end()); }
    };
    
    class tensor_strided_shape : public tensor_shape
    {
        std::vector<index_type> m_strides;
    public:
        tensor_strided_shape(const std::initializer_list<index_type>& dimensions)
            : tensor_shape(dimensions), m_strides(dimensions.size())
        {
            initSequential();
        }
        
        tensor_strided_shape(const tensor_shape& shape) : tensor_shape( shape ), m_strides(shape.numDimensions())
        {
            initSequential();
        }
        
        tensor_strided_shape getValueReplicated(const tensor_shape& replication) const
        {
            tensor_strided_shape res(*this);
            res.m_shape.insert(     res.m_shape.end(), replication.begin(), replication.end() );
            res.m_strides.insert( res.m_strides.end(), replication.numDimensions(), 0);
            ASSERT(res.m_strides.size()==res.m_shape.size());
            return res;
        }
        tensor_strided_shape getTensorReplicated(const tensor_shape& replication) const
        {
            tensor_strided_shape res(*this);
            res.m_shape.insert(     res.m_shape.begin(), replication.begin(), replication.end() );
            res.m_strides.insert( res.m_strides.begin(), replication.numDimensions(), 0);
            ASSERT(res.m_strides.size()==res.m_shape.size());
            return res;
        }

        tensor_strided_shape getWindowed(int axis, int step, int size) const
        {
            ASSERT(axis>=0 && axis<numDimensions());
            ASSERT(size<=m_shape[axis]);
            tensor_strided_shape res(*this);
            res.m_shape.push_back(size);
            res.m_strides.push_back(m_strides[axis]);
            res.m_shape[axis] = (m_shape[axis]-size)/step + 1;
            res.m_strides[axis] = m_strides[axis]*step;
            return res;
        }
        
//        tensor_strided_shape(const strided_shape_ptr& shape)
//            : tensor_shape( shape ), m_strides(shape.numDimensions())
//        {
//            initSequential();
//        }
        
        // append indices with 0-strides
        
        void append(index_type i) { tensor_shape::append(i); m_strides.push_back(0); }
        void append(const tensor_shape& s)
        {
            tensor_shape::append(s);
            m_strides.insert(m_strides.end(), s.numDimensions(), 0 );
        }
        
        index_type stride(index_type i) const { return m_strides[i]; }
        const index_type* stride_ptr() const { return std::data(m_strides); }
        
        bool canReshapeTo(const tensor_shape& shape) const
        {
            return strided_shape_ptr().canReshapeTo(shape.shape_ptr(), shape.numDimensions());
        }
        
        index_type getDisplace(const index_type * indices, int n) const
        {
            ASSERT(n<=numDimensions());
            index_type displace = 0;
            for(int i=0; i<n; ++i)
            {
                int index = indices[i]>=0 ? indices[i] : m_shape[i]+indices[i];
                ASSERT(index>=0 && index<m_shape[i]);
                displace+=m_strides[i]*index;
            }
            return displace;
        }
        
        template<class Index>
        index_type getDisplace(Index&& index) const
        {
            return getDisplace(std::data(index), int(std::size(index)) );
        }
        
        tensor_strided_shape getReshaped(const tensor_shape& shape) const
        {
            tensor_strided_shape reshaped(shape);
            reshaped.copyStridesFrom(*this);
            return reshaped;
        }
        
        tensor_strided_shape getPermuted(tensor_index indices) const
        {
            tensor_strided_shape res = *this;
            ASSERT(indices.size() < numDimensions());
            int n = int(indices.size());
            
            for(int i=0; i<n; ++i)
            {
                ASSERT(indices[i]>=0 && indices[i]<n);
                for(int j=i+1; j<n; ++j) ASSERT(indices[j]!=indices[i]);
                
                res.m_shape[i] = m_shape[ indices[i] ];
                res.m_strides[i] = m_strides[ indices[i] ];
            }
            return res;
        }
        
        void copyStridesFrom(const tensor_strided_shape& other)
        {
            int i1 = numDimensions()-1;
            int i2 = other.numDimensions()-1;
            index_type s1 = 1, s2 = 1;
            
            while(i1>=0)
            {
                if (s1==s2)
                {
                    ASSERT(i2>=0);
                    s1 *= i1>=0 ? m_shape[i1] : 1;
                    s2 *= i2>=0 ? other.m_shape[i2] : 1;
                    m_strides[i1--] = other.m_strides[i2--];
                }
                else if (s1<s2)
                {
                    m_strides[i1] = m_strides[i1+1]*m_shape[i1+1];
                    s1 *= m_shape[i1--];
                }
                else if (s1>s2)
                {
                    ASSERT(i2>=0);
                    s2 *= other.m_shape[i2--];
                }
            }
        }
        
        bool isSequential() const
        {
            index_type p = 1;
            for(int i=numDimensions()-1;i>=0;--i)
            {
                if (m_strides[i]!=p) return false;
                p*=m_shape[i];
            }
            return true;
        }
    private:
        tensor_impl::strided_shape_ptr strided_shape_ptr() const
        {
            return tensor_impl::strided_shape_ptr(shape_ptr(), stride_ptr(), numDimensions());
        }
        
        void initSequential()
        {
            index_type p = 1;
            for(int i=numDimensions()-1;i>=0; --i)
            {
                m_strides[i] = p;
                p*=m_shape[i];
            }
        }
    };
    
    // use tensor<const T> for tensor of constants
    template<class T>
    class [[nodiscard]] tensor : protected tensor_impl
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
        // TODO: think about this field protection
        tensor_strided_shape shape;   // m_ is omitted because shape is a standard field in NumPy
        
    private:
        strided_array_ptr<T> strided_ptr() const
        {
            return strided_array_ptr<T>(m_data,
                                        shape.shape_ptr(),
                                        shape.stride_ptr(),
                                        shape.numDimensions() );
        }

    public:
        tensor(const tensor_shape& shape) : shape(shape)
        {
            m_data = new T[shape.numElements()];
            m_data_holder = abstractDataHolder(ArrayPtr<T>(m_data));
        }
        
        tensor(const tensor_strided_shape& shape, T* data, std::shared_ptr<AbstractData> data_holder)
            : shape(shape), m_data(data), m_data_holder(data_holder)
        {
        }
        
        tensor(const tensor&) = default;
        tensor(tensor&&) = default;
        tensor& operator=(const tensor&) = default;
        tensor& operator=(tensor&&) = default;
        
        tensor(const tensor_shape& shape,
               const initializer_t<T>& init)
            : tensor(shape)
        {
            T init_v = init;
            this->init(init_v);
        }
        
        T& operator[](const tensor_index& index) { return m_data[shape.getDisplace(index)]; }
        const T& operator[](const tensor_index& index) const { return m_data[shape.getDisplace(index)]; }
        
        const T* data() const { return m_data; }
        T* data() { return m_data; }
        std::shared_ptr<AbstractData> dataHolder() const { return m_data_holder; }
        int numElements() const { return shape.numElements(); }
        int numDimensions() const { return shape.numDimensions(); }
        bool isSequential() const { return shape.isSequential(); }
        
        index_type getDisplace(const tensor_index& index) const
        {
            return shape.getDisplace(index);
        }
        
        bool canReshapeTo(const tensor_shape& other_shape) const
        {
            return shape.canReshapeTo(other_shape);
        }
        tensor reshape(const tensor_shape& other_shape) const
        {
            tensor_strided_shape ss = shape.getReshaped(other_shape);
            return tensor(ss, m_data, m_data_holder);
        }
        tensor trim(const tensor_index& index) const
        {
            tensor_strided_shape ss = shape;
            ss.trim(index);
            return tensor(ss, m_data, m_data_holder);
        }
        tensor trimTail(const tensor_index& index) const
        {
            tensor_strided_shape ss = shape;
            ss.trimTail(index);
            return tensor(ss, m_data, m_data_holder);
        }
        tensor trimStart(const tensor_index& index) const
        {
            tensor_strided_shape ss = shape;
            ss.trimTail(index);
            return tensor(ss, m_data+getDisplace(index), m_data_holder);
        }
        tensor crop(const tensor_index& begin, const tensor_index& end) const
        {
            tensor_strided_shape ss = shape;
            ss.crop(begin, end);
            return tensor(ss, m_data+getDisplace(begin), m_data_holder);
        }
        // permute first len(i) channels with indices \in {0,1, len(i)-1}
        tensor permute(tensor_index indices)
        {
            tensor_strided_shape ss = shape.getPermuted(indices);
            return tensor(ss, m_data, m_data_holder);
        }
        // add repeated dimensions to the end of a tensor
        tensor replicate_values(const tensor_shape& s)
        {
            return tensor( shape.getValueReplicated(s), m_data, m_data_holder);
        }
        tensor replicate_tensor(const tensor_shape& s)
        {
            return tensor( shape.getTensorReplicated(s), m_data, m_data_holder);
        }

        
        void init(const T& val) { strided_ptr().init(val); }
        
        //template<class OP> void apply(OP&& op) { strided_ptr().apply(op); }
        template<class OP> void apply(OP&& op) const { strided_ptr().apply(op); }
        
        template<class U, class OP2>
        void apply(const tensor<U>& a, OP2&& op) const
        {
            strided_ptr().apply(a.strided_ptr(), op);
        }
        
        template<class U, class V, class OP3>
        void apply(const tensor<U>& a, const tensor<V>& b, OP3&& op) const
        {
            strided_ptr().apply(a.strided_ptr(), b.strided_ptr(), op);
        }
        
        friend std::ostream& operator<<(std::ostream& os, const tensor& a)
        {
            a.strided_ptr().print(os);
            return os;
        }
        
        template<class U>
        void copyFrom(const tensor<U>& a)
        {
            apply(a, [](T& r, const U& a) {r = a;});
        }
        
        tensor<T> copy() const
        {
            tensor<T> res(shape);
            res.copyFrom(*this);
            return res;
        }
        
    public: // tensor operations
        template<class U>
        auto operator+(const tensor<U>& a)
        {
            typedef decltype(std::declval<T>() + std::declval<U>()) ResType;
            tensor< ResType > res( shape );
            res.apply( *this, a, [](ResType& r, const T& a, const U& b) {r = a+b;} );
            return res;
        }
        
        template<class U>
        auto operator-(const tensor<U>& a)
        {
            typedef decltype(std::declval<T>() - std::declval<U>()) ResType;
            tensor< ResType > res( shape );
            res.apply( *this, a, [](ResType& r, const T& a, const U& b) {r = a-b;} );
            return res;
        }
        
        template<class U>
        auto operator*(const tensor<U>& a)
        {
            typedef decltype(std::declval<T>() * std::declval<U>()) ResType;
            tensor< ResType > res( shape );
            res.apply( *this, a, [](ResType& r, const T& a, const U& b) {r = a*b;} );
            return res;
        }
        
        template<class U>
        auto operator/(const tensor<U>& a)
        {
            typedef decltype(std::declval<T>() / std::declval<U>()) ResType;
            tensor< ResType > res( shape );
            res.apply( *this, a, [](ResType& r, const T& a, const U& b) {r = a/b;} );
            return res;
        }
    public: // tensor/scalar operations
        template<class U>
        auto operator+(U b)
        {
            typedef decltype(std::declval<T>() + std::declval<U>()) ResType;
            tensor< ResType > res( shape );
            res.apply( *this, [b](ResType& r, const T& a) {r = a+b;} );
            return res;
        }
        template<class U>
        auto operator-(U b)
        {
            typedef decltype(std::declval<T>() - std::declval<U>()) ResType;
            tensor< ResType > res( shape );
            res.apply( *this, [b](ResType& r, const T& a) {r = a-b;} );
            return res;
        }
        template<class U>
        auto operator*(U b)
        {
            typedef decltype(std::declval<T>() * std::declval<U>()) ResType;
            tensor< ResType > res( shape );
            res.apply( *this, [b](ResType& r, const T& a) {r = a*b;} );
            return res;
        }
        template<class U>
        auto operator/(U b)
        {
            typedef decltype(std::declval<T>() / std::declval<U>()) ResType;
            tensor< ResType > res( shape );
            res.apply( *this, [b](ResType& r, const T& a) {r = a/b;} );
            return res;
        }
    public: // tensor/scalar operations
        template<class U> tensor& operator+=(const U& b) { apply( [b](T& a) {a += b;} ); return *this; }
        template<class U> tensor& operator-=(const U& b) { apply( [b](T& a) {a -= b;} ); return *this; }
        template<class U> tensor& operator*=(const U& b) { apply( [b](T& a) {a *= b;} ); return *this; }
        template<class U> tensor& operator/=(const U& b) { apply( [b](T& a) {a /= b;} ); return *this; }
    public:
        
        template<class U=T>
        U sum() const
        {
            U sum = 0;
            apply( [&sum](const T& a) {sum += a;} );
            return sum;
        }
        
        template<class U=T>
        tensor<U> partial_sum(int num_last_dims) const
        {
            ASSERT(num_last_dims>=0 && num_last_dims<numDimensions());
            tensor<U> res( shape.first(numDimensions()-num_last_dims), initializer<U>(0.0) );
            apply( res.replicate_values( shape.last(num_last_dims) ), [](const T& a, U& sum) {sum += a;} );
            return res;
        }
        
        tensor window(int axis, int step, int size) const
        {
            return tensor( shape.getWindowed(axis, step, size), m_data, m_data_holder);
        }
        
        tensor avg_pool(const tensor_index& indices,
                        const tensor_shape& step,
                        const tensor_shape& window_size)
        {
            ASSERT(window_size.numDimensions() == indices.size());
            ASSERT(window_size.numDimensions() == step.numDimensions());
            ASSERT(!indices.empty() && indices.size() <= numDimensions());
            tensor w = *this;
            for(int i=0; i < int(indices.size());++i)
            {
                ASSERT(indices[i] >=0 && indices[i] < numDimensions());
                for(int j=i+1; j < int(indices.size());++j) ASSERT(indices[i]!=indices[j]);
                w = w.window(indices[i], step[i], window_size[i]);
            }
            w = w.partial_sum( int(indices.size()) );
            w /= T(window_size.numElements());
            return w;
        }
        
        tensor avg_pool(const tensor_index& indices,
                        const tensor_shape& step)
        {
            return avg_pool(indices, step, step);
        }
        
        template<class U>
        tensor bilinear_sample(const tensor<U>& coords)
        {
            int nd = coords.numDimensions();
            index_type stride = coords.shape.stride(nd-1);
            tensor res(coords.shape.first(nd-1));
            index_type sh0 = shape[0] - 1;
            index_type sh1 = shape[1] - 1;
            res.apply(coords,
            [this, stride, sh0, sh1](T& r, const U& c)
            {
                const U* pc = &c;
                U x = pc[0];
                U y = pc[stride];
                U ix = floor(x);
                U iy = floor(y);
                index_type i0 = clamp<index_type>( ix,   0, sh0 );
                index_type i1 = clamp<index_type>( ix+1, 0, sh0 );
                
                index_type j0 = clamp<index_type>( iy,   0, sh1 );
                index_type j1 = clamp<index_type>( iy+1, 0, sh1 );
                
                float ai = x - ix, aj = y-iy;
                
                r = T( (*this)[{i0,j0}] * (1.0-ai)*(1.0-aj) +
                       (*this)[{i1,j0}] *       ai*(1.0-aj) +
                       (*this)[{i0,j1}] * (1.0-ai)*aj       +
                       (*this)[{i1,j1}] *       ai*aj       );
            });
            
            return res;
        }
        
    public: // other operations
        template<class U> friend class tensor;
        
        // returns tensor with 1 more channel representing index of the cell
        // i.e. index_grid({2,2}) will be
        // [ [ [0,0], [0,1] ],
        //   [ [1,0], [1,1] ] ]
        static tensor<T> index_grid(const tensor_shape& s)
        {
            index_type nc = index_type(s.numDimensions());
            
            tensor_shape res_shape( s, tensor_shape({nc}) );
            std::vector<index_type> cc = res_shape.cumulative_count();
            
            const index_type * shape_ptr = s.shape_ptr();
            const index_type * cc_ptr = std::data(cc)+1;
            
            tensor<T> res( res_shape );
            index_type i = 0;
            res.apply( [&i, nc, shape_ptr, cc_ptr](T& t)
            {
                index_type c = i%nc;
                t = T( (i/cc_ptr[c]) % shape_ptr[c] );
                ++i;
            } );
            return res;
        }
        
        static tensor arange(index_type size)
        {
            tensor res({size});
            T i = 0;
            res.apply( [&i](T& t) {t = i++;} );
            return res;
        }
        
        static tensor linspace(const T& min_ref, const T& max_ref, index_type size)
        {
            tensor res({size});
            T i = 0, min = min_ref, max = max_ref;
            res.apply( [&i, min, max, size](T& t) {t = min + (max-min)*T(i++)/(size-1);} );
            return res;
        }

        static tensor zeros(const tensor_shape& s)
        {
            return tensor(s, initializer(T(0)));
        }

        static tensor ones(const tensor_shape& s)
        {
            return tensor(s, initializer(T(1)));
        }



        tensor meshgrid(const tensor& other)
        {
            ASSERT(numDimensions()==1 && other.numDimensions()==1);
            
            tensor res({shape[0], other.shape[0], 2});
            int i=0;
            int s1 = shape[0];
            res.apply( [&i, this, &other, s1](T& t)
                {
                    t = (i&1)? other[{(i/2)%s1}]:(*this)[{(i/2/s1)}];
                    ++i;
                } );
            return res;
        }
        
        static tensor indices(const tensor_shape& s)
        {
            index_type nc = index_type(s.numDimensions());
            tensor_shape res_shape( tensor_shape{nc}, s );
            std::vector<index_type> cc = s.cumulative_count();
            cc.push_back(1);
            
            const index_type num_elements = s.numElements();
            const index_type * shape_ptr = s.shape_ptr();
            const index_type * cc_ptr = std::data(cc)+1;
            
            tensor<T> res( res_shape );
            index_type i = 0;
            res.apply( [&i, num_elements, shape_ptr, cc_ptr](T& t)
            {
                index_type c = i/num_elements;
                t = T( (i/cc_ptr[c]) % shape_ptr[c] );
                ++i;
            } );
            return res;
        }
    };
}

#endif // algotest_tensor_included
