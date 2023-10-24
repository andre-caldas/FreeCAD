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

We shall use the word *exclusive* instead of *unique*.

When threads just want to read the data, they get a *shared lock*.
Many threads can read at the same time.
And they have the assurance that the data will not change while it is being processed.
Those threads might block waiting for a *exclusive lock* to be released.

When a thread wants to change the data, it must acquire a *exclusive lock*.
It might block waiting for any thread holding a *shared lock* or a *exclusive lock*
to finish processing the data.

We shall implement two classes: `ExclusiveLock` and `SharedLock`.
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
We shall state some rules to be implemented by `ExclusiveLock` and `SharedLock`:
1. A `ExclusiveLock` can request a set of mutexes at once.
2. A `ExclusiveLock` is allowed to be called if the same thread already has an `ExclusiveLock`.
   But only if all mutexes being locked are in fact already locked.
   Otherwise, an `ExclusiveLock` cannot be constructed by a thread
   that already owns any kind of lock on any mutex.
   This is a programming error and should not happen. An exception will be thrown.
3. If `SharedLock` requests a mutex G and the thread already has an `ExclusiveLock U`:
    1. If U **does not own** G, throw and exception.
       This is a programming error and should not happen.
    2. If U **owns** G, do nothing. Just pretend we acquired the lock.

Those two classes, both descend from `LockPolicy`.
The common ancestor holds one `static thread_local` set of mutexes, `threadMutexes`,
and one `static thread_local bool isExclusive` to indicate if the already locked mutexes
are exclusive or not.
Those sets indicate for which mutexes we hold an `ExclusiveLock` and a `SharedLock`.
This way we can control if the program is complying with the rules above.

**Cavet:** the `SharedLock` after an `ExclusiveLock` is basically "useless" (although it does make sense).
If you destroy `ExclusiveLock` and do not destroy `SharedLock`,
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
```cpp
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
    friend ExclusiveLock<ContainerTypes...>;

    mutable std::shared_mutex mutex;
    ContainerType container;
};
```

To the template class `LockedIterator<ItType>`,
we pass the `ThreadSafeContainer<T>::mutex` and the iterator.
It holds a `SharedLock{mutex}`.
This way, we can iterate and traverse a `ThreadSafeContainer<std::map<std::string,AttomicSharedPtr<xxx>>> map`:
```cpp
  for(auto& [k,attomic]: map)
  {
    attomic = getNewShape();

    // Or...
    // Variable v is a shared_ptr<xxx>.
    auto v = attomic.load();
    // Use v->...
  }
```

To change the container structure, we shall acquire an `ExclusiveLock`.
The exclusive lock overloads `operator ->` when there is only one container associated to it (not implemented!).
And it overloads `operator []`:
you pass a (reference to the) `ThreadSafeContainer`
and it returns a reference to the actual container.

If you want to acquire an `ExclusiveLock` on container A and iterate through container B,
you actually have to acquire an `ExclusiveLock` on both A and B.
This is because, after we get an `ExclusiveLock` on A, we do not want the thread to block.
So, when we have an `ExclusiveLock`, we have to acquire them all at once.
For example:
```cpp
  ExclusiveLock lock(map, vec); // If we do not add "map" here, the for will throw.
  for(auto& [k,attomic]: map) // SharedLock acquired here is "dummy".
  {
    lock[vec].push_back(attomic);
  }
```


## Deadlock free

We shall say that a *locking strategy* is *strongly deadlock free* when
while it holds its mutexes, the code is assured to never try to lock any other mutex ever.
This happens, for example, when `std::atomic<int>` attempts to increment an integer.
It blocks while it has no locks.
After it acquires the lock, it increments `i` and then releases the lock.

We shall say that a *locking strategy* is *deadlock free*
when it never blocks, waiting for any mutexes after some mutex has already been acquired,
except for those that are using a *strongly deadlock free* strategy.

Also, let's call *alien lock* any lock not managed by `LockPolicy`.

Finally, when thread A blocks and waits for thread B
we call it a *thread wait*.

Let's make some hypothesis.
By this, we might understand that those are rules
that the FreeCAD developer (and the libraries) shall not violate.

