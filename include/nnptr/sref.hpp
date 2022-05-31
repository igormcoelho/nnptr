
#ifndef NNPTR_SREF_HPP
#define NNPTR_SREF_HPP
// ====================================================
// Not Null Shared Pointer (nnptr::sref)
// github.com/igormcoelho/nnptr
// Maintainer: Igor M. Coelho
// First version by  Igor M. Coelho & Fellipe Pessanha
// MIT License (2021)
// ====================================================

// This library depends on...

// ========================================================================
// NotNull is an alternative to gsl::not_null, but with NDEBUG removing all
// 'NotNull' during Release (or use flag NO_NNPTR_CHECKS instead of NDEBUG).
// ========================================================================

#ifdef NDEBUG
#define NO_NNPTR_CHECKS
#endif

#include <memory> // shared_ptr

// =============
// For nnptr::NotNull
// =============

#include <algorithm>    // for forward
#include <iosfwd>       // for ptrdiff_t, nullptr_t, ostream, size_t
#include <memory>       // for shared_ptr, unique_ptr
#include <system_error> // for hash
#include <type_traits>  // for enable_if_t, is_convertible, is_assignable

// =========================
// begin nnptr::NotNull part
// =========================

// =============================================
// Same assumptions are valid from gsl::not_null
// =============================================
//
// nnptr::NotNull restricts a pointer to only hold non-null values (enforced during
// Debug time, not Release)
//
// Has zero size overhead over T (during both Debug and Release)
//
// For NotNull<T>, if T is a pointer (i.e. T == U*) then
// - allow construction from U*
// - disallow construction from nullptr_t
// - disallow default construction
// - ensure construction from null U* fails
// - allow implicit conversion to U*
//

// ==============================================
// code for NotNull (compatible with gsl::not_null)
// reference: https://github.com/microsoft/GSL/blob/b26f6d5ec7b043f9d459c1dfdd6da4d930d4e9b4/include/gsl/pointers
// date: 17-jun-2021
// ==============================================

namespace nnptr {

namespace details {
template<typename T, typename = void>
struct is_comparable_to_nullptr : std::false_type
{
};

template<typename T>
struct is_comparable_to_nullptr<
  T,
  std::enable_if_t<std::is_convertible<decltype(std::declval<T>() != nullptr), bool>::value>>
  : std::true_type
{
};
} // namespace details

template<class T>
class NotNull
{
public:
   static_assert(details::is_comparable_to_nullptr<T>::value, "T cannot be compared to nullptr.");

   template<typename U, typename = std::enable_if_t<std::is_convertible<U, T>::value>>
   constexpr NotNull(U&& u)
     : ptr_(std::forward<U>(u))
   {
#ifndef NO_NNPTR_CHECKS
      if (ptr_ == nullptr)
         std::terminate();
#endif
   }

   template<typename = std::enable_if_t<!std::is_same<std::nullptr_t, T>::value>>
   constexpr NotNull(T u)
     : ptr_(std::move(u))
   {
#ifndef NO_NNPTR_CHECKS
      if (ptr_ == nullptr)
         std::terminate();
#endif
   }

   template<typename U, typename = std::enable_if_t<std::is_convertible<U, T>::value>>
   constexpr NotNull(const NotNull<U>& other)
     : NotNull(other.get())
   {}

   NotNull(const NotNull& other) = default;
   NotNull& operator=(const NotNull& other) = default;
   constexpr std::conditional_t<std::is_copy_constructible<T>::value, T, const T&> get() const
   {
#ifndef NO_NNPTR_CHECKS
      if (ptr_ == nullptr)
         std::terminate();
#endif
      return ptr_;
   }

   constexpr operator T() const { return get(); }
   constexpr decltype(auto) operator->() const { return get(); }
   constexpr decltype(auto) operator*() const { return *get(); }

   // prevents compilation when someone attempts to assign a null pointer constant
   NotNull(std::nullptr_t) = delete;
   NotNull& operator=(std::nullptr_t) = delete;

