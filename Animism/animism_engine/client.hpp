#pragma once

#include "compiler_check.hpp"
#include "ut.hpp"
#include "kt.hpp"
#include "trace.hpp"

namespace animism_engine {

    /**
     * <summary>
     * Specialization of the base trace class for user traces.
     * </summary>
     */
    typedef animism_engine::trace<animism_engine::details::ut> user_trace;

    /**
     * <summary>
     * Specialization of the base trace class for kernel traces.
     * </summary>
     */
    typedef animism_engine::trace<animism_engine::details::kt> kernel_trace;
}