> **Hypothesis 1:**<br/>
> We shall assume that except for `SharedLock`, `ExclusiveLock`, and *thread waits*
> FreeCAD (or its libraries, like OCCT) only uses *deadlock free* strategies.

For example, we shall never wait for a thread that might wait for a mutex.

> **Hypothesis 2:**<br/>
> We shall assume that `SharedLock` and `ExclusiveLock` are never instantiated
> while the thread already holds an alien lock.

> **Hypothesis 3:**<br/>
> We shall never wait for a thread while holding any `SharedLock` or `ExclusiveLock`.

> **Hypothesis 4:**<br/>
> We shall assume that `SharedLock` and `ExclusiveLock`
> are deleted **exactly** when the scope they were created ceases to exist.
> They are not stored (live longer then the context)
> and they are not destructed before the end of the context.

To complement *hypothesis 4*:

> **Hypothesis 5:**<br/>
> We shall assum that `SharedLock` and `ExclusiveLock` always "live"
> in the same thread where they were created.
> They shall not be accessed by other threads.

The two classes `SharedLock` and `ExclusiveLock` not being movable
might help avoid violation of *hypothesis 4* and *hypothesis 5*.

> **Lemma (`LockPolicy`):**<br/>
> The above *hypothesis 5* makes `LockPolicy` able to correctly know
> which locks managed by `LockPolicy` this thread already has,
> and if those locks are *shared* or *exclusive*.

> **Implementation property A:**<br/>
> `LockPolicy` does not allow a `SharedLock` to be constructed
> when `LockPolicy::threadMutexes` is not empty and `isExclusive == true`.
> After constructed, `isExclusive == false`.

> **Implementation property B:**<br/>
> `LockPolicy` does not allow an `ExclusiveLock`
> when `LockPolicy::threadMutexes` is not empty and `isExclusive == false`.
> After constructed, `isExclusive == true`.

>> **Theorem X (`ExclusiveLock`):**<br/>
>> Under the hypothesis above, `ExclusiveLock` has a *deadlock free strategy*.
>> In particular, it never *deadlocks*.
>
> **Demonstration:**<br/>
> In fact, when an `ExclusiveLock` is created, `LockPolicy` warrants that:
> 1. Nothing is actually locked; or
> 2. This thread did not hold any locks managed by `LockPolicy` prior to `ExclusiveLock` creation.
>
> In case 1, there is nothing to prove.
> Let's treat case 2.
>
> For the *current thread* `LockPolicy::threadMutexes` is not empty anymore
> and `isExclusive == true`.
> The *Lemma** shows that this information will be consistent all the time.
> We need to prove that the *current thread* will not block waiting for any locks
> with policy different from *strong deadlock free*.
>
> The `LockPolicy` *implementation property A*
> warrants that the *current thread* will not wait
> for any other `ExclusiveLock` or any `SharedLock`.
>
> Now, *hypothesis 3* says that we will not go into a *thread wait*.
> And *hypothesis 1* says the only possible waits are *strongly deadlock free*.
> (q.e.d)

>> **Theorem S (`SharedLock`):**<br/>
>> Under the hypothesis above, `SharedLock` never *deadlocks*.
>
> **Demonstration:**<br/>
> When `SharedLock` is created,
> it might block and wait only for some `ExclusiveLock` to be released.
> But *theorem X* says that `ExclusiveLock` never *deadlocks*.
> And when created, a `SharedLock` never blocks waiting for another `SharedLock`
> per definition of *shared lock*.
> Therefore, the locks are eventually acquired. (with probability 1, one might say)
>
> When a `SharedLock` is instantiated it might happen that nothing actually gets locked.
> In this case, it obviously does not *deadlock*.
> So, we shall assume that after instantiation, `threadMutexes` is not empty.
> Also, *implementation property A* implies that `isExclusive == false`.
>
> While the `SharedLock` exists, *implementation property B*
> says that the *current thread* will not instantiate and `ExclusiveLock`.
> Notice that *hypothesis 3* says that we will not go into a *thread wait*.
> The begining of this proof shows that a nested `SharedLock` instantiation will never block.
> So, *hypothesis 1* says the only possible waits are *strongly deadlock free*.
> Therefore, the thread shall not block until it eventually releases the `SharedLock`.
> (q.e.d.)
