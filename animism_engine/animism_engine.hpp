#pragma once

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ole32.lib")

#pragma warning(push)
#pragma warning(disable: 4512) // stupid spurious "can't generate assignment error" warning
#pragma warning(disable: 4634) // DocXml comment warnings in native C++
#pragma warning(disable: 4635) // DocXml comment warnings in native C++

#include "compiler_check.hpp"
#include "ut.hpp"
#include "kt.hpp"
#include "guid.hpp"
#include "trace.hpp"
#include "trace_context.hpp"
#include "client.hpp"
#include "errors.hpp"
#include "schema.hpp"
#include "schema_locator.hpp"
#include "parse_types.hpp"
#include "collection_view.hpp"
#include "size_provider.hpp"
#include "parser.hpp"
#include "property.hpp"
#include "provider.hpp"
#include "etw.hpp"
#include "tdh_helpers.hpp"
#include "kernel_providers.hpp"

#include "testing/proxy.hpp"
#include "testing/filler.hpp"
#include "testing/synth_record.hpp"
#include "testing/record_builder.hpp"
#include "testing/event_filter_proxy.hpp"
#include "testing/record_property_thunk.hpp"

#include "filtering/view_adapters.hpp"
#include "filtering/comparers.hpp"
#include "filtering/predicates.hpp"
#include "filtering/event_filter.hpp"

#pragma warning(pop)
