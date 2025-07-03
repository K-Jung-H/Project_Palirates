#pragma once
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <DirectXMath.h>

using namespace DirectX;

class CAnimationSet
{
public:
    std::string   name;
    float         length = 0.f;   // seconds
    int           fps = 30;
    int           nKeyFrames = 0;
    int           nBones = 0;

    std::vector<float>                   keyTimes;   // [key]
    std::vector<std::vector<XMFLOAT4X4>> keys;       // [key][bone]

    CAnimationSet(float len, int fps, int keys,
        int bones, const std::string& n);

    XMFLOAT4X4 GetMatrix(int boneIdx, float time) const;
};

class CAnimationSets
{
public:
    explicit CAnimationSets(int nSets) : sets(nSets) {}

    CAnimationSets() = default;              

    std::vector<std::shared_ptr<CAnimationSet>> sets;
    int                                         nBones = 0;
    std::vector<void*>                          boneCaches;  

    void PrepareSkinning();                    

    static std::shared_ptr<CAnimationSet>
        AddOrGetSharedAnimationSet(std::shared_ptr<CAnimationSet> animSet,
            const std::string& filename);

private:
    static std::unordered_map<std::string,
        std::shared_ptr<CAnimationSet>> s_globalCache;
};
