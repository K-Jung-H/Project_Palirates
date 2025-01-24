#include "stdafx.h"
#include "UILayer.h"

using namespace std;

Text_UI_Manager::Text_UI_Manager()
{

}

Text_UI_Manager::Text_UI_Manager(IDWriteFactory* d2dWriteFactory, ID2D1DeviceContext2* d2dDeviceContext)
{
    m_pd2dWriteFactory = d2dWriteFactory;
    d2dWriteFactory->AddRef();

    m_pd2dDeviceContext = d2dDeviceContext;
    d2dDeviceContext->AddRef();
}


Text_UI_Manager::~Text_UI_Manager()
{
    m_pd2dWriteFactory->Release();
    m_pd2dDeviceContext->Release();

    for (TextBlock* block_ptr : TextBlock_list)
        delete block_ptr;
    TextBlock_list.clear();
    text_design_list.clear();
}

ID2D1SolidColorBrush* Text_UI_Manager::CreateBrush(D2D1::ColorF d2dColor)
{
    ID2D1SolidColorBrush* pd2dDefaultTextBrush = NULL;
    m_pd2dDeviceContext->CreateSolidColorBrush(d2dColor, &pd2dDefaultTextBrush);

    return(pd2dDefaultTextBrush);
}

IDWriteTextFormat* Text_UI_Manager::CreateTextFormat(WCHAR* pszFontName, float fFontSize)
{
    IDWriteTextFormat* pdwDefaultTextFormat = NULL;
    HRESULT hr = m_pd2dWriteFactory->CreateTextFormat(pszFontName, nullptr, DWRITE_FONT_WEIGHT_MEDIUM, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fFontSize, L"en-us", &pdwDefaultTextFormat);

#ifdef _DEBUG
    if (FAILED(hr))
    {
        // 오류 메시지 출력
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        OutputDebugString(errMsg); // 디버그 창에 출력

        return nullptr; // NULL 반환
    }
#endif

    pdwDefaultTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED);
    pdwDefaultTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);


    return(pdwDefaultTextFormat);
}

std::shared_ptr<TextDesign> Text_UI_Manager::Create_Text_Design(std::string_view design_name, D2D1::ColorF d2dColor, WCHAR* FontName, float FontSize)
{
    ID2D1SolidColorBrush* text_color = CreateBrush(d2dColor);
    IDWriteTextFormat* text_format = CreateTextFormat(FontName, FontSize);

    std::shared_ptr<TextDesign> text_design = std::make_shared<TextDesign>(design_name, text_format, text_color);

    return text_design;
}

bool Text_UI_Manager::Add_Text_Design(std::shared_ptr<TextDesign>new_text_design)
{
    new_text_design->d_name;
    auto it = std::find_if(text_design_list.begin(), text_design_list.end(), [&new_text_design](const std::shared_ptr<TextDesign>& text_design)
        {
            return text_design->d_name == new_text_design->d_name;
        }
    );

    if (it != text_design_list.end())
        return false;
    else
    {
        text_design_list.push_back(new_text_design);
        return true;
    }
}

std::shared_ptr<TextDesign>Text_UI_Manager::Find_Text_Design(std::string_view design_name)
{
    auto it = std::find_if(text_design_list.begin(), text_design_list.end(), [&design_name](const std::shared_ptr<TextDesign>& text_design) 
        {
            return text_design->d_name == design_name;
        }
    );

    if (it != text_design_list.end()) 
        return *it; 

    return NULL; 
}

void Text_UI_Manager::Add_TextBlock(TextBlock* new_text_block)
{
    if(new_text_block)
        TextBlock_list.push_back(new_text_block);
}


void Text_UI_Manager::UpdateTextBlock(UINT nIndex, WCHAR* pstrUIText, D2D1_RECT_F* pd2dLayoutRect, std::shared_ptr<TextDesign> new_text_design)
{
    if (nIndex >= TextBlock_list.size())
    {
#ifdef DEBUG_MESSAGE
        DebugOutput("\nText_UI_Manager - Wrong index\n");
#endif
        return;
    }

    if (pstrUIText)
        wcscpy_s(TextBlock_list[nIndex]->m_pstrText, 256, pstrUIText);

    if (pd2dLayoutRect)
        TextBlock_list[nIndex]->m_d2dLayoutRect = *pd2dLayoutRect;

    if (new_text_design)
        TextBlock_list[nIndex]->text_design = new_text_design;
}


//=======================================================================================

Text_UI_Renderer::Text_UI_Renderer(UINT nFrames, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight)
{
    m_fWidth = static_cast<float>(nWidth);
    m_fHeight = static_cast<float>(nHeight);
    m_nRenderTargets = nFrames;
    m_ppd3d11WrappedRenderTargets = new ID3D11Resource*[nFrames];
    m_ppd2dRenderTargets = new ID2D1Bitmap1*[nFrames];

    InitializeDevice(pd3dDevice, pd3dCommandQueue, ppd3dRenderTargets);
}

