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

#ifndef algotest_tensor_shape_included
#define algotest_tensor_shape_included

#include <algorithm>
#include <concepts>
#include <array>
#include "algotest_tensor_impl.h"

namespace algotest
{
    template<typename T>
    concept scalar_type = std::same_as<T, half> || std::numeric_limits<T>::is_specialized;
    
    template<typename T>
    concept non_scalar_type = !std::same_as<T, half> && !std::numeric_limits<T>::is_specialized;

    
    // tensor_index represent vestor of indexes on tensor axes
    class tensor_index : public std::vector< tensor_settings::index_type >,
                         public tensor_settings
    {
        typedef std::vector< tensor_settings::index_type > Base;
    public:
        tensor_index() {}
        tensor_index(const Base& v) : Base(v) {}
        tensor_index(Base&& v) : Base(std::move(v)) {}
        tensor_index(int ndim, index_type init = 0) : Base( size_t(ndim), init) {}
        tensor_index(size_t ndim, index_type init = 0) : Base( ndim, init) {}
        
        tensor_index(const std::initializer_list<index_type>& index) : Base(index) {}
        
        template<non_scalar_type It>
        tensor_index(It begin, It end) : Base(begin, end) {}
        
        int ndim() const { return int(size()); }
        
        template<int N>
        std::array<tensor_settings::index_type, N> tuple() const
        {
            std::array<tensor_settings::index_type, N> res;
            std::copy( data(), data() + std::min(N, ndim()), res.begin() );
            return res;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const algotest::tensor_index& ind)
        {
            os << "( ";
            for(index_type i : ind) os << i << " ";
            return os << ")";
        }
        
        std::string summary() const
        {
            std::ostringstream os;
            os << *this;
            return os.str();
        }
    };
    
    class tensor_indices_iterator
    {
        tensor_index m_index;
        tensor_index m_shape;
    public:
        // end() iterator has void tensor_index
        tensor_indices_iterator() {}
        tensor_indices_iterator(const tensor_index& shape) : m_shape(shape), m_index(shape.size(), 0) {}
        
        const tensor_index& operator*() const { return m_index; }
        tensor_indices_iterator& operator++()
        {
            size_t i = m_index.size()-1;
            while( ++m_index[i] >= m_shape[i] )
            {
                m_index[i]=0;
                if (i==0) { m_index.clear(); break; }
                --i;
            }
            return *this;
        }
        bool operator==(const tensor_indices_iterator& other) const { return m_index == other.m_index; }
    };
    
    class tensor_indices_range
    {
        tensor_index m_shape;
    public:
        tensor_indices_range(const tensor_index& s) : m_shape(s) {}
        auto begin() const { return tensor_indices_iterator(m_shape); }
        auto end() const { return tensor_indices_iterator(); }
    };
    
    template<int N>
    struct tensor_index_n
    {
        typedef std::array<tensor_settings::index_type, N> type;
        
        static type construct(const tensor_index& i)
        {
            ASSERT(i.ndim()>=N);
            type res;
            std::copy( i.data(), i.data()+N, res.data() );
            return res;
        }
        static constexpr type fill(tensor_settings::index_type i)
        {
            type res;
            std::fill( res.data(), res.data()+N, i );
            return res;
        }
    };
    
    template<int N>
    class tensor_indices_iterator_n
    {
        typename tensor_index_n<N>::type m_index;
        typename tensor_index_n<N>::type m_shape;
    public:
        // end() iterator has void tensor_index
        tensor_indices_iterator_n()
        {
            m_index = tensor_index_n<N>::fill(-1);
        }
        tensor_indices_iterator_n(const typename tensor_index_n<N>::type& shape) : m_shape(shape)
        {
            m_index = tensor_index_n<N>::fill(0);
        }
        tensor_indices_iterator_n(const tensor_index& shape)
        {
            m_index = tensor_index_n<N>::fill(0);
            m_shape = tensor_index_n<N>::construct(shape);
        }
        
