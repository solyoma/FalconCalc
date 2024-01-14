#pragma once


namespace NLIBNS
{


//
// sparse_list
//
// Template class for storing values in a sparse vector where most indexes are not set.
// The vector data is broken up to blocks, where each block starts with a continuous array of data
// and a continuous empty part. The 'pos' list holds information about which index positions are
// set. Each value in 'pos' specifies a block with the number of used and the number of empty index
// positions.
// The values are stored in the 'data' vector. The data in the vector elements are in order of their
// index. Looking up a value at a given index is done by first finding the block for the index in
// 'pos', with the number of elements in 'data' that has to be skipped before the first element in
// the block is found, and the real index after those elements.
template<typename _Value, class _Alloc> class sparse_list;
template<typename _SparseIter> class sparse_data_iterator;

// Iterators for sparse_list
template<typename _List, typename _Ptr, typename _Ref>
class sparse_list_iterator : public std::iterator<std::random_access_iterator_tag, typename _List::value_type, typename _List::difference_type, _Ptr, _Ref>
{
public:
    typedef _Ref                                        reference;
    typedef _Ptr                                        pointer;
    typedef typename _List::size_type                   size_type;
    typedef typename _List::difference_type             difference_type;
    typedef typename _List::value_type                  value_type;
    typedef typename _List::allocator_type              allocator_type;

    typedef _List                                       list_type;
protected:
    typedef sparse_list_iterator<_List, _Ptr, _Ref>     self_type;
    typedef typename _List::sparse_data                 sparse_data;
#ifdef DEBUG
    typedef typename _List::iter_checker_base           iter_checker_base;
#endif

    sparse_data *storage;
    size_type index;

#ifdef DEBUG
    struct iter_checker : public iter_checker_base
    {
        self_type *owner;
        iter_checker(self_type *owner) : owner(owner) {}
        virtual void InvalidateAll()
        {
            if (this->next)
                this->next->InvalidateAll();
            owner->Invalidate();
        }

        virtual void InvalidateFrom(size_type from)
        {
            if (this->next)
                this->next->InvalidateFrom(from);
            owner->InvalidateMeFrom(from);
        }
    };

    iter_checker checker;

    void InsertChecker()
    {
        if (storage == NULL)
            return;
        checker.next = storage->first;
        storage->first = &checker;
    }

    void Invalidate()
    {
        storage = NULL;
        checker.next = NULL;
    }

    void InvalidateMeFrom(size_type from)
    {
        if (index < from)
            return;
        SingleInvalidate();
    }

    void SingleInvalidate()
    {
        if (!storage)
            return;

        iter_checker_base *it = storage->first;
        if (it == &checker)
        {
            storage->first = checker.next;
            Invalidate();
            return;
        }

        while (it->next != &checker)
            it = it->next;
        it->next = checker.next;
        Invalidate();
    }

    void CheckSameOwner(sparse_data *other) const
    {
        if (other != storage)
            throw L"Invalid operation on sparse list iterators with different owner.";
    }

    void CheckHasOwner() const
    {
        if (!storage)
            throw L"The sparse list iterator has been invalidated.";
    }

    sparse_list_iterator(sparse_data *storage, size_type index) : storage(storage), index(index), checker(this)
    {
        InsertChecker();
    }
#else
    sparse_list_iterator(sparse_data *storage, size_type index) : storage(storage), index(index)
    {
    }
#endif

    void IndexChange(difference_type diff)
    {
        if (diff == 0)
            return;
        size_type lsize = _List::sparse_size(storage);
        size_type ix = index == _List::max_size(storage) ? lsize : index;
#ifdef DEBUG
        if ((diff < 0 && (size_type)-diff > ix) || (diff > 0 && lsize - ix < (size_type)diff))
            throw L"Iterator index out of range.";
#endif
        index = ix + diff == lsize ? _List::max_size(storage) : ix + diff;
    }

    friend class sparse_list<value_type, allocator_type>;
    friend class sparse_data_iterator<self_type>;
public:
#ifdef DEBUG
    sparse_list_iterator() : storage(NULL), index(0), checker(this)
    {
    }

    sparse_list_iterator(const self_type &other) : storage(NULL), index(0), checker(this)
    {
        *this = other;
    }

    template<typename _Ptr2, typename _Ref2>
    sparse_list_iterator(const sparse_list_iterator<_List, _Ptr2, _Ref2> &other) : storage(NULL), index(0), checker(this)
    {
        *this = other;
    }

    ~sparse_list_iterator()
    {
        SingleInvalidate();
    }
#else
    sparse_list_iterator() : storage(NULL), index(0)
    {
    }

    template<typename _Ptr2, typename _Ref2>
    sparse_list_iterator(const sparse_list_iterator<_List, _Ptr2, _Ref2> &other) : storage(NULL), index(0)
    {
        *this = other;
    }

    sparse_list_iterator(const self_type &other) : storage(NULL), index(0)
    {
        *this = other;
    }
#endif

    self_type& operator=(const self_type &other)
    {
#ifdef DEBUG
        if (storage != other.storage)
        {
            SingleInvalidate();
            storage = other.storage;
            InsertChecker();
        }
#else
        storage = other.storage;
#endif
        index = other.index;
        return *this;
    }

    template<typename _Ptr2, typename _Ref2>
    self_type& operator=(const sparse_list_iterator<_List, _Ptr2, _Ref2> &other)
    {
#ifdef DEBUG
        if (storage != other._get_storage())
        {
            SingleInvalidate();
            storage = other._get_storage();
            InsertChecker();

        }
#else
        storage = other._get_storage();
#endif
        index = other._get_index();
        return *this;
    }

    bool operator==(const self_type &other) const
    {
        return storage == other.storage && index == other.index;
    }

    bool operator!=(const self_type &other) const
    {
        return storage != other.storage || index != other.index;
    }

    template<typename _Ptr2, typename _Ref2>
    bool operator==(const sparse_list_iterator<_List, _Ptr2, _Ref2> &other) const
    {
        return storage == other._get_storage() && index == other._get_index();
    }

    template<typename _Ptr2, typename _Ref2>
    bool operator!=(const sparse_list_iterator<_List, _Ptr2, _Ref2> &other) const
    {
        return storage != other._get_storage() || index != other._get_index();
    }

    reference operator*() const
    {
#ifdef DEBUG
        CheckHasOwner();
#endif
        return _List::item_at(storage, index);
    }

    pointer operator->() const
    {
#ifdef DEBUG
        CheckHasOwner();
#endif
        return &_List::item_at(storage, index);
    }

    self_type& operator++()
    {
#ifdef DEBUG
        CheckHasOwner();
#endif
        IndexChange(1);
        return *this;
    }

    self_type operator++(int)
    {
        self_type orig = *this;
        ++*this;
        return orig;
    }

    self_type& operator--()
    {
#ifdef DEBUG
        CheckHasOwner();
#endif
        IndexChange(-1);
        return *this;
    }

    self_type operator--(int)
    {
        self_type orig = *this;
        --*this;
        return orig;
    }

    self_type& operator+=(difference_type offs)
    {
#ifdef DEBUG
        CheckHasOwner();
#endif
        IndexChange(offs);
        return *this;
    }

    self_type operator+(difference_type offs) const
    {
#ifdef DEBUG
        CheckHasOwner();
#endif
        self_type other = *this;
        other.IndexChange(offs);
        return other;
    }

    self_type& operator-=(difference_type offs)
    {
#ifdef DEBUG
        CheckHasOwner();
#endif
        IndexChange(-offs);
        return *this;
    }

    self_type operator-(difference_type offs) const
    {
#ifdef DEBUG
        CheckHasOwner();
#endif
        self_type other = *this;
        other.IndexChange(-offs);
        return other;
    }

    difference_type operator-(const self_type &other) const
    {
#ifdef DEBUG
        CheckSameOwner(other.storage);
        CheckHasOwner();
#endif

        size_type maxsize = _List::max_size(storage);
        if (index == maxsize && other.index == maxsize)
            return 0;
        if (index == maxsize)
            return _List::sparse_size(storage) - other.index;
        if (other.index == maxsize)
            return other.index - _List::sparse_size(storage);
        return index - other.index;
    }

    self_type operator[](difference_type offs) const
    {
#ifdef DEBUG
        CheckHasOwner();
#endif
        self_type other = *this;
        other.IndexChange(offs);
        return other;
    }

    bool operator<(const self_type &other) const
    {
#ifdef DEBUG
        CheckSameOwner(other.storage);
        CheckHasOwner();
#endif
        size_type maxsize = _List::max_size(storage);
        if (index == maxsize && other.index == maxsize)
            return false;
        if (index == maxsize)
            return false;
        if (other.index == maxsize)
            return true;
        return index < other.index;
    }

    bool operator<=(const self_type &other) const
    {
#ifdef DEBUG
        CheckSameOwner(other.storage);
        CheckHasOwner();
#endif
        size_type maxsize = _List::max_size(storage);
        if (index == maxsize && other.index == maxsize)
            return true;
        if (index == maxsize)
            return false;
        if (other.index == maxsize)
            return true;
        return index <= other.index;
    }

    bool operator>(const self_type &other) const
    {
#ifdef DEBUG
        CheckSameOwner(other.storage);
        CheckHasOwner();
#endif
        size_type maxsize = _List::max_size(storage);
        if (index == maxsize && other.index == maxsize)
            return false;
        if (index == maxsize)
            return true;
        if (other.index == maxsize)
            return false;
        return index > other.index;
    }

