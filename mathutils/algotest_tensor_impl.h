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

#ifndef algotest_tensor_impl_included
#define algotest_tensor_impl_included

#include <math.h>
#include <initializer_list>
#include <type_traits>
#include <ostream>
#include <memory>
#include <functional>

#include "stlutil.h"


namespace algotest
{
    struct tensor_settings
    {
        // index_type should be signed
        typedef int index_type;
    };
    
    class tensor_impl : public tensor_settings
    {
    protected:
    
        struct strided_shape_ptr
        {
            const index_type * m_shape;
            const index_type * m_strides;
            int m_dims;
        public:
            strided_shape_ptr(const index_type * shape, const index_type * strides, int dims)
                : m_shape(shape), m_strides(strides), m_dims(dims)
            {
                ASSERT(dims>0);
            }
            
            bool isSequential() const
            {
                index_type p = 1;
                for(int i=m_dims-1;i>=0;--i)
                {
                    if (m_strides[i]!=p) return false;
                    p*=m_shape[i];
                }
                return true;
            }
            index_type product() const
            {
                index_type res = 1;
                for(int i=0; i<m_dims; ++i) res *= m_shape[i];
                return res;
            }
            
            bool isPrefixOf(const strided_shape_ptr& other) const
            {
                if (m_dims > other.m_dims) return false;
                for(int i=0;i<m_dims;++i) if (m_shape[i]!=other.m_shape[i]) return false;
                return true;
            }
            
            
            // check if stride is uniform in the range [from, to)
            bool checkUniform(int from, int to) const
            {
                for(int i=from; i<to-1; ++i)
                {
                    LOGI_("Check uniformness of " << i << "..." << (i+1) );
                    if (m_strides[i] != m_shape[i+1]*m_strides[i+1]) return false;
                }
                return true;
            }
            
            bool canReshapeTo(const index_type * o_shape, int o_dims) const
            {
                index_type s1 = 1, s2 = 1;
                int        i1 = 0, i2 = 0;
                int        c = 0;  // number of checked dimensions

                for(;;)
                {
                    if (i1==m_dims && i2==o_dims) return s1==s2;  // all checked
                    
                    if (s1<s2)
                    {
                        s1 *= i1>=m_dims? 1 : m_shape[i1++];        // pad missed dimensions as 1-sized
                    }
                    else if (s1>s2)
                    {
                        s2 *= i2>=o_dims? 1 : o_shape[i2++];
                    }
                    
                    if (s1==s2)
                    {
                        if (!checkUniform(c, i1)) return false;
                        c = i1;
                        s1 = i1>=m_dims? 1 : m_shape[i1++];        // pad missed dimensions as 1-sized
                        s2 = i2>=o_dims? 1 : o_shape[i2++];
                    }
                }
                return true;
            }
            
            void print(std::ostream& os) const
            {
                os << "shape = ( ";
                for(int i=0;i<m_dims;++i) { os << m_shape[i] << " "; }
                os << ")";
            }
        };
    
        /// tensor_tail represents part of the tensor starting from some dimention index
        template<class T>
        struct strided_array_ptr : public strided_shape_ptr
        {
            T* m_ptr;
        public:
            strided_array_ptr(T* ptr, const index_type * shape, const index_type * strides, int dims)
                : m_ptr(ptr), strided_shape_ptr(shape, strides, dims)
            {
            }
            
            strided_array_ptr<T> subarray(T*p) const
            {
                return strided_array_ptr<T>(p, m_shape+1, m_strides+1, m_dims-1);
            }
            
