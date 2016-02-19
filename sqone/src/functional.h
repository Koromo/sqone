#ifndef _SQONE_FUNCTIONAL_H_
#define _SQONE_FUNCTIONAL_H_

namespace sqone
{
    namespace detail
    {
        // For bindHead()
        template <class Fun, class Head>
        struct BindHead
        {
            Fun fun_;
            Head head_;

            BindHead(Fun fun, Head head)
                : fun_(fun)
                , head_(head)
            {
            }

            template <class... Args>
            auto operator ()(Args... args)
                -> decltype(fun_(head_, args...))
            {
                return fun_(head_, args...);
            }
        };
    }

    /** Bind first argment into functor */
    template <class Fun, class Head>
    auto bindHead(Fun fun, Head head)
        -> decltype(detail::BindHead<Fun, Head>(fun, head))
    {
        return detail::BindHead<Fun, Head>(fun, head);
    }
}

#endif