    bool operator>=(const self_type &other) const
    {
#ifdef DEBUG
        CheckSameOwner(other.storage);
        CheckHasOwner();
#endif
        size_type maxsize = _List::max_size(storage);
        if (index == maxsize && other.index == maxsize)
            return true;
        if (index == maxsize)
            return true;
        if (other.index == maxsize)
            return false;
        return index >= other.index;
    }

    sparse_data* _get_storage() const
    {
        return storage;
    }

    size_type _get_index() const
    {
        return index;
    }
};

template<typename _SpList, typename _Ptr, typename _Ref>
inline sparse_list_iterator<_SpList, _Ptr, _Ref>& operator+(typename _SpList::difference_type offs, sparse_list_iterator<_SpList, _Ptr, _Ref> &other)
{
    return (other += offs);
}

template<typename _SparseIter>
class sparse_data_iterator : public std::iterator<std::random_access_iterator_tag, typename _SparseIter::value_type, typename _SparseIter::difference_type, typename _SparseIter::pointer, typename _SparseIter::reference>
{

public:
    typedef _SparseIter                             iterator_type;
    typedef typename iterator_type::pointer         pointer;
    typedef typename iterator_type::reference       reference;
    typedef typename iterator_type::size_type       size_type;
    typedef typename iterator_type::difference_type difference_type;
    typedef typename iterator_type::value_type      value_type;
    typedef typename iterator_type::allocator_type  allocator_type;
private:
    typedef sparse_data_iterator<_SparseIter>   self_type;
    typedef typename iterator_type::list_type   list_type;

    iterator_type iter;

#ifdef DEBUG
    typedef typename list_type::sparse_iter_checker_base    sparse_iter_checker_base;

    struct iter_checker : public sparse_iter_checker_base
    {
        self_type *owner;
        iter_checker(self_type *owner) : owner(owner) {}

        virtual void InvalidateFrom(size_type from)
        {
            if (this->next)
                this->next->InvalidateFrom(from);
            owner->InvalidateMeFrom(from);
        }

        virtual void InvalidateRange(size_type from, size_type cnt)
        {
            if (this->next)
                this->next->InvalidateRange(from, cnt);
            owner->InvalidateMeBetween(from, cnt);
        }
    };

    iter_checker checker;

    void InsertChecker()
    {
        if (iter.storage == NULL)
            return;
        checker.next = iter.storage->sfirst;
        iter.storage->sfirst = &checker;
    }

    void Invalidate()
    {
        iter.SingleInvalidate();
        checker.next = NULL;
    }

    void InvalidateMeBetween(size_type from, size_type cnt)
    {
        if (iter.index < from || iter.index >= from + cnt)
            return;
        SingleInvalidate();
    }

    void InvalidateMeFrom(size_type from)
    {
        if (iter.index < from)
            return;
        SingleInvalidate();
    }

    void SingleInvalidate()
    {
        if (!iter.storage)
            return;

        sparse_iter_checker_base *it = iter.storage->sfirst;
        if (it == &checker)
        {
            iter.storage->sfirst = checker.next;
            Invalidate();
            return;
        }

        while (it->next != &checker)
            it = it->next;
        it->next = checker.next;
        Invalidate();
    }

#endif

    void IndexChange(difference_type offs)
    {
        if (offs == 0)
            return;
        size_type datapos, virtualpos;
        auto it = iterator_type::list_type::index_pos(iter.storage, iter.index, datapos, virtualpos);
        size_type blockpos = virtualpos == iter.storage->vsize ? 0 : iter.index - virtualpos;
        if (offs > 0)
        {
            size_type diff = (size_type)-(difference_type)blockpos;
            blockpos += offs;
            while (it != iter.storage->pos.end() && blockpos >= it->used)
            {
                blockpos -= it->used;
                diff += it->empty + it->used;
                ++it;
            }

            iter += diff + blockpos;
        }
        else if (offs < 0)
        {
            auto rit = make_reverse_iterator(it);
            size_type p = (size_type)-offs;
            size_type diff = blockpos;
            while (p > blockpos)
            {
                blockpos += rit->used;
                diff += rit->empty + rit->used;
                ++rit;
            }

            iter -= diff - (blockpos - p);
        }
    }

    // Dummy int argument to avoid automatic calls of the constructor for functions accepting both sparse_data_iterators and sparse_list_iterators.

#ifdef DEBUG
    template<typename _Ptr, typename _Ref>
    explicit sparse_data_iterator(const sparse_list_iterator<list_type, _Ptr, _Ref> &iter) : iter(iter), checker(this)
    {
        InsertChecker();
    }
#else
    template<typename _Ptr, typename _Ref>
    explicit sparse_data_iterator(const sparse_list_iterator<list_type, _Ptr, _Ref> &iter) : iter(iter)
    {
    }
#endif

    friend typename iterator_type::list_type;
public:
#ifdef DEBUG
    sparse_data_iterator() : checker(this)
    {
    }

    sparse_data_iterator(const self_type &other) : checker(this)
    {
        *this = other;
    }

    template<typename _Iter>
    sparse_data_iterator(const sparse_data_iterator<_Iter> &other) : checker(this)
    {
        *this = other;
    }

    sparse_data_iterator& operator=(const self_type &other)
    {
        if (iter.storage != other.iter.storage)
        {
            SingleInvalidate();
            iter = other.iter;
            InsertChecker();
        }
        else
            iter = other.iter;
        return *this;
    }

    ~sparse_data_iterator()
    {
        SingleInvalidate();
    }
#else
    sparse_data_iterator()
    {
    }

    sparse_data_iterator(const self_type &other)
    {
        *this = other;
    }

    template<typename _Iter>
    sparse_data_iterator(const sparse_data_iterator<_Iter> &other)
    {
        *this = other;
    }

    sparse_data_iterator& operator=(const self_type &other)
    {
        iter = other.iter;
        return *this;
    }
#endif

    iterator_type base() const
    {
        return iter;
    }

    template<typename _OtherIter>
    sparse_data_iterator& operator=(const sparse_data_iterator<_OtherIter> &other)
    {
#ifdef DEBUG
        if (iter.storage != other.base()._get_storage())
        {
            SingleInvalidate();
            iter = other.base();
            InsertChecker();
        }
        else
#endif
        iter = other.base();

        return *this;
    }

    bool operator==(const self_type &other) const
    {
        return iter == other.iter;
    }

    bool operator!=(const self_type &other) const
    {
        return iter != other.base();
    }

    template<typename _OtherIter>
    bool operator==(const sparse_data_iterator<_OtherIter> &other) const
    {
        return iter == other.base();
    }

    template<typename _OtherIter>
    bool operator!=(const sparse_data_iterator<_OtherIter> &other) const
    {
        return iter != other.base();
    }

    reference operator*() const
    {
        return *iter;
    }

    pointer operator->() const
    {
        return iter.operator->();
    }

    self_type& operator++()
    {
        size_type datapos, virtualpos;
        auto it = iterator_type::list_type::index_pos(iter.storage, iter.index, datapos, virtualpos);
        size_type blockpos = iter.index - virtualpos;
        if (blockpos < it->used - 1)
            ++iter;
        else
            iter += it->used + it->empty - blockpos;
        return *this;
    }

    self_type operator++(int)
    {
        self_type orig = *this;
        ++*this;
        return orig;
    }

    self_type& operator--()
    {
        size_type datapos, virtualpos;
        auto it = iterator_type::list_type::index_pos(iter.storage, iter.index, datapos, virtualpos);
        size_type blockpos = virtualpos == iter.storage->vsize ? 0 : iter.index - virtualpos;
        if (blockpos > 0)
            --iter;
        else
        {
            --it;
            iter -= it->empty + 1;
        }
        return *this;
    }

    self_type operator--(int)
    {
        self_type orig = *this;
        --*this;
        return orig;
    }

    self_type& operator+=(difference_type offs)
    {
        IndexChange(offs);
        return *this;
    }

    self_type operator+(difference_type offs) const
    {
        self_type other = *this;
        return other += offs;
    }

    self_type& operator-=(difference_type offs)
    {
        IndexChange(-offs);
        return *this;
    }

    self_type operator-(difference_type offs) const
    {
        self_type other = *this;
        return other -= offs;
    }

    difference_type operator-(const self_type &other) const
    {
        size_type datapos, virtualpos;
        if (iter <= other.iter)
        {
            if (iter.index == other.iter.index)
                return 0;
            auto it = iterator_type::list_type::index_pos(other.iter.storage, other.iter.index, datapos, virtualpos);
            size_type blockpos = other.iter.index - virtualpos;
            while (virtualpos > iter.index)
            {
                --it;
                virtualpos -= it->used + it->empty;
                blockpos += it->used;
            }
            return blockpos - (iter.index - virtualpos);
        }
        else
        {
            auto it = iterator_type::list_type::index_pos(iter.storage, iter.index, datapos, virtualpos);
            size_type blockpos = virtualpos == iter.storage->vsize ? 0 : iter.index - virtualpos;
            while (virtualpos > other.iter.index)
            {
                --it;
                virtualpos -= it->used + it->empty;
                blockpos += it->used;
            }
            return blockpos - (other.iter.index - virtualpos);
        }
    }

    self_type operator[](difference_type offs) const
    {
        self_type other = *this;
        return other += offs;
    }

