#include "stdafx.h"
#include "ServerAnimLoader.h"
#include <cstdio>
#include <cstring>

using namespace DirectX;

/* ───────── 디버그 토큰 출력 ───────── */
#ifdef _DEBUG
#  define TRACE_TOKEN(tok) \
     printf("[%-10lld] TOK=\"%s\"\n", static_cast<long long>(_ftelli64(fp)), tok)
#else
#  define TRACE_TOKEN(tok) ((void)0)
#endif

/* ───────── 헬퍼 ───────── */
static bool ReadPrefixedString(FILE* fp, char* buf /* [64] */)
{
    unsigned char len;
    if (fread(&len, 1, 1, fp) != 1) return false;      // EOF
    if (len > 63) len = 63;                            // safety
    if (len && fread(buf, 1, len, fp) != len) return false;
    buf[len] = '\0';
    TRACE_TOKEN(buf);
    return true;
}
static int   ReadInt(FILE* fp) { int   v; fread(&v, 4, 1, fp); return v; }
static float ReadFloat(FILE* fp) { float v; fread(&v, 4, 1, fp); return v; }

/* ───────── 블록 스킵 ───────── */
static void SkipBlock(FILE* fp, const char* endTok)
{
    char tok[64];
    while (ReadPrefixedString(fp, tok))
        if (!strcmp(tok, endTok)) break;
}

/* ───────── 계층 재귀 ───────── */
static void ParseFrame(FILE* fp,
    std::vector<std::string>& names,
    std::vector<XMFLOAT4X4>& invBind)
{
    ReadInt(fp); ReadInt(fp);                // nFrame, nTex
    char name[64]; ReadPrefixedString(fp, name);
    names.emplace_back(name);

    char tok[64];
    for (;;)
    {
        if (!ReadPrefixedString(fp, tok)) return;     // EOF safety

        if (!strcmp(tok, "<Transform>:"))
        {
            float dummy[13]; fread(dummy, 4, 13, fp);
        }
        else if (!strcmp(tok, "<TransformMatrix>:"))
        {
            XMFLOAT4X4 m; fread(&m, 4, 16, fp);
        }
        else if (!strcmp(tok, "<SkinningInfo>:"))
        {
            for (;;)
            {
                ReadPrefixedString(fp, tok);
                if (!strcmp(tok, "<BoneOffsets>:"))
                {
                    int n = ReadInt(fp);
                    invBind.resize(n);
                    fread(invBind.data(), sizeof(XMFLOAT4X4), n, fp);
                }
                else if (!strcmp(tok, "</SkinningInfo>")) break;
            }
        }
        else if (!strcmp(tok, "<Children>:"))
        {
            int nChild = ReadInt(fp);
            for (int i = 0; i < nChild; ++i)
            {
                ReadPrefixedString(fp, tok);   // "<Frame>:"
                ParseFrame(fp, names, invBind);
            }
        }
        else if (!strcmp(tok, "<Mesh>:"))       SkipBlock(fp, "</Mesh>");
        else if (!strcmp(tok, "<Materials>:"))  SkipBlock(fp, "</Materials>");
        else if (!strcmp(tok, "</Frame>"))      break;
    }
}

/* ───────── 상위 계층 로더 ───────── */
static void LoadHierarchy(FILE* fp,
    std::vector<std::string>& names,
    std::vector<XMFLOAT4X4>& invBind)
{
    char tok[64];
    ReadPrefixedString(fp, tok);          // "<Frame>:"
    ParseFrame(fp, names, invBind);
    ReadPrefixedString(fp, tok);          // "</Hierarchy>"
}

/* ───────── 애니메이션 세트 로더 ───────── */
static void LoadAnimation(FILE* fp, ServerAnimationAsset& out)
{
    char tok[64];
    ReadPrefixedString(fp, tok);          // "<AnimationSets>:"
    int setCnt = ReadInt(fp);
    out.animSets.resize(setCnt);

    ReadPrefixedString(fp, tok);          // "<FrameNames>:"
    int boneCnt = ReadInt(fp);
    for (int i = 0; i < boneCnt; ++i) ReadPrefixedString(fp, tok);

    for (int s = 0; s < setCnt; ++s)
    {
        ReadPrefixedString(fp, tok);      // "<AnimationSet>:"
        int idx = ReadInt(fp);
        AnimSet& set = out.animSets[idx];

        char nm[64]; ReadPrefixedString(fp, nm); set.name = nm;
        set.length = ReadFloat(fp);
        set.fps = ReadInt(fp);
        int keyCnt = ReadInt(fp);
        set.keys.resize(keyCnt);

        for (int k = 0; k < keyCnt; ++k)
        {
            ReadPrefixedString(fp, tok);          // "<Transforms>:"
            int   keyIdx = ReadInt(fp);
            float kt = ReadFloat(fp);
            set.keys[keyIdx].time = kt;
            set.keys[keyIdx].matrices.resize(boneCnt);
            fread(set.keys[keyIdx].matrices.data(),
                sizeof(XMFLOAT4X4), boneCnt, fp);
        }
    }
    ReadPrefixedString(fp, tok);          // "</AnimationSets>"
    ReadPrefixedString(fp, tok);          // "</Animation>"
}

/* ───────── 퍼블릭 진입점 ───────── */
std::shared_ptr<ServerAnimationAsset>
LoadServerAnimationOnly(const char* binPath)
{
    FILE* fp = nullptr;
    if (fopen_s(&fp, binPath, "rb") || !fp) return nullptr;

    auto asset = std::make_shared<ServerAnimationAsset>();
    char tok[64];

    while (ReadPrefixedString(fp, tok))
    {
        if (!strcmp(tok, "<Hierarchy>:"))
            LoadHierarchy(fp, asset->boneNames, asset->invBindPose);
        else if (!strcmp(tok, "<Animation>:"))
            LoadAnimation(fp, *asset);
        else if (!strcmp(tok, "</Animation>:"))
            break;
    }
    fclose(fp);
    return asset;
}
