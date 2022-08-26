#pragma once

#include <string>

#include "../compiler_check.hpp"
#include "../parser.hpp"

namespace animism_engine { namespace predicates {

    namespace adapters {

        /**
        * View adapter for counted_string strings
        */
        struct counted_string
        {
            using value_type = animism_engine::counted_string::value_type;
            using const_iterator = animism_engine::counted_string::const_iterator;

            collection_view<const_iterator> operator()(const property_info& propInfo) const
            {
                auto cs_ptr = reinterpret_cast<const animism_engine::counted_string*>(propInfo.pPropertyIndex_);
                return animism_engine::view(cs_ptr->string(), cs_ptr->length());
            }
        };
        
        /**
         * View adapter for fixed width and null-terminated strings
         */
        template <typename ElemT>
        struct generic_string
        {
            using value_type = ElemT;
            using const_iterator = const value_type*;

            collection_view<const_iterator> operator()(const property_info& propInfo) const
            {
                auto pString = reinterpret_cast<const value_type*>(propInfo.pPropertyIndex_);
                auto length = get_string_content_length(pString, propInfo.length_);

                return animism_engine::view(pString, length);
            }
        };

    } /* namespace adapters */

} /* namespace predicates */ } /* namespace animism_engine */