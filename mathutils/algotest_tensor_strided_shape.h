/*  The Mathutil library
 Copyright (C) 2007-2022 Maksym Davydov
 
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

#ifndef algotest_tensor_strided_shape_included
#define algotest_tensor_strided_shape_included

#include <algorithm>
#include "algotest_tensor_shape.h"
#include "algotest_tensor_impl.h"

namespace algotest
{
    class tensor_strided_shape;
    
    struct index_slice
    {
        inline static constexpr tensor_settings::index_type npos =
                std::numeric_limits<tensor_settings::index_type>::max();
        
        tensor_settings::index_type b=0;
        tensor_settings::index_type e = npos;
        tensor_settings::index_type step=1;
        tensor_settings::index_type n = -1;
        tensor_settings::index_type i = npos;
    };
    
    // all public methods of tensor_strided_shape should be "const"
    // to protect vtensor.shape field from external change
    class [[nodiscard]] const_tensor_strided_shape : protected tensor_shape
    {
    protected:
        std::vector<index_type> m_strides;
        template<class T> friend class vtensor;
        friend class tensor_strided_shape;
    public:
        const_tensor_strided_shape() {}
        const_tensor_strided_shape(const std::initializer_list<index_type>& dimensions)
            : tensor_shape(dimensions), m_strides(dimensions.size())
        {
            initSequential();
        }
        
        const_tensor_strided_shape(const tensor_shape& shape) : tensor_shape( shape ), m_strides(shape.ndim())
        {
            initSequential();
        }
        
        const_tensor_strided_shape(int ndim, const index_type* shape_ptr, const index_type* strides_ptr )
            : tensor_shape(ndim, shape_ptr), m_strides(strides_ptr, strides_ptr+ndim) {}
        
        const_tensor_strided_shape(const const_tensor_strided_shape& shape) = default;
        const_tensor_strided_shape(const_tensor_strided_shape&& shape) = default;
    
    public:
        operator const tensor_index&() const { return m_shape; }
       
        index_type stride(index_type i) const { return m_strides[i]; }
        const index_type* stride_ptr() const { return std::data(m_strides); }
        index_type operator[](int i) const { return m_shape[i]; }
        index_type at(int i) const { return m_shape[i]; }
        
        // function with negative index support like python does
        index_type operator()(int i) const
        {
            if (i<0) i = ndim()+i;
            ASSERT(i>=0 && i<ndim());
            return m_shape[i];
        }
        
        using tensor_shape::first;
        using tensor_shape::last;
        using tensor_shape::indices;
        using tensor_shape::ndim;
        using tensor_shape::toIndex;
        using tensor_shape::tuple;
        
        friend bool operator==(const tensor_shape& a, const const_tensor_strided_shape& b)
        {
            return a == b.shape();
        }
        friend bool operator==(const const_tensor_strided_shape& a, const tensor_shape& b)
        {
            return a.shape() == b;
        }
        friend bool operator==(const const_tensor_strided_shape& a, const const_tensor_strided_shape& b)
        {
            return a.shape() == b.shape();
        }
        
    public:
        
        const_tensor_strided_shape stridedTail(int n) const
        {
            ASSERT(n>=0 && n<=ndim());
            return const_tensor_strided_shape( n, shape_ptr()+ndim()-n, stride_ptr() + ndim()-n );
        }
        
        tensor_index displaceToIndex(index_type displace) const
        {
            ASSERT(displace>=0);
            int n = ndim();
            tensor_index ti(n,0);
            // find the biggest stride, divide, subtract... ?
            while(displace>=0)
            {
                bool changed = false;
                for(int i=0;i<n;++i)
                {
                    if (m_strides[i] <= displace && displace < m_strides[i]*m_shape[i])
                    {
                        ti[i] = displace/m_strides[i];
                        displace -= ti[i]*m_strides[i];
                        changed = true;
                    }
                }
                if (!changed) break;
            }
            ASSERT(displace==0);
            return ti;
        }
        
        bool canReshapeTo(const tensor_shape& shape) const
        {
            return strided_shape_ptr().canReshapeTo(shape.shape_ptr(), shape.ndim());
        }
        
        index_type getDisplace(const index_type * indices, int n) const
        {
            ASSERT(n<=ndim());
            index_type displace = 0;
            for(int i=0; i<n; ++i)
            {
                int index = indices[i]>=0 ? indices[i] : m_shape[i]+indices[i];
                ASSERT(index>=0 && index<m_shape[i]);
                displace+=m_strides[i]*index;
            }
            return displace;
        }
        
        template<size_t N>
        index_type getDisplaceC(const std::array<index_type, N>& index) const
        {
            index_type displace = 0;
            const index_type * p1 = std::data( m_strides );
            const index_type * p2 = std::data( index );
            
            for(size_t i=0; i<N; ++i) displace+=p1[i]*p2[i];
            
            return displace;
        }
        
        template<class Index>
        index_type getDisplace(Index&& index) const
        {
            return getDisplace(std::data(index), int(std::size(index)) );
        }
        
        index_type getDisplaceByAxis(int axis, index_type index) const
        {
            ASSERT(0<=axis && axis<ndim());
            ASSERT(0<=index && index < m_shape[axis]);
            return m_strides[axis]*index;
        }
        
        bool isSequential() const
        {
            index_type p = 1;
            for(int i=ndim()-1;i>=0;--i)
            {
                // ignore 1-shaped dimensions because their stride does not matter
                if (m_shape[i]!=1 && m_strides[i]!=p) return false;
                p*=m_shape[i];
            }
            return true;
        }
        
        const tensor_shape& shape() const { return *this; }
        
        friend std::ostream& operator<<(std::ostream& os, const const_tensor_strided_shape& s)
        {
            return os << s.shape();
        }
        
        tensor_strided_shape copy() const;
        tensor_shape copyShape() const { return *this; }

    protected:
        tensor_impl::strided_shape_ptr strided_shape_ptr() const
        {
            return tensor_impl::strided_shape_ptr(shape_ptr(), stride_ptr(), ndim());
        }
        
        void initSequential()
        {
            index_type p = 1;
            for(int i=ndim()-1;i>=0; --i)
            {
                m_strides[i] = p;
                p*=m_shape[i];
            }
        }
        
        const_tensor_strided_shape& operator=(const const_tensor_strided_shape& other) = default;
        const_tensor_strided_shape& operator=(const_tensor_strided_shape&& other) = default;
    };
    
    
    class [[nodiscard]] tensor_strided_shape : public const_tensor_strided_shape
    {
        typedef const_tensor_strided_shape Base;
    public:
        tensor_strided_shape() {}
        
        tensor_strided_shape(const std::initializer_list<index_type>& dimensions) : Base(dimensions) {}
        tensor_strided_shape(const tensor_shape& shape) : Base( shape ) {}
        tensor_strided_shape(const const_tensor_strided_shape& shape) : Base(shape) {}
        
        tensor_strided_shape(const tensor_strided_shape&) = default;
        tensor_strided_shape(tensor_strided_shape&&) = default;
        tensor_strided_shape& operator=(const tensor_strided_shape&) = default;
        tensor_strided_shape& operator=(tensor_strided_shape&&) = default;
    public:
        // insert new axes with 0-stride before the given axis
        tensor_strided_shape& insertAxes(int axis_index, const tensor_shape& replication)
        {
            ASSERT(axis_index>=0 && axis_index<=ndim());
            m_shape.insert( m_shape.begin() + axis_index, replication.begin(), replication.end() );
            m_strides.insert( m_strides.begin() + axis_index, replication.ndim(), 0);
            ASSERT(m_strides.size()==m_shape.size());
            return *this;
        }
        tensor_strided_shape& window(int axis, int step, int size)
        {
            ASSERT(axis>=0 && axis<ndim());
            ASSERT(size<=m_shape[axis]);
            m_shape.push_back(size);
            m_strides.push_back(m_strides[axis]);
            m_shape[axis] = (m_shape[axis]-size)/step + 1;
            m_strides[axis] = m_strides[axis]*step;
            return *this;
        }
        tensor_strided_shape& reshape(const tensor_shape& shape)
        {
            tensor_strided_shape reshaped(shape);
            reshaped.copyStridesFrom(*this);
            *this = std::move(reshaped);
            return *this;
        }
        
        tensor_strided_shape& slice(const std::initializer_list<index_slice>& slices, index_type& displace)
        {
            ASSERT( slices.size()<=ndim() );
            index_type d = 0;
            for(size_t i = 0, n = slices.size(); i!=n; ++i )
            {
                const index_slice& s = std::data(slices)[i];
                ASSERT(s.step!=0);
                
                if (s.i!=s.npos)
                {
                    int ind = s.i >= 0 ? s.i : m_shape[i]+s.i;
                    ASSERT( 0<=ind && ind < m_shape[i] );
                    d+=m_strides[i]*ind;
                    m_shape[i]=1;
                }
                else
                {
                    index_type trim1 = std::clamp( (s.b>=0? s.b : m_shape[i]+s.b), 0, m_shape[i]);
                    index_type trim2;
                    if (s.step >= 0)
                    {
                        trim2 = std::clamp( (s.e>=0? s.e : m_shape[i]+s.e), 0, m_shape[i]);
                        trim2 = std::clamp( (s.n>0 ? trim1 + s.n : m_shape[i]), 0, trim2);
                    }
                    else
                    {
                        trim2 = std::clamp( (s.e>=0? s.e : m_shape[i]+s.e), 0, m_shape[i]);
                        if (s.e==s.npos) trim2 = -1;
                        if (s.n > 0) trim2 = std::max( trim1 - s.n, trim2);
                    }
                    
                    m_shape[i] = 1 + (trim2 - trim1 - (s.step>0?1:-1) )/s.step;
                    
                    d+=m_strides[i]*trim1;
                    m_strides[i]*=s.step;
                    ASSERT(m_shape[i]>0);
                }
            }
            displace = d;
            return *this;
        }
        
        void splitAxis(int axis, int num_parts)
        {
            if (axis<0) axis = ndim()+axis;
            ASSERT(0<=axis && axis < ndim());
            ASSERT(m_shape[axis]%num_parts == 0);
            m_shape[axis]/=num_parts;
            index_type new_stride = m_strides[axis]*m_shape[axis];
            m_shape.insert(m_shape.begin()+axis, index_type(num_parts));
            m_strides.insert(m_strides.begin()+axis, new_stride);
        }
        
        void sliceAxis(int axis, index_type index_b, index_type index_e, index_type step = 1)
        {
            tensor_shape::sliceAxis(axis, index_b, index_e, step);
            m_strides[axis]*=step;
        }
        
        /// returns new shape that replicates all 1-shaped axes to match "shape"
        /// if the current shape has less dimensions than "shape" missing axes are added
        tensor_strided_shape& upshape(const tensor_shape& shape)
        {
            if (shape.ndim()>ndim()) insertAxes(ndim(), shape.last(shape.ndim()-ndim()));
            for(int i=0;i<shape.ndim();++i)
            {
                if (m_shape[i]==1 && shape[i]>1) { m_shape[i]=shape[i]; m_strides[i] = 0; }
                ASSERT(m_shape[i]>=shape[i]); // can't upshape
            }
            return *this;
        }
        tensor_strided_shape& destroyAxis(int axis)
        {
            ASSERT(0<=axis && axis<ndim());
            m_shape.erase(m_shape.begin() + axis);
            m_strides.erase(m_strides.begin() + axis);
            return *this;
        }
        tensor_strided_shape& flip(int axis)
        {
            ASSERT(0<=axis && axis<ndim());
            m_strides[axis] = -m_strides[axis];
            return *this;
        }
        tensor_strided_shape& permuteAxes(const std::vector<int>& axes)
        {
            tensor_strided_shape res = *this;
            ASSERT(axes.size() <= ndim());
            int n = int(axes.size());
            
            for(int i=0; i<n; ++i)
            {
                ASSERT(axes[i]>=0 && axes[i]<n);
                for(int j=i+1; j<n; ++j) ASSERT(axes[j]!=axes[i]);
                
                res.m_shape[i] = m_shape[ axes[i] ];
                res.m_strides[i] = m_strides[ axes[i] ];
            }
            *this = std::move(res);
            return *this;
        }
        tensor_strided_shape& swapAxes(int axis1, int axis2)
        {
            if (axis1 < 0) axis1 = ndim()+axis1;
            if (axis2 < 0) axis2 = ndim()+axis2;
            ASSERT(0 <= axis1 && axis1 < ndim());
            ASSERT(0 <= axis2 && axis2 < ndim());
            if (axis1!=axis2)
            {
                std::swap(m_shape[axis1], m_shape[ axis2 ]);
                std::swap(m_strides[axis1], m_strides[ axis2 ]);
            }
            return *this;
        }
        tensor_strided_shape& copyStridesFrom(const const_tensor_strided_shape& other)
        {
            // if tensor can't be directly reshaped user should make copy() before reshaping
            ASSERT(other.canReshapeTo(shape()));
            int i1 = ndim()-1;
            int i2 = other.ndim()-1;
            index_type s1 = 1, s2 = 1;  // accumulated sizes
            
            while(i1>=0)
            {
                if (m_shape[i1]==1)
                {
                    m_strides[i1--]=1;
                }
                else if (s1==s2)
                {
                    while(i2>=0 && other.m_shape[i2]==1) --i2;
                    // Here m_shape[i1]!=1, so there should be some data in other shape
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
            return *this;
        }
    };
    
    inline tensor_strided_shape const_tensor_strided_shape::copy() const { return *this; }
}

#endif // algotest_tensor_strided_shape_included
