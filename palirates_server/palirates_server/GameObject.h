#pragma once
#include "stdafx.h"
#include "ServerAnimLoader.h"
//#include "AnimationRegistry.h"
//#include <unordered_set> 

using namespace std;

class CSkinnedMesh;
class CLoadedModelInfo;
class CAnimationSets;
class Scene;

enum class Object_Type
{
    player,
    monster,
    etc,
    weapon
};

class GameObject : public std::enable_shared_from_this<GameObject>
{
protected:
    Object_Type obj_type;
    string Obj_Name;
    int Obj_ID;

    shared_ptr<GameObject> child_obj = NULL;
    shared_ptr<GameObject> sibling_obj = NULL;
    shared_ptr<GameObject> m_pParent = NULL;

    std::shared_ptr <BoundingOrientedBox> m_OBB = NULL;

    bool bActive = true;
    bool bCanCollide = true;
    bool bIsInvincible = false;
    float invincibleTimeRemaining = 0.0f;
    float invincibleDuration = 2.0f;

public:
    XMFLOAT4X4            m_xmf4x4Parent{};
    XMFLOAT4X4            m_xmf4x4World{};
    char                     m_pstrFrameName[64];

    std::shared_ptr<CStandardMesh> m_pMesh = NULL;
    std::shared_ptr<GameObject> m_pRootModel = NULL;

    XMMATRIX WeaponCustomRotation = XMMatrixIdentity();
    float m_fScale = 1.0f;
    XMFLOAT3 CustomOBBScale = XMFLOAT3(1.0f, 1.0f, 1.0f);
    bool bDead = false;
	bool BreathObject = false;
    bool bHittingCmd{ false };

    Scene* m_pOwnerScene = nullptr;

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
    void RotateTowardsTarget(const XMFLOAT3& targetPos, float deltaTime, float rotationSpeed);

    void SetScale(float x, float y, float z, bool keepPosition = false);

    void Move(XMFLOAT3 xmf3Offset);


    XMFLOAT3 GetPosition();
    XMFLOAT3 GetLook();
    XMFLOAT3 GetUp();
    XMFLOAT3 GetRight();


    void SetLook(const XMFLOAT3& xmf3Look);
    void SetUp(XMFLOAT3 xmf3Up);
    void SetRight(XMFLOAT3 xmf3Right);

    static CLoadedModelInfo* LoadGeometryAndAnimationFromFile(const char* pstrFileName);
    static void LoadAnimationFromFile(FILE* pInFile, CLoadedModelInfo* pLoadedModel, const char* pstrFileName);
    static std::shared_ptr<GameObject> LoadFrameHierarchyFromFile(std::shared_ptr<GameObject> pParent, FILE* pInFile, int* pnSkinnedMeshes);

    static std::shared_ptr<GameObject> Load_Scene(char* pstrFileName);
    static std::shared_ptr<GameObject> Load_Scene_FrameHierarchyFromFile(std::shared_ptr<GameObject> pParent, FILE* pInFile);

    static void FlattenGameObjectHierarchy(std::shared_ptr<GameObject> node, std::vector<shared_ptr<GameObject>>& outList);


    void FindAndSetSkinnedMesh(std::vector<std::shared_ptr<CSkinnedMesh>>& outSkinnedMeshes);
    void SetSkinnedMesh(std::shared_ptr<CSkinnedMesh> pMesh);
    void SetMesh(std::shared_ptr<CStandardMesh> pMesh);

    void Obj_Info(int depth = 0);
    void SetType(Object_Type Object_Type) { obj_type = Object_Type; }
    Object_Type GetType() { return obj_type; }

    virtual void UpdateWorldOBB();
    virtual std::shared_ptr<BoundingOrientedBox> Get_Collider_OBB() { return m_OBB; }
    virtual void Set_Collider_OBB(std::shared_ptr<BoundingOrientedBox> obb_ptr) { m_OBB = obb_ptr; }

    std::shared_ptr<CAnimationController> m_pSkinnedAnimationController = NULL;


    void SetID(int ID) { Obj_ID = ID; }
    int GetID() { return Obj_ID; }

    std::unordered_set<int> RootMotionTrackSet;
    
    virtual ServerSyncData MakeSyncData() { return ServerSyncData(); };

    virtual void update(float deltaTime) {};

    void Set_Active(bool active) { bActive = active; }
    bool Get_Active() const { return bActive; }

    void SetCanCollide(bool canCollide) { bCanCollide = canCollide; }
    bool CanCollide() const { return bCanCollide; }

    void SetIsInvincible(bool IsInvincible) { bIsInvincible = IsInvincible; }
    bool IsInvincible() const { return bIsInvincible; }

    void RotateTowardsDirection(const XMFLOAT3& direction, float deltaTime);
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

class Skinned_GameObject : public GameObject
{
private:
    ServerSyncData animation_sync_data;

protected:
    std::shared_ptr<CAnimationController> m_pSkinnedAnimationController = NULL;
    //Monster_Type monsterType = Monster_Type::ETC;
public:
    void SetAnimationSyncData(const ServerSyncData& data) { animation_sync_data = data; }
    void SetTrackInfoList(const std::vector<Animation_Sync>& list) { animation_sync_data.track_info_list = list; }
    void SetStateChanged(bool changed) { animation_sync_data.stateChanged = changed; }
    void SetStateChangeNum(int stateNum) { animation_sync_data.changedStateNum = stateNum; }

    const ServerSyncData& GetAnimationSyncData() const { return animation_sync_data; }
    const std::vector<Animation_Sync>& GetTrackInfoList() const { return animation_sync_data.track_info_list; }

    
    ServerSyncData& GetAnimationSyncData() { return animation_sync_data; }
    std::vector<Animation_Sync>& GetTrackInfoList() { return animation_sync_data.track_info_list; }
    bool GetStateChanged() const { return animation_sync_data.stateChanged; }

    std::shared_ptr<CAnimationController> GetSkinnedAnimationController() { return m_pSkinnedAnimationController; }
    void DelSkinnedAnimationController() { m_pSkinnedAnimationController.reset(); }

    int n_Animation = 0;
    int RootIndex{ 0 };

    std::vector<float> prevWeights;
    std::vector<float> targetWeights;

    char* WeaponName = "";
    std::shared_ptr<GameObject> Weapon_ptr = nullptr;

    bool bDead = false;
    int currStateTrackIdx = 0;
    float stateElapsedTime = 0.0f;

    virtual void update(float deltaTime) override {};
    void SetupWeaponCollider();

    virtual void InitAnimationController(const std::string& filepath, int animCount, int rootIdx, const std::unordered_set<int>& onceTracks) = 0;

    //virtual Get_Collider_OBB
};