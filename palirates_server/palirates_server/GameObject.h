#pragma once
#include "stdafx.h"
#include "ServerAnimLoader.h"

using namespace std;

class CSkinnedMesh;
class CLoadedModelInfo;
class CAnimationSets;

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
    float Obj_ID;

    shared_ptr<GameObject> child_obj = NULL;
    shared_ptr<GameObject> sibling_obj = NULL;
    shared_ptr<GameObject> m_pParent = NULL;

    std::shared_ptr<CAnimationController> m_pSkinnedAnimationController = NULL;

public:
    XMFLOAT4X4            m_xmf4x4Parent{};
    XMFLOAT4X4            m_xmf4x4World{};
    char                     m_pstrFrameName[64];

    std::shared_ptr<CStandardMesh> m_pMesh = NULL;

public:
    GameObject()
        : obj_type(Object_Type::etc)
        , Obj_Name("")
        , child_obj(nullptr)
        , sibling_obj(nullptr)
        , m_pParent(nullptr)
    {
        XMStoreFloat4x4(&m_xmf4x4Parent, XMMatrixIdentity());
        XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
    }
    ~GameObject() = default;
    void Set_Child(shared_ptr<GameObject> pChild);

    void Set_Name(string_view name);
    const string Get_Name() const { return Obj_Name; }

    shared_ptr<GameObject> Get_Child();
    shared_ptr<GameObject> Get_Sibling();
    shared_ptr<GameObject> GetParent() { return(m_pParent); }
    shared_ptr<GameObject> FindFrame(const char* pstrFrameName);

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


    void SetLook(const XMFLOAT3& xmf3Look);
    void SetUp(XMFLOAT3 xmf3Up);
    void SetRight(XMFLOAT3 xmf3Right);

    static CLoadedModelInfo* LoadGeometryAndAnimationFromFile(char* pstrFileName);
    static std::shared_ptr<GameObject> LoadFrameHierarchyFromFile(std::shared_ptr<GameObject> pParent, FILE* pInFile, int* pnSkinnedMeshes);
    static void LoadAnimationFromFile(FILE* pInFile, CLoadedModelInfo* pLoadedModel, char* pstrFileName);
    void FindAndSetSkinnedMesh(std::vector<std::shared_ptr<CSkinnedMesh>>& outSkinnedMeshes);
    void SetSkinnedMesh(std::shared_ptr<CSkinnedMesh> pMesh);
    void SetMesh(std::shared_ptr<CStandardMesh> pMesh);

    void Obj_Info(int depth = 0);
    Object_Type GetType() { return obj_type; }

    void SetID(float ID) { Obj_ID = ID; }
    float GetID() { return Obj_ID; }

    std::unordered_set<int> RootMotionTrackSet;

    std::shared_ptr<CAnimationController> GetSkinnedAnimationController() { return m_pSkinnedAnimationController; }
    void DelSkinnedAnimationController() { m_pSkinnedAnimationController.reset(); }

    virtual ServerSyncData MakeSyncData();
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

//struct Animation_Sync
//{
//    int track_index;
//    float weight;
//    float track_position;
//};
//
//struct AnimationTrackData
//{
//    std::vector<Animation_Sync> track_info_list;
//    bool stateChanged = false;
//};


class Skinned_GameObject : public GameObject
{
private:
    ServerSyncData animation_sync_data;


public:
    void SetAnimationSyncData(const ServerSyncData& data) { animation_sync_data = data; }
    void SetTrackInfoList(const std::vector<Animation_Sync>& list) { animation_sync_data.track_info_list = list; }
    void SetStateChanged(bool changed) { animation_sync_data.stateChanged = changed; }


    const ServerSyncData& GetAnimationSyncData() const { return animation_sync_data; }
    const std::vector<Animation_Sync>& GetTrackInfoList() const { return animation_sync_data.track_info_list; }

    
    ServerSyncData& GetAnimationSyncData() { return animation_sync_data; }
    std::vector<Animation_Sync>& GetTrackInfoList() { return animation_sync_data.track_info_list; }
    bool GetStateChanged() const { return animation_sync_data.stateChanged; }

    int n_Animation = 0;
    int RootIndex{ 0 };

    std::vector<float> prevWeights;
    std::vector<float> targetWeights;
};