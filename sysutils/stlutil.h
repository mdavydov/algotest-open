/*  The Sysutils library for Threads/Sockets/PG Database/RPC
    Copyright (C) 2007 Maksym Davydov (http://www.adva-soft.com)

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

/* stlutil.h - useful function to work with stl
 **/
#pragma once
#ifndef __STLUTILINCLUDED__
#define __STLUTILINCLUDED__

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4290)
#endif

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <atomic>
#include <cassert>

#define SYSUTILS_DECLARE_NO_COPY(a) a(const a&) = delete; a&operator=(const a&) = delete;

template<class T> struct range_t
{
    struct range_el
    {
        T el;
        range_el(T el) : el(el) {}
        inline T operator*() const { return el; }
        inline void operator++() { ++el; }
        inline bool operator!=(const range_el& other) { return el!=other.el; }
    };
    T m_begin, m_end;
    range_t(T begin, T end) : m_begin(begin), m_end(end) {}
    inline range_el begin() const { return m_begin; }
    inline range_el end() const { return m_end; }
    T length() const { return m_end-m_begin; }
    bool valid() const {return m_begin <= m_end; }
};

template<class T>
inline range_t<T> range(T beg, T end) { return range_t<T>(beg,end); }

template<class T>
inline range_t<T> range(T end) { return range_t<T>(T(0),end); }

template<class T>
inline range_t<size_t> index_range(std::vector<T>& v) { return range(v.size()); }

template<class inIt, class outIt>
void copy_n(int N, inIt i, outIt o)
{
	while(N--) *o++ = *i++;
}

//! Calls delete *i for i in [beg,end)
template<class T>
inline void DeleteAll(T beg,T end)
{
	for(;beg!=end;++beg)
		delete *beg;
}

template<class T, class X>
inline void RemoveFromVector(T& container,const X& value)
{
	typename T::iterator i = std::find( container.begin(), container.end(), value );
	if ( i==container.end() ) return;
	container.erase(i);
}


//! Calls delete *i for i in [beg,end)
template<class T>
inline void SafeDeleteAll(T beg,T end)
{
	for(;beg!=end;++beg)
		if (*beg) delete *beg;
}

template<class T>
inline void SafeReleaseAll(T beg,T end)
{
	for(;beg!=end;++beg)
		if (*beg) (*beg)->Release();
}


//! Calls delete i->second for i in [beg,end)
template<class T>
inline void DeleteAllMapped(T beg,T end)
{
	for(;beg!=end;++beg)
		delete beg->second;
}

//! does **i=x for i in [beg,end)
template<class T, class Q>
inline void AssignPointed(T beg,T end, Q x)
{
	for(;beg!=end;++beg) **beg = x;
}

//! Verifies wether key belongs to set (map or set)
template<class T>
bool is_in( T& set, const typename T::key_type& key)
{
	return set.find(key)!=set.end();
}


template<class T>
typename T::mapped_type peek( const T& some_map, const typename T::key_type& key)
{
    typename T::const_iterator i = some_map.find(key);
    if (i != some_map.end()) return i->second;
    return typename T::mapped_type();
}

template<class T>
bool is_in_vect( T& vect, const typename T::value_type& key)
{
	return std::find(vect.begin(), vect.end(), key)!=vect.end();
}

//! Does for iterator named i in vector v of type T for all entries
#define FORALL(T,v,i) for(T::iterator i=(v).begin();i!=(v).end();++i)
#define FORALLC(T,v,i) for(T::const_iterator i=(v).begin();i!=(v).end();++i)
#define FORALLMAP(K,V,v,i) for(std::map<K,V>::iterator i=(v).begin();i!=(v).end();++i)
#define FORALLMAPC(K,V,v,i) for(std::map<K,V>::const_iterator i=(v).begin();i!=(v).end();++i)
#define FORALLPAIRS(T,v,i,j) \
	for(T::iterator i=v.begin();i!=v.end();++i) \
	for(T::iterator j=i+1;j!=v.end();++j)
#define FORALLPAIRSC(T,v,i,j) \
	for(T::const_iterator i=v.begin();i!=v.end();++i) \
	for(T::const_iterator j=i+1;j!=v.end();++j)

class ref_ptr_destruction_method_delete
{
public:
	template<class T> static void Destroy(T * p) { delete p; }
};

