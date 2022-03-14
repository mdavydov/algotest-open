//
//  MemberIterator.hpp
//  ImageBlending_Mac
//
//  Created by Maksym Hrytsai on 05.07.2021.
//  Copyright © 2021 AdvaSoft. All rights reserved.
//

#ifndef member_iterator_h
#define member_iterator_h

#include <algorithm>
#include <iostream>
#include <limits>
#include <iterator>
#include <vector>

namespace memit
{
    template<typename ValueType, typename PointerToMemberType, bool IsMemberFunctionPointer = std::is_member_function_pointer<PointerToMemberType>::value>
    struct result_of_pointer_to_member_dereference
    {
        using type = decltype(std::declval<ValueType>().*std::declval<PointerToMemberType>());
    };

    template<typename ValueType, typename PointerToMemberType>
    struct result_of_pointer_to_member_dereference<ValueType, PointerToMemberType, true>
    {
        using type = decltype((std::declval<ValueType>().*std::declval<PointerToMemberType>())());
    };

    template<typename... ArgTypes>
    using result_of_pointer_to_member_dereference_t = typename result_of_pointer_to_member_dereference<ArgTypes...>::type;

    template<typename Iterator, typename PointerToMemberType, bool = std::is_member_function_pointer<PointerToMemberType>::value>
    struct dereference_pointer_to_member_helper
    {
        static decltype(auto) dereference(Iterator iterator, PointerToMemberType ptm)
        {
            return *iterator.*ptm;
        }
    };
    template<typename Iterator, typename PointerToMemberType>
    struct dereference_pointer_to_member_helper<Iterator, PointerToMemberType, true>
    {
        static decltype(auto) dereference(Iterator iterator, PointerToMemberType ptm)
        {
            return (*iterator.*ptm)();
        }
    };

    template<typename Iterator, typename PointerToMemberType>
    class member_iterator
    {
        using current_iterator_type = Iterator;
        using current_value_type = typename std::iterator_traits<Iterator>::value_type;

    public:
        using iterator_category = typename current_iterator_type::iterator_category;
        using iterator_value_type = result_of_pointer_to_member_dereference_t<current_value_type, PointerToMemberType>;
        using value_type = std::remove_reference_t<iterator_value_type>;
        using difference_type = typename current_iterator_type::difference_type;
        using pointer = std::add_pointer_t<value_type>;
        using reference = std::add_lvalue_reference_t<value_type>;

        member_iterator(current_iterator_type it, PointerToMemberType ptm)
          : it(it)
          , pointer_to_member(ptm)
        {}

        decltype(auto) operator*()
        {
            return dereference_pointer_to_member_helper<current_iterator_type, PointerToMemberType>::dereference(it, pointer_to_member);
        }

        member_iterator& operator++()
        {
            ++it;
            return *this;
        }

        member_iterator operator++(int)
        {
            return {++it, pointer_to_member};
        }

        member_iterator operator+(int increment)
        {
            return {it + increment, pointer_to_member};
        }

        member_iterator& operator+=(int increment)
        {
            it += increment;
            return *this;
        }

        member_iterator& operator--()
        {
            --it;
            return *this;
        }

        member_iterator operator--(int)
        {
            return {--it, pointer_to_member};
        }

        member_iterator operator-(int decrement)
        {
            return {it - decrement, pointer_to_member};
        }

        member_iterator& operator-=(int decrement)
        {
            it -= decrement;
            return *this;
        }

        difference_type operator-(const member_iterator& other)
        {
            return it - other.it;
        }

        bool operator!=(const member_iterator& other) const
        {
            return !(*this == other);
        }
        bool operator==(const member_iterator& other) const
        {
            return pointer_to_member == other.pointer_to_member && it == other.it;
        }
        bool operator<(const member_iterator& other) const
        {
            return it < other.it;
        }
        bool operator>(const member_iterator& other) const
        {
            return it > other.it;
        }


        bool operator>=(const member_iterator& other) const
        {
            return it >= other.it;
        }
        
        bool operator<=(const member_iterator& other) const
        {
            return it <= other.it;
        }


        void swap(member_iterator& other)
        {
            std::swap(it, other.it);
            std::swap(pointer_to_member, other.pointer_to_member);
        }

        const current_iterator_type& underlying_iterator() const
        {
            return it;
        }

        current_iterator_type& underlying_iterator()
        {
            return it;
        }

    private:
        current_iterator_type it;
        PointerToMemberType pointer_to_member;
    };

    //-- Helpers that are obsoleted by C++17 constructor template argument deduction --
    template<typename Iterator, typename PointerToMember>
    decltype(auto) create_member_iterator(Iterator it, PointerToMember pointer_to_member )
    {
        return member_iterator<Iterator, PointerToMember>(it, pointer_to_member);
    }

    template<typename Container, typename PointerToMember>
    decltype(auto) begin(Container& container, PointerToMember pointer_to_member)
    {
        return create_member_iterator(begin(container), pointer_to_member);
    }

    template<typename Container, typename PointerToMember>
    decltype(auto) end(Container& container, PointerToMember pointer_to_member )
    {
        return create_member_iterator(end(container), pointer_to_member);
    }
}

#endif /* member_iterator_h */
