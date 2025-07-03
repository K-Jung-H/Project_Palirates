#pragma once
#include <vector>
#include <string>
#include <memory>
#include <DirectXMath.h>

struct AnimKey {
    float time = 0.f;
    std::vector<DirectX::XMFLOAT4X4> matrices;
};
struct AnimSet {
    std::string name;
    float length = 0.f;
    int   fps = 30;
    std::vector<AnimKey> keys;
};
struct ServerAnimationAsset {
    std::vector<std::string>                 boneNames;
    std::vector<DirectX::XMFLOAT4X4>         invBindPose;
    std::vector<AnimSet>                     animSets;
};

std::shared_ptr<ServerAnimationAsset>
LoadServerAnimationOnly(const char* binPath);
