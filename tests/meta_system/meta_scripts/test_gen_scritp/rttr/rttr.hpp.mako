// BEGIN RTTR GENERATED
#include "SkrRT/rttr/enum_traits.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"
namespace skr::rttr
{
// enum traits
%for enum in header_db.enums:
template <>
struct ${api} EnumTraits<${enum.name}>
{
    static skr::span<EnumItem<${enum.name}>> items();
    static skr::StringView to_string(const ${enum.name}& value);
    static bool from_string(skr::StringView str, ${enum.name}& value);
};
%endfor
}

// rttr traits
%for record in header_db.records:
%if record.attrs.rttr.enable:
SKR_RTTR_TYPE(::${record.name}, "${record.attrs.guid}")
%endif
%endfor
%for enum in header_db.records:
%if enum.attrs.rttr.enable:
SKR_RTTR_TYPE(::${enum.name}, "${enum.attrs.guid}")
%endif
%endfor
// END RTTR GENERATED
