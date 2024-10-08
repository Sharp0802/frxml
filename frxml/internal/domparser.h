#pragma once

#include "frxml.h"
#include "chariterator.h"

namespace frxml
{
    class domparser
    {
    public:
        [[nodiscard]]
        static int parseelementlike(
            char_iterator& cur,
            char_iterator  end,
            dom&           dom);

        [[nodiscard]]
        static int parsepcinstr(
            char_iterator& cur,
            char_iterator  end,
            dom&           dom);

        [[nodiscard]]
        static int parsecomment(
            char_iterator& cur,
            char_iterator  end,
            dom&           dom);

        [[nodiscard]]
        static int parseelement(
            char_iterator& cur,
            char_iterator  end,
            dom&           dom);
    };
}
