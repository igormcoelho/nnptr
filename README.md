# nnshared

## What is it?

**nnshared** is a **Not Null Shared Pointer** library (single-header) for Modern C++.

## How is it implemented?

**nnshared** basically uses `not_null` from [Guidelines Support Library - GSL](https://github.com/Microsoft/GSL) together with `std::shared_ptr` (since C++11).

## Why not just use `shared_ptr`?

A `shared_ptr` may be null, which is not desirable in many scenarios.

## Why not just use `gsl::not_null<std::shared_ptr<T>>` (for some type `T`)?

The intention is to avoid ugly error messages, when things go wrong, specially for non-experts in C++.

Suppose one wants to create a vector of five elements (with value `1`):

```
std::vector<int> v(10, 1);
```

Easy to do as an stack element, let's do it with a *shared ownership* strategy between several entities.
Although it may look the same for experienced developers (example for `std::vector<int>`):

```
gsl::not_null<std::shared_ptr<std::vector<int>>> nnsptr_1{
        std::shared_ptr<std::vector<int>>(new std::vector<int>(10, 1))
    };
//
std::cout << "v[0] = " << nnsptr_1.get().get()->at(0) << std::endl;
```

This is not as readable to non-experts...

## Why not just rename `gsl::not_null<std::shared_ptr<T>>` into some readable `nn_shared_ptr` type?

This partially resolves the problem:

```
template<class T>
using nn_shared_ptr = gsl::not_null<std::shared_ptr<T>>;
```

However, it still requires complex initialization of the passed pointer:

```
nn_shared_ptr<std::vector<int>> nnsptr_2{
        std::shared_ptr<std::vector<int>>(new std::vector<int>(10, 1))
    };

std::cout << "v[0] = " << nnsptr_2.get().get()->at(0) << std::endl;
```

Access of the internal object is still complex, and `auto` is not very helpful.

## Example with `nn::shared`

Finally, we have some simple implementation using `nn:shared` class:

```
nn::shared<std::vector<int>> nnsptr_3{ new std::vector<int>(10, 1) };
std::cout << "v[0] = " << nnsptr_3->at(0) << std::endl;
```

## Why not call it shared references or `shared_ref`?

A C++ reference is an *alias* to other variable, so meaning/purpose is different when compared to *const pointers* (which are *real* variables).

So, naming is precise: `nnshared` provides an implementation of *not null shared pointers*.


## Why not just use references?

This is a tricky question. For most scenarios, references are enough, although some uncertainty arises on memory ownership in a large project (such as [OptFrame](https://github.com/optframe/optframe)). 

Given a set of distinct component implementations, it is unfortunate that some may be passed as references, while others must be received as pointers, e.g., when a list (or vector) of components is given. This yields different treatments for each type of component (*should I delete it or not?*), thus making it hard for new users to understand each case.

By using `nn::shared` strategy, one may introduce some overheads (when compared to native references), but of negligible costs since employed strategies heavily depend on compile-time optimizations.

## Some functionality is missing (or wrong), how can I contribute?

Please open an Issue or a Pull Request, if something is missing or wrong.
Interface is quite simple, although sufficient for most of our existing scenarios. 

Feel free to suggest other interesting extensions.

## License

The initial version was developed by [@igormcoelho](https://github.com/igormcoelho) and [@fellipessanha](https://github.com/fellipessanha), released under **MIT License**. 

Copyleft 2021