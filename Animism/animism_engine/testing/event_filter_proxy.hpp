#pragma once

#define INITGUID


#include "../compiler_check.hpp"
#include "../filtering/event_filter.hpp"
#include "synth_record.hpp"

namespace animism_engine { namespace testing {

    /**
     * <summary>
     *   Serves as a fill-in for the event_filter class for testing purposes.
     *   It acts as a liason for the actual filter instance and allows for forced event
     *   testing.
     * </summary>
     */
    class event_filter_proxy {
    public:

        /**
         * <summary>
         * Constructs a proxy for the given event_filter.
         * </summary>
         * <example>
         *     animism_engine::event_filter event_filter;
         *     animism_engine::testing::event_filter_proxy proxy(event_filter);
         * </example>
         */
        event_filter_proxy(animism_engine::event_filter &filter);

        /**
         * <summary>
         * Pushes an event through to the proxied filter instance.
         * </summary>
         * <example>
         *     animism_engine::event_filter event_filter;
         *     animism_engine::testing::event_filter_proxy proxy(event_filter);
         *
         *     animism_engine::guid powershell(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
         *     animism_engine::testing::record_builder builder(powershell, animism_engine::id(7942), animism_engine::version(1));
         *
         *     builder.add_properties()
         *             (L"ClassName", L"FakeETWEventForRealz")
         *             (L"Message", L"This message is completely faked");
         *
         *     auto record = builder.pack_incomplete();
         *     proxy.push_event(record);
         * </example>
         */
        void push_event(const synth_record &record);

    private:
        animism_engine::event_filter &event_filter_;
        animism_engine::trace_context trace_context_;
    };

    // Implementation
    // ------------------------------------------------------------------------

    inline event_filter_proxy::event_filter_proxy(animism_engine::event_filter &event_filter)
    : event_filter_(event_filter)
    {
    }

    inline void event_filter_proxy::push_event(const synth_record &record)
    {
        event_filter_.on_event(record, trace_context_);
    }

} /* namespace testing */ } /* namespace animism_engine */
