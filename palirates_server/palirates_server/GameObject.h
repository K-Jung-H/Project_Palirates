#pragma once
#include "DX_Setter.h"
#include <vector>
#include <sstream>

using namespace std;

enum class Object_Type
{
    player,
    monster,
    etc
};

class GameObject : public std::enable_shared_from_this<GameObject>
{
protected:
    Object_Type obj_type;
    string Obj_Name;

    shared_ptr<GameObject> child_obj = NULL;
    shared_ptr<GameObject> sibling_obj = NULL;
    shared_ptr<GameObject> m_pParent = NULL;

public:
    XMFLOAT4X4            m_xmf4x4Parent{};
    XMFLOAT4X4            m_xmf4x4World{};


public:
    GameObject() = default;
    ~GameObject() = default;
    void Set_Child(shared_ptr<GameObject> pChild);

    void Set_Name(string_view name);
    const string Get_Name() const { return Obj_Name; }

    shared_ptr<GameObject> Get_Child();
    shared_ptr<GameObject> Get_Sibling();
    shared_ptr<GameObject> GetParent() { return(m_pParent); }
    shared_ptr<GameObject> FindFrame(std::string_view name);

    void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);


    virtual void SetPosition(float x, float y, float z);
    virtual void SetPosition(XMFLOAT3 xmf3Position);

    void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
    void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
    void Rotate(XMFLOAT4* pxmf4Quaternion);

    void Move(XMFLOAT3 xmf3Offset);


    XMFLOAT3 GetPosition();
    XMFLOAT3 GetLook();
    XMFLOAT3 GetUp();
    XMFLOAT3 GetRight();


    void SetLook(XMFLOAT3 xmf3Look);
    void SetUp(XMFLOAT3 xmf3Up);
    void SetRight(XMFLOAT3 xmf3Right);

};


class Skinned_GameObject : public GameObject
{
protected:
    vector<float> animPositions;
    vector<float> animWeights;

    vector<float> trackPositions;
    vector<float> trackWeights;

public:
    // Getter - 참조 반환 (수정 가능)
    std::vector<float>& GetAnimPositions() { return animPositions; }
    std::vector<float>& GetAnimWeights() { return animWeights; }
    std::vector<float>& GetTrackPositions() { return trackPositions; }
    std::vector<float>& GetTrackWeights() { return trackWeights; }

    // Setter - 통째로 대입
    void SetAnimPositions(const std::vector<float>& v) { animPositions = v; }
    void SetAnimWeights(const std::vector<float>& v) { animWeights = v; }
    void SetTrackPositions(const std::vector<float>& v) { trackPositions = v; }
    void SetTrackWeights(const std::vector<float>& v) { trackWeights = v; }
};

