#pragma once
#include "Shader.h"
#include "Timer.h"

#define MONSTER_HP_UI_MAX_NUM				70

#define CHARAACTER_SELECT_CHECK_INDEX 6
#define CHARAACTER_SELECT_CANCEL_INDEX 7

struct MonsterUIData
{
    XMFLOAT3 position;
    float hp;
    float dist;
    float yOffset;
    bool bBossHP{ false };
};

inline D2D1_RECT_F MakeNormalizedRect(
    float normCX, float normCY, float normW,
    const CTexture* pTexture,
    float scaleH = 1.0f 
)
{
    if (!pTexture) return D2D1::RectF(0, 0, 0, 0);

    UINT texW = pTexture->GetTextureWidth();
    UINT texH = pTexture->GetTextureHeight();
    if (texH == 0) texH = 1; 

    float textureAspect = static_cast<float>(texW) / texH;

    float cx = normCX * FRAME_BUFFER_WIDTH;
    float cy = normCY * FRAME_BUFFER_HEIGHT;

    float w = normW * FRAME_BUFFER_WIDTH;
    float h = (w / textureAspect) * scaleH;

    return D2D1::RectF(cx - w * 0.5f, cy - h * 0.5f, cx + w * 0.5f, cy + h * 0.5f);
}

inline bool IsPointInRect(const D2D1_RECT_F& rect, float x, float y)
{
    return x >= rect.left && x <= rect.right &&
        y >= rect.top && y <= rect.bottom;
}