    bool operator<(const self_type &other) const
    {
        return iter < other.iter;
    }

    bool operator<=(const self_type &other) const
    {
        return iter <= other.iter;
    }

    bool operator>(const self_type &other) const
    {
        return iter > other.iter;
    }

    bool operator>=(const self_type &other) const
    {
        return iter >= other.iter;
    }

};

template<typename _SparseIter>
inline sparse_data_iterator<_SparseIter>& operator+(typename _SparseIter::difference_type offs, sparse_data_iterator<_SparseIter> &other)
{
    return (other += offs);
}

// The class sparse_list
template<typename _Value, class _Alloc = typename std::allocator<_Value>>
class sparse_list
{
public:
    typedef _Value                                      value_type;
    typedef _Alloc                                      allocator_type;
    typedef typename _Alloc::difference_type            difference_type;
    typedef typename _Alloc::size_type                  size_type;
private:
    typedef sparse_list<value_type, allocator_type>         self_type;
    typedef std::allocator_traits<_Alloc>                   _alloc_traits;
    typedef std::vector<value_type, allocator_type>         data_vector;
    typedef const std::vector<value_type, allocator_type>   const_data_vector;

    struct sparse_data;

    struct pos_elem
    {
        size_type used;
        size_type empty;

        pos_elem() : used(0), empty(0) {}
        pos_elem(const pos_elem& other) : used(other.used), empty(other.empty) {}
        pos_elem(size_type used, size_type empty) : used(used), empty(empty) {}
    };

    typedef typename allocator_type::template rebind<pos_elem>::other       pos_allocator_type;
    typedef std::list<pos_elem, pos_allocator_type>                         pos_list;
    typedef const std::list<pos_elem, pos_allocator_type>                   const_pos_list;
    typedef typename pos_list::iterator                                     pos_iterator;
    typedef typename pos_list::const_iterator                               const_pos_iterator;
public:
    typedef typename data_vector::iterator                  data_iterator;
    typedef typename data_vector::const_iterator            const_data_iterator;
    typedef typename data_vector::reverse_iterator          reverse_data_iterator;
    typedef typename data_vector::const_reverse_iterator    const_reverse_data_iterator;

    typedef typename _alloc_traits::pointer             pointer;
    typedef typename _alloc_traits::const_pointer       const_pointer;

    typedef typename data_vector::reference                                     reference;
    typedef typename data_vector::const_reference                               const_reference;
    typedef sparse_list_iterator<self_type, pointer, reference>                 iterator;
    typedef sparse_list_iterator<self_type, const_pointer, const_reference>     const_iterator;
    typedef std::reverse_iterator<iterator>                                     reverse_iterator;
    typedef std::reverse_iterator<const_iterator>                               const_reverse_iterator;
    typedef sparse_data_iterator<iterator>                                      sparse_iterator;
    typedef sparse_data_iterator<const_iterator>                                const_sparse_iterator;
    typedef std::reverse_iterator<sparse_data_iterator<iterator>>               reverse_sparse_iterator;
    typedef std::reverse_iterator<sparse_data_iterator<const_iterator>>         const_reverse_sparse_iterator;
private:
    friend class sparse_list_iterator<self_type, pointer, reference>;
    friend class sparse_list_iterator<self_type, const_pointer, const_reference>;
    friend class sparse_data_iterator<iterator>;
    friend class sparse_data_iterator<const_iterator>;

#ifdef DEBUG
    struct iter_checker_base
    {
        iter_checker_base *next;
        iter_checker_base() : next(NULL) {}
        virtual void InvalidateAll() = 0;
        virtual void InvalidateFrom(size_type from) = 0;
    };

    struct sparse_iter_checker_base
    {
        sparse_iter_checker_base *next;
        sparse_iter_checker_base() : next(NULL) {}
        virtual void InvalidateRange(size_type from, size_type cnt) = 0;
        virtual void InvalidateFrom(size_type from) = 0;
    };
#endif

    struct sparse_data;
    struct pos_index_cache
    {
        pos_iterator posbegin;
        pos_iterator posend;

        pos_iterator it;
        size_type datapos;
        size_type virtualpos;
        typename pos_list::size_type posindex;
    };

    struct sparse_data
    {
        value_type def; // Default value to return when getting constant value from unset index.
        pos_list pos; // List containing blocks to data and empty space.
        data_vector data; // Vector that stores all the data in the sparse_list.
        pos_iterator poslast; // Last element in the forward list for inserting.
        size_type vsize; // Virtual size of the vector holding the data.

        mutable pos_index_cache cache;

#ifdef DEBUG
        iter_checker_base *first;
        sparse_iter_checker_base *sfirst;

        void SparseInvalidateRange(size_type from, size_type count)
        {
            if (sfirst)
                sfirst->InvalidateRange(from, count);
        }

        void SparseInvalidateFrom(size_type from)
        {
            if (sfirst)
                sfirst->InvalidateFrom(from);
        }

        void InvalidateIterators()
        {
            SparseInvalidateFrom(0);
            if (first)
                first->InvalidateAll();
            first = NULL;
        }

        void InvalidateFrom(size_type from)
        {
            SparseInvalidateFrom(from);
            if (first)
                first->InvalidateFrom(from);
        }

        sparse_data() : first(NULL), sfirst(NULL)
        {
        }

        ~sparse_data()
        {
            InvalidateIterators();
        }
#endif
    };
    sparse_data *storage;

    static void ClearCache(sparse_data *storage)
    {
        storage->cache.it = storage->pos.end();
    }

#ifdef DEBUG
    static void CheckEnd(const pos_list &pos, const pos_iterator &it)
    {
        if (it == pos.end())
            throw L"Position out of range.";
    }
#endif

    static size_type max_size(const sparse_data *storage)
    {
        return min(storage->data.max_size(), storage->pos.max_size());
    }

    static size_type sparse_size(const sparse_data *storage)
    {
        return storage->vsize;
    }

    static bool is_set(const sparse_data *storage, size_type index)
    {
        if (storage->data.empty())
            return false;
        size_type datapos, virtualpos;
        auto it = index_pos(storage, index, datapos, virtualpos);
        return it != storage->pos.end() && (virtualpos + it->used > index);
    }

    static size_type last_set_pos(const sparse_data *storage, size_type index) // Returns the virtual index of the last position which has a set value in data up to index.
    {
        if (storage->data.empty())
            return max_size(storage);
        size_type datapos, virtualpos;
        auto it = index_pos(storage, index, datapos, virtualpos);
        if (it == storage->pos.end())
            return storage->vsize - (--it)->empty - 1;
        size_type blockpos = index - virtualpos;
        if (blockpos < it->used)
            return index;
        return index - (it->used - blockpos) - 1;
    }

    static bool CacheCloser(const sparse_data *storage, size_type index)
    {
        return ((storage->cache.virtualpos - index) / max(2, storage->cache.virtualpos * 3 / 4 / max(1, storage->cache.posindex))) < (storage->cache.posindex / 2);
    }

    static void CacheDec(const sparse_data *storage, unsigned int n = 1)
    {
        while (n--)
        {
            auto p = *--storage->cache.it;
            --storage->cache.posindex;
            storage->cache.datapos -= p.used;
            storage->cache.virtualpos -= p.used + p.empty;
        }
    }


    // Returns an interator to the block element in the 'pos' list which holds the array position of index.
    // The search for the position is starting from 'fpos', which must be a block that passed 'fdatapos'
    // data indexes and comes after 'fvirtualpos' virtual elements.
    // Updates 'datapos' to the index in 'data' where the 'index' points if it points to a real element, or its
    // position where to insert a new value at 'index'.
    // Sets 'virtualpos' to the number of elements that would come before the returned block, if all elements would be set.
    // Return type is (const_)pos_iterator. Constness depends on the constness of the passed sparse_data.
    template<typename SPARSE_DATA>
    static typename std::conditional<std::is_const<SPARSE_DATA>::value, const_pos_iterator, pos_iterator>::type
        index_pos(SPARSE_DATA *storage, size_type index, size_type &datapos, size_type &virtualpos)
    {
        if (index >= sparse_size(storage))
        {
            datapos = storage->data.size();
            virtualpos = sparse_size(storage);
            return storage->pos.end();
        }

        size_type tmp;

        if (storage->cache.it != storage->cache.posend && (storage->cache.virtualpos <= index || CacheCloser(storage, index) ))
        {
            if (storage->cache.virtualpos <= index)
                tmp = index - storage->cache.virtualpos;
        }
        else
        {
            storage->cache.it = storage->cache.posbegin; //storage->pos.begin();
            storage->cache.datapos = 0;
            storage->cache.virtualpos = 0;
            storage->cache.posindex = 0;
            tmp = index;
        }

        if (index >= storage->cache.virtualpos)
        {
            for ( ; storage->cache.it != storage->cache.posend; ++storage->cache.it, ++storage->cache.posindex)
            {
                auto &p = *storage->cache.it;
                size_type vdif = p.used + p.empty;
                if (tmp < vdif)
                {
                    datapos = storage->cache.datapos;
                    virtualpos = storage->cache.virtualpos;

                    datapos += min(p.used, index - virtualpos);
                    return storage->cache.it;
                }
                tmp -= vdif;
                storage->cache.virtualpos += vdif;
                storage->cache.datapos += p.used;
            }
        }
        else
        {
            while (storage->cache.virtualpos >= 0)
            {
                CacheDec(storage);
                if (storage->cache.virtualpos <= index)
                {
                    datapos = storage->cache.datapos;
                    virtualpos = storage->cache.virtualpos;

                    datapos += min(storage->cache.it->used, index - virtualpos);
                    return storage->cache.it;
                }
            }
        }
        throw L"Reaching end of indexpos";
        //return storage->pos.end();
    }

