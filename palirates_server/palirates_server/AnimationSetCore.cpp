#include "stdafx.h"
#include "AnimationSetCore.h"
#include <cmath>            

using std::shared_ptr;
using std::string;

CAnimationSet::CAnimationSet(float len, int f, int k, int b, const string& n)
    : name(n), length(len), fps(f), nKeyFrames(k), nBones(b)
{
    keyTimes.resize(k);
    keys.resize(k, std::vector<XMFLOAT4X4>(b));
}

XMFLOAT4X4 CAnimationSet::GetMatrix(int boneIdx, float t) const
{
    if (nKeyFrames == 1) return keys[0][boneIdx];

    if (t < 0.f) t = 0.f;
    t = std::fmod(t, length);

    int k1 = 0;
    while (k1 < nKeyFrames - 1 && keyTimes[k1 + 1] < t) ++k1;
    int k2 = (k1 + 1) % nKeyFrames;

    float dt = keyTimes[k2] - keyTimes[k1];
    if (dt <= 0.f) return keys[k1][boneIdx];
    float alpha = (t - keyTimes[k1]) / dt;

    XMFLOAT4X4 a = keys[k1][boneIdx];
    XMFLOAT4X4 b = keys[k2][boneIdx];
    XMFLOAT4X4 out{};

    for (int i = 0; i < 16; ++i)
        reinterpret_cast<float*>(&out)[i] =
        reinterpret_cast<float*>(&a)[i] * (1 - alpha) +
        reinterpret_cast<float*>(&b)[i] * alpha;

    return out;
}

std::unordered_map<std::string,
    shared_ptr<CAnimationSet>> CAnimationSets::s_globalCache;

shared_ptr<CAnimationSet>
CAnimationSets::AddOrGetSharedAnimationSet(shared_ptr<CAnimationSet> a,
    const string& file)
{
    string key = file + "::" + a->name;
    auto it = s_globalCache.find(key);
    if (it != s_globalCache.end()) return it->second;
    s_globalCache[key] = a;
    return a;
}

void CAnimationSets::PrepareSkinning() {}