        const auto& operator*() const { return m_index; }
        tensor_indices_iterator_n& operator++()
        {
            size_t i = N-1;
            while( ++m_index[i] >= m_shape[i] )
            {
                m_index[i]=0;
                if (i==0) { m_index = tensor_index_n<N>::fill(-1); break; }
                --i;
            }
            return *this;
        }
        bool operator==(const tensor_indices_iterator_n& other) const { return m_index == other.m_index; }
    };
    
    template<int N>
    class tensor_indices_range_n
    {
        typename tensor_index_n<N>::type m_shape;
    public:
        tensor_indices_range_n(const tensor_index& s) : m_shape( tensor_index_n<N>::construct(s) ) {}
        auto begin() const { return tensor_indices_iterator_n<N>(m_shape); }
        auto end() const { return tensor_indices_iterator_n<N>(); }
    };
    
//    template<typename T>
//    concept IsTensorShape = std::same_as<T, const tensor_shape&> ||
//                            std::same_as<T, tensor_shape> ||
//                            std::same_as<T, tensor_shape&& >;
    
    // tensor_shape represents shape of a vtensor.
    // Tensor with 0 dimensions represents a scalar
    class [[nodiscard]] tensor_shape : protected tensor_impl
    {
    protected:
        tensor_index m_shape;    // m_shape.size()>0
    public:
        typedef typename std::vector<index_type>::const_iterator const_iterator;
    public:
        tensor_shape() {}
        tensor_shape(const std::initializer_list<index_type>& dimensions)
            : m_shape( dimensions )
        {
        }
        tensor_shape(const std::initializer_list<size_t>& dimensions)
        {
            m_shape.insert(m_shape.begin(), dimensions.begin(), dimensions.end());
        }
        
        tensor_shape(const tensor_shape& shape)
            : m_shape( shape.m_shape )
        {
        }
        
        
        explicit tensor_shape(int ndim, const index_type * ptr)
            : m_shape( ptr, ptr+ndim )
        {
            
        }
  
        tensor_shape(const tensor_shape& s1, const tensor_shape& s2)
        {
            m_shape.insert(m_shape.end(), s1.begin(), s1.end());
            m_shape.insert(m_shape.end(), s2.begin(), s2.end());
        }
        
        template<class T>
        tensor_shape(const std::vector<T>& v)
        {
            m_shape.insert(m_shape.end(), v.begin(), v.end());
        }
        
        const tensor_index& toIndex() const { return m_shape; }
        template<int N> std::array<tensor_settings::index_type, N> tuple() const { return m_shape.tuple<N>(); }

        
    public:
        bool isPrefixOf(const tensor_shape& other) const
        {
            if (ndim() > other.ndim()) return false;
            for(size_t i=0, n=m_shape.size(); i<n; ++i) if (m_shape[i]!=other.m_shape[i]) return false;
            return true;
        }
        
        
//        explicit tensor_shape(auto&& ... shape)
//        {
//            ( m_shape.insert(m_shape.end(), shape.begin(), shape.end()), ... );
//        }
        
        auto indices() const { return tensor_indices_range(m_shape); }
        
        template<int N>
        auto indices() const { return tensor_indices_range_n<N>(m_shape); }
        
        tensor_shape first(int n) const
        {
            ASSERT(n>0 && n<=ndim());
            return tensor_shape( n, shape_ptr());
        }
        tensor_shape last(int n) const
        {
            ASSERT(n>0 && n<=ndim());
            return tensor_shape( n, shape_ptr()+ndim()-n);
        }
        
        const_iterator begin() const { return m_shape.begin(); }
        const_iterator end() const { return m_shape.end(); }
        
        /// number of vtensor dimensions
        index_type operator[](int i) const { return m_shape[i]; }
        index_type at(int i) const { return m_shape[i]; }
        
        index_type& operator[](int i) { return m_shape[i]; }
        index_type& at(int i) { return m_shape[i]; }
        
        const index_type* shape_ptr() const { return std::data(m_shape); }
        
        int ndim() const { return int(m_shape.size()); }
        /// total count of elements
        index_type numElements() const
        {
            index_type res = 1;
            for(int i=0, n = ndim(); i<n; ++i) res *= m_shape[i];
            return res;
        }
        
