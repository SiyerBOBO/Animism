#pragma once

#include <evntcons.h>
#include <functional>
#include <deque>
#include <vector>

#include "../compiler_check.hpp"
#include "../trace_context.hpp"

namespace animism_engine { namespace testing {
    class event_filter_proxy;
} /* namespace testing */} /* namespace animism_engine */

namespace animism_engine { namespace details {
    template <typename T> class base_provider;
} /* namespace details */} /* namespace animism_engine */


namespace animism_engine {

    typedef void(*c_provider_callback)(const EVENT_RECORD &, const animism_engine::trace_context &);
    typedef std::function<void(const EVENT_RECORD &, const animism_engine::trace_context &)> provider_callback;
    typedef std::function<bool(const EVENT_RECORD &, const animism_engine::trace_context &)> filter_predicate;

    template <typename T> class provider;

    /**
     * <summary>
     *   Use this to provide event filtering before an event bubbles to
     *   specific callbacks.
     * </summary>
     * <remarks>
     *   Each event_filter has a single predicate (which can do complicated
     *   checks and logic on the event). All callbacks registered under the
     *   filter are invoked only if the predicate returns true for a given
     *   event.
     * </remarks>
     */
    class event_filter {
    public:

        /**
         * <summary>
         *   Constructs an event_filter that applies the given predicate to all
         *   events.
         * </summary>
         */
        event_filter(filter_predicate predicate);

        /**
         * <summary>
         *   Constructs an event_filter that applies event id filtering by event_id
         *   which will be added to list of filtered event ids in ETW API.
         *   This way is more effective from performance point of view.
         *   Given optional predicate will be applied to ETW API filtered results
         * </summary>
         */
        event_filter(unsigned short event_id, filter_predicate predicate=nullptr);

        /**
         * <summary>
         *   Constructs an event_filter that applies event id filtering by event_id
         *   which will be added to list of filtered event ids in ETW API.
         *   This way is more effective from performance point of view.
         *   Given optional predicate will be applied to ETW API filtered results
         * </summary>
         */
        event_filter(std::vector<unsigned short> event_ids, filter_predicate predicate = nullptr);

        /**
         * <summary>
         * Adds a function to call when an event for this filter is fired.
         * </summary>
         */
        void add_on_event_callback(c_provider_callback callback);

        template <typename U>
        void add_on_event_callback(U &callback);

        template <typename U>
        void add_on_event_callback(const U &callback);

        const std::vector<unsigned short>& provider_filter_event_ids() const
        {
            return provider_filter_event_ids_;
        }

    private:

        /**
         * <summary>
         *   Called when an event occurs, forwards to callbacks if the event
         *   satisfies the predicate.
         * </summary>
         */
        void on_event(const EVENT_RECORD &record, const animism_engine::trace_context &trace_context) const;

    private:
        std::deque<provider_callback> callbacks_;
        filter_predicate predicate_{ nullptr };
        std::vector<unsigned short> provider_filter_event_ids_;

    private:
        template <typename T>
        friend class details::base_provider;

        friend class animism_engine::testing::event_filter_proxy;
    };

    // Implementation
    // ------------------------------------------------------------------------

    inline event_filter::event_filter(filter_predicate predicate)
    : predicate_(predicate)
    {}

    inline event_filter::event_filter(std::vector<unsigned short> event_ids, filter_predicate predicate/*=nullptr*/)
    : provider_filter_event_ids_{ event_ids },
      predicate_(predicate)
    {}

    inline event_filter::event_filter(unsigned short event_id, filter_predicate predicate/*=nullptr*/)
    : provider_filter_event_ids_{ event_id },
      predicate_(predicate)
    {}

    inline void event_filter::add_on_event_callback(c_provider_callback callback)
    {
        // C function pointers don't interact well with std::ref, so we
        // overload to take care of this scenario.
        callbacks_.push_back(callback);
    }

    template <typename U>
    void event_filter::add_on_event_callback(U &callback)
    {
        // std::function copies its argument -- because our callbacks list
        // is a list of std::function, this causes problems when a user
        // intended for their particular instance to be called.
        // std::ref lets us get around this and point to a specific instance
        // that they handed us.
        callbacks_.push_back(std::ref(callback));
    }

    template <typename U>
    void event_filter::add_on_event_callback(const U &callback)
    {
        // This is where temporaries bind to. Temporaries can't be wrapped in
        // a std::ref because they'll go away very quickly. We are forced to
        // actually copy these.
        callbacks_.push_back(callback);
    }

    inline void event_filter::on_event(const EVENT_RECORD &record, const animism_engine::trace_context &trace_context) const
    {
        if (callbacks_.empty()) {
            return;
        }

        if (predicate_ != nullptr && !predicate_(record, trace_context)) {
            return;
        }

        for (auto &callback : callbacks_) {
            callback(record, trace_context);
        }
    }
} /* namespace animism_engine */
