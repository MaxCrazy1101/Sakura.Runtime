//DO NOT EDIT THIS FILE, IT IS AUTO GENERATED BY CODEGEN
#pragma once
#ifdef __meta__
#error "this file should not be inspected by meta"
#endif
#include "${config}"
#include <inttypes.h>

#ifdef __cplusplus
%for record in db.records:
%if hasattr(record, "namespace"):
namespace ${record.namespace} { struct ${record.short_name}; }
%endif
%endfor
%for enum in db.enums:
<% prefix = "class" if enum.isScoped else ""  %>
%if hasattr(enum, "namespace"):
namespace ${enum.namespace} {
%endif
enum ${prefix} ${enum.short_name} : ${enum.underlying_type};
%if hasattr(enum, "namespace"):
}
%endif
%endfor
#endif

%for record in db.records:
%if not hasattr(record, "namespace"):
typedef struct ${record.short_name} ${record.short_name};
%endif
%endfor