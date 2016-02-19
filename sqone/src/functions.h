#ifndef _SQONE_FUNCTIONS_H_
#define _SQONE_FUNCTIONS_H_

#include "stackhandle.h"
#include <squirrel.h>
#include <type_traits>
#include <utility>

namespace sqone
{
    namespace detail
    {
        extern void* enabler;

        // For callFunction()
        template <class... Args>
        bool callFunctionImpl(HSQUIRRELVM vm, const SQChar* name, Args... args)
        {
            // Get closure
            sq_pushroottable(vm);
            sq_pushstring(vm, name, -1);
            if (SQ_SUCCEEDED(sq_get(vm, -2)))
            {
                // Push argments
                sq_pushroottable(vm);
                pushArgments(vm, args...);
                return true;
            }
            return false;
        }
    }

    /** Call function */
    template <class Result, typename std::enable_if_t<!std::is_void<Result>::value>*& = detail::enabler, class... Args>
    std::pair<bool, Result> callFunction(HSQUIRRELVM vm, const SQChar* name, Args... args) // For has return value
    {
        StackResetter reset(vm);
        detail::callFunctionImpl(vm, name, args...);

        // Call
        if (SQ_SUCCEEDED(sq_call(vm, sizeof...(Args) + 1, SQTrue, SQTrue)))
        {
            return getValue<Result>(vm, -1);
        }
        return std::make_pair(false, Result());
    }

    template <class Result, typename std::enable_if_t<std::is_void<Result>::value>*& = detail::enabler, class... Args>
    std::pair<bool, std::nullptr_t> callFunction(HSQUIRRELVM vm, const SQChar* name, Args... args) // For no return value
    {
        StackResetter reset(vm);
        detail::callFunctionImpl(vm, name, args...);

        // Call
        if (SQ_SUCCEEDED(sq_call(vm, sizeof...(Args) + 1, SQFalse, SQTrue)))
        {
            return std::make_pair(true, nullptr);
        }
        return std::make_pair(false, nullptr);
    }
}

#endif