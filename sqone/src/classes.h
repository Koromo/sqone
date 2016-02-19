#ifndef _SQONE_CLASSES_H_
#define _SQONE_CLASSES_H_

#include "functions.h"
#include "stackhandle.h"
#include "traits.h"
#include <squirrel.h>
#include <functional>
#include <cstring>

namespace sqone
{
    namespace detail
    {
        // Closure for constructor
        template <class Class, class... Args>
        SQInteger constructorClosure(HSQUIRRELVM vm)
        {
            auto construct = [](Args... args)
            {
                return new Class(args...);
            };

            auto destruct = [](SQUserPointer p, SQInteger)
                -> SQInteger
            {
                delete static_cast<Class*>(p);
                return 0;
            };
            
            // Construct instance
            const auto instance = bindArgments(vm, construct)();
            sq_setinstanceup(vm, 1, instance);
            sq_setreleasehook(vm, 1, destruct);

            return 0;
        }

        // Closure for member function
        template <class Class, class Fun, class... Args>
        SQInteger memberFunClosure(HSQUIRRELVM vm)
        {
            using Result = Result_t<Fun>;

            Fun* fun;
            sq_getuserdata(vm, -1, reinterpret_cast<SQUserPointer*>(&fun), nullptr);

            Class* instance;
            sq_getinstanceup(vm, 1, reinterpret_cast<SQUserPointer*>(&instance), nullptr);

            // call
            auto binder = bindHead<std::function<Result(Class*, Args...)>>(*fun, instance);
            return call(vm, bindArgments<decltype(binder), TypeList<Args...>>(vm, binder));
        }

        // For defineClass()
        template <class Class>
        class DefineClass
        {
        private:
            HSQUIRRELVM vm_;
            HSQOBJECT classTable_;
            const SQChar* name_;

        public:
            DefineClass(HSQUIRRELVM vm, const SQChar* name)
                : vm_(vm)
                , classTable_()
                , name_(name)
            {
                StackResetter reset(vm_);

                // Create class table
                sq_resetobject(&classTable_);
                sq_newclass(vm_, SQFalse);
                sq_getstackobj(vm_, -1, &classTable_);
                sq_addref(vm_, &classTable_);
            }

            ~DefineClass()
            {
                StackResetter reset(vm_);

                // Add class table into root table
                sq_pushroottable(vm_);
                sq_pushstring(vm_, name_, -1);
                sq_pushobject(vm_, classTable_);
                sq_newslot(vm_, -3, SQFalse);
                sq_release(vm_, &classTable_);
            }

            /** Define constructor */
            template <class... Args>
            DefineClass& constructor()
            {
                StackResetter reset(vm_);

                // Add closure for constructor
                sq_pushobject(vm_, classTable_);
                sq_pushstring(vm_, _SC("constructor"), -1);
                sq_newclosure(vm_, constructorClosure<Class, Args...>, 0);
                sq_newslot(vm_, -3, SQFalse);

                return *this;
            }

            /** Define member function */
            template <class Result, class... Args>
            DefineClass& fun(const SQChar* name, Result(Class::*fp)(Args...))
            {
                addMemberFunClosure(name, fp, memberFunClosure<Class, decltype(fp), Args...>, SQFalse);
                return *this;
            }

            /** Define const member function */
            template <class Result, class... Args>
            DefineClass& fun(const SQChar* name, Result(Class::*fp)(Args...) const)
            {
                addMemberFunClosure(name, fp, memberFunClosure<Class, decltype(fp), Args...>, SQFalse);
                return *this;
            }

            /** Define static member function */
            template <class Fun>
            DefineClass& staticFun(const SQChar* name, Fun fp)
            {
                addMemberFunClosure(name, fp, functionClosure<Fun>, SQTrue);
                return *this;
            }

        private:
            template <class Fun, class Closure>
            void addMemberFunClosure(const SQChar* name, Fun fp, Closure closure, SQBool isStatic)
            {
                StackResetter reset(vm_);

                // Add closure for member function
                sq_pushobject(vm_, classTable_);
                sq_pushstring(vm_, name, -1);
                const auto usrData = sq_newuserdata(vm_, sizeof(fp));
                std::memcpy(usrData, &fp, sizeof(fp));
                sq_newclosure(vm_, closure, 1);
                sq_newslot(vm_, -3, isStatic);
            }
        };
    }

    /** Define class */
    template <class Class>
    auto defineClass(HSQUIRRELVM vm, const SQChar* name)
        -> decltype(detail::DefineClass<Class>(vm, name))
    {
        return detail::DefineClass<Class>(vm, name);
    }
}

#endif