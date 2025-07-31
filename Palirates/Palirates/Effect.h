#pragma once
#include "Object.h"

class Sprite_Shader;

enum class Sprite_Effect_Lifetime
{
    OneShot,
    Looping,
    etc,
};

enum class Sprite_Effect_Type
{
    Hit_1,
    Hit_2,
    etc,
};

struct SpriteInfo
{
    UINT frameCols;
    UINT frameRows;
    UINT totalFrames;
    float frameTime;
};

class Sprite_Object : public CGameObject
{
protected:
    shared_ptr<CMaterial> sprite_material = nullptr;

    SpriteInfo sprite_info{};

    Sprite_Effect_Type effect_type = Sprite_Effect_Type::etc;
    Sprite_Effect_Lifetime life_type = Sprite_Effect_Lifetime::etc;

    float TimeElapsed = 0.0f;
    float Scale_value = 1.0f;
public:
    Sprite_Object();
    virtual ~Sprite_Object() {}

    virtual void Set_BaseTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* filename);
    virtual void Set_BaseTexture(shared_ptr<CTexture> new_texture);



    virtual void Animate(float fTimeElapsed);
    virtual void Update_Sprite_Info(ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
    virtual void Instance_Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VERTEX_BUFFER_VIEW instance_buffer, int instance_num);
    

    virtual void Reset();

    void Set_Sprite_Info(SpriteInfo new_sprite_info) { sprite_info = new_sprite_info; }
    void Set_Life_Style(Sprite_Effect_Lifetime new_life_style) { life_type = new_life_style; }
    void Set_Sprite_Effect_Type(Sprite_Effect_Type type) { effect_type = type; }
  
    Sprite_Effect_Lifetime Get_Sprite_Life_Style() { return life_type; }
    Sprite_Effect_Type Get_Sprite_Effect_Type() { return effect_type; }
    float Get_Elapsed_Time() { return TimeElapsed; }
    float Get_Scale_Value() { return Scale_value; }
};



class Aura_Object : public Sprite_Object
{
protected:
    shared_ptr<CGameObject> m_pTargetObject = nullptr;

public:
    Aura_Object(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float bottom_radius = 20.0f, float top_radius = 10.0f, float height = 5.0f);
    virtual ~Aura_Object() {}

    virtual void Animate(float fTimeElapsed);
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

    void Set_Aura_Target(shared_ptr<CGameObject> target) { m_pTargetObject = target; }
    shared_ptr<CGameObject> Get_Aura_Target() { return m_pTargetObject; }
};


struct Sprite_Effect_Format
{
    shared_ptr<CMesh> mesh;
    shared_ptr<CTexture> texture;
    SpriteInfo spriteInfo;
    Sprite_Effect_Lifetime lifetime;
};

struct Sprite_Effect_Instance_Data
{
    XMFLOAT3 position;
    float scale_value;

    XMFLOAT3 blending_color;
    float elapsed_time;
};

struct Sprite_Effect_Instance_Info
{
    ID3D12Resource* Instance_data = NULL;
    Sprite_Effect_Instance_Data* Mapped_Instance_data = NULL;
    D3D12_VERTEX_BUFFER_VIEW m_d3dInstancingBufferView;
    int Instancing_num = 0;

    void Create_Instance_Data_ShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    void Update_Instance_Data(std::vector<std::shared_ptr<Sprite_Object>> obj_list);
    void Release_Instance_Data_ShaderVariables();
};

#define MAX_EFFECT_NUM 128

class Sprite_Effect_Manager
{
public:
    static Sprite_Shader* sprite_shader;
    static void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature);
protected:
    std::unordered_map<std::string, std::shared_ptr<CMesh>> mesh_map;
    std::unordered_map<std::string, pair<SpriteInfo, std::shared_ptr<CTexture>>> sprite_texture_map;
    std::unordered_map <Sprite_Effect_Type, Sprite_Effect_Format> sprite_format_map;

    std::unordered_map<Sprite_Effect_Type, Sprite_Effect_Instance_Info> instance_info_map;
    std::unordered_map < Sprite_Effect_Type, std::vector<std::shared_ptr<Sprite_Object>>> effect_object_map;

public:
    Sprite_Effect_Manager(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    ~Sprite_Effect_Manager() {}

    void Init_Format(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

    void Init_Sprite_Format(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    void Init_Mesh_Format(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

    void Create_Instance_Data_ShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    void  Release_Instance_Data_ShaderVariables() {}


    shared_ptr<Sprite_Object> Add_Effect(Sprite_Effect_Type type, Sprite_Effect_Lifetime style, XMFLOAT3 effect_position);
    shared_ptr<Sprite_Object> Add_Effect(Sprite_Effect_Type type, XMFLOAT3 effect_position);
    shared_ptr<Sprite_Object> Recycle_Effect(Sprite_Effect_Type type, Sprite_Effect_Lifetime style);

    void Animate_Effects_All(float fTimeElapsed);
    void Animate_Effects(float fTimeElapsed, Sprite_Effect_Type type);

    void Update_Effects_All();
    void Update_Effects(Sprite_Effect_Type type);

    void Render_Effects(ID3D12GraphicsCommandList* cmdList, CCamera* pCamera, Sprite_Effect_Type type); 
    void Render_Effects_All(ID3D12GraphicsCommandList* cmdList, CCamera* pCamera);

};