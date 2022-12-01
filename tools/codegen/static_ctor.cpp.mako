//BEGIN STATIC_CTOR GENERATED
#include "platform/debug.h"
#include "platform/guid.hpp"
#include "platform/memory.h"
#include "type/type.hpp"

%for record in generator.filter_records(db.records):
static struct StaticConstructor${record.id}Helper
{
    StaticConstructor${record.id}Helper()
    {
        %if hasattr(record, "namespace"):
        using namespace ${record.namespace};
        %endif

        %for realized_expr in record.realized_expr:
        ${realized_expr};
        %endfor
    }
} _StaticConstructor${record.id}Helper;
%endfor


%for enum in generator.filter_enums(db.enums):
static struct StaticConstructor${enum.id}Helper
{
    StaticConstructor${enum.id}Helper()
    {
        %for realized_expr in enum.realized_expr:
        ${realized_expr};
        %endfor
    }
} _StaticConstructor${enum.id}Helper;
%endfor
//END STATIC_CTOR GENERATED