    // Returns reference to item at a given index position.
    static reference item_at(sparse_data *storage, size_type index)
    {
        size_type datapos, virtualpos;
        auto it = index_pos(storage, index, datapos, virtualpos);
#ifdef DEBUG
        CheckEnd(storage->pos, it);
#endif
        if (virtualpos + it->used <= index)
            return *insert_value(storage, it, index, virtualpos, datapos);
        return storage->data[datapos];
    }

    static const_reference item_at(const sparse_data *storage, size_type index)
    {
        size_type datapos, virtualpos;
        auto it = index_pos(storage, index, datapos, virtualpos);
#ifdef DEBUG
        CheckEnd(storage->pos, it);
#endif
        if (virtualpos + it->used <= index)
            return storage->def;
        return storage->data[datapos];
    }

    // Inserts a new value, modifying, erasing and joining existing blocks as needed.
    // * 'it' is the iterator of the block which would hold the new value, or which would come before a not yet
    // existing block holding the new value.
    // * 'index' is the position in 'it' where the new value is to be inserted.
    // * 'virtualpos' is the starting position of the block that it points to.
    // * 'datapos' is the data position where newvalue should be inserted.
    static data_iterator insert_value(sparse_data *storage, pos_iterator it, size_type index, size_type virtualpos, size_type datapos)
    {
        size_type blockpos = index - virtualpos;

        if (it == storage->pos.end())
        {
            // TODO: CHECK, probably never comes here:
            if (blockpos == 0 && storage->poslast->empty == 0) // The new element is directly appended to the last block.
                ++storage->poslast->used;
            else
            {
                // In this case blockpos is the number of empty elements that must be appended to the last block before inserting a new block.
                storage->poslast->empty += blockpos;
                storage->poslast = storage->pos.insert(storage->pos.end(), pos_elem(1, 0));
            }
            storage->data.push_back(storage->def);
            return --storage->data.end();
        }

        bool cachevalid = storage->cache.it != storage->cache.posend;

        if (blockpos < it->used + it->empty - 1 || it == storage->poslast) // The new item wouldn't be appended in 'it', nor would it join it with a block after it.
        {
            if (blockpos == it->used) // The new value is appended right after the last used value.
            {
                ++it->used;
                --it->empty;
                if (cachevalid && storage->cache.virtualpos > virtualpos)
                    ++storage->cache.datapos;
            }
            else // The new value is inserted in a new block after 'it'.
            {
                auto inserted = std::next(it);
                inserted = storage->pos.insert(inserted, pos_elem(1, (it->used + it->empty == blockpos ? 0 : it->used + it->empty - blockpos - 1)));
                it->empty = blockpos - it->used;
                if (it == storage->poslast)
                    storage->poslast = inserted;

                if (cachevalid && storage->cache.virtualpos > virtualpos)
                {
                    ++storage->cache.posindex;
                    ++storage->cache.datapos;
                }
            }
        }
        else if (it->empty == 1) // 'it' and the block after must be joined.
        {
            auto after = std::next(it);

            if (cachevalid && storage->cache.it == after)
                CacheDec(storage);

            it->empty = after->empty;
            it->used += after->used + 1;
            if (after == storage->poslast)
                storage->poslast = it;
            storage->pos.erase(after);
        }
        else // The inserted value is appended to the front of the next block, not joining anything.
        {
            --it->empty;
            ++(++it)->used;
            if (cachevalid && storage->cache.it == it)
            {
                --storage->cache.datapos;
                --storage->cache.virtualpos;
            }
        }
        return storage->data.insert(storage->data.begin() + datapos, storage->def);
    }

    // Checks the iterators around 'mid' and merges as necessary. 'mid' is the first iterator that was
    // changed in erase or unset, 'last' is the last changed one.
    // Positions between 'mid' and 'last' are erased.
    //
    // About position caching:
    // All caching must be done outside fix_merge. The only change this function makes in the position cache
    // is to decrement the posindex if the mid or last iterators get deleted, and only updates virtualpos
    // and datapos by the values in mid and last, not in any other block between them.
    static void fix_merge(sparse_data *storage, pos_iterator& mid, size_type midvirtualpos, pos_iterator& last, size_type lastvirtualpos)
    {
        auto posbegin = storage->pos.begin();
        if ((mid == posbegin || mid->used) && (mid->empty && last->used))
        {
            if (last != mid)
                storage->pos.erase(std::next(mid), last);
            return;
        }

        auto premid = mid;
        if (mid != posbegin)
            --premid;

        bool cachevalid = storage->cache.it != storage->cache.posend;

        if (last->used == 0 || mid->empty == 0)
        {
            if (mid != last)
            {
                if (cachevalid && storage->cache.virtualpos == lastvirtualpos)
                {
                    lastvirtualpos += last->empty + last->used;
                    storage->cache.virtualpos = lastvirtualpos;
                }

                mid->empty += last->empty;
                last->empty = 0;
                mid->used += last->used;
                last->used = 0;
            }

            if (mid->empty == 0 && last != storage->poslast)
            {
                if (cachevalid && storage->cache.virtualpos == lastvirtualpos + last->used + last->empty)
                    storage->cache.virtualpos -= mid->used;

                std::next(last)->used += mid->used;
                mid->used = 0;
            }
        }

        if (mid->used == 0 && mid != premid)
        {
            if (cachevalid && storage->cache.virtualpos == midvirtualpos)
            {
                storage->cache.virtualpos += mid->empty;
                midvirtualpos += mid->empty;
            }

            premid->empty += mid->empty;
            mid->empty = 0;
        }

        auto firsterase = mid;
        auto lasterase = last;
        if (mid->used != 0 || mid->empty != 0)
            ++firsterase;
        else if (cachevalid)
        {
            if (storage->cache.virtualpos == midvirtualpos)
            {
                if (midvirtualpos)
                    CacheDec(storage);
                else
                {
                    storage->cache.it = storage->pos.end();
                    cachevalid = false;
                }
            }
            else if (storage->cache.virtualpos > lastvirtualpos)
                --storage->cache.posindex;
        }

        if (last->used == 0)
        {
            if (last == storage->poslast)
                storage->poslast = std::prev(firsterase);

            if (cachevalid && storage->cache.virtualpos > lastvirtualpos)
                --storage->cache.posindex;
            ++lasterase;
        }

        if (mid == last && (mid->used != 0 || mid->empty != 0) && last->used != 0)
            return;

        if (storage->cache.posbegin == firsterase)
            storage->cache.posbegin = lasterase;
        storage->pos.erase(firsterase, lasterase);
    }

    // Erases 'cnt' virtual elements starting at the 'index' position.
    static iterator erase(sparse_data *storage, size_type index, size_type cnt)
    {
        if (cnt == 0)
            return iterator(storage, index);

#ifdef DEBUG
        storage->InvalidateFrom(index);
#endif
        size_type fdatapos, fvirtualpos;
        // Get the position of the element at index.
        pos_iterator first = index_pos(storage, index, fdatapos, fvirtualpos);
        // Get the position of the element at index + cnt - 1. The last virtual position to erase.
        size_type ldatapos, lvirtualpos;
        pos_iterator last;
        if (cnt == 1)
        {
            last = first;
            ldatapos = fdatapos;
            lvirtualpos = fvirtualpos;
        }
        else
            last = index_pos(storage, index + cnt - 1, ldatapos, lvirtualpos);

        size_type fblockpos = index - fvirtualpos; // Position in 'first' to the first to be deleted element.
        size_type lblockpos = index + cnt - lvirtualpos; // Position in 'last' which is after the last to be deleted.

        if (lblockpos - 1 < last->used)
            ++ldatapos;

        size_type datacnt = ldatapos - fdatapos;

        //size_type datacnt = fblockpos < first->used ? first->used - fblockpos : 0;

        //int miditcnt = 0; // Number of affected block iterators in the range of [first, last).
        //if (first != last)
        //{
        //    auto it = first;
        //    do
        //    {
        //        ++it;
        //        ++miditcnt;
        //        datacnt += it->used;
        //    }
        //    while (it != last);
        //}
        //
        //if (lblockpos < last->used)
        //    datacnt -= last->used - lblockpos;

        bool cachevalid = storage->cache.it != storage->cache.posend;

        if (datacnt != 0)
            storage->data.erase(storage->data.begin() + fdatapos, storage->data.begin() + (fdatapos + datacnt));

        if (first == last)
        {
            if (cachevalid && storage->cache.virtualpos > fvirtualpos)
            {
                storage->cache.datapos -= datacnt;
                storage->cache.virtualpos -= cnt;
            }

            first->empty -= cnt - datacnt;
            first->used -= datacnt;
        }
        else
        {
            if (cachevalid && storage->cache.virtualpos > fvirtualpos)
            {
                if (storage->cache.virtualpos <= lvirtualpos)
                {
                    if (CacheCloser(storage, fvirtualpos))
                    {
                        while(storage->cache.virtualpos > fvirtualpos)
                            CacheDec(storage);
                    }
                    else
                        ClearCache(storage);
                }
                else
                {
                    storage->cache.virtualpos -= cnt;
                    storage->cache.datapos -= datacnt;
                    auto it = first;
                    while (++it != last)
                        --storage->cache.posindex;
                    //storage->cache.posindex -= miditcnt - 1;
                }
            }

            first->empty = (first->used < fblockpos ? fblockpos - first->used : 0);
            first->used = min(first->used, fblockpos);

            last->empty -= last->used < lblockpos ? lblockpos - last->used : 0;
            last->used -= min(last->used, lblockpos);

            lvirtualpos += lblockpos;
        }

        storage->vsize -= cnt;

        fix_merge(storage, first, fvirtualpos, last, lvirtualpos);

        return iterator(storage, (index == sparse_size(storage) ? max_size(storage) : index));
    }

