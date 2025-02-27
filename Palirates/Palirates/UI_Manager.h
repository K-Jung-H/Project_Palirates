#pragma once

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


// 프레임워크에서 동작하는 객체
// 씬에서 TextBlock배열을 전달받아, 일괄 렌더링

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