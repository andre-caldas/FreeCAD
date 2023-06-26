# Accessing variables

In FreeCAD,
data is organized in a *document tree*.
Theoretically,
it is possible to chase pieces of information by following a *path* trough
the document tree.


## The *path*

A *path* represents a sequence of objects that if followed trough (*resolved*),
result in an object to be accessed, **or** a **list** of objects.
So, the result of resolving a path is a `Accessor::Reference`:
an iterable object.


### Path string representation

A path has a *string representation*.
The string is split into pieces that are separated by a "dot": `.`.
Each piece represents a derived class of `Accessor::PathComponent`.
There are three kinds of `Accessor::PathComponent`.

1. `SimplePathComponent`: a *string* not containing `[` or `{`.
2. `ArrayPathComponent`: a *string* of the shape `abc[...]`, where `...` represents a range of indexes. The `abc` is a *simple path component* that identifies the *array*.
3. `MapPathComponent` a *string* of the shape `abc{...}`, where `...` represents a list of keys. the `abc` is a *simple path component* that identifies an object that implements a getter and a setter for each *key*.

The ranges and lists can be *static* (like [2:15:3]), can contain expressions or other paths. Everything boils down to *expressions*, because they can represent constants and can manipulate other paths.


### Path components

The *path* is a sequence of *components*: `Accessor::PathComponent`.
Each component is an object of a class derived from `Accessor::PathComponent` that is able to access a specific variable or list of variables of a certain object.
The last component in the sequence is called a *terminal component*.


## Referenced objects

### Non-terminal components

The objects referenced by **non-terminal** components **must** derive from `Accessor::ChainableObject`.
Such a `ChainableObject` can be queried to resolve the *next object*.
This is implemented by the *virtual method*:
```
  // In Accessor::ChainableObject.
  virtual Reference resolveComponent(std::string name) {/*throw 'not found' exception*/}
```

### Terminal components

Terminal components are simply "not non-terminal".
They can be anything, as long as there is an `Accessor::Reference` specialized to access its data.


## Resolving a *path*

Given a "root" `Accessor::ChainableObject`,
a *path* can be resolved by recursively iterating through every component in the path.

One path represents many objects, because some components are ranges of integers or lists of strings.
The path *iterator* is a **stack** of iterators.
Whenever the topmost iterator reaches its end,
it should be popped and the next iterator of the lower level, advanced.


## Exceptions

Resolving a path might throw:

1. `Base::BadFormatError` when the path string is grammatically incorrect (cannot be parsed).
2. `Base::AccessViolation` for forbidden references. This could be, for example, to avoid circular references.
3. `Base::IndexError` when some range item is out of the array bounds.
4. `Base::NameError` when a name or a key is not found.

