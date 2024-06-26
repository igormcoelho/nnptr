# nnptr

## What is it?

**nnptr** is a **Not Null Shared Pointer** (or **shared reference**) library ([single-header](./src/nnptr/sref.hpp)) for Modern C++.

## Demo

In this library we use `nnptr::sref` to denote a "shared reference", which is "more or less" the
interpretation of "a not-null shared pointer". 
In the same way that most compilers translate `int&` as `int*`, we can use `sref<int>` as `int&`,
with the extra benefit of having "shared ownership" semantics 
(so, it's guaranteed that memory will never dangle for sref, differently from a native reference).

```   
#include <iostream>          // just for printing in demo
#include <nnptr/sref.hpp>    // single header 

using nnptr::sref;           // simplify notation using 'sref'

// one can read 'foo' as: double& foo(int& si) { ... }

sref<double> foo(sref<int> si) {
   double d = si;                // gets copy of element using implicit operator*
   return new double{ d };       // creates new 'sref' by passing pointer
}

int main() {
   auto sd = foo(10);            // converts 10 into a shared_ptr<int> with value 10
   std::cout << sd << std::endl; // prints 10.0
   sd++;                         // this is NOT pointer arithmetic, but operator double&
   std::cout << *sd << std::endl;// prints 11.0 (operator* is optional here)
   return 0;                     // no memory is leaked or lost
}
```

## FAQ

### How is it implemented?

**nnptr** basically uses `not_null` from [Guidelines Support Library - GSL](https://github.com/Microsoft/GSL) together with `std::shared_ptr` (since C++11). No GSL support/dependency is needed here, REALLY just add the header, and that's all.

### Why not just use `shared_ptr`?

A `shared_ptr` may be null, which is not desirable in many scenarios.

### Why not just use `gsl::not_null<std::shared_ptr<T>>` (for some type `T`)?

The intention is to avoid ugly error messages, when things go wrong, specially for non-experts in C++.

Suppose one wants to create a vector of five elements (with value `1`):

```
std::vector<int> v(5, 1);
```

Easy to do as an stack element, let's do it with a *shared ownership* strategy between several entities (such as `std::vector<int>&` working as `std::shared_ptr`).
Although it may look the same for experienced developers (example for `std::vector<int>`):

```
gsl::not_null<std::shared_ptr<std::vector<int>>> nnsptr_1{
        std::shared_ptr<std::vector<int>>(new std::vector<int>(5, 1))
    };
// getting the first element in vector
std::cout << "v[0] = " << nnsptr_1.get().get()->at(0) << std::endl;
```

This is not as readable to non-experts...

### Why not just rename `gsl::not_null<std::shared_ptr<T>>` into some readable `nn_shared_ptr` type?

This partially resolves the problem:

```
template<class T>
using nn_shared_ptr = gsl::not_null<std::shared_ptr<T>>;
```

However, it still requires complex initialization of the passed pointer:

```
nn_shared_ptr<std::vector<int>> nnsptr_2{
        std::shared_ptr<std::vector<int>>(new std::vector<int>(5, 1))
    };
// getting the first element in vector
std::cout << "v[0] = " << nnsptr_2.get().get()->at(0) << std::endl;
```

Access of the internal object is still complex, and `auto` is not very helpful.

### And how to do the same with `nnptr::sref<T>` as a readable `sref` type?

```
nnptr::sref<std::vector<int>> nnsptr_3{ std::vector<int>(5, 1) };
// getting the first element in vector
std::cout << "v[0] = " << nnsptr_3->at(0) << std::endl;
```

Perfect.

### Is `gsl::not_null` necessary here?

Not anymore. We decided to create our own version of `gsl::not_null` named `nnptr::NotNull`.
One difference is that we disable all checks during `-DNDEBUG`, bringing absolute Zero Overhead to Release/Production.

### What's the difference of `gsl::not_null` and `nnptr::NotNull`?

Basically, we disallow comparisons with nullptr (see operator== special cases) and disable checks with `-DNDEBUG`.
The rest of the logic is kept pretty much the same as `gsl::not_null`, for compatibility reasons.

### Which C++ Standard is required?

Currently, you will need `C++14` or newer.

### How to implement this with `nnptr::sref`?

Finally, we have some simple implementation using `nnptr::sref` class:

```
nnptr::sref<std::vector<int>> nnsptr_3{ new std::vector<int>(10, 1) };
std::cout << "v[0] = " << nnsptr_3->at(0) << std::endl;
```

### Can I pass `nullptr` into `nnptr::sref`?

**No.** You cannot do it (by contract).

Something like this would fail in compile-time:

```
nnptr::sref<int> p { nullptr };  // compile time error
```

Something like this would fail in runtime:

```
int* p_int = nullptr;
nnptr::sref<int> p { p_int };    // runtime error
```


### Why not call it shared references or `shared_ref`?

It's a possibility, but `sref` is shorter.
Note that a C++ reference `T&` is an *alias* to other variable, so meaning/purpose is slightly different when compared to *const pointers* like `T* const` (which are *real* pass-by-copy objects).

The semantics adopted here are the same as references: no nullability allowed and including shared ownership semantics (with reference counting).

So, `sref` provides an implementation of *shared references* or even *not null shared pointers*, if you prefer this way.


### Why not just use references?

This is a tricky question. For most scenarios, references are enough, although some uncertainty arises on memory ownership in a large project (such as [OptFrame](https://github.com/optframe/optframe)). 

Given a set of distinct component implementations, it is unfortunate that some may be passed as references, while others must be received as pointers, e.g., when a list (or vector) of components is given. This yields different treatments for each type of component (*should I delete it or not?*), thus making it hard for new users to understand each case.

By using `nnptr::sref` strategy, one may introduce some overheads (when compared to native references), but of negligible costs since employed strategies heavily depend on compile-time optimizations.

### Can you give me a more concrete example of usage?

Please take a look at [demo.cpp](./demo/demo.cpp), it has tiny examples.

A more detailed example (that motivated this whole thing!) is discussed next.
Imagine `Company` class that holds a `Person` as a *manager*, and several `Person` as *employees*:


```{.cpp}
class Person
{
public:
};

class Company
{
public:
   Company(Person& _manager, std::vector<Person*> _employees)
     : manager{ _manager }
     , employees{ _employees }
   {
   }

private:
   Person& manager;                // shared ownership?
   std::vector<Person*> employees; // shared ownership?
};
```

In this scenario, both *manager* and *employees* may have distinct storages (*manager* on the stack and each *employee* on the heap). It is hard to realize memory ownership in this scenario, so as to prevent empty *employee* pointer to be passed.

The intention is clear when explicitly defining memory model with `nnptr::sref`:

```
class Company2
{
public:
   Company2(nnptr::sref<Person>& _manager, std::vector<nnptr::sref<Person>>& _employees)
     : manager{ _manager }
     , employees{ _employees }
   {
   }

private:
   nnptr::sref<Person> manager;                // shared ownership!
   std::vector<nnptr::sref<Person>> employees; // shared ownership!
};
```

## Some functionality is missing (or wrong), how can I contribute?

Please open an Issue or a Pull Request, if something is missing or wrong.
Interface is quite simple, although sufficient for most of our existing scenarios. 

Feel free to suggest other interesting extensions.

## License

This project is maintained by [@igormcoelho](https://github.com/igormcoelho), under **MIT License**. 

Copyleft 2021