        std::vector<index_type> cumulative_count() const
        {
            std::vector<index_type> res(m_shape.size());
            index_type count = 1;
            for(int i=ndim()-1; i>=0; --i)
            {
                res[i] = (count*=m_shape[i]);
            }
            return res;
        }
        
        void extend(const std::vector<int>& axes, int add)
        {
            ASSERT(axes.size()<=ndim());
            for(int ax : axes)
            {
                ASSERT(0<=ax && ax < ndim());
                m_shape[ax] += add;
            }
        }
        
        /// Trim shape to contain less data, but preserve number of dimensions
        void trim(const index_type * indices, int n)
        {
            ASSERT(n<=ndim());
            for(int i=0; i<n; ++i)
            {
                index_type trim_v = indices[i]>=0 ? indices[i] : m_shape[i]+indices[i];
                m_shape[i] = std::clamp(trim_v, 0, m_shape[i]);
            }
        }
        
        /// Trim shape to contain less data, but preserve number of dimensions
        void trimTail(const index_type * indices, int n)
        {
            ASSERT(n<=ndim());
            for(int i=0; i<n; ++i)
            {
                index_type trim_v = indices[i]>=0 ? m_shape[i]-indices[i] : -indices[i];
                m_shape[i] = std::clamp(trim_v, 0, m_shape[i]);
            }
        }
        
        void crop(const index_type * index_b, const index_type * index_e, int n)
        {
            ASSERT(n<=ndim());
            for(int i=0; i<n; ++i)
            {
                index_type trim1 = std::clamp(index_b[i]>=0 ? index_b[i] : m_shape[i]+index_b[i], 0, m_shape[i]);
                index_type trim2 = std::clamp(index_e[i]>=0 ? index_e[i] : m_shape[i]+index_e[i], trim1, m_shape[i]);
                m_shape[i] = trim2-trim1;
            }
        }
        void crop_size(const index_type * index_b, const index_type * index_size, int n)
        {
            ASSERT(n<=ndim());
            for(int i=0; i<n; ++i)
            {
                index_type trim1 = std::clamp(index_b[i]>=0 ? index_b[i] : m_shape[i]+index_b[i], 0, m_shape[i]);
                index_type trim2 = std::clamp(trim1 + index_size[i], trim1, m_shape[i]);
                m_shape[i] = trim2-trim1;
            }
        }
        void sliceAxis(int axis, index_type index_b, index_type index_e, index_type step = 1)
        {
            ASSERT(0<=axis && axis<ndim());
            ASSERT(index_e>index_b);
            ASSERT(step>=1);
            
            index_type trim1 = index_b>=0 ? index_b : m_shape[axis]+index_b;
            index_type trim2 = index_e>=0 ? index_e : m_shape[axis]+index_e;
            m_shape[axis] = ( std::clamp(trim2-trim1, 0, m_shape[axis]) + (step-1) )/step;
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
        void crop_size(const tensor_index& begin, const tensor_index& size)
        {
            ASSERT(begin.size()==size.size());
            crop_size(std::data(begin), std::data(size), int(std::size(begin)) );
        }
        
        tensor_shape& appendAxis(index_type i)
        {
            m_shape.push_back(i);
            return *this;
        }
        
        tensor_shape& appendAxes(const tensor_shape& s)
        {
            m_shape.insert(m_shape.end(), s.m_shape.begin(), s.m_shape.end());
            return *this;
        }
        
        
        tensor_shape& insertAxis(int before_axis, index_type size)
        {
            ASSERT(before_axis>=0 && before_axis<=ndim());
            m_shape.insert(m_shape.begin() + before_axis, size);
            return *this;
        }
        
        tensor_shape& removeAxis(int axis)
        {
            ASSERT(axis>=0 && axis<ndim());
            m_shape.erase(m_shape.begin() + axis);
            return *this;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const tensor_shape& s)
        {
            os << "( ";
            for(auto i : s) os << i << " ";
            return os<<")";
        }
        
        bool operator==(const tensor_shape& a) const { return m_shape == a.m_shape; }

        std::string summary() const
        {
            std::ostringstream os;
            os << *this;
            return os.str();
        }
    };
}

#endif // algotest_tensor_shape_included