    static void unset(sparse_data *storage, size_type index, size_type cnt)
    {
#ifdef DEBUG
        storage->SparseInvalidateRange(index, cnt);
#endif
        size_type fdatapos, fvirtualpos;
        // Get the position of the element at index.
        pos_iterator first = index_pos(storage, index, fdatapos, fvirtualpos);
        // Get the position of the element at index + cnt - 1 which is non-exclusive. Includes the last virtual position to erase.
        size_type ldatapos, lvirtualpos;
        pos_iterator last;
        if (cnt == 1)
        {
            last = first;
            ldatapos = fdatapos;
            lvirtualpos = fvirtualpos;
        }
        else
            last = index_pos(storage, index + cnt - 1, ldatapos, lvirtualpos);

        size_type fblockpos = index - fvirtualpos; // Position in 'first' to the first to be deleted element.
        size_type lblockpos = index + cnt - lvirtualpos; // Position in 'last' which is after the last to be deleted.

        if (lblockpos - 1 < last->used)
            ++ldatapos;

        size_type datacnt = ldatapos - fdatapos;

        if (datacnt == 0)
            return;

        storage->data.erase(storage->data.begin() + fdatapos, storage->data.begin() + (fdatapos + datacnt));

        bool cachevalid = storage->cache.it != storage->cache.posend;

        if (first == last)
        {
            if (cachevalid && storage->cache.virtualpos > fvirtualpos)
                storage->cache.datapos -= datacnt;

            if (lblockpos >= first->used)
            {
                first->empty += first->used - fblockpos;
                first->used = fblockpos;
            }
            else
            {
                if (cachevalid && storage->cache.virtualpos > fvirtualpos)
                    ++storage->cache.posindex;

                storage->pos.insert(std::next(first), pos_elem(first->used - lblockpos, first->empty));
                if (storage->poslast == first)
                    ++storage->poslast;

                first->used = fblockpos;
                first->empty = lblockpos - fblockpos;
            }
        }
        else
        {
            if (cachevalid && storage->cache.virtualpos > fvirtualpos)
            {
                if (storage->cache.virtualpos <= lvirtualpos)
                {
                    if (CacheCloser(storage, fvirtualpos))
                    {
                        while(storage->cache.virtualpos > fvirtualpos)
                            CacheDec(storage);
                    }
                    else
                        ClearCache(storage);
                }
                else
                {
                    storage->cache.datapos -= datacnt;

                    auto it = first;
                    while (++it != last)
                        --storage->cache.posindex;
                }
            }

            // The empty part of 'first' takes over all mid blocks and some of the last block as well. Last block also starts a bit higher.
            first->empty = fblockpos < first->used ? cnt : cnt + (fblockpos - first->used);
            first->used = min(first->used, fblockpos);
            lvirtualpos += lblockpos;

            if (lblockpos >= last->used)
            {
                last->empty = last->empty - (lblockpos - last->used);
                last->used = 0;
            }
            else
                last->used -= lblockpos;
        }

        fix_merge(storage, first, fvirtualpos, last, lvirtualpos);
    }

    static void clear(sparse_data *storage)
    {
#ifdef DEBUG
        storage->InvalidateIterators();
#endif
        value_type def(std::move(storage->def));
        storage->data.clear();
        storage->pos.clear();
        storage->def = std::move(def);
        storage->poslast = storage->pos.insert(storage->pos.end(), pos_elem());
        storage->vsize = 0;
        ClearCache(storage);
        storage->cache.posbegin = storage->pos.begin();
        storage->cache.posend = storage->pos.end();
    }

    static void resize(sparse_data *storage, size_type n)
    {
        if (n == 0)
        {
            clear(storage);
            return;
        }

#ifdef DEBUG
        if (storage->vsize > n)
            storage->InvalidateFrom(n);
#endif

        if (n > storage->vsize)
        {
            if (max_size(storage) < n)
                throw L"The new size is over the maximum.";
            storage->poslast->empty += n - storage->vsize;
        }
        else
        {
            if (n > max_size(storage))
                throw L"Size over max_size.";

            if (storage->cache.it != storage->pos.end() && storage->cache.virtualpos >= n)
                ClearCache(storage);

            // Find the last block position to be kept when the size shrinks.
            storage->poslast = storage->pos.end();
            size_type bdif = 0;
            size_type vdif = 0;
            do
            {
                --storage->poslast;
                bdif += storage->poslast->used;
                vdif += storage->poslast->empty + storage->poslast->used;
            }
            while (storage->vsize - n >= vdif);
            storage->pos.erase(std::next(storage->poslast), storage->pos.end());

            size_type d = vdif - (storage->vsize - n); // New size of the new last block position including used and empty.
            if (d >= storage->poslast->used)
            {
                bdif -= storage->poslast->used;
                storage->poslast->empty = d - storage->poslast->used;
            }
            else
            {
                bdif -= d;
                storage->poslast->used = d;
                storage->poslast->empty = 0;
            }
            storage->data.resize(storage->data.size() - bdif);
        }
        storage->vsize = n;
    }

    // Updates the usage data of the passed position block which starts at 'virtualpos' at absolute 'index' to hold an
    // additional number of used positions, without updating the data vector. Creates a new block with the specified
    // number of used positions if 'index' points to the middle of the block's empty region.
    static void insert_used(sparse_data *storage, pos_iterator &it, size_type index, size_type virtualpos, size_type num)
    {
        if (max_size(storage) - storage->vsize < num)
            throw L"New size over the maximum.";
        size_type blockpos = index - virtualpos;

        if (it == storage->pos.end())
        {
            --it;
            blockpos = it->used + it->empty;
            virtualpos -= blockpos;
        }

        bool cacheused = storage->cache.it != storage->pos.end() && storage->cache.virtualpos > virtualpos;
        if (cacheused)
        {
            storage->cache.virtualpos += num;
            storage->cache.datapos += num;
        }

        if (!blockpos || blockpos <= it->used)
            it->used += num;
        else
        {
            if (cacheused)
                ++storage->cache.posindex;
            auto it2 = std::next(it);
            it2 = storage->pos.insert(it2, pos_elem(num, it->empty - (blockpos - it->used)));
            it->empty = blockpos - it->used;
            if (storage->poslast == it)
                storage->poslast = it2;
        }

        storage->vsize += num;
    }

    // Updates the usage data of the block at 'index' to hold an additional number of used positions, without updating the data vector.
    // Creates a new block with the specified number of used positions if the 'index' points to the middle of an empty region.
    // Returns the position in the data vector where the new elements should be inserted to match the used count.
    static size_type insert_used(sparse_data *storage, size_type index, size_type num)
    {
#ifdef DEBUG
        storage->InvalidateFrom(index);
#endif
        size_type datapos, virtualpos;
        auto it = index_pos(storage, index, datapos, virtualpos);
        insert_used(storage, it, index, virtualpos, num);

        return datapos;
    }

    // Updates the empty data of the block at 'index' to hold an additional number of empty positions.
    // If 'index' points to the middle of the block, a new one is created.
    static void insert_unused(sparse_data *storage, pos_iterator &it, size_type index, size_type virtualpos, size_type num)
    {
        if (max_size(storage) - storage->vsize < num)
            throw L"New size over the maximum.";

        storage->vsize += num;

        if (it == storage->pos.end())
        {
            storage->pos.back().empty += num;
            return;
        }

        bool cacheused = storage->cache.it != storage->pos.end();

        size_type blockpos = index - virtualpos;
        if (blockpos >= it->used)
        {
            if (cacheused && storage->cache.virtualpos > virtualpos)
                storage->cache.virtualpos += num;
            it->empty += num;
            return;
        }
        if (blockpos == 0)
        {
            if (it != storage->pos.begin())
            {
                if (cacheused && storage->cache.virtualpos >= virtualpos)
                    storage->cache.virtualpos += num;
                (--it)->empty += num;
            }
            else
            {
                storage->pos.insert(storage->pos.begin(), pos_elem(0, num));
                if (cacheused)
                {
                    storage->cache.virtualpos += num;
                    ++storage->cache.posindex;
                    storage->cache.posbegin = storage->pos.begin();
                }
            }
            return;
        }
        if (cacheused && storage->cache.virtualpos > virtualpos)
        {
            storage->cache.virtualpos += num;
            ++storage->cache.posindex;
        }
        storage->pos.insert(it, pos_elem(blockpos, num));
        it->used -= blockpos;
    }

