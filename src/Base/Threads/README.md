# Multithreads in FreeCAD

Using multiple threads to do heavy computations is a great way to...<br/>
... get into trouble!!! :-)

So, we want to organize things in such a way that multithreading in FreeCAD
will not get messy... and will not deadlock.


## Atomic variables

With multiple threads we need to worry about atomicity.
A sequence of operations done to a variable might give inconsistent results
if there is another thread manipulating the same variable.

To assure that no such inconsistencies arise we can,
for example, use some mutual exclusion mechanism (mutex).
You are telling the other threads that
they have to wait you finish what you are doing before they access the same data.
This causes deadlocks and all sorts of nightmares.

The classical example is the increment by one of an `int i` variable.
If two threads A and B do concurrently a non atomic increment (`++i`),
the assembly code might be:
1. Store the value of `i` in a register. (the hardware computes values stored in registers)
2. Adds 1 to the stored value.
3. Saves the result back to the variable `i`.

Suppose we start with `i = 0`.
Then, if thread A executes step 1 and is interrupted by B that executes 1,2 and 3.
And only then, thread A executes step 2 and 3.
The final result will be that `i == 1`, despite the fact that it was "incremented" twice.


### Mutex

Some hardware witchcraft (atomic hardware operations) allows for the existence of mutexes.
A mutex is like a physical flag that is such that
the witchcraft do not allow people do dispute its ownership.
It might be that no one is holding the flag.
It might be that someone is holding it.
But there is no dispute and if two people try to acquire it at the same time,
only one of them will succeed. It is magical... :-)

This gives us a raw method for organizing data access:
> We all **agree** that someone is capable of changing (or even accessing) the data,
> only if this person is holding the flag.

A mutex is acquired.
When a thread attempts to acquire it (let's call it "request"), the thread might block and wait for it to be released.
This is the moment where a thread might block:
when it tries to acquire (requests) a mutex.

A deadlock happens when there are two flags F and G, and two threads A and B.
If:
1. Thread A acquires flag F.
2. Thread B acquires flag G.
3. Thread A tries to acquire flag G, but since B has it, thread A simply blocks and waits for B to release it.
4. Thread B does the same: try to acquire flag F and blocks, waiting for A to release it.

In this specific case, a rule would help:
> We could all **agree** that whenever we acquire flags F and G,
> we shall always acquire flag F first.

This way, the deadlock will not occur.

The STL library in C++ provides methods to lock many mutexes at "once".
And by that, we mean that the mutexes can be ordered somehow.

It is important to notice that when dealing with mutexes and alike,
there are lots of **rules we need to respect**.
The library will not do everything for us.
For example, one might attempt to access the resource without acquiring the mutex first.


#### Shared mutex

A *shared mutex* implements two kinds of lock.
The idea is to have many threads reading the data if no one is modifying it.
But when a thread is modifying the data, no other thread can be reading or modifying it.

So, a shared mutex has two kinds of lock.
1. A *unique lock*. It blocks until no one holds a lock. Then it acquires the lock.
2. A *shared lock*. It only blocks if some one is holding a *unique lock*.

When threads just want to read the data, they get a *shared lock*.
Many threads can read at the same time.
And they have the assurance that the data will not change while it is being processed.
Those threads might block waiting for a *unique lock* to be released.

When a thread wants to change the data, it must acquire a *unique lock*.
It might block waiting for any thread holding a *shared lock* or a *unique lock*
to finish processing the data.

We shall implement two classes: `UniqueLock` and `SharedLock`.
We use *RAII*.
So, when we construct these objects the mutex is locked.
When an instance of those objects is destructed, the locks are automatically released.
The constructors might block, waiting for the locks to be available.

#### Avoiding deadlocks

A deadlock occurs when:
1. Thread A blocks waiting for a lock G.
2. Some other tread B holds the lock for G.
3. Thread B **does not release G** for *some reason*.

If we design things such that no thread will ever block while holding the lock G,
the situation above will not happen.

Of course, no one should store a lock and hold it forever.
We use *RAII* and **local variables**, so that the locks are always released.
Even if exceptions are uncaught, we will not hold a lock forever.


So, assume we have lots of *shared mutexes* G1, G2, ..., GN.
We shall state some rules to be implemented by `UniqueLock` and `SharedLock`:
1. A `UniqueLock` can request a set of mutexes at once.
2. A `UniqueLock` is allowed to be called if the same thread already has a `UniqueLock`. But only if all mutexes being locked are in fact already locked. Otherwise, a `UniqueLock` cannot be constructed by a thread that already owns any kind of lock on any mutex. This is a programming error, and should not happen. An exception will be thrown.
3. If `SharedLock` requests a mutex G and the thread already has a `UniqueLock U`:
    1. If U **does not own** G, throw and exception. This is a programming error, and should not happen.
    2. If U **owns** G, do nothing. Just pretend we acquired the lock.

Those two classes, both descend from `LockBase`.
The common ancestor holds two `static thread_local` set of mutexes.
Those sets indicate for which mutexes we hold a `UniqueLock` and a `SharedLock`.
This way we can control if the program is complying with the rules above.

**Cavet:** the `SharedLock` after a `UniqueLock` is basically "useless" (although it does make sense).
If you destroy `UniqueLock` and do not destroy `SharedLock`,
the shared lock will not hold any lock.


### Wrapped by `std::atomic`

Variables that are simple to copy can be wrapped in `std::atomic`.
The value held by `std::atomic` can be atomically
*read* (`load()`), *set* (`store()`) and *swapped* (`exchange()`).

Different specializations can have extra functionality.
For example, `std::atomic<int>` implements atomic increment and decrement: `++`, `--`, `+=` and `-=`.

In this case, multithreading has a cost:
> You do not change the data on the fly!
> You produce another data and *swap* it with the current data.
> The data is always consistent. But you need to have two copies.
> One copy stored in `std::atomic` and one copy where you work.
> After you finish producing the new data, you *swap* them and discard (or not) the old one.


#### `std::atomic<std::shared_ptr>`

This is available only in C++20.
But equivalent functionality is available since C++11.

The idea is that we do not have an easy to copy structure, like `int`.
Let's think about the [class `Part::TopoShape`](https://github.com/FreeCAD/FreeCAD/blob/master/src/Mod/Part/App/TopoShape.h).
This class holds a `Part::TopoShape::_Shape` of type `TopoDS_Shape`.

Usually, `_Shape` is not messed with. It is not tweaked around. Usually one:
1. Reads the `_Shape`; or
2. Swaps the `_Shape` with some other `TopoDS_Shape`.

So, to do this atomically, `std::atomic<TopoDS_Shape>` would be nice!
Except that... `TopoDS_Shape` is not a good thing to be copied around (assigned).

Remember that `TopoShape` is our "model"... but we are not talking only about `TopoShape`.
We are talking about an object that holds some other object that:
1. Is usually not tweaked;
2. When it changes, it is basically replaced by a brand new (consistent) object; and
3. Is expensive to copy.

Remember that `std::atomic` does not handle you the object it holds.
It handles you a copy!

The solution is to use `std::atomic<std::shared_ptr<TopoDS_Shape>>`.
Now, we still have items 1 and 2, but 3 became:
1. The shape is usually not tweaked;
2. When it changes, it is basically replaced by a brand new (consistent) object; and
3. The `std::shared_ptr<>` is cheap to copy! It is basically just a pointer! :-)

