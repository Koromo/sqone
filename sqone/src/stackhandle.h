#ifndef _SQONE_STACKHANDLE_H_
#define _SQONE_STACKHANDLE_H_

#include <squirrel.h>
#include <utility>

namespace sqone
{
    /** Push value into stack */
    inline void pushValue(HSQUIRRELVM vm, const SQChar* str) // For string
    {
        sq_pushstring(vm, str, -1);
    }

    inline void pushValue(HSQUIRRELVM vm, int n) // For integer
    {
        sq_pushinteger(vm, n);
    }

    inline void pushValue(HSQUIRRELVM vm, float f) // For floating point
    {
        sq_pushfloat(vm, f);
    }

    inline void pushValue(HSQUIRRELVM vm, bool b) // For boolean
    {
        sq_pushbool(vm, b);
    }

    /** Get value from stack */
    template <class T>
    std::pair<bool, T> getValue(HSQUIRRELVM vm, int id);

    template <>
    inline std::pair<bool, const SQChar*> getValue<const SQChar*>(HSQUIRRELVM vm, int id) // For string
    {
        auto pair = std::make_pair<bool, const SQChar*>(false, nullptr);
        pair.first = SQ_SUCCEEDED(sq_getstring(vm, id, &pair.second));
        return pair;
    }

    template <>
    inline std::pair<bool, int> getValue<int>(HSQUIRRELVM vm, int id) // For integer
    {
        auto pair = std::make_pair<bool, int>(false, 0);
        pair.first = SQ_SUCCEEDED(sq_getinteger(vm, id, &pair.second));
        return pair;
    }

    template <>
    inline std::pair<bool, float> getValue<float>(HSQUIRRELVM vm, int id) // For floating point
    {
        auto pair = std::make_pair<bool, float>(false, 0);
        pair.first = SQ_SUCCEEDED(sq_getfloat(vm, id, &pair.second));
        return pair;
    }

    template <>
    inline std::pair<bool, bool> getValue<bool>(HSQUIRRELVM vm, int id) // For boolean
    {
        SQBool d;
        if (SQ_SUCCEEDED(sq_getbool(vm, id, &d)))
        {
            return std::make_pair(true, !!d);
        }
        return std::make_pair(false, false);
    }

    /** Push argments into stack from first argment */
    inline void pushArgments(HSQUIRRELVM vm) // No argments
    {
    }

    template <class Arg, class... Args>
    void pushArgments(HSQUIRRELVM vm, Arg arg, Args... args) // Any argments
    {
        pushValue(vm, arg);
        pushArgments(vm, args...);
    }

    /** Stack status resetter */
    struct StackResetter
    {
        HSQUIRRELVM vm_;
        int top_;

        explicit StackResetter(HSQUIRRELVM vm)
            : vm_(vm)
            , top_(sq_gettop(vm))
        {
        }

        ~StackResetter()
        {
            sq_settop(vm_, top_);
        }
    };
}

#endif