class ref_ptr_destruction_method_delete_array
{
public:
    template<class T> static void Destroy(T * p) {delete[] p;}
};


template<> inline void ref_ptr_destruction_method_delete::Destroy<FILE>(FILE *f)
{
    if (f) { fclose(f); }
}

class destruction_method_free
{
public:
    static void Destroy(void* p) {free(p);}
};

namespace sysutils
{
	typedef std::atomic_int * pAtomic;
	pAtomic atomicAlloc(int init_value);
	void atomicFree(pAtomic p);
	bool atomicDecAndZeroTest(pAtomic p);
	void atomicInc(pAtomic p);
	int atomicRead(pAtomic p);
}

class ref_ptr_never_destroy {};

class ref_ptr_void
{
	sysutils::pAtomic p_counter;
protected:
	void* m_p;
protected:
	ref_ptr_void() : p_counter(0), m_p(0)  {}
	ref_ptr_void(void* p) : m_p(p) { p_counter = p ? sysutils::atomicAlloc(1) : 0; }
    ref_ptr_void(void* p, ref_ptr_never_destroy ) : m_p(p), p_counter(0) {}
	ref_ptr_void(const ref_ptr_void& other) : p_counter(other.p_counter), m_p(other.m_p) { if (p_counter) sysutils::atomicInc(p_counter); }
    ref_ptr_void(const ref_ptr_void& other, void * ptr) : p_counter(ptr?other.p_counter:0), m_p(ptr)
    {
        if (p_counter) sysutils::atomicInc(p_counter);
    }
    ref_ptr_void(ref_ptr_void&& other) : p_counter(other.p_counter), m_p(other.m_p) { other.p_counter = 0; other.m_p = 0; }
    ref_ptr_void(ref_ptr_void&& other, void * ptr) : p_counter(ptr?other.p_counter:0), m_p(ptr)
    {
        if (ptr) { other.p_counter = 0;  other.m_p = 0; }
    }
	~ref_ptr_void() {}
protected:
	ref_ptr_void& AddRef() { if (p_counter) sysutils::atomicInc(p_counter); return *this; }
    // delete reference to a pointer and return pointer if all references are deleted
	void* DelRefInternal() { if (p_counter && sysutils::atomicDecAndZeroTest(p_counter)) { void * p = m_p; sysutils::atomicFree(p_counter); m_p = 0; p_counter = 0; return p; } return 0; }
	void* ptr() const { return m_p; }
protected:
	void copyFrom(const ref_ptr_void& other)
	{
		m_p = other.m_p;
		p_counter = other.p_counter;
		if (p_counter) sysutils::atomicInc(p_counter);
	}
    void copyFrom(const ref_ptr_void& other, void * ptr)
    {
        m_p = ptr;
        p_counter = other.p_counter;
        if (p_counter) sysutils::atomicInc(p_counter);
    }
    void moveFrom(ref_ptr_void&& other)
    {
        m_p = other.m_p;
        p_counter = other.p_counter;
        other.p_counter = 0;
        other.m_p = 0;
    }
    void moveFrom(ref_ptr_void&& other, void * ptr)
    {
        m_p = ptr;
        p_counter = other.p_counter;
        other.p_counter = 0;
        other.m_p = 0;
    }
public:
	int getRefCount() const { return p_counter ? sysutils::atomicRead(p_counter) : 0; }
    bool isUnique() const { return getRefCount()==1; }
	bool operator<(const ref_ptr_void& other) const {return m_p<other.m_p;}
};

#ifdef DEBUG
	#define DEFINE_DEBUG_PTR T** m_debug_ptr;
	#define INIT_DEBUG_PTR m_debug_ptr = (T**)&m_p;
#else
	#define DEFINE_DEBUG_PTR
	#define INIT_DEBUG_PTR
#endif

template<class T, class DestructionMethod = ref_ptr_destruction_method_delete >
class ref_ptr : public ref_ptr_void
{
	DEFINE_DEBUG_PTR
    
protected:
    ref_ptr(const ref_ptr_void& other, T * ptr)
        : ref_ptr_void(other, ptr ) { INIT_DEBUG_PTR; }
    
