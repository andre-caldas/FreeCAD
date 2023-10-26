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

#ifndef BASE_Threads_MultiIndexContainer_H
#define BASE_Threads_MultiIndexContainer_H

#include "type_traits/MultiIndexRecordInfo.h"
#include "IteratorWrapper.h"

namespace Base::Threads
{

template<typename Record, auto ...LocalPointers>
class MultiIndexContainer
{
public:
    using self_t = MultiIndexContainer;
    using map_iterator_t = typename std::map<double, Record*>::iterator;
    using map_const_iterator_t = typename std::map<double, Record*>::const_iterator;
    using iterator = IteratorSecondPtrAsRef<self_t, map_iterator_t>;
    using const_iterator = IteratorSecondPtrAsRef<self_t, map_const_iterator_t>;

    auto begin();
    auto begin() const;
    auto cbegin() const;

    auto end();
    auto end() const;
    auto cend() const;

    auto size() const;
    bool empty() const;
    void clear();

    template<typename Key>
    auto find(const Key& key);

    template<std::size_t I, typename Key>
    auto find(const Key& key);

    template<typename Key>
    auto find(const Key& key) const;

    template<std::size_t I, typename Key>
    auto find(const Key& key) const;

    template<std::size_t I, typename Key>
    bool contains(const Key& key) const;

    template<typename Key>
    bool contains(const Key& key) const;

    template<typename ...Vn>
    auto emplace(Vn&& ...vn);

    auto erase(const Record& record);

    template<typename ItType>
    auto erase(ItType it);

    auto extract(const Record& record);

    template<typename ItType>
    auto extract(ItType it);

    auto move_back(const Record& record);

    template<typename ItType>
    auto move_back(const ItType& it);


    using RecordInfo = MultiIndexRecordInfo<Record, LocalPointers...>;

    using record_t = Record;
    template<typename X>
    static constexpr auto index_from_raw_v = RecordInfo::template index_from_raw_v<X>;
    template<std::size_t I>
    using raw_from_index_t = typename RecordInfo::template raw_from_index_t<I>;
    template<std::size_t I>
    using type_from_index_t = typename RecordInfo::template type_from_index_t<I>;

private:
    std::unordered_map<const record_t*, std::unique_ptr<record_t>> data;

    /// @brief Incremented on every insertion.
    std::atomic_int counter;

    /**
     * @brief Items oredered (in principle) by insetion order.
     * @attention We use double instead of an integer so it becomes easy
     * to insert a record between two existing records.
     */
    std::map<double, record_t*> ordered_data;

    /**
     * @brief Reverse of ordered_data.
     * It is used to get the key for "ordered_data".
     */
    std::map<const record_t*, double> ordered_data_reverse;


    /**
     * @brief The Record struct show us what indexes can be used to search the list.
     * Each tuple entry is an std::map<raw_key, Record&>.
     */
    typename RecordInfo::indexes_tuple_t indexes;

    template<std::size_t ...In>
    void insertIndexes(const record_t& record, const std::index_sequence<In...>&);

    template<std::size_t I>
    void insertIndex(const record_t& record);


    template<std::size_t ...In>
    void eraseIndexes(const record_t& record, const std::index_sequence<In...>&);

    template<std::size_t I>
    void eraseIndex(const record_t& record);


    template<std::size_t ...In>
    void clearIndexes(const std::index_sequence<In...>&);

    template<std::size_t I>
    void clearIndex();

    template<typename iterator, std::size_t I, typename Key>
    auto _find(const Key& key);
};

} //namespace ::Threads

#include "MultiIndexContainer.inl"

#endif // BASE_Threads_MultiIndexContainer_H