Whenever one wants to change `_Shape` s/he has to construct a new `TopoDS_Shape`
and `std::atomic::store()` it.
This is a price to pay for multithreading.
We cannot tweak the already existent `_Shape`.
We have to construct a new one using `std::make_shared<TopoDS_Shape>` and store it.
The old shape is released from the `shared_ptr`
and might get destructed if it is the last `shared_ptr`.
But it **will not get destructed** if it is still being accessed by someone somewhere. :-)

So, a good and important part is:
> When a thread loads a copy of the `shared_ptr` from the
> `std::atomic<std::shared_ptr<TopoDS_Shape>>`,
> the `shared_ptr` will assure that the pointed resource will still be available (not deleted).

It might be that the variable `_Shape` has already changed in the meantime,
meaning the value we hold is "outdated". But it is valid and shall not cause a crash.


## Multithreaded containers

Our model now is the [list (map) of objects](https://github.com/FreeCAD/FreeCAD/blob/ed72c03df3eea864668a616dedc9a3d1c41d93be/src/App/private/DocumentP.h#L62)
of type `DocumentObject` in a `Document doc`.

This list is much more delicate then the variable `_Shape`, above.
With `_Shape`, we could simply discard everything and replace by a brand new `TopoDS_Shape`.
Now, the situation is different:
1. Many threads might want to simultaneously traverse / search the container.
2. Many threads might want to insert or delete a new `DocumentObject` in this container.
Or change its structure somehow.

While the container is traversed, it cannot have its structure changed.
Many threads might traverse it concurrently, as long as its structure does not change.
The values of each node can be changed.
But they must be made atomic somehow if one thread might be writing while others access its value.

For example,
operations that invalidate iterators are operations that change the container structure.

The template `ThreadSafeContainer<ContainerType>`
implements a (supposedly) "thread safe container".
```c_cpp
template <typename ContainerType>
class ThreadSafeContainer
{
public:
    using iterator = LockedIterator<ContainerType::iterator>;
    using const_iterator = LockedIterator<ContainerType::const_iterator>;

    iterator begin();
    ContainerType::iterator end();
    const_iterator cbegin() const;
    ContainerType::const_iterator cend() const;

    void clear();
    size_t size() const;
    // [snip: other methods]

protected:
    template<typename ContainerTypes...>
    friend UniqueLock<ContainerTypes...>;

    mutable std::shared_mutex mutex;
    ContainerType container;
};
```

To the template class `LockedIterator<ItType>`,
we pass the `ThreadSafeContainer<T>::mutex` and the iterator.
It holds a `SharedLock{mutex}`.
This way, we can iterate and traverse a `ThreadSafeContainer<std::map<std::string,AttomicSharedPtr<xxx>>> map`:
```c_cpp
  for(auto& [k,attomic]: map)
  {
    attomic = getNewShape();

    // Or...
    // Variable v is a shared_ptr<xxx>.
    auto v = attomic.load();
    // Use v->...
  }
```

To change the container structure, we shall acquire a `UniqueLock`.
The unique lock overloads `operator ->` when there is only one container associated to it.
And it overloads `operator []`:
you pass a reference to the `ThreadSafeContainer`
and it returns a reference to the actual container.

If you want to acquire a `UniqueLock` on container A and iterate through container B,
you actually have to acquire a `UniqueLock` on both A and B.
This is because, after we get a `UniqueLock` on A, we do not want the thread to block.
So, when we have a `UniqueLock`, we have to acquire them all at once.
For example:
```c_cpp
  UniqueLoc lock(map, vec); // If we do not add "map" here, the for will throw.
  for(auto& [k,attomic]: map) // SharedLock acquired here is "dummy".
  {
    lock[vec].push_back(attomic);
  }
```

