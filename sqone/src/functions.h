#ifndef _SQONE_FUNCTIONS_H_
#define _SQONE_FUNCTIONS_H_

#include "stackhandle.h"
#include "traits.h"
#include <squirrel.h>
#include <type_traits>
#include <utility>
#include <cstring>

namespace sqone
{
    // Closure for function
    /// TODO: We can not notify errors
    template <class Fun>
    SQInteger functionClosure(HSQUIRRELVM vm)
    {
        Fun* fun;
        sq_getuserdata(vm, -1, reinterpret_cast<SQUserPointer*>(&fun), nullptr);
        return call(vm, bindArgments(vm, *fun));
    }

    /** Define function */
    template <class Fun>
    bool defineFunction(HSQUIRRELVM vm, const SQChar* name, Fun fun)
    {
        StackResetter reset(vm);

        // Create function closure
        sq_pushroottable(vm);
        sq_pushstring(vm, name, -1);
        const auto usrData = sq_newuserdata(vm, sizeof(fun));
        std::memcpy(usrData, &fun, sizeof(fun));
        sq_newclosure(vm, functionClosure<Fun>, 1);
        return SQ_SUCCEEDED(sq_newslot(vm, -3, SQFalse));
    }

    namespace detail
    {
        // For callFunction()
        template <class... Args>
        bool callFunctionPrepare(HSQUIRRELVM vm, const SQChar* name, Args... args)
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
    template <class Result, typename std::enable_if_t<!std::is_void<Result>::value>*& = enabler, class... Args>
    std::pair<bool, Result> callFunction(HSQUIRRELVM vm, const SQChar* name, Args... args) // For has return value
    {
        StackResetter reset(vm);
        detail::callFunctionPrepare(vm, name, args...);

        // Call
        if (SQ_SUCCEEDED(sq_call(vm, sizeof...(Args) + 1, SQTrue, SQTrue)))
        {
            return getValue<Result>(vm, -1);
        }
        return std::make_pair(false, Result());
    }

    template <class Result, typename std::enable_if_t<std::is_void<Result>::value>*& = enabler, class... Args>
    std::pair<bool, std::nullptr_t> callFunction(HSQUIRRELVM vm, const SQChar* name, Args... args) // For no return value
    {
        StackResetter reset(vm);
        detail::callFunctionPrepare(vm, name, args...);

        // Call
        if (SQ_SUCCEEDED(sq_call(vm, sizeof...(Args) + 1, SQFalse, SQTrue)))
        {
            return std::make_pair(true, nullptr);
        }
        return std::make_pair(false, nullptr);
    }
}

#endif