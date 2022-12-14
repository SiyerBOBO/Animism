#pragma once

#include <deque>

#include "compiler_check.hpp"
#include "guid.hpp"
#include "provider.hpp"
#include "trace_context.hpp"
#include "etw.hpp"


namespace animism_engine { namespace details {
    template <typename T> class trace_manager;
} /* namespace details */ } /* namespace animism_engine */

namespace animism_engine { namespace testing {
    template <typename T> class trace_proxy;
} /* namespace testing */ } /* namespace animism_engine */


namespace animism_engine {

    template <typename T>
    class provider;

    /**
     * <summary>
     * Selected statistics about an ETW trace
     * </summary>
     */
    class trace_stats
    {
    public:
        const uint32_t buffersCount;
        const uint32_t buffersFree;
        const uint32_t buffersWritten;
        const uint32_t buffersLost;
        const uint64_t eventsTotal;
        const uint64_t eventsHandled;
        const uint32_t eventsLost;

        trace_stats(uint64_t eventsHandled, const EVENT_TRACE_PROPERTIES& props)
            : buffersCount(props.NumberOfBuffers)
            , buffersFree(props.FreeBuffers)
            , buffersWritten(props.BuffersWritten)
            , buffersLost(props.RealTimeBuffersLost)
            , eventsTotal(eventsHandled + props.EventsLost)
            , eventsHandled(eventsHandled)
            , eventsLost(props.EventsLost)
        { }
    };

    /**
     * <summary>
     *    Represents a single trace session that can have multiple
     *    enabled providers. Ideally, there should only need to be a
     *    single trace instance for all ETW user traces.
     * </summary>
     */
    template <typename T>
    class trace {
    public:

        typedef T trace_type;

        /**
         * <summary>
         *   Constructs a trace with an optional trace name, which can be
         *   any arbitrary, unique name.
         * </summary>
         *
         * <example>
         *   trace trace;
         *   trace namedTrace(L"Some special name");
         * </example>
         */
        trace(const std::wstring &name);
        trace(const wchar_t *name = L"");

        /**
         * <summary>
         *   Destructs the trace session and unregisters the session, if
         *   applicable.
         * </summary>
         *
         * <example>
         *   trace trace;
         *   // ~trace implicitly called
         * </example>
         */
        ~trace();

        /**
         * <summary>
         * Sets the trace properties for a session.
         * Must be called before open()/start().
         * See https://docs.microsoft.com/en-us/windows/win32/etw/event-trace-properties
         * for important details and restrictions.
         * Configurable properties are ->
         *  - BufferSize.  In KB. The maximum buffer size is 1024 KB.
         *  - MinimumBuffers. Minimum number of buffers is two per processor*.
         *  - MaximumBuffers.
         *  - FlushTimer. How often, in seconds, the trace buffers are forcibly flushed.
         *  - LogFileMode. EVENT_TRACE_NO_PER_PROCESSOR_BUFFERING simulates a *single* sequential processor.
         * </summary>
         * <example>
         *    animism_engine::trace trace;
         *    EVENT_TRACE_PROPERTIES properties = { 0 };
         *    properties.BufferSize = 256;
         *    properties.MinimumBuffers = 12;
         *    properties.MaximumBuffers = 48;
         *    properties.FlushTimer = 1;
         *    properties.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
         *    trace.set_trace_properties(&properties);
         *    animism_engine::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
         *    provider<> powershell(id);
         *    trace.enable(powershell);
         *    trace.start();
         * </example>
         */
        void set_trace_properties(const PEVENT_TRACE_PROPERTIES properties);

        /**
         * <summary>
         * Enables the provider on the given user trace.
         * </summary>
         * <example>
         *    animism_engine::trace trace;
         *    animism_engine::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
         *    provider<> powershell(id);
         *    trace.enable(powershell);
         * </example>
         */
        void enable(const typename T::provider_type &p);

        /**
         * <summary>
         * Starts a trace session.
         * </summary>
         * <example>
         *    animism_engine::trace trace;
         *    animism_engine::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
         *    provider<> powershell(id);
         *    trace.enable(powershell);
         *    trace.start();
         * </example>
         */
        void start();

        /**
         * <summary>
         * Closes a trace session.
         * </summary>
         * <example>
         *    animism_engine::trace trace;
         *    animism_engine::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
         *    provider<> powershell(id);
         *    trace.enable(powershell);
         *    trace.start();
         *    trace.stop();
         * </example>
         */
        void stop();

        /**
        * <summary>
        * Opens a trace session.
        * This is an optional call before start() if you need the trace
        * registered with the ETW subsystem before you start processing events.
        * </summary>
        * <example>
        *    animism_engine::trace trace;
        *    animism_engine::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
        *    provider<> powershell(id);
        *    trace.enable(powershell);
        *    auto logfile = trace.open();
        * </example>
        */
        EVENT_TRACE_LOGFILE open();