inline bool IsMouseClicked()
{
    return (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
}

struct TextDesign
{
    std::string_view d_name;
    IDWriteTextFormat* d_text_format;
    ID2D1SolidColorBrush* d_text_color;

    TextDesign(std::string_view name, IDWriteTextFormat* text_format, ID2D1SolidColorBrush* text_color) : d_name(name), d_text_format(text_format), d_text_color(text_color) {}
    ~TextDesign() { d_text_format->Release(); d_text_color->Release(); }
};

struct TextBlock
{
    std::shared_ptr<TextDesign> text_design;

    WCHAR                           m_pstrText[256];
    D2D1_RECT_F                     m_d2dLayoutRect;

    TextBlock(const std::shared_ptr<TextDesign>& design, const WCHAR* text, D2D1_RECT_F layout_rect) : text_design(design), m_d2dLayoutRect(layout_rect) 
    {
        wcsncpy_s(m_pstrText, text, _TRUNCATE);
    }

};

class Text_UI_Manager
{
private:
    std::vector<TextBlock*> TextBlock_list;
    std::vector<std::shared_ptr<TextDesign>> text_design_list;

    IDWriteFactory* m_pd2dWriteFactory = NULL;
    ID2D1DeviceContext2* m_pd2dDeviceContext = NULL;


public:
    Text_UI_Manager();
    Text_UI_Manager(IDWriteFactory* d2dWriteFactory, ID2D1DeviceContext2* d2dDeviceContext);
    ~Text_UI_Manager();

    void Set_WriteFactory(IDWriteFactory* d2dWriteFactory) { m_pd2dWriteFactory = d2dWriteFactory; }
    void Set_DeviceContext(ID2D1DeviceContext2* d2dDeviceContext) { m_pd2dDeviceContext = d2dDeviceContext; }

    ID2D1SolidColorBrush* CreateBrush(D2D1::ColorF d2dColor);
    IDWriteTextFormat* CreateTextFormat(WCHAR* pszFontName, float fFontSize);

    std::shared_ptr<TextDesign>Create_Text_Design(std::string_view design_name, D2D1::ColorF d2dColor, WCHAR* pszFontName, float fFontSize);
    bool Add_Text_Design(std::shared_ptr<TextDesign>new_text_design);
    std::shared_ptr<TextDesign>Find_Text_Design(std::string_view design_name);

    void Add_TextBlock(TextBlock* text_block);
    void UpdateTextBlock(UINT nIndex, WCHAR* pstrUIText, D2D1_RECT_F* pd2dLayoutRect, std::shared_ptr<TextDesign> new_text_design);


    std::vector<TextBlock*>* Get_Text_Block_List() { return &TextBlock_list; }
};

class Text_UI_Renderer
{
public:
    Text_UI_Renderer(UINT nFrames, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight);
    ~Text_UI_Renderer();

    void InitializeDevice(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Resource** ppd3dRenderTargets);
    
    void Render(UINT nFrame, std::vector<TextBlock*>* block_list_ptr);

public:
    float                           m_fWidth = 0.0f;
    float                           m_fHeight = 0.0f;

    ID3D11DeviceContext*            m_pd3d11DeviceContext = NULL;
    ID3D11On12Device*               m_pd3d11On12Device = NULL;
    IDWriteFactory*                 m_pd2dWriteFactory = NULL;
    ID2D1Factory3*                  m_pd2dFactory = NULL;
    ID2D1Device2*                   m_pd2dDevice = NULL;
    ID2D1DeviceContext2*            m_pd2dDeviceContext = NULL;

    UINT                            m_nRenderTargets = 0;
    ID3D11Resource**                m_ppd3d11WrappedRenderTargets = NULL;
    ID2D1Bitmap1**                  m_ppd2dRenderTargets = NULL;

};

#define UI_EFFECT_NONE      0
#define UI_EFFECT_CUT_HP     (1 << 0) // 1
#define UI_EFFECT_FADE_OUT   (1 << 1) // 2
#define UI_EFFECT_SLIDE_DOWN   (1 << 2) // 4
#define UI_EFFECT_FADE_IN    (1 << 3) // 8
#define UI_EFFECT_TRANSLUCENT    (1 << 4) // 16

enum class UILayer : uint32_t
{
    None = 0,
    Default = 1 << 0,   
    Interactable = 1 << 1,   
    Debug = 1 << 2,   
    HP_bar = 1 << 3,  
    Menu = 1 << 4,   
    Dialogue = 1 << 5,
    screen = 1 << 6,
    Dialogue_Button = 1 << 7,
    Start = 1 << 8,
    Screen_Fade = 1 << 9,
    Clear = 1 << 10,
    All = 0xFFFFFFFF
};

inline UILayer operator|(UILayer a, UILayer b)
{
    return static_cast<UILayer>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline UILayer operator&(UILayer a, UILayer b)
{
    return static_cast<UILayer>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline UILayer& operator|=(UILayer& a, UILayer b)
{
    a = a | b;
    return a;
}

struct TextureBlock
{
    CTexture* pTexture = nullptr;
    D2D1_RECT_F screenRect;
    D2D1_RECT_F hitboxRect;
    std::shared_ptr<CTextureMesh> mesh = nullptr;

    std::function<void()> onClick;

    bool bHovered = false;
    bool bClicked = false;
    bool bActive = true;

    float hp = 1.0f;
    int ui_type = 0;
    float start_time = 0.0f;

    bool bPendingActivation = false;
    bool wasActive{ false };
    bool firstActive{ false };
    UILayer layer = UILayer::Default;

    XMFLOAT4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };     
    XMFLOAT4 hoverGlowColor = { 1.0f, 0.0f, 0.0f, 1.0f };      

    TextureBlock(CTexture* texture, const D2D1_RECT_F& rect, std::shared_ptr<CTextureMesh> meshPtr, UILayer layerMask = UILayer::Default, const XMFLOAT2& offsetNormalized = { 0.0f, 0.0f }, const XMFLOAT2& scale = { 1.0f, 1.0f });
    TextureBlock() = default;
    void UpdateScreenRect(float normCX, float normCY, float normW, float scaleH);
};

class Texture_UI_Renderer
{
public:
    Texture_UI_Renderer(ID3D12Device* device);
    ~Texture_UI_Renderer();

    void CreateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    void UpdateShaderVariables(float currentTime, float elapsedTime, ID3D12GraphicsCommandList* cmdList);

    void Render_UI_Textures(ID3D12GraphicsCommandList* cmdList, std::vector<TextureBlock*>* pTextureList);

private:
    ID3D12Device* m_pd3dDevice = nullptr;
    
    struct CB_FRAMEWORK_INFO
    {
        float m_fCurrentTime = 0.0f;
        float m_fElapsedTime = 0.0f;
    };

    ID3D12Resource* m_pCBFrameInfo = nullptr;
    CB_FRAMEWORK_INFO* m_pMappedCBFrameInfo = nullptr;
    ID3D12Resource* m_pVertexBuffer = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView = {};
};

class Texture_UI_Manager
{
public:
    static std::array<std::shared_ptr<CTexture>, 6> s_MugTextures;

private:
    std::vector<std::unique_ptr<TextureBlock>> textureBlockList;
    std::vector<TextureBlock*> rawTextureBlockList;
    std::unique_ptr<CTextureToScreenShader> textureShader;
    std::unique_ptr<Texture_UI_Renderer> textureRenderer;
    std::shared_ptr<ID3D12RootSignature> m_TextureUI_GraphicsRootSignature = NULL;
    std::vector<TextureBlock*> monsterHPBlocks;
    std::vector<std::shared_ptr<TextureBlock>> mugBlocks;
    std::vector<TextureBlock*> rawMugPtrs;
    std::vector<std::shared_ptr<TextureBlock>> readyCheckBlocks;
    std::vector<TextureBlock*> rawReadyCheckPtrs;

public:

    void SetShader(std::unique_ptr<CTextureToScreenShader> shader) { textureShader = std::move(shader); }
    void SetRenderer(std::unique_ptr<Texture_UI_Renderer> renderer) { textureRenderer = std::move(renderer);}
    Texture_UI_Renderer* GetRenderer() const { return textureRenderer.get(); }
    void SetRootSignature(std::shared_ptr<ID3D12RootSignature> rootSignature) { m_TextureUI_GraphicsRootSignature = rootSignature; }
    void Add_TextureBlock(std::unique_ptr<TextureBlock> block) { textureBlockList.emplace_back(std::move(block)); }
    void RenderAll(ID3D12GraphicsCommandList* cmdList, float currentTime, float elapsedTime);
    std::vector<TextureBlock*> GetTextureBlockPtrs();
    CTextureToScreenShader* GetShader() const { return textureShader.get(); }
    std::vector<TextureBlock*>& GetMonsterHPBlocks() { return monsterHPBlocks; }
    void AddMonsterHPBlock(TextureBlock* block) { monsterHPBlocks.emplace_back(block); }
    void DeactivateAllMonsterHPBlocks()
    {
        for (auto& block : monsterHPBlocks)
        {
            if (block) {
                block->bActive = false;
                if (!block->wasActive) {
                    block->firstActive = true;
                }
                if (!block->firstActive) {
                    block->wasActive = false;
                }
            }
        }
    }
    std::vector<std::shared_ptr<TextureBlock>> GetMugBlocks() { return mugBlocks; }
    void AddMugBlock(const std::shared_ptr<TextureBlock>& block)
    {
        mugBlocks.push_back(block);             
        rawMugPtrs.push_back(block.get());      
    }
    void DeactivateAllMugBlocks()
    {
        for (auto& block : rawMugPtrs)
        {
            if (block) block->bActive = false;
        }
    }
    std::vector<TextureBlock*>& GetReadyCheckBlocks() { return rawReadyCheckPtrs; }
    void AddReadyCheckBlock(TextureBlock* block) { rawReadyCheckPtrs.emplace_back(block); }
    void DeactivateReadyCheckBlocks() 
    {
        uint32_t targetMask = static_cast<uint32_t>(UILayer::Interactable);

        for (auto& block : rawReadyCheckPtrs)
        {
            if (block) {
                if (block && (static_cast<uint32_t>(block->layer) & targetMask) == 0)
                    block->bActive = false;
            }
        }
    }
};