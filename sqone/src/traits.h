#ifndef _SQONE_TRAITS_H_
#define _SQONE_TRAITS_H_

#include <type_traits>

namespace sqone
{
    /** For SFINAE */
    extern void* enabler;

    /** Type list */
    template <class... Args>
    struct TypeList;

    template <>
    struct TypeList<>
    {
        static constexpr size_t length = 0;
    };

    template <class Arg, class... Args>
    struct TypeList<Arg, Args...>
    {
        using Head = Arg;
        using Tails = TypeList<Args...>;
        static constexpr size_t length = Tails::length + 1;
    };

    /** Signature of function */
    template <class T>
    struct SignatureOf;

    template <class R, class... Args>
    struct SignatureOf<R(*)(Args...)> // For function pointer
    {
        using Result = R;
        using Argments = TypeList<Args...>;
    };

    template <class C, class R, class... Args>
    struct SignatureOf<R(C::*)(Args...)> // For member function pointer
    {
        using Result = R;
        using Argments = TypeList<Args...>;
    };

    template <class C, class R, class... Args>
    struct SignatureOf<R(C::*)(Args...) const> // For const member function pointer
    {
        using Result = R;
        using Argments = TypeList<Args...>;
    };

    template <class C>
    struct SignatureOf : SignatureOf<decltype(&C::operator())> // For functor
    {
    };

    /** Type of return value */
    template <class Fun>
    using Result_t = typename SignatureOf<Fun>::Result;

    /** Type list of argments */
    template <class Fun>
    using Argments_t = typename SignatureOf<Fun>::Argments;
}

#endif