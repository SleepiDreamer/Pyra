#include "MikkT.h"
#include "mikktspace.h"

#include <cstring>

struct MikkTContext
{
    std::vector<Vertex>* vertices;
    const std::vector<uint32_t>* indices;

    uint32_t GetIndex(const int face, const int vert) const
    {
        if (indices->empty())
            return face * 3 + vert;
        return (*indices)[face * 3 + vert];
    }
};

static int getNumFaces(const SMikkTSpaceContext* ctx)
{
    auto* c = static_cast<MikkTContext*>(ctx->m_pUserData);
    int numVerts = c->indices->empty()
        ? static_cast<int>(c->vertices->size())
        : static_cast<int>(c->indices->size());
    return numVerts / 3;
}

static int getNumVerticesOfFace(const SMikkTSpaceContext*, int)
{
    return 3;
}

static void getPosition(const SMikkTSpaceContext* ctx, float outpos[], const int face, const int vert)
{
    auto* c = static_cast<MikkTContext*>(ctx->m_pUserData);
    auto& v = (*c->vertices)[c->GetIndex(face, vert)];
    outpos[0] = v.position.x;
    outpos[1] = v.position.y;
    outpos[2] = v.position.z;
}

static void getNormal(const SMikkTSpaceContext* ctx, float outnormal[], const int face, const int vert)
{
    auto* c = static_cast<MikkTContext*>(ctx->m_pUserData);
    auto& v = (*c->vertices)[c->GetIndex(face, vert)];
    outnormal[0] = v.normal.x;
    outnormal[1] = v.normal.y;
    outnormal[2] = v.normal.z;
}

static void getTexCoord(const SMikkTSpaceContext* ctx, float outuv[], const int face, const int vert)
{
    auto* c = static_cast<MikkTContext*>(ctx->m_pUserData);
    auto& v = (*c->vertices)[c->GetIndex(face, vert)];
    outuv[0] = v.texCoord.x;
    outuv[1] = v.texCoord.y;
}

static void setTSpaceBasic(const SMikkTSpaceContext* ctx, const float tangent[], float sign, const int face, const int vert)
{
    auto* c = static_cast<MikkTContext*>(ctx->m_pUserData);
    auto& v = (*c->vertices)[c->GetIndex(face, vert)];
    v.tangent = { tangent[0], tangent[1], tangent[2], sign };
}

bool MikkT::Generate(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
    MikkTContext userData{ &vertices, &indices };

    SMikkTSpaceInterface iface{};
    iface.m_getNumFaces = getNumFaces;
    iface.m_getNumVerticesOfFace = getNumVerticesOfFace;
    iface.m_getPosition = getPosition;
    iface.m_getNormal = getNormal;
    iface.m_getTexCoord = getTexCoord;
    iface.m_setTSpaceBasic = setTSpaceBasic;

    SMikkTSpaceContext ctx{};
    ctx.m_pInterface = &iface;
    ctx.m_pUserData = &userData;

    return genTangSpaceDefault(&ctx) != 0;
}