        /**
        * <summary>
        * Start processing events for an already opened session.
        * </summary>
        * <example>
        *    animism_engine::trace trace;
        *    animism_engine::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
        *    provider<> powershell(id);
        *    trace.enable(powershell);
        *    trace.open();
        *    trace.process();
        * </example>
        */
        void process();

        /**
         * <summary>
         * Queries the trace session to get stats about
         * events lost and buffers handled.
         * </summary>
         */
        trace_stats query_stats();

        /**
         * <summary>
         * Returns the number of buffers that were processed.
         * </summary>
         * <example>
         *    animism_engine::trace trace;
         *    animism_engine::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
         *    provider<> powershell(id);
         *    trace.enable(powershell);
         *    trace.start();
         *    trace.stop();
         *    std::wcout << trace.buffers_processed() << std::endl;
         * </example>
         */
        size_t buffers_processed() const;

        /**
         * <summary>
         * Adds a function to call when an event is fired which has no corresponding provider.
         * </summary>
         *
         * <param name="callback">the function to call into</param>
         * <example>
         *    void my_fun(const EVENT_RECORD &record) { ... }
         *    // ...
         *    animism_engine::trace trace;
         *    trace.set_default_event_callback(my_fun);
         * </example>
         *
         * <example>
         *    auto fun = [&](const EVENT_RECORD &record) {...}
         *    animism_engine::trace trace;
         *    trace.set_default_event_callback(fun);
         * </example>
         */
        void set_default_event_callback(c_provider_callback callback);

    private:

        /**
         * <summary>
         *   Invoked when an event occurs in the underlying ETW session.
         * </summary>
         */
        void on_event(const EVENT_RECORD &);

    private:
        std::wstring name_;
        std::deque<std::reference_wrapper<const typename T::provider_type>> providers_;

        TRACEHANDLE registrationHandle_;
        TRACEHANDLE sessionHandle_;

        size_t buffersRead_;
        uint64_t eventsHandled_;

        EVENT_TRACE_PROPERTIES properties_;

        const trace_context context_;

        provider_callback default_callback_ = nullptr;

    private:
        template <typename T>
        friend class details::trace_manager;

        template <typename T>
        friend class testing::trace_proxy;

        friend typename T;
    };

    // Implementation
    // ------------------------------------------------------------------------

    template <typename T>
    trace<T>::trace(const std::wstring &name)
    : registrationHandle_(INVALID_PROCESSTRACE_HANDLE)
    , sessionHandle_(INVALID_PROCESSTRACE_HANDLE)
    , eventsHandled_(0)
    , buffersRead_(0)
    , context_()
    {
        name_ = T::enforce_name_policy(name);
        ZeroMemory(&properties_, sizeof(EVENT_TRACE_PROPERTIES));
    }

    template <typename T>
    trace<T>::trace(const wchar_t *name)
    : registrationHandle_(INVALID_PROCESSTRACE_HANDLE)
    , sessionHandle_(INVALID_PROCESSTRACE_HANDLE)
    , eventsHandled_(0)
    , buffersRead_(0)
    , context_()
    {
        name_ = T::enforce_name_policy(name);
        ZeroMemory(&properties_, sizeof(EVENT_TRACE_PROPERTIES));
    }

    template <typename T>
    trace<T>::~trace()
    {
        stop();
    }

    template <typename T>
    void trace<T>::set_trace_properties(const PEVENT_TRACE_PROPERTIES properties)
    {
        properties_.BufferSize = properties->BufferSize;
        properties_.MinimumBuffers = properties->MinimumBuffers;
        properties_.MaximumBuffers = properties->MaximumBuffers;
        properties_.FlushTimer = properties->FlushTimer;
        properties_.LogFileMode = properties->LogFileMode;
    }

    template <typename T>
    void trace<T>::on_event(const EVENT_RECORD &record)
    {
        ++eventsHandled_;
        T::forward_events(record, *this);
    }

    template <typename T>
    void trace<T>::enable(const typename T::provider_type &p)
    {
        providers_.push_back(std::ref(p));
    }

    template <typename T>
    void trace<T>::start()
    {
        eventsHandled_ = 0;

        details::trace_manager<trace> manager(*this);
        manager.start();
    }

    template <typename T>
    void trace<T>::stop()
    {
        details::trace_manager<trace> manager(*this);
        manager.stop();
    }

    template <typename T>
    EVENT_TRACE_LOGFILE trace<T>::open()
    {
        eventsHandled_ = 0;

        details::trace_manager<trace> manager(*this);
        return manager.open();
    }

    template <typename T>
    void trace<T>::process()
    {
        eventsHandled_ = 0;

        details::trace_manager<trace> manager(*this);
        manager.process();
    }

    template <typename T>
    trace_stats trace<T>::query_stats()
    {
        details::trace_manager<trace> manager(*this);
        return { eventsHandled_, manager.query() };
    }

    template <typename T>
    size_t trace<T>::buffers_processed() const
    {
        return buffersRead_;
    }

    template <typename T>
    void trace<T>::set_default_event_callback(c_provider_callback callback)
    {
        default_callback_ = callback;
    }

}