            template<class OP>
            void apply(OP&& op)
            {
                if (isSequential())
                {
                    for(T * p=m_ptr, *pe = p+product(); p!=pe; ++p) op(*p);
                    return;
                }
                
                switch(m_dims)
                {
                    case 1:
                    {
                        index_type s0 = m_strides[0];
                        index_type sh0 = m_shape[0];
                        for(T* p=m_ptr, *e0 = p+sh0*s0; p!=e0; p+=s0) op(*p);
                        return;
                    }
                    case 2:
                    {
                        index_type s0 = m_strides[0], s1 = m_strides[1];
                        index_type sh0 = m_shape[0], sh1 = m_shape[1];
                        for(T* p0=m_ptr,  *e0 = p0+sh0*s0; p0!=e0; p0+=s0)
                            for(T* p=p0, *e1 = p+sh1*s1; p!=e1; p+=s1) op(*p);
                        return;
                    }
                    case 3:
                    {
                        index_type s0 = m_strides[0], s1 = m_strides[1], s2 = m_strides[2];
                        index_type sh0 = m_shape[0], sh1 = m_shape[1], sh2 = m_shape[2];
                        for(T* p0 = m_ptr,       *e0 = p0+sh0*s0; p0!=e0; p0+=s0)
                            for(T* p1 = p0,      *e1 = p1+sh1*s1; p1!=e1; p1+=s1)
                                for(T* p =  p1,  *e2 =  p+sh2*s2;  p!=e2;  p+=s2) op(*p);
                        return;
                    }
                    default:
                    {
                        index_type s0 = m_strides[0];
                        for(T* p=m_ptr, *e0 = p+m_shape[0]*s0; p!=e0; p+=s0)
                        {
                            subarray(p).apply(op);
                        }
                    }
                }
            }
            
            template<class U, class OP2>
            void apply(strided_array_ptr<U> a, OP2&& op)
            {
                ASSERT(isPrefixOf(a));
                if (isSequential() && a.isSequential() && m_dims == a.m_dims)
                {
                    U* pa = a.m_ptr;
                    for(T * p=m_ptr, *pe = p+product(); p!=pe; ++p, ++pa) op(*p, *pa);
                    return;
                }
                
                switch(m_dims)
                {
                    case 1:
                    {
                        index_type s0 = m_strides[0];
                        index_type sh0 = m_shape[0];
                        index_type as0 = a.m_strides[0];
                        U* pa = a.m_ptr;
                        for(T* p=m_ptr, *e0 = p+sh0*s0; p!=e0; p+=s0, pa+=as0) op(*p, *pa);
                        return;
                    }
                    case 2:
                    {
                        index_type s0 = m_strides[0], s1 = m_strides[1];
                        index_type sh0 = m_shape[0], sh1 = m_shape[1];
                        index_type as0 = a.m_strides[0], as1 = a.m_strides[1];
                        U* pa0 = a.m_ptr;
                        
                        for(T* p0=m_ptr,  *e0 = p0+sh0*s0; p0!=e0; p0+=s0, pa0+=as0)
                        {
                            U* pa = pa0;
                            for(T* p=p0, *e1 = p+sh1*s1; p!=e1; p+=s1, pa+=as1) op(*p, *pa);
                        }
                        return;
                    }
                    case 3:
                    {
                        index_type s0 = m_strides[0], s1 = m_strides[1], s2 = m_strides[2];
                        index_type sh0 = m_shape[0], sh1 = m_shape[1], sh2 = m_shape[2];
                        index_type as0 = a.m_strides[0], as1 = a.m_strides[1], as2 = a.m_strides[2];
                        U* pa0 = a.m_ptr;
                        for(T* p0 = m_ptr,       *e0 = p0+sh0*s0; p0!=e0; p0+=s0, pa0+=as0)
                        {
                            U* pa1 = pa0;
                            for(T* p1 = p0,      *e1 = p1+sh1*s1; p1!=e1; p1+=s1, pa1+=as1)
                            {
                                U* pa = pa1;
                                for(T* p =  p1,  *e2 =  p+sh2*s2;  p!=e2;  p+=s2, pa+=as2) op(*p, *pa);
                            }
                        }
                        return;
                    }
                    default:
                    {
                        index_type s0 = m_strides[0];
                        index_type as0 = a.m_strides[0];
                        U* pa = a.m_ptr;
                        for(T* p=m_ptr, *e0 = p+m_shape[0]*s0; p!=e0; p+=s0, pa+=as0)
                        {
                            subarray(p).apply(a.subarray(pa), op);
                        }
                    }
                }
            }
            