    ref_ptr(ref_ptr_void&& other, T * ptr)
        : ref_ptr_void( std::move(other), ptr ) { INIT_DEBUG_PTR; }
    
public:
	ref_ptr() { INIT_DEBUG_PTR; }
	ref_ptr(T* p) : ref_ptr_void(p)  { INIT_DEBUG_PTR; }
	ref_ptr(const ref_ptr& other) : ref_ptr_void(other)  { INIT_DEBUG_PTR; }
    ref_ptr(ref_ptr&& other) : ref_ptr_void(std::move(other))  { INIT_DEBUG_PTR; }
    
    template<class U> ref_ptr(const ref_ptr<U, DestructionMethod>& other)
        : ref_ptr(other, other.ptr() ) {}
    template<class U> ref_ptr(ref_ptr<U, DestructionMethod>&& other)
        : ref_ptr( std::move(other), other.ptr() ) {}
    
    ref_ptr(T* p, ref_ptr_never_destroy nd) : ref_ptr_void(p, nd)  { INIT_DEBUG_PTR; }

	~ref_ptr() { DelRef(); }
protected:
    void copyFromTyped(const ref_ptr_void& other, T * ptr)
    {
        copyFrom(other, ptr);
    }
    void moveFromTyped(ref_ptr_void&& other, T * ptr)
    {
        moveFrom( std::move(other), ptr );
    }

public:
	ref_ptr& AddRef() { ref_ptr_void::AddRef(); return *this; }
	void DelRef() { void * p = DelRefInternal(); if (p) DestructionMethod::Destroy((T*)p); }
    
	T* detach() { return (T*)DelRefInternal(); }
    void release() { DelRef(); }
public:
    ref_ptr& operator=(const ref_ptr& other)
    {
        if (&other != this)
        {
            DelRef();
            copyFrom(other);
            INIT_DEBUG_PTR;
        }
        return *this;
    }
    ref_ptr& operator=(ref_ptr&& other)
    {
        if (&other != this)
        {
            DelRef();
            moveFrom(std::move(other));
            INIT_DEBUG_PTR;
        }
        return *this;
    }
	template<class U> ref_ptr& operator=(const ref_ptr<U,DestructionMethod> & other)
	{
        DelRef();
        copyFromTyped(other, other.ptr());
        INIT_DEBUG_PTR;
		return *this;
	}
    template<class U> ref_ptr& operator=(ref_ptr<U,DestructionMethod>&& other)
    {
        DelRef();
        U* ptr = other.ptr();
        moveFromTyped(std::move(other), ptr);
        INIT_DEBUG_PTR;
        return *this;
    }
    
    inline T* ptr() const { return (T*)m_p; }
	operator T*() const {return (T*)m_p;}
	T* operator->() const {return (T*)m_p;}
	T& operator*() const { return *(T*)m_p;}
    
    operator ref_ptr<const T>() {return ref_ptr<const T>( (const T*)m_p, *this);}
    friend class ref_ptr< typename std::remove_const<T>::type, DestructionMethod >;
};

/// ref_ptr to array of data
/// @brief destroys data using delete[] operator
///        allows indexint with operator[]
template<class T> class ref_ptr_arr : public ref_ptr< T, ref_ptr_destruction_method_delete_array >
{
    typedef ref_ptr< T, ref_ptr_destruction_method_delete_array > TBase;
public:
    ref_ptr_arr() : TBase() {}
    ref_ptr_arr(T* p) : TBase(p)  {}
    ref_ptr_arr(const ref_ptr_arr& other) : TBase(other) {}
    ref_ptr_arr(ref_ptr_arr&& other) : TBase(other)  {}
public:
    inline const T& operator[](int i) const { return TBase::ptr()[i];}
    inline T& operator[](int i) { return TBase::ptr()[i]; }
    
    ref_ptr_arr& operator=(const ref_ptr_arr& other) = default;
    
    template<class U> ref_ptr_arr& operator=(const ref_ptr_arr<U>& other)
    {
        TBase::operator=(other);
        return *this;
    }

};

template<class T>
inline ref_ptr<T> ref_ptr_no_destroy(T* t)
{
    return ref_ptr<T>(t, ref_ptr_never_destroy{} );
}

template<class T>
inline ref_ptr<T> make_ref_ptr(T* t)
{
    return ref_ptr<T>(t );
}

template<class T>
inline ref_ptr<T> make_ref_ptr(ref_ptr<T> other)
{
    return other;
}

