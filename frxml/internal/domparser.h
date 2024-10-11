#pragma once

#include "frxml.h"
#include "chariterator.h"

namespace frxml
{
    class domparser
    {
    public:
        [[nodiscard]]
        static int ParseElementLike(
            char_iterator& cur,
            char_iterator  end,
            dom&           dom);

        [[nodiscard]]
        static int ParsePI(
            char_iterator& cur,
            char_iterator  end,
            dom&           dom);

        [[nodiscard]]
        static int ParseComment(
            char_iterator& cur,
            char_iterator  end,
            dom&           dom);

        [[nodiscard]]
        static int ParseElement(
            char_iterator& cur,
            char_iterator  end,
            dom&           dom);
    };
}