Text_UI_Renderer::~Text_UI_Renderer()
{
    for (UINT i = 0; i < m_nRenderTargets; i++)
    {
        ID3D11Resource* ppResources[] = { m_ppd3d11WrappedRenderTargets[i] };
        m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    }

    m_pd2dDeviceContext->SetTarget(nullptr);
    m_pd3d11DeviceContext->Flush();

    for (UINT i = 0; i < m_nRenderTargets; i++)
    {
        m_ppd2dRenderTargets[i]->Release();
        m_ppd3d11WrappedRenderTargets[i]->Release();
    }

    m_pd2dDeviceContext->Release();
    m_pd2dWriteFactory->Release();
    m_pd2dDevice->Release();
    m_pd2dFactory->Release();
    m_pd3d11DeviceContext->Release();
    m_pd3d11On12Device->Release();
}



void Text_UI_Renderer::InitializeDevice(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Resource** ppd3dRenderTargets)
{
    UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D2D1_FACTORY_OPTIONS d2dFactoryOptions = { };

#if defined(_DEBUG) || defined(DBG)
    d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ID3D11Device* pd3d11Device = NULL;
    ID3D12CommandQueue* ppd3dCommandQueues[] = { pd3dCommandQueue };
    ::D3D11On12CreateDevice(pd3dDevice, d3d11DeviceFlags, nullptr, 0, reinterpret_cast<IUnknown**>(ppd3dCommandQueues), _countof(ppd3dCommandQueues), 0, (ID3D11Device **)&pd3d11Device, (ID3D11DeviceContext **)&m_pd3d11DeviceContext, nullptr);

    pd3d11Device->QueryInterface(__uuidof(ID3D11On12Device), (void **)&m_pd3d11On12Device);
    pd3d11Device->Release();

#if defined(_DEBUG) || defined(DBG)
    ID3D12InfoQueue* pd3dInfoQueue;
    if (SUCCEEDED(pd3dDevice->QueryInterface(IID_PPV_ARGS(&pd3dInfoQueue))))
    {
        D3D12_MESSAGE_SEVERITY pd3dSeverities[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_MESSAGE_ID pd3dDenyIds[] = { D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE };

        D3D12_INFO_QUEUE_FILTER d3dInforQueueFilter = { };
        d3dInforQueueFilter.DenyList.NumSeverities = _countof(pd3dSeverities);
        d3dInforQueueFilter.DenyList.pSeverityList = pd3dSeverities;
        d3dInforQueueFilter.DenyList.NumIDs = _countof(pd3dDenyIds);
        d3dInforQueueFilter.DenyList.pIDList = pd3dDenyIds;

        pd3dInfoQueue->PushStorageFilter(&d3dInforQueueFilter);
    }
    pd3dInfoQueue->Release();
#endif

    IDXGIDevice* pdxgiDevice = NULL;
    m_pd3d11On12Device->QueryInterface(__uuidof(IDXGIDevice), (void **)&pdxgiDevice);

    ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, (void **)&m_pd2dFactory);
    HRESULT hResult = m_pd2dFactory->CreateDevice(pdxgiDevice, (ID2D1Device2 **)&m_pd2dDevice);
    m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, (ID2D1DeviceContext2 **)&m_pd2dDeviceContext);

    m_pd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

    ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **)&m_pd2dWriteFactory);
    pdxgiDevice->Release();

    D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

    for (UINT i = 0; i < m_nRenderTargets; i++)
    {
        D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
        m_pd3d11On12Device->CreateWrappedResource(ppd3dRenderTargets[i], &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&m_ppd3d11WrappedRenderTargets[i]));
        IDXGISurface* pdxgiSurface = NULL;
        m_ppd3d11WrappedRenderTargets[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&pdxgiSurface);
        m_pd2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface, &d2dBitmapProperties, &m_ppd2dRenderTargets[i]);
        pdxgiSurface->Release();
    }
}



void Text_UI_Renderer::Render(UINT nFrame, std::vector<TextBlock*>* block_list_ptr)
{

    ID3D11Resource* ppResources[] = { m_ppd3d11WrappedRenderTargets[nFrame] };

    m_pd2dDeviceContext->SetTarget(m_ppd2dRenderTargets[nFrame]);
    m_pd3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));

    
    m_pd2dDeviceContext->BeginDraw();
    if (block_list_ptr != NULL)
    {
        for (const TextBlock* txt_ptr : *block_list_ptr)
        {
            ID2D1SolidColorBrush* color = txt_ptr->text_design->d_text_color;
            IDWriteTextFormat* format = txt_ptr->text_design->d_text_format;
            m_pd2dDeviceContext->DrawText(txt_ptr->m_pstrText, (UINT)wcslen(txt_ptr->m_pstrText), format, txt_ptr->m_d2dLayoutRect, color);
        }
    }
    m_pd2dDeviceContext->EndDraw();

    
    m_pd3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    m_pd3d11DeviceContext->Flush();
}