   // unwanted operators...pointers only point to single objects!
   NotNull& operator++() = delete;
   NotNull& operator--() = delete;
   NotNull operator++(int) = delete;
   NotNull operator--(int) = delete;
   NotNull& operator+=(std::ptrdiff_t) = delete;
   NotNull& operator-=(std::ptrdiff_t) = delete;
   void operator[](std::ptrdiff_t) const = delete;

private:
   T ptr_;
};

template<class T>
std::ostream&
operator<<(std::ostream& os, const NotNull<T>& val)
{
   os << val.get();
   return os;
}

template<class T, class U>
auto
operator==(const NotNull<T>& lhs,
           const NotNull<U>& rhs) noexcept(noexcept(lhs.get() == rhs.get()))
  -> decltype(lhs.get() == rhs.get())
{
   return lhs.get() == rhs.get();
}
// should prevent comparisons with nullptr_t (from NotNull)
template<class T>
bool
operator==(const NotNull<T>& lhs, std::nullptr_t nptr) = delete;

template<class T, class U>
auto
operator!=(const NotNull<T>& lhs,
           const NotNull<U>& rhs) noexcept(noexcept(lhs.get() != rhs.get()))
  -> decltype(lhs.get() != rhs.get())
{
   return lhs.get() != rhs.get();
}
// should prevent comparisons with nullptr_t (from NotNull)
template<class T>
bool
operator!=(const NotNull<T>& lhs, std::nullptr_t nptr) = delete;

template<class T, class U>
auto
operator<(const NotNull<T>& lhs,
          const NotNull<U>& rhs) noexcept(noexcept(lhs.get() < rhs.get()))
  -> decltype(lhs.get() < rhs.get())
{
   return lhs.get() < rhs.get();
}

template<class T, class U>
auto
operator<=(const NotNull<T>& lhs,
           const NotNull<U>& rhs) noexcept(noexcept(lhs.get() <= rhs.get()))
  -> decltype(lhs.get() <= rhs.get())
{
   return lhs.get() <= rhs.get();
}

template<class T, class U>
auto
operator>(const NotNull<T>& lhs,
          const NotNull<U>& rhs) noexcept(noexcept(lhs.get() > rhs.get()))
  -> decltype(lhs.get() > rhs.get())
{
   return lhs.get() > rhs.get();
}

template<class T, class U>
auto
operator>=(const NotNull<T>& lhs,
           const NotNull<U>& rhs) noexcept(noexcept(lhs.get() >= rhs.get()))
  -> decltype(lhs.get() >= rhs.get())
{
   return lhs.get() >= rhs.get();
}

// more unwanted operators
template<class T, class U>
std::ptrdiff_t
operator-(const NotNull<T>&, const NotNull<U>&) = delete;
template<class T>
NotNull<T>
operator-(const NotNull<T>&, std::ptrdiff_t) = delete;
template<class T>
NotNull<T>
operator+(const NotNull<T>&, std::ptrdiff_t) = delete;
template<class T>
NotNull<T>
operator+(std::ptrdiff_t, const NotNull<T>&) = delete;

} // namespace gsl

namespace std {
template<class T>
struct hash<nnptr::NotNull<T>>
{
   std::size_t operator()(const nnptr::NotNull<T>& value) const { return hash<T>{}(value.get()); }
};
} // namespace std

// ======================
// begin nnptr::sref part
// ======================

namespace nnptr {
//
template<typename T>
class sref
{
   using shared_type = T;

public:
   NotNull<std::shared_ptr<T>> data_;

   // this constructor can be used to "move" into new shared_ptr versions
   // this requires a copy constructor over the type X (must be concrete)
   template<
     class X,
     typename =
       typename std::enable_if<std::is_convertible<X*, T*>::value>::type,
     typename =
       typename std::enable_if<std::is_copy_constructible<X>::value>::type>
   sref(X&& other)
     : data_{ std::shared_ptr<T>{ new X(other) } }
   {}

   // this is for existing references (must have copy constructor)
   template<
     class X,
     typename =
       typename std::enable_if<std::is_convertible<X*, T*>::value>::type,
     typename =
       typename std::enable_if<std::is_copy_constructible<X>::value>::type>
   sref(const X& other)
     : data_{ std::shared_ptr<T>{ new X(other) } }
   {}

   sref(const sref<T>& other)
     : data_{ other.data_ }
   {}

   sref(const sref<T>&& corpse)
     : data_{ corpse.data_ }
   {}

   sref(std::shared_ptr<T>& data)
     : data_{ data }
   {}

   sref(T* data)
     : data_{ std::shared_ptr<T>{ data } }
   {}

   // disallow explicit nullptr
   sref(std::nullptr_t data) = delete;

   T* operator->() { return data_.get().get(); }

   const T* operator->() const { return data_.get().get(); }

   T& operator*()
   {
      // return dereferenced shared object
      return *data_;
   }

   const T& operator*() const
   {
      // return dereferenced shared object
      return *data_;
   }

   T& get()
   {
      // return dereferenced shared object
      return *data_;
   }

   const T& get() const
   {
      // return dereferenced shared object
      return *data_;
   }

   // method 'sptr' should be taken only in extreme/compatibility cases
   std::shared_ptr<T> sptr() const
   {
      return data_;
   }

   sref<T>& operator=(const sref<T>& other)
   {
      // self-reference
      if (this == &other)
         return *this;

      // ASSUME NON-NULL FOR operator=
      (*this->data_) = (*other.data_);

      return *this;
   }

   operator T&()
   {
      // return dereferenced shared object
      return *data_;
   }

   template<class Y, typename = typename std::enable_if<std::is_convertible<T*, Y*>::value>::type>
   operator sref<Y>() // explicit? not necessary... (until now)
   {
      std::shared_ptr<Y> py = data_.get(); // remove encapsulation from 'NotNull'
      return py;
   }
};

} // namespace nn

#endif // NNPTR_sref_HPP
