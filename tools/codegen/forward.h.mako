//DO NOT EDIT THIS FILE, IT IS AUTO GENERATED BY CODEGEN
#pragma once
#include "${config}"
#include <inttypes.h>

#ifdef __cplusplus
%for record in db.records:
%if hasattr(record, "namespace"):
namespace ${record.namespace} { struct ${record.short_name}; }
%endif
%endfor
%for enum in db.enums:
%if hasattr(enum, "namespace"):
namespace ${enum.namespace} { enum ${enum.short_name} : ${enum.underlying_type}; }
%else:
enum ${enum.short_name} : ${enum.underlying_type};
%endif
%endfor
#endif

%for record in db.records:
%if not hasattr(record, "namespace"):
typedef struct ${record.short_name} ${record.short_name};
%endif
%endfor