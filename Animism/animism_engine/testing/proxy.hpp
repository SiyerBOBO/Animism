#pragma once

#define INITGUID

#include "../compiler_check.hpp"
#include "../trace.hpp"
#include "../client.hpp"
#include "../testing/synth_record.hpp"

namespace animism_engine { namespace testing {

    /**
     * <summary>
     *   Serves as a fill-in for the trace class for testing purposes. It acts
     *   as a liason for the actual trace instance and allows for forced event
     *   testing.
     * </summary>
     */
    template <typename T>
    class trace_proxy {
    public:

        /**
         * <summary>
         * Constructs a proxy for the given trace.
         * </summary>
         * <example>
         *     animism_engine::user_trace trace;
         *     animism_engine::testing::trace_proxy proxy(trace);
         * </example>
         */

        trace_proxy(T &trace);

        /**
         * <summary>
         * Mocks starting the underlying trace.
         * </summary>
         * <example>
         *     animism_engine::user_trace trace;
         *     animism_engine::testing::trace_proxy proxy(trace);
         *     proxy.start(); // do not call trace.start()
         * </example>
         */
        void start();

        /**
         * <summary>
         * Pushes an event through to the proxied trace instance.
         * </summary>
         * <remarks>
         *   This is the primary mechanism for testing providers and their
         *   callbacks. Create a fake event with an record_builder instance
         *   and then push the created synth_record through the object
         *   graph.
         * </remarks>
         * <example>
         *     animism_engine::user_trace trace;
         *     animism_engine::testing::trace_proxy proxy(trace);
         *     proxy.start(); // do not call trace.start()
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
        T &trace_;
    };

    /**
     * <summary>Specific instantiation for user traces.</summary>
     */
    typedef trace_proxy<animism_engine::user_trace> user_trace_proxy;

    /**
     * <summary>Specific instantiation for kernel traces.</summary>
     */
    typedef trace_proxy<animism_engine::kernel_trace> kernel_trace_proxy;

    // Implementation
    // ------------------------------------------------------------------------

    template <typename T>
    trace_proxy<T>::trace_proxy(T &trace)
    : trace_(trace)
    {
    }

    template <typename T>
    void trace_proxy<T>::start()
    {
    }

    template <typename T>
    void trace_proxy<T>::push_event(const synth_record &record)
    {
        trace_.on_event(record);
    }

} /* namespace testing */ } /* namespace animism_engine */
