#include "constant_buffer.h"

#include "logger/log.h"
#include "render_private.h"

#include <cstring>

namespace cw::render
{
    struct ConstantEntry
    {
        uint64 NameHash = 0;
        Vec4   Elements[CONSTANT_MAX_ELEMENTS]{};
        usize  Count    = 0;
    };

    struct ConstantBuffer
    {
        ConstantEntry* Entries  = nullptr;
        usize          Count    = 0;
        usize          Capacity = 0;
    };

    ConstantBuffer* CreateConstantBuffer(usize maxConstants)
    {
        ConstantBuffer* cb = new ConstantBuffer;
        cb->Entries        = new ConstantEntry[maxConstants]{};
        cb->Capacity       = maxConstants;
        return cb;
    }

    void DestroyConstantBuffer(ConstantBuffer* cb)
    {
        if (cb == nullptr)
        {
            return;
        }

        delete[] cb->Entries;
        delete cb;
    }

    static ConstantEntry* FindOrAddEntry(ConstantBuffer* cb, uint64 nameHash)
    {
        for (usize i = 0; i < cb->Count; ++i)
        {
            if (cb->Entries[i].NameHash == nameHash)
            {
                return &cb->Entries[i];
            }
        }

        if (cb->Count == cb->Capacity)
        {
            CW_ERROR("Constant buffer is full (%zu entries)", cb->Capacity);
            return nullptr;
        }

        ConstantEntry* entry = &cb->Entries[cb->Count++];
        entry->NameHash      = nameHash;
        return entry;
    }

    void SetConstant(ConstantBuffer* cb, uint64 nameHash, const Vec4* values, usize count)
    {
        if (cb == nullptr || values == nullptr || count == 0)
        {
            return;
        }

        if (count > CONSTANT_MAX_ELEMENTS)
        {
            CW_ERROR("Constant has %zu elements, at most %zu fit", count, CONSTANT_MAX_ELEMENTS);
            return;
        }

        ConstantEntry* entry = FindOrAddEntry(cb, nameHash);
        if (entry == nullptr)
        {
            return;
        }

        memcpy(entry->Elements, values, count * sizeof(Vec4));
        entry->Count = count;
    }

    void SetConstantMat4(ConstantBuffer* cb, uint64 nameHash, const Mat4* value)
    {
        SetConstant(cb, nameHash, reinterpret_cast<const Vec4*>(value->Data), 4);
    }

    bool GetConstant(
        const ConstantBuffer* cb,
        uint64                nameHash,
        const Vec4**          outValues,
        usize*                outCount
    )
    {
        if (cb == nullptr)
        {
            return false;
        }

        for (usize i = 0; i < cb->Count; ++i)
        {
            if (cb->Entries[i].NameHash != nameHash)
            {
                continue;
            }

            *outValues = cb->Entries[i].Elements;
            *outCount  = cb->Entries[i].Count;
            return true;
        }

        return false;
    }

    void ClearConstants(ConstantBuffer* cb)
    {
        if (cb != nullptr)
        {
            cb->Count = 0;
        }
    }

    void ApplyConstants(RenderContext* ctx, const Pipeline* pipeline, const ConstantBuffer* cb)
    {
        if (cb == nullptr)
        {
            return;
        }

        for (usize i = 0; i < cb->Count; ++i)
        {
            const ConstantEntry& entry = cb->Entries[i];

            for (usize u = 0; u < pipeline->UniformsCount; ++u)
            {
                const PipelineUniform& uniform = pipeline->Uniforms[u];
                if (uniform.NameHash != entry.NameHash)
                {
                    continue;
                }

                ApplyUniform(ctx, pipeline, uniform, entry.Elements);
                break;
            }
        }
    }
}
