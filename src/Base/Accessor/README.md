# Access objects and their (exported) members

The *Accessor* infrastructure allows one to use a *string path*
to identify variables, and get access to them.
This is very similar to *ObjectIdentifier*, but cleaner.
We do not use generic types like *void\**, *boost::any* or *std::any*.
We do not use python.

We also do not use raw pointers.
Instead, we use `std::shared_ptr`.
This scheme, in my opinion,
is very suited to work with resources that are shared between different objects.


## Referenced objects

The *Accessor* infrastructure is very declarative.
Objects that can be referenced declare themselves as so,
by subclassing `Accessor::ReferencedObject`.
If this referenced object *exports* some of its members,
it declares so by subclassing the template `Accessor::IExport<T>`.
For example:
```
class Line
    : public ReferencedObject
    , public IExport<Point>
    , public IExport<double>
{
public:
    ...
    double* IExport<double>::resolve_ptr(token_iterator& start, const token_iterator& end) override;
    Point* IExport<Point>::resolve_ptr(token_iterator& start, const token_iterator& end) override;
    Point* IExport<Point>::resolve_share(token_iterator& start, const token_iterator& end) override;

private:
    double value_i_export;
    Point point_i_export;
    std::shared_ptr<Point> point_i_share;
};
```

To share a member that is guaranteed to exist as long as the object exists,
implement `IExport<>::resolve_ptr`.
To share a member that is owned through a `shared_ptr`,
implement `IExport<>::resolve_share`.


## References to objects and members

The template class `Accessor::ReferenceTo<T>`
represents a reference to a variable of type `T`
that can be accessed through the *Accessor scheme*.
An object of type `Accessor::ReferenceTo<T>` is instantiated by passing to the constructor:
1. A `std::shared_ptr<ReferencedObject>` that holds a known *root object*.
2. A sequence of "strings" (or tag/uuid) that represent a *path* to the shared resource.

For convenience, there is an `Accessor::Chainable` class,
that subclasses `Accessor::ReferencedObject` and `Accessor::IExport<ReferencedObject>`.


## The path

The referenced objects have an `Accessor::NameAndTag name_and_tag`.
This gives to each of them a unique id (we use `boost::uuid` by now).
And it can also have an optional *name*.
By the way... this optional name cannot "look like" a *tag*.

A *path* is basically a *(probably chainable) root object*
and a sequence of strings (*names* or *tags*) that identifies a chain of objects.
But the *path* can be longer than that!
When we reach the last `Accessor::ReferencedObject` in the chain,
the remaining part of the *path* identifies the variable this `ReferencedObject` exports.


## Specifying a path

For example:
```
    ReferenceTo<double> ref_len(document_root, "walls", "sketch", "room wall", "length");
    ReferenceTo<double> ref_x(document_root, "walls", "sketch", "room wall", "start_point", "x");
```

Notice how declarative this is!!!
The object that exports a `double` declares it by subclassing `IExport<double>`.
It then implements a `double* resolve_ptr(...)` method.
A function that expects a *reference to a double*
states that **(usually) at compile time**:
```
    void i_use_a_double(Accessor::ReferenceTo<double> ref);
```
There is no messing around with `boost::any` or the like.
There are no chains of `if(...TypeId...)` tests.
What needs to be treated is the case where *path (to a double)* does not resolve.
We never get something that is not a double to deal with!


## Resolving a path

All `ReferencedObject` needs to be handled with `std::shared_ptr`.
After resolving a `ReferenceTo<double>`, for example,
we hold a `std::shared_ptr` to the last `ReferencedObject` on the path.
We call it a *lock*.
And we also get a `double* reference`.
This pointer is (shall be) guaranteed to be valid as long as we hold the lock,
although later the pointed variable might not correspond to the *path*
if the objects are relocated or renamed on the document tree.
In particular, an `IExport<T>` object cannot export a raw pointer to something
that might get destroyed.

The *Accessor infrastructure* does not inform references about changes
on the tree structure. But the reference can be "refreshed".
In this case, it is resolved again and a boolean is set to inform if
the pointed address has changed since the last time.
For example, *FreeCAD* has signals and *recompute* calls that might be used.

PS: Technically, the `lock` might not be on the "last" `ReferencedObject` if there is an object that is not a `ReferencedObject` that precedes it.


# Future work

1. Allow *arrays*, *ranges* and *maps* in paths.
2. Allow one to iterate over a path that has *arrays*, *ranges* and/or *maps*.
3. Allow expressions inside paths.
4. Allow references inside paths.

