// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2023 André Caldas <andre.em.caldas@gmail.com>            *
 *                                                                          *
 *   This file is part of FreeCAD.                                          *
 *                                                                          *
 *   FreeCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FreeCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with FreeCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ***************************************************************************/

#ifndef BASE_Threads_ThreadSafeMultiIndex_H
#define BASE_Threads_ThreadSafeMultiIndex_H

#include <utility>
#include <type_traits>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/mem_fun.hpp>

#include "AtomicSharedPtr.h"
#include "ThreadSafeContainer.h"

namespace App{class TransactionObject; class TransactionalObject;}
namespace Base::Threads
{

namespace bmi = boost::multi_index;

template<typename X, typename Y, typename ...Vn>
class BmiElement
{
public:
    BmiElement(X&& x, Y&& y, Vn&&... vn)
        : tuple(std::forward(x), std::forward(y), std::forward(vn)...) {}

    template<int I>
    auto& get() const
    {return std::get<I, X, Y, Vn...>(tuple);}

    template<int I>
    auto& get_get() const
    {return get<I>().get();}

    operator const std::tuple<X, Y, Vn...>&() {return tuple;}

    const X& first = get<0>();
    const Y& second = get<1>();

private:
    const std::tuple<X, Y, Vn...> tuple;
};


template<typename Element, int I, typename SmartPtr>
struct SmartPtrExtractor
{
    using element_type = typename std::pointer_traits<SmartPtr>::element_type;

    element_type* operator()(const Element& e) const {return e.template get<I>().get();}
    element_type* operator()(const SmartPtr& p) const {return p.get();}
    element_type* operator()(element_type* p) const {return p;}
};

template<typename Element, int I, typename SmartPtr>
struct AtomicPtrExtractor
{
    using element_type = typename std::pointer_traits<SmartPtr>::element_type;

    element_type* operator()(const Element& e) const {return e.template get<I>().threadUnsafePointer();}
    element_type* operator()(const SmartPtr& p) const {return p.threadUnsafePointer();}
    element_type* operator()(const std::shared_ptr<element_type>& p) const {return p.get();}
    element_type* operator()(element_type* p) const {return p;}
};


template<typename Element, int I, typename V, typename T = V>
struct key_info
{
    using extractor = bmi::const_mem_fun<Element, const T&, &Element::template get<I>>;
};
template<typename Element, int I, typename T>
struct key_info<Element, I, std::shared_ptr<T>, T>
{
    using extractor = SmartPtrExtractor<Element, I, std::shared_ptr<T>>;
};
template<typename Element, int I, typename T>
struct key_info<Element, I, std::unique_ptr<T>, T>
{
    using extractor = SmartPtrExtractor<Element, I, std::unique_ptr<T>>;
};
template<typename Element, int I, typename T>
struct key_info<Element, I, AtomicSharedPtr<T>, T>
{
    using extractor = AtomicPtrExtractor<Element, I, AtomicSharedPtr<T>>;
};


// Just for static asserts.
struct BmiHashBase {};

template<class Element, int I, typename V>
struct BmiHash : BmiHashBase
{
    static_assert(!std::is_base_of_v<BmiHashBase, V>, "V should not have been processed.");
    using key_extractor = typename key_info<Element, I, V>::extractor;
    using type = bmi::hashed_unique<key_extractor>;
};

template<typename ...Hashes>
struct BmiIndexByAux
{
    using type = typename bmi::indexed_by<
        bmi::sequenced<>,
        typename Hashes::type...
    >;
};

template<class Element, int RevCount, int Max, typename V, typename ...HashOrVs>
struct BmiIndexBy
    : BmiIndexBy<Element, RevCount-1, Max, HashOrVs..., BmiHash<Element, Max-RevCount, V>>
{
    // V was not processed, yet.
    static_assert(!std::is_base_of_v<BmiHashBase, V>, "V should not have been processed.");
};

template<class Element, int Max, typename Hash, typename ...Hashes>
struct BmiIndexBy<Element, 0, Max, Hash, Hashes...>
{
    static_assert(Max == 1 + sizeof...(Hashes), "Not all types were converted!?");
    static_assert((std::is_base_of_v<BmiHashBase, Hash>), "All types must be an entry.");
    static_assert((std::is_base_of_v<BmiHashBase, Hashes> &&...), "All types must be an entry.");
    typedef typename BmiIndexByAux<Hash, Hashes...>::type type;
};

template<typename ...Vn>
struct BmiContainerTypeAux
{
    using element = BmiElement<Vn...>;
    using type = typename bmi::multi_index_container<
        element,
        typename BmiIndexBy<element, sizeof...(Vn), sizeof...(Vn), Vn...>::type
    >;
};

template<typename ...Vn>
using BmiContainerType = typename BmiContainerTypeAux<Vn...>::type;

/**
 * @brief This wraps a boost::multi_index_container and provides a shared_mutex
 * to make the container readable by many only when the map structure
 * is not being modified.
 * The container elements cannot change.
 *
 * If some element type is repeated in two entries, you need to specify
 * which entry you want:
 * bmi_cont.find<0>(key);
 * bmi_cont.find<1>(key);
 */
template<typename ...Vn>
class ThreadSafeMultiIndex
    : public ThreadSafeContainer<BmiContainerType<Vn...>>
{
public:
    template<int I>
    auto& get() const
    {return container.template get<I>();}

    template<typename Key>
    auto& get() const
    {return container.template get<Key>();}


    template<int I, typename Key>
    auto find(const Key& key) const
    {return LockedIterator<nth_iterator<I>>(mutex, get<I>().find(key));}

    template<typename Key>
    auto find(const Key& key) const
    {return LockedIterator<iterator<Key>>(mutex, get<Key>().find(key));}


    template<int I, typename Key>
    size_t count(const Key& key) const
    {SharedLock l(mutex); return get<I>().count(key);}

    template<typename Key>
    size_t count(const Key& key) const
    {SharedLock l(mutex); return container.template get<Key>().count(key);}

    bool empty() const {return container.empty();}
    void clear() {container.clear();}

private:
    using ThreadSafeContainer<BmiContainerType<Vn...>>::mutex;
    using ThreadSafeContainer<BmiContainerType<Vn...>>::container;
    template<int I>
    using nth_iterator = typename BmiContainerType<Vn...>::nth_index<I>::type::iterator;
    template<typename Key>
    using iterator = typename BmiContainerType<Vn...>::index<Key>::iterator;
    template<int I>
    using nth_const_iterator = typename BmiContainerType<Vn...>::nth_index<I>::const_iterator;
    template<typename Key>
    using const_iterator = typename BmiContainerType<Vn...>::index<Key>::const_iterator;
};

} //namespace Base::Threads

#endif // BASE_Threads_ThreadSafeMultiIndex_H