    // Updates the empty data of the block at 'index' to hold an additional number of empty positions.
    // If 'index' points to the middle of the block, a new one is created.
    static void insert_unused(sparse_data *storage, size_type index, size_type num)
    {
#ifdef DEBUG
        storage->InvalidateFrom(index);
#endif
        size_type datapos, virtualpos;
        auto it = index_pos(storage, index, datapos, virtualpos);
        insert_unused(storage, it, index, virtualpos, num);
    }

    static void pop_back(sparse_data *storage)
    {
        --storage->vsize;
        if (storage->cache.it != storage->pos.end() && storage->cache.virtualpos > 0 && storage->cache.virtualpos == storage->vsize)
            CacheDec(storage);

        if (storage->poslast->empty > 0)
        {
            --storage->poslast->empty;
            return;
        }
        --storage->poslast->used;
        storage->data.pop_back();
        if (storage->poslast->used == 0 && storage->poslast != storage->pos.begin())
            --(storage->poslast = storage->pos.erase(storage->poslast));
    }

    template<typename Val>
    static void push_back(sparse_data *storage, Val&& val)
    {
        if (sparse_size(storage) == max_size(storage))
            throw L"New size over the maximum.";

        storage->data.push_back(std::forward<Val>(val));

        if (storage->poslast->empty == 0)
            ++storage->poslast->used;
        else
        {
            storage->pos.push_back(pos_elem(1, 0));
            ++storage->poslast;
        }
        ++storage->vsize;
    }

    // Inserts a list of positions at index in storage and returns the position in data, where
    // the non virtual elements are to be inserted. The inserted blocks in 'otherpos' must
    // be valid. Only the first element can have 0 used and the last 0 empty values.
    static size_type insert_pos(sparse_data *storage, size_type index, pos_list &otherpos)
    {
        size_type datapos, virtualpos;
        auto it = index_pos(storage, index, datapos, virtualpos);
        if (otherpos.empty())
            return datapos;

        size_type blockpos = index == max_size(storage) ? 0 : index - virtualpos;

        pos_iterator ob = otherpos.begin();

        bool cachevalid = storage->cache.it != storage->pos.end();

        // Placement 1: before next used block.
        if (blockpos == 0)
        {
            if (it != storage->pos.begin())
            {
                auto pit = std::prev(it);
                if (ob->used == 0 || pit->empty == 0) // pit->empty only 0 when it was storage->pos.end()
                {
                    if (cachevalid && storage->cache.virtualpos >= virtualpos)
                    {
                        storage->cache.virtualpos += ob->empty;
                        virtualpos += ob->empty;
                    }

                    pit->empty += ob->empty;
                    pit->used += ob->used;
                    otherpos.pop_front();
                    if (otherpos.empty())
                        return datapos;
                }
            }

            if (it != storage->pos.end())
            {
                ob = std::prev(otherpos.end());
                if (ob->empty == 0 || it->used == 0)
                {
                    if (cachevalid && storage->cache.virtualpos > virtualpos)
                    {
                        storage->cache.virtualpos += ob->used + ob->empty;
                        storage->cache.datapos += ob->used;
                    }

                    it->used += ob->used;
                    it->empty += ob->empty;
                    otherpos.pop_back();
                    if (otherpos.empty())
                        return datapos;
                }
            }
        }
        // Placement 2: middle or end of used block.
        else if (blockpos <= it->used)
        {
            if (cachevalid && storage->cache.virtualpos > virtualpos)
            {
                storage->cache.virtualpos += ob->used;
                storage->cache.datapos += ob->used;
            }

            it->used += ob->used;
            blockpos += ob->used;
            ob->used = 0;
            if (ob->empty == 0) // It's not possible to have a first position block with 0 empty if it is not the last block as well. Nothing else to do.
                return datapos;

            size_type it_empty = it->empty;
            size_type it_used = it->used - blockpos;
            it->empty = ob->empty;
            it->used = blockpos;
            otherpos.pop_front();

            if (it_used == 0) // When entering placement 2: blockpos == it->used
            {
                if (otherpos.empty())
                {
                    if (cachevalid && storage->cache.virtualpos > virtualpos)
                        storage->cache.virtualpos += it->empty;

                    it->empty += it_empty;
                    return datapos;
                }

                if (cachevalid && storage->cache.virtualpos > virtualpos)
                    storage->cache.virtualpos -= it_empty - it->empty;

                ob = std::prev(otherpos.end());
                ob->empty += it_empty;
            }
            else // When entering placement 2: blockpos < it->used
            {
                if (cachevalid && storage->cache.virtualpos > virtualpos)
                {
                    storage->cache.virtualpos -= it_empty + it_used - it->empty;
                    storage->cache.datapos -= it_used;
                }

                if (!otherpos.empty())
                    ob = std::prev(otherpos.end());
                else
                    ob = otherpos.end();
                if (ob != otherpos.end() && ob->empty == 0)
                {
                    ob->used += it_used;
                    ob->empty += it_empty;
                }
                else
                    otherpos.push_back(pos_elem(it_used, it_empty));
            }

            virtualpos += it->used + it->empty;
            ++it;
        }
        // Placement 3: middle of empty block.
        else
        {
            if (ob->used == 0)
            {
                if (cachevalid && storage->cache.virtualpos > virtualpos)
                    storage->cache.virtualpos += ob->empty;

                it->empty += ob->empty;
                blockpos += ob->empty;
                otherpos.pop_front();
                if (otherpos.empty())
                    return datapos;
            }

            size_type it_empty = it->used + it->empty - blockpos;
            if (cachevalid && storage->cache.virtualpos > virtualpos)
                storage->cache.virtualpos -= it_empty;
            it->empty -= it_empty;

            virtualpos += it->used + it->empty;
            ++it;

            ob = std::prev(otherpos.end());
            ob->empty += it_empty;
        }

        pos_iterator pit = it == storage->pos.begin() ? it : std::prev(it);

        storage->pos.splice(it, std::move(otherpos));
        storage->cache.posbegin = storage->pos.begin();

        if (cachevalid && storage->cache.virtualpos >= virtualpos)
        {
            if (pit == it)
                pit = storage->pos.begin();
            else
                ++pit;
            for ( ; pit != it; ++pit, ++storage->cache.posindex)
            {
                storage->cache.virtualpos += pit->empty + pit->used;
                storage->cache.datapos += pit->used;
            }
        }

        if (it == storage->pos.end())
            storage->poslast = --storage->pos.end();

        return datapos;
    }

    static iterator insert(sparse_data *storage, size_type index, const_iterator first, const_iterator last)
    {
#ifdef DEBUG
        if (last < first)
            throw L"Invalid range. The last iterator should be higher or equal to first.";
#endif
        size_type cnt = last - first;
        if (max_size(storage) - storage->vsize < cnt)
            throw L"New size over the maximum.";

        if (first == last)
            return iterator(storage, index);
        pos_list newpos;

        size_type datapos, virtualpos;
        sparse_data *sdata = first.storage;
        auto it = index_pos(sdata, first.index, datapos, virtualpos);
        size_type blockpos = first.index - virtualpos;

        size_type datacnt = it->used > blockpos ? it->used - blockpos : 0;
        newpos.push_back(pos_elem(datacnt, it->empty - (it->used > blockpos ? 0 : blockpos - it->used)));
        size_type lblockpos = last.index - virtualpos;
        if (virtualpos + it->used + it->empty > last.index) // 'last' in same block as 'first'
        {
            lblockpos -= blockpos;
            it = --newpos.end();
            if (lblockpos < it->used)
            {
                datacnt -= it->used - lblockpos;
                it->used = lblockpos;
                it->empty = 0;
            }
            else
                it->empty = lblockpos - it->used;
        }
        else
        {
            lblockpos -= it->used + it->empty;
            ++it;
            while (lblockpos != 0 && lblockpos >= it->used + it->empty) // Keep the seemingly irrelevant 'lblockpos != 0', because in that case 'it' might point to the back of a position list.
            {
                newpos.push_back(*it);
                lblockpos -= it->used + it->empty;
                datacnt += it->used;
                ++it;
            }

            if (lblockpos > 0)
            {
                if (lblockpos < it->used)
                {
                    datacnt += lblockpos;
                    newpos.push_back(pos_elem(lblockpos, 0));
                }
                else
                {
                    datacnt += it->used;
                    newpos.push_back(pos_elem(it->used, lblockpos - it->used));
                }
            }
        }

        size_type owndatapos = insert_pos(storage, index, newpos);
        storage->data.insert(storage->data.begin() + owndatapos, sdata->data.begin() + datapos, sdata->data.begin() + (datapos + datacnt));
        storage->vsize += cnt;
#ifdef DEBUG
        storage->InvalidateFrom(index);
#endif
        return iterator(storage, index);
    }

