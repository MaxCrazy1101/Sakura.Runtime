// BEGIN BINARY GENERATED
#include "utils/hash.h"
#include "platform/debug.h"
#include "utils/log.h"
#include "binary/reader.h"
#include "binary/writer.h"
#include "binary/blob.h"

<%def name="archive_field(name, field, array)">
%if hasattr(field.attrs, "arena"):
    ret = Archive(archive, record.${field.attrs.arena}, record.${name}${array});
%else:
    ret = Archive(archive, record.${name}${array});
%endif
</%def>

namespace skr::binary {

%for record in generator.filter_types(db.records):
%if generator.filter_blob_type(record):
template<class S>
int __Archive(S* archive, skr_blob_arena_t& arena, ${record.name}& record)
{
    int ret = 0;
    %for name, field in generator.filter_fields(record.fields):
    %if field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        ret = Archive(archive, arena, record.${name}[i]);
        if(ret != 0)
            return ret;
    }
    %else:
    ret = Archive(archive, arena, record.${name});
    if(ret != 0)
        return ret;
    %endif
    %endfor
    return ret;
}
%else:
template<class S>
int __Archive(S* archive, ${record.name}& record)
{
    int ret = 0;
    %for name, field in generator.filter_fields(record.fields):
    %if field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        ${archive_field(name, field, "[i]")}
        if(ret != 0)
            return ret;
    }
    %else:
    ${archive_field(name, field, "")}
    if(ret != 0)
        return ret;
    %endif
    %endfor
    return ret;
}
%endif

%if generator.filter_blob_type(record):
void BlobTrait<${record.name}>::BuildArena(skr_blob_arena_builder_t& arena, ${record.name}& dst, const ${record.name}Builder& src)
{
%for base in record.bases:
    BlobTrait<${base}>::BuildArena(arena, (${base}&)dst, (${base}Builder&) src);
%endfor
%for name, field in generator.filter_fields(record.fields):
%if field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        ::skr::binary::BuildArena<${field.type}>(arena, dst.${name}[i], src.${name}[i]);
    }
%else:
    ::skr::binary::BuildArena<${field.type}>(arena, dst.${name}, src.${name});
%endif
%endfor
}
void BlobTrait<${record.name}>::Remap(skr_blob_arena_t& arena, ${record.name}& dst)
{
%for base in record.bases:
    BlobTrait<${base}>::Remap(arena, (${base}&)dst);
%endfor
%for name, field in generator.filter_fields(record.fields):
%if field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        ::skr::binary::Remap<${field.type}>(arena, dst.${name}[i]);
    }
%else:
    ::skr::binary::Remap<${field.type}>(arena, dst.${name});
%endif
%endfor
}
int ReadTrait<${record.name}>::Read(skr_binary_reader_t* archive, skr_blob_arena_t& arena, ${record.name}& record)
{
%for base in record.bases:
    int ret = ReadTrait<const ${base}&>::Read(archive, arena, (${base}&)record);
    if(ret != 0)
        return ret;
%endfor
    return __Archive(archive, arena, record);
}
int WriteTrait<const ${record.name}&>::Write(skr_binary_writer_t* archive, skr_blob_arena_t& arena, const ${record.name}& record)
{
%for base in record.bases:
    int ret = WriteTrait<const ${base}&>::Write(archive, arena, (${base}&)record);
    if(ret != 0)
        return ret;
%endfor
    return __Archive(archive, arena, (${record.name}&)record);
} 
%else:
int ReadTrait<${record.name}>::Read(skr_binary_reader_t* archive, ${record.name}& record)
{
%for base in record.bases:
    int ret = skr::binary::Read(archive, (${base}&)record);
    if(ret != 0)
        return ret;
%endfor
    return __Archive(archive, record);
}
int WriteTrait<const ${record.name}&>::Write(skr_binary_writer_t* archive, const ${record.name}& record)
{
%for base in record.bases:
    int ret = skr::binary::Write<const ${base}&>(archive, record);
    if(ret != 0)
        return ret;
%endfor
    return __Archive(archive, (${record.name}&)record);
} 
%endif
%endfor
}
//END BINARY GENERATED