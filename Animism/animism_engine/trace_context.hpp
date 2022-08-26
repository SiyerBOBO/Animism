#pragma once

#include "schema_locator.hpp"

namespace animism_engine {

    /**
     * <summary>
     * Additional ETW trace context passed to event callbacks
     * to enable processing.
     * </summary>
     */
    struct trace_context
    {
        const schema_locator schema_locator;
        /* Add additional trace context here. */
    };

}