    static iterator insert(sparse_data *storage, size_type index, const_reverse_iterator first, const_reverse_iterator last)
    {
#ifdef DEBUG
        if (last < first)
            throw L"Invalid range. The last iterator should be higher or equal to first.";
#endif
        size_type cnt = last - first;
        if (max_size(storage) - storage->vsize < cnt)
            throw L"New size over the maximum.";

        if (first == last)
            return iterator(storage, index);
        pos_list newpos;

        size_type datapos, virtualpos;
        size_type firstindex = (first + 1).base().index;
        size_type lastindex = last.base().index; // Non-exclusive. Includes index of last value to insert.
        sparse_data *sdata = first.base().storage;
        auto it = index_pos(sdata, firstindex, datapos, virtualpos);
        size_type blockpos = firstindex - virtualpos;

        size_type lastused = blockpos + 1;
        size_type datacnt = min(it->used, lastused);
        if (blockpos >= it->used)
            --datapos;
        if (virtualpos <= lastindex)
        {
            size_type lblockpos = lastindex - virtualpos;
            if (blockpos >= it->used)
                newpos.push_back(pos_elem(0, lastused - max(it->used, lblockpos)));
            if (it->used > lblockpos)
                newpos.push_back(pos_elem(min(lastused, it->used) - lblockpos, 0));

            datacnt -= min(it->used, lblockpos);
        }
        else
        {
            if (blockpos >= it->used)
            {
                newpos.push_back(pos_elem(0, lastused - it->used));
                lastused = it->used;
            }

            while (virtualpos > lastindex)
            {
                --it;
                newpos.push_back(pos_elem(lastused, it->empty));
                lastused = it->used;
                datacnt += lastused;
                virtualpos -= it->used + it->empty;
            }
            size_type lblockpos = lastindex - virtualpos;
            datacnt -= min(lblockpos, lastused);

            if (lblockpos > lastused)
                newpos.back().empty -= lblockpos - lastused;
            else if (lastused > lblockpos)
                newpos.push_back(pos_elem(lastused - lblockpos, 0));
        }

        size_type owndatapos = insert_pos(storage, index, newpos);
        if (datacnt)
            storage->data.insert(storage->data.begin() + owndatapos, make_reverse_iterator(sdata->data.begin() + datapos + 1), make_reverse_iterator(sdata->data.begin() + datapos + 1 - datacnt));
        storage->vsize += cnt;
#ifdef DEBUG
        storage->InvalidateFrom(index);
#endif

        return iterator(storage, index);
    }
public:
    sparse_list(size_type n = 0) : storage(NULL)
    {
        storage = new sparse_data;
        storage->def = value_type();
        storage->poslast = storage->pos.insert(storage->pos.end(), pos_elem());
        storage->vsize = 0;

        ClearCache(storage);
        storage->cache.posend = storage->pos.end();
        storage->cache.posbegin = storage->pos.begin();

        resize(n);
    }

    sparse_list(size_type n, value_type def) : storage(NULL)
    {
        storage = new sparse_data;
        storage->def = def;
        storage->poslast = storage->pos.insert(storage->pos.end(), pos_elem());
        storage->vsize = 0;

        ClearCache(storage);
        storage->cache.posend = storage->pos.end();
        storage->cache.posbegin = storage->pos.begin();

        resize(n);
    }

    sparse_list(const self_type& other) : storage(NULL)
    {
        *this = other;
    }

    sparse_list(self_type&& other) : storage(NULL)
    {
        *this = std::move(other);
    }

    ~sparse_list()
    {
        delete storage;
    }

#ifdef TESTING_SPARSE_LIST
    // TODO: remove test.
    void test_integrity()
    {
        std::wstring error;

        bool cachefound = storage->cache.it == storage->pos.end();

        int ix = 0;
        size_type datacnt = 0;
        size_type vsize = 0;
        for (auto it = storage->pos.begin(); it != storage->pos.end(); ++it, ++ix)
        {
            if (!cachefound && storage->cache.it == it)
            {
                cachefound = true;
                if (storage->cache.virtualpos != vsize)
                    error += L"Cache virtualpos mismatch.";
                if (storage->cache.datapos != datacnt)
                    error += L"Cache datapos mismatch.";
                if (storage->cache.posindex != ix)
                    error += L"Cache position index mismatch.";
            }

            datacnt += it->used;
            vsize += it->empty + it->used;
            if (it != storage->pos.begin() && it->used == 0)
                error += L"Empty block in the middle of the list.\n";
            if (it != storage->poslast && it->empty == 0)
                error += L"Full block in middle of the list.\n";
        }
        if (vsize != storage->vsize)
            error += L"Virtual size mismatch.\n";
        if (datacnt != storage->data.size())
            error += L"Data size mismatch.\n";
        if (storage->poslast != --storage->pos.end())
            error += L"Poslast mismatch.\n";
        if (storage->cache.posbegin != storage->pos.begin())
            error += L"Posbegin mismatch.\n";
        if (storage->cache.posend != storage->pos.end())
            error += L"Posend mismatch.\n";
        if (!cachefound)
            error += L"Cache iterator points to non existing item.\n";

        if (!error.empty())
            throw error;
    }
#endif

    self_type& operator=(const self_type& other)
    {
        if (storage == other.storage)
            return *this;
        delete storage;
        storage = new sparse_data;
        storage->data = other.storage->data;
        storage->pos = other.storage->pos;
        storage->poslast = --storage->pos.end();
        storage->def = other.storage->def;
        storage->vsize = other.storage->vsize;
        storage->cache.it = storage->pos.end();
        storage->cache.posbegin = storage->pos.begin();
        storage->cache.posend = storage->pos.end();

        return *this;
    }

    self_type& operator=(self_type&& other)
    {
        if (storage == other.storage)
            return *this;
        delete storage;
        storage = other.storage;
        other.storage = NULL;

        return *this;
    }

    void clear()
    {
        clear(storage);
    }

    void resize(size_type n)
    {
        if (sparse_size(storage) == n)
            return;

        resize(storage, n);
    }

    iterator erase(const_iterator position)
    {
#ifdef DEBUG
        position.CheckSameOwner(storage);
#endif
        return erase(storage, position.index, 1);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        int cnt = last - first;
#ifdef DEBUG
        if (cnt < 0)
            throw L"Invalid indexes, last is before first.";
        first.CheckSameOwner(storage);
#endif
        if (last.index == max_size())
        {
            resize(first.index + 1);
            return end();
        }
        else
            return erase(storage, first.index, cnt);
    }

    sparse_iterator erase(const_sparse_iterator position)
    {
        iterator it = erase(position.base());
        if (is_set(it))
            return sparse_iterator(it);
        size_type datapos, virtualpos;
        auto s = index_pos(storage, it.index, datapos, virtualpos);
        size_type blockpos = it.index - virtualpos;
        return sparse_iterator(it + (s->used + s->empty - blockpos));
    }

    iterator insert(const_iterator position, const value_type &val)
    {
#ifdef DEBUG
        position.CheckSameOwner(storage);
#endif
        size_type ix = position.index == max_size() ? storage->vsize : position.index;
        size_type datapos = insert_used(storage, ix, 1);
        storage->data.insert(storage->data.begin() + datapos, val);
        return iterator(storage, ix);
    }

    iterator insert(const_iterator position, value_type &&val)
    {
#ifdef DEBUG
        position.CheckSameOwner(storage);
#endif
        size_type ix = position.index == max_size() ? storage->vsize : position.index;
        size_type datapos = insert_used(storage, ix, 1);
        storage->data.insert(storage->data.begin() + datapos, std::move(val));
        return iterator(storage, ix);
    }

    iterator insert(const_iterator position, size_type n, const value_type &val)
    {
#ifdef DEBUG
        position.CheckSameOwner(storage);
#endif
        if (!n)
            return iterator(storage, position.index);
        size_type ix = position.index == max_size() ? storage->vsize : position.index;;
        size_type datapos = insert_used(storage, ix, n);
        storage->data.insert(storage->data.begin() + datapos, n, val);
        return iterator(storage, ix);
    }

    iterator insert(const_iterator position, const_iterator first, const_iterator last)
    {
#ifdef DEBUG
        position.CheckSameOwner(storage);
#endif
        return insert(storage, position.index, first, last);
    }

    iterator insert(const_iterator position, iterator first, iterator last)
    {
#ifdef DEBUG
        position.CheckSameOwner(storage);
#endif
        return insert(storage, position.index, const_iterator(first), const_iterator(last));
    }

    iterator insert(const_iterator position, const_reverse_iterator first, const_reverse_iterator last)
    {
#ifdef DEBUG
        position.CheckSameOwner(storage);
#endif
        return insert(storage, position.index, first, last);
    }

    iterator insert(const_iterator position, reverse_iterator first, reverse_iterator last)
    {
#ifdef DEBUG
        position.CheckSameOwner(storage);
#endif
        return insert(storage, position.index, const_reverse_iterator(first), const_reverse_iterator(last));
    }

    iterator insert(const_iterator position, const_sparse_iterator first, const_sparse_iterator second)
    {
#ifdef DEBUG
        position.CheckSameOwner(storage);
#endif
        int cnt = second - first;
        size_type datapos, virtualpos;
        index_pos(first.base().storage, first.base().index, datapos, virtualpos);
        return insert(position, first.base().storage->data.begin() + datapos, first.base().storage->data.begin() + (datapos + cnt));
    }

    iterator insert(const_iterator position, sparse_iterator first, sparse_iterator second)
    {
#ifdef DEBUG
        position.CheckSameOwner(storage);
#endif
        return insert(position, const_sparse_iterator(first), const_sparse_iterator(second));
    }

    iterator insert(const_iterator position, const_reverse_sparse_iterator first, const_reverse_sparse_iterator second)
    {
#ifdef DEBUG
        position.CheckSameOwner(storage);
#endif
        if (first == second)
            return iterator(storage, position.index);

        int cnt = second - first;
        size_type datapos, virtualpos;
        index_pos(first.base().base().storage, (first.base() - 1).base().index, datapos, virtualpos);
        datapos = first.base().base().storage->data.size() - 1 - datapos;

        return insert(position, first.base().base().storage->data.rbegin() + datapos, first.base().base().storage->data.rbegin() + (datapos + cnt));
    }

