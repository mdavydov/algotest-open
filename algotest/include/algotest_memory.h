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

#ifndef algotest_memory_included
#define algotest_memory_included

namespace algotest
{
    /**
     @brief const_holder class is used to implement const semantics for returned objects
     */
    template<class T>
    class const_holder
    {
        T m_t;
    public:
        const_holder(T&& t) : m_t(std::move(t)) {}
        operator const T&() const { return m_t; }
        const T* operator->() const { return &m_t; }
    };

    /// @brief initializer_t is a helper class for implicit initialization arguments
    template<class T> class initializer_t : public const_holder<T>
    {
        public: initializer_t(T&& t) : const_holder<T>(std::move(t)) {}
    };

    template<class T> auto initializer(T&& val) { return initializer_t<T>(std::move(val)); }


    /// Abstract data holder
    class AbstractData
    {
    public:
        virtual ~AbstractData() {}
    };

    template<class T>
    class ArrayPtr
    {
        T* ptr;
    public:
        ArrayPtr(T* ptr) : ptr(ptr) {}
        ArrayPtr(ArrayPtr&& o) : ptr(o.ptr) { o.ptr = 0; }
        ArrayPtr(const ArrayPtr& o)=delete;
        ArrayPtr& operator=(const ArrayPtr& o)=delete;
        ~ArrayPtr() { delete[] ptr; }
    };

    template<class T>
    class TypedData : public AbstractData
    {
        T m_data;
    public:
        TypedData(const T& data) : m_data(data) {}
        TypedData(T&& data) : m_data( std::move(data) ) {}
    };

    template<class T>
    std::shared_ptr<AbstractData> abstractDataHolder(const T& data)
    {
        return std::make_shared<TypedData<T>>(data);
    }

    template<class T>
    std::shared_ptr<AbstractData> abstractDataHolder(T&& data)
    {
        return std::make_shared<TypedData<T>>( std::move(data) );
    }

    class memory_block
    {
        void * m_data;
        size_t m_reserved_size;
    public:
        memory_block() : m_data(0), m_reserved_size(0) {}
        memory_block(void * data, size_t reserved_size)
            : m_data(data), m_reserved_size(reserved_size) {}
        memory_block(void * data, int reserved_size)
            : m_data(data), m_reserved_size(size_t(reserved_size)) { assert(reserved_size>=0); }

        memory_block(const memory_block& d) = default;
        memory_block& operator=(const memory_block& d) = default;
    public:
        void * data() const { return m_data; }
        size_t reserved_size() const { return m_reserved_size; }
        bool empty() const { return m_reserved_size==0; }
    };
}

#endif // algotest_memory_included