            template<class U, class V, class OP3>
            void apply(strided_array_ptr<U> a, strided_array_ptr<V> b, OP3&& op)
            {
                ASSERT(isPrefixOf(a) && isPrefixOf(b));
                if (isSequential() && a.isSequential() && b.isSequential() && m_dims == a.m_dims  && m_dims == b.m_dims)
                {
                    U* pa = a.m_ptr;
                    V* pb = b.m_ptr;
                    for(T * p=m_ptr, *pe = p+product(); p!=pe; ++p, ++pa, ++pb) op(*p, *pa, *pb);
                    return;
                }
                
                switch(m_dims)
                {
                    case 1:
                    {
                        index_type s0 = m_strides[0];
                        index_type sh0 = m_shape[0];
                        index_type as0 = a.m_strides[0];
                        index_type bs0 = b.m_strides[0];
                        U* pa = a.m_ptr;
                        V* pb = b.m_ptr;
                        for(T* p=m_ptr, *e0 = p+sh0*s0; p!=e0; p+=s0, pa+=as0, pb+=bs0) op(*p, *pa, *pb);
                        return;
                    }
                    case 2:
                    {
                        index_type s0 = m_strides[0], s1 = m_strides[1];
                        index_type sh0 = m_shape[0], sh1 = m_shape[1];
                        index_type as0 = a.m_strides[0], as1 = a.m_strides[1];
                        index_type bs0 = b.m_strides[0], bs1 = a.m_strides[1];
                        U* pa0 = a.m_ptr;
                        V* pb0 = b.m_ptr;
                        
                        for(T* p0=m_ptr,  *e0 = p0+sh0*s0; p0!=e0; p0+=s0, pa0+=as0, pb0+=bs0 )
                        {
                            U* pa = pa0;
                            V* pb = pb0;
                            for(T* p=p0, *e1 = p+sh1*s1; p!=e1; p+=s1, pa+=as1, pb+=bs1) op(*p, *pa, *pb);
                        }
                        return;
                    }
                    case 3:
                    {
                        index_type s0 = m_strides[0], s1 = m_strides[1], s2 = m_strides[2];
                        index_type sh0 = m_shape[0], sh1 = m_shape[1], sh2 = m_shape[2];
                        index_type as0 = a.m_strides[0], as1 = a.m_strides[1], as2 = a.m_strides[2];
                        index_type bs0 = b.m_strides[0], bs1 = b.m_strides[1], bs2 = b.m_strides[2];
                        U* pa0 = a.m_ptr;
                        V* pb0 = b.m_ptr;
                        for(T* p0 = m_ptr,       *e0 = p0+sh0*s0; p0!=e0; p0+=s0, pa0+=as0, pb0+=bs0)
                        {
                            U* pa1 = pa0;
                            V* pb1 = pb0;
                            for(T* p1 = p0,      *e1 = p1+sh1*s1; p1!=e1; p1+=s1, pa1+=as1, pb1+=bs1)
                            {
                                U* pa = pa1;
                                V* pb = pb1;
                                for(T* p =  p1,  *e2 =  p+sh2*s2;  p!=e2;  p+=s2, pa+=as2, pb+=bs2) op(*p, *pa, *pb);
                            }
                        }
                        return;
                    }
                    default:
                    {
                        index_type s0 = m_strides[0];
                        index_type as0 = a.m_strides[0];
                        index_type bs0 = b.m_strides[0];
                        U* pa = a.m_ptr;
                        V* pb = b.m_ptr;
                        for(T* p=m_ptr, *e0 = p+m_shape[0]*s0; p!=e0; p+=s0, pa+=as0, pb+=bs0)
                        {
                            subarray(p).apply(a.subarray(pa), b.subarray(pb), op);
                        }
                    }
                }
            }
            
            void init(const T& val)
            {
                // copy for "half" support
                T local_val = val;
                if (isSequential()) { for(T * p=m_ptr, *pe = p+product(); p!=pe; ++p) *p=local_val; }
                else apply([local_val](T& x) {x=local_val;});
            }
            
            void print_values(std::ostream& os, int tab=0) const
            {
                if (m_dims==1)
                {
                    os << std::string(tab, ' ') << "[ ";
                    index_type s0 = m_strides[0];
                    index_type sh0 = m_shape[0];
                    T* p=m_ptr;
                    for(index_type i=0; i<sh0; ++i, p+=s0) os << *p << " ";
                    os << "]\n";
                    return;
                }
                else
                {
                    os << std::string(tab, ' ') << "[\n";
                    index_type s0 = m_strides[0];
                    index_type sh0 = m_shape[0];
                    T* p=m_ptr;
                    for(index_type i=0; i<sh0; ++i, p+=s0)
                    {
                        subarray(p).print_values(os, tab+4);
                    }
                    os << std::string(tab, ' ') << "]\n";
                }
            }
            
            void print(std::ostream& os) const
            {
                strided_shape_ptr::print(os);
                os << std::endl;
                print_values(os);
            }
        };
    };
}

#endif // algotest_tensor_impl_included