template<class T>
class ref_ptr_static_cast : public ref_ptr<T>
{
public:
	template<class U> ref_ptr_static_cast(const ref_ptr<U>& p) : ref_ptr<T>( (ref_ptr_void&)p, static_cast<T*>(p.ptr()) ) {};
    template<class U> ref_ptr_static_cast(ref_ptr<U>&& p) : ref_ptr<T>( (ref_ptr_void&&)p, static_cast<T*>(p.ptr()) ) {};
};

template<class T>
class ref_ptr_dynamic_cast : public ref_ptr<T>
{
public:
	template<class U> ref_ptr_dynamic_cast(const ref_ptr<U>& p) : ref_ptr<T>( (ref_ptr_void&)p, dynamic_cast<T*>(p.ptr()) ) {};
    template<class U> ref_ptr_dynamic_cast(ref_ptr<U>&& p) : ref_ptr<T>( (ref_ptr_void&&)p, dynamic_cast<T*>(p.ptr()) ) {};
};

template<class T>
class ref_ptr_malloc : public ref_ptr<T, destruction_method_free >
{
public:
    ref_ptr_malloc(T*ptr) : ref_ptr<T, destruction_method_free>( (T*)ptr) {};
	ref_ptr_malloc(size_t n) : ref_ptr<T, destruction_method_free>( (T*)malloc(sizeof(T) * n) ) {};
};

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
class TypedData : public AbstractData
{
    T m_data;
public:
    TypedData(const T& data) : m_data(data) {}
    TypedData(T&& data) : m_data( std::move(data) ) {}
};

template<class T>
ref_ptr<AbstractData> abstractDataHolder(const T& data)
{
    return make_ref_ptr(new TypedData<T>(data));
}

template<class T>
ref_ptr<AbstractData> abstractDataHolder(T&& data)
{
    return make_ref_ptr(new TypedData<T>( std::move(data) ));
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

//! For each i in [b,e) does *out++ = a[i]
template<class Array, class ReindexIt, class OutI>
void ReindexArray(Array a, ReindexIt b, ReindexIt e, OutI out)
{
	for(;b!=e; ++b) *out++ = a[*b];
}

template<class T>
inline T* add_stride(T* ptr, int stride)
{
	return reinterpret_cast<T*>( reinterpret_cast<char*>(ptr)+stride );
}

template<class T, class U> class Same { public:enum {is=0}; };
template<class T> class Same<T,T> { public:enum {is=1}; };

#define ARE_SAME_TYPES(a,b) (Same<a,b>::is)

/**
 FlagSet is a set of flags based on custom enum type
 Values of enum type should be power of 2 (1,2,4,8, etc)
 */
template<class E> class FlagSet
{
    typedef typename std::underlying_type<E>::type int_type;
    int_type m_set;
public:
    FlagSet() : m_set(0) {}
    explicit FlagSet(int_type t) : m_set(t) {}
    FlagSet(std::initializer_list<E> list) : m_set(0) { for(E e : list) m_set |= e; }
    FlagSet(E e) : m_set(e) {}
    
    friend FlagSet operator&(FlagSet s, E e) { return FlagSet(s.m_set&e); }
    friend FlagSet operator&(E e, FlagSet s) { return FlagSet(e&s.m_set); }

    friend FlagSet operator|(FlagSet s, E e) { return FlagSet(s.m_set|e); }
    friend FlagSet operator|(E e, FlagSet s) { return FlagSet(e|s.m_set); }

    friend FlagSet operator&(FlagSet s1, FlagSet s2) { return FlagSet(s1.m_set&s2.m_set); }
    friend FlagSet operator|(FlagSet s1, FlagSet s2) { return FlagSet(s1.m_set|s2.m_set); }
    
    friend FlagSet operator~(FlagSet s1) { return FlagSet(~s1.m_set); }
    
    FlagSet& operator&=(FlagSet s) { m_set &= s.m_set; return *this; }
    FlagSet& operator|=(FlagSet s) { m_set |= s.m_set; return *this; }
    
    operator int_type() { return m_set; }
    
    FlagSet& set(E e) { m_set|=e; return *this; }
    FlagSet& unset(E e) { m_set &= ~e; return *this; }
    FlagSet pop(E e) { FlagSet res = FlagSet{m_set&e}; m_set &= ~e; return res; }
};

template<class E> FlagSet<E> flagSet(E e) { return FlagSet<E>(e); }
template<class E> FlagSet<E> flagSet(std::initializer_list<E> el) { return FlagSet<E>(el); }

#endif //__STLUTILINCLUDED__
