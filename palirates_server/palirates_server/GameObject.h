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


class Boat_Object : public GameObject
{
    XMFLOAT3 m_xmf3Velocity{};
    float m_fMaxVelocityXZ = 200.0f;
    float m_fFriction = 50.0f;
    float m_fRotationSpeed = 0.0f;

public:
    Boat_Object();
    virtual ~Boat_Object();

    void MoveForward(float speed);
    void Add_Rotate(float angleDelta);
    void Animate(float fTimeElapsed);
    void HandleBoundaryReflection(float boundary);

    void Set_Velocity(const XMFLOAT3& v) { m_xmf3Velocity = v; }
    XMFLOAT3 Get_Velocity() const { return m_xmf3Velocity; }
};

struct Animation_Sync
{
    int track_index;
    float weight;
    float track_position;
};

struct AnimationTrackData
{
    std::vector<Animation_Sync> track_info_list;
    bool stateChanged = false;
};


class Skinned_GameObject : public GameObject
{
private:
    AnimationTrackData animation_sync_data;

public:
    void SetAnimationSyncData(const AnimationTrackData& data) { animation_sync_data = data; }
    void SetTrackInfoList(const std::vector<Animation_Sync>& list) { animation_sync_data.track_info_list = list; }
    void SetStateChanged(bool changed) { animation_sync_data.stateChanged = changed; }


    const AnimationTrackData& GetAnimationSyncData() const { return animation_sync_data; }
    const std::vector<Animation_Sync>& GetTrackInfoList() const { return animation_sync_data.track_info_list; }

    
    AnimationTrackData& GetAnimationSyncData() { return animation_sync_data; }
    std::vector<Animation_Sync>& GetTrackInfoList() { return animation_sync_data.track_info_list; }
    bool GetStateChanged() const { return animation_sync_data.stateChanged; }
};