    iterator insert(const_iterator position, reverse_sparse_iterator first, reverse_sparse_iterator second)
    {
#ifdef DEBUG
        position.CheckSameOwner(storage);
#endif
        return insert(position, const_reverse_sparse_iterator(first), const_reverse_sparse_iterator(second));
    }

    template<class Iter>
    typename std::enable_if<std::integral_constant<bool, !std::is_integral<Iter>::value>::value, iterator>::type // Return value. It is 'iterator' for non integral types. Iterators can't be integral types.
        insert(const_iterator position, Iter first, Iter last)
    {
#ifdef DEBUG
        position.CheckSameOwner(storage);
#endif
        if (first == last)
            return iterator(storage, position.index);
        size_type ix = position.index == max_size() ? storage->vsize : position.index;
#ifdef DEBUG
        storage->InvalidateFrom(ix);
#endif

        size_type datapos, virtualpos;
        auto it = index_pos(storage, ix, datapos, virtualpos);

        auto ds = storage->data.size();
        storage->data.insert(storage->data.begin() + datapos, first, last);
        ds = storage->data.size() - ds;

        insert_used(storage, it, ix , virtualpos, ds);
        return iterator(storage, ix);
    }

    iterator insert_space(const_iterator position, size_type num = 1)
    {
#ifdef DEBUG
        position.CheckSameOwner(storage);
#endif
        size_type ix = position.index == max_size() ? storage->vsize : position.index;
        if (num > 0)
            insert_unused(storage, position.index, num);
        return iterator(storage, ix);
    }

    void swap(self_type &other)
    {
        std::swap(storage, other.storage);
    }

    // Similarly to count in std::map, returns 1 if the value is set at index and 0 if not.
    bool is_set(size_type index) const
    {
        return is_set(storage, index);
    }

    bool is_set(const const_iterator &it) const
    {
        return is_set(it.storage, it.index);
    }

    // Removes the value at position or between positions.
    void unset(size_type index)
    {
#ifdef DEBUG
        if (index >= sparse_size(storage))
            throw L"Unset range out of bounds.";
#endif
        unset(storage, index, 1);
    }

    void unset(iterator position)
    {
#ifdef DEBUG
        position.CheckSameOwner(storage);
#endif
        if (position.index == max_size())
            return;
        unset(storage, position.index, 1);
    }

    void unset(sparse_iterator position)
    {
        unset(position.base());
    }

    void unset(iterator first, iterator last)
    {
        int cnt = last - first;
#ifdef DEBUG
        if (cnt < 0)
            throw L"Invalid indexes, last is before first.";
        first.CheckSameOwner(storage);
#endif
        if (first.index == max_size())
            return;
        unset(storage, first.index, cnt);
    }

    void unset(sparse_iterator first, sparse_iterator last)
    {
        unset(first.base(), last.base());
    }

    void push_back(const value_type &val)
    {
        push_back(storage, val);
    }

    void push_back(value_type &&val)
    {
        push_back(storage, std::forward(val));
    }

    void shrink_to_fit()
    {
        storage->data.shrink_to_fit();
    }

    // Returns the element at the index position or the default value if there is nothing set at the given position.
    reference operator[](size_type index)
    {
        return item_at(storage, index);
    }

    const_reference operator[](size_type index) const
    {
        return item_at(storage, index);
    }

    reference at(size_type index)
    {
        if (index >= sparse_size(storage)) // Exception even when not in DEBUG mode.
            throw std::out_of_range("Index to access sparse list element out of range.");
        return operator[](index);
    }

    const_reference at(size_type index) const
    {
        if (index >= sparse_size(storage)) // Exception even when not in DEBUG mode.
            throw std::out_of_range("Index to access const sparse list element out of range.");
        return operator[](index);
    }

    reference front()
    {
        return operator[](0);
    }

    const_reference front() const
    {
        return operator[](0);
    }

    reference back()
    {
        return operator[](sparse_size(storage) - 1);
    }

    const_reference back() const
    {
        return operator[](sparse_size(storage) - 1);
    }

    size_type size() const
    {
        return sparse_size(storage);
    }

    void pop_back()
    {
#ifdef DEBUG
        if (sparse_size(storage) == 0)
            throw L"Pop_back called for empty sparse list.";
#endif
        pop_back(storage);
    }

    bool empty() const
    {
        return sparse_size(storage) == 0;
    }

    iterator begin()
    {
        if (empty())
            return end();
        return iterator(storage, 0);
    }

    const_iterator begin() const
    {
        if (empty())
            return end();
        return const_iterator(storage, 0);
    }

    sparse_iterator sbegin()
    {
        if (storage->data.empty())
            return send();
        size_type p = storage->pos.begin()->used > 0 ? 0 : storage->pos.begin()->empty;
        return sparse_iterator(iterator(storage, p));
    }

    const_sparse_iterator sbegin() const
    {
        if (storage->data.empty())
            return send();
        size_type p = storage->pos.begin()->used > 0 ? 0 : storage->pos.begin()->empty;
        return const_sparse_iterator(const_iterator(storage, p));
    }

    // Returns an iterator to the last position which is set, up to the given position.
    sparse_iterator spos(size_type position)
    {
        if (is_set(position))
            return sparse_iterator(iterator(storage, position));
        return sparse_iterator(iterator(storage, last_set_pos(storage, position)));
    }

    // Returns an iterator to the last position which is set, up to the given position.
    sparse_iterator spos(const_iterator position)
    {
        if (is_set(position))
            return sparse_iterator(iterator(storage, position.index));
        return sparse_iterator(iterator(storage, last_set_pos(storage, position.index)));
    }

    // Returns an iterator to the last position which is set, up to the given position.
    const_sparse_iterator spos(size_type position) const
    {
        if (is_set(position))
            return const_sparse_iterator(const_iterator(storage, position));
        return const_sparse_iterator(const_iterator(storage, last_set_pos(storage, position)));
    }

    // Returns an iterator to the last position which is set, up to the given position.
    const_sparse_iterator spos(const_iterator position) const
    {
        if (is_set(position))
            return const_sparse_iterator(position);
        return const_sparse_iterator(const_iterator(storage, last_set_pos(storage, position.index)));
    }

    iterator end()
    {
        return iterator(storage, max_size(storage));
    }

    const_iterator end() const
    {
        return const_iterator(storage, max_size(storage));
    }

    sparse_iterator send()
    {
        return sparse_iterator(end());
    }

    const_sparse_iterator send() const
    {
        return const_sparse_iterator(end());
    }

    const_iterator cbegin() const
    {
        return begin();
    }

    const_sparse_iterator csbegin() const
    {
        return sbegin();
    }

    const_iterator cend() const
    {
        return end();
    }

    const_sparse_iterator csend() const
    {
        return send();
    }

    reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }

    reverse_sparse_iterator rsbegin()
    {
        return reverse_sparse_iterator(send());
    }

    const_reverse_sparse_iterator rsbegin() const
    {
        return const_reverse_sparse_iterator(send());
    }

    const_reverse_iterator crbegin() const
    {
        return rbegin();
    }

    const_reverse_sparse_iterator crsbegin() const
    {
        return rsbegin();
    }

    reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }

    reverse_sparse_iterator rsend()
    {
        return reverse_sparse_iterator(sbegin());
    }

    const_reverse_sparse_iterator rsend() const
    {
        return const_reverse_sparse_iterator(sbegin());
    }

    const_reverse_iterator crend() const
    {
        return rend();
    }

    const_reverse_sparse_iterator crsend() const
    {
        return rsend();
    }

    typename data_vector::value_type* data_pointer()
    {
        return storage->data.data();
    }

    typename data_vector::value_type* data_pointer() const
    {
        return storage->data.data();
    }

    typename data_vector::size_type data_size() const
    {
        return storage->data.size();
    }

    //data_iterator data_begin()
    //{
    //    return storage->data.begin();
    //}

    //const_data_iterator data_begin() const
    //{
    //    return storage->data.begin();
    //}

    //const_data_iterator data_cbegin() const
    //{
    //    return storage->data.cbegin();
    //}

    //data_iterator data_end()
    //{
    //    return storage->data.end();
    //}

    //const_data_iterator data_end() const
    //{
    //    return storage->data.end();
    //}

    //const_data_iterator data_cend() const
    //{
    //    return storage->data.cend();
    //}

    //reverse_data_iterator data_rbegin()
    //{
    //    return storage->data.rbegin();
    //}

    //const_reverse_data_iterator data_rbegin() const
    //{
    //    return storage->data.rbegin();
    //}

    //const_reverse_data_iterator data_crbegin() const
    //{
    //    return storage->data.crbegin();
    //}

    //reverse_data_iterator data_rend()
    //{
    //    return storage->data.rend();
    //}

    //const_reverse_data_iterator data_rend() const
    //{
    //    return storage->data.rend();
    //}

    //const_reverse_data_iterator data_crend() const
    //{
    //    return storage->data.crend();
    //}

    size_type max_size() const
    {
        return max_size(storage);
    }
};


}
/* End of NLIBNS */


namespace std
{
    template<class T, class Alloc>
    void swap(NLIBNS::sparse_list<T, Alloc>& x, NLIBNS::sparse_list<T, Alloc>& y)
    {
        x.swap(y);
    }
}
