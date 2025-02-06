//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

ID3D12DescriptorHeap *CScene::m_pd3dCbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorNextHandle;



CScene::CScene()
{
}

CScene::~CScene()
{
	DebugOutput("\nDelete Scene");

}

void CScene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 5;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights[0].m_fRange = 300.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.3f, 0.8f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(230.0f, 330.0f, 480.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights[1].m_bEnable = true;
	m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights[1].m_fRange = 500.0f;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[1].m_fFalloff = 8.0f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

	m_pLights[2].m_bEnable = true;
	m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, -1.0f);

	m_pLights[3].m_bEnable = true;
	m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights[3].m_fRange = 600.0f;
	m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.7f, 0.0f, 1.0f);
	m_pLights[3].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[3].m_xmf3Position = XMFLOAT3(550.0f, 330.0f, 530.0f);
	m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[3].m_fFalloff = 8.0f;
	m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));
	m_pLights[4].m_bEnable = true;
	m_pLights[4].m_nType = POINT_LIGHT;
	m_pLights[4].m_fRange = 200.0f;
	m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.3f, 0.3f, 1.0f);
	m_pLights[4].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[4].m_xmf3Position = XMFLOAT3(600.0f, 250.0f, 700.0f);
	m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
}

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 76); //SuperCobra(17), Gunship(2), Player:Mi24(1), Angrybot()

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature); 

	BuildDefaultLightsAndMaterials();

	obj_manager = new Object_Manager();

#ifdef RENDER_OBB
	obj_manager->Create_OBB_Drawer(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
#endif

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);


	XMFLOAT3 xmf3Scale(20.0f, 10.0f, 20.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.3f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 0, 0, 257, 257, xmf3Scale, xmf4Color, 32,3);
//	m_pTerrain->SetPosition(XMFLOAT3(1000.0f, 0.0f, 1000.0f));


	CLoadedModelInfo* pHumanModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Human.bin", NULL);

	string obj_name_1 = "test_obj_name_1";
	string obj_name_2 = "test_obj_name_2";
	string obj_name_3 = "test_obj_name_3";


	std::string_view name_view = obj_name_1;
	std::shared_ptr<CHumanObject> humanObject_1 = std::make_shared<CHumanObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pHumanModel, 2);
	humanObject_1->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	humanObject_1->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 2);
	humanObject_1->m_pSkinnedAnimationController->SetTrackEnable(0, true);
	humanObject_1->m_pSkinnedAnimationController->SetTrackEnable(1, true);
	humanObject_1->SetPosition(410.0f, m_pTerrain->Get_Mesh_Height(NULL, 400.0f, 735.0f), 735.0f);
	humanObject_1->SetScale(10.0f, 10.0f, 10.0f);
	humanObject_1->Set_Name(name_view);
	obj_manager->Add_Object(humanObject_1);
	
	//====================================================
	// 테스트용 코드	
//	humanObject_1->m_pSkinnedAnimationController->Bone_Info();
	CGameObject* test_obj  = humanObject_1->FindFrame("MiddleFinger3_R");
	CGameObject* test_obj2 = humanObject_1->FindFrame("Shoulder_R");

	test_obj->Add_Collider(0.0f);
	test_obj2->Add_Collider(1.0f);

	//====================================================

	name_view = obj_name_2;
	std::shared_ptr<CHumanObject> humanObject_2 = std::make_shared<CHumanObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pHumanModel, 1);
	humanObject_2->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	humanObject_2->SetPosition(430.0f, m_pTerrain->Get_Mesh_Height(400.0f, 700.0f), 700.0f);
	humanObject_2->SetScale(10.0f, 10.0f, 10.0f);
	humanObject_2->Set_Name(name_view);
	obj_manager->Add_Object(humanObject_2);


	name_view = obj_name_3;
	std::shared_ptr<CHumanObject> humanObject_3 = std::make_shared<CHumanObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pHumanModel, 1);
	humanObject_3->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
	humanObject_3->SetPosition(400.0f, m_pTerrain->Get_Mesh_Height(400.0f, 720.0f), 720.0f);
	humanObject_3->SetScale(10.0f, 10.0f, 10.0f);
	humanObject_3->Set_Name(name_view);
	obj_manager->Add_Object(humanObject_3);



	//float* pfData = new float[2];
	//pfData[0] = 0.0f;
	//pfData[1] = 1.0f;
	//m_ppHierarchicalGameObjects[6] = new CEthanObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEthanModel, 1);
	//m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	//m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetCallbackKeys(0, 2);
	//m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetCallbackKey(0, 0, 0.0f, &pfData[0]);
	//CAnimationSet* pAnimationSet = m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[1];
	//m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetCallbackKey(0, 1, pAnimationSet->m_fLength, &pfData[1]);
	//CRootMotionCallbackHandler* pRootMotionCallbackHandler = new CRootMotionCallbackHandler();
	//m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pRootMotionCallbackHandler);
	//m_ppHierarchicalGameObjects[6]->SetRootMotion(true);
	//m_ppHierarchicalGameObjects[6]->SetPosition(350.0f, m_pTerrain->GetHeight(350.0f, 670.0f), 670.0f);
	//m_ppHierarchicalGameObjects[6]->Rotate(0.0f, -90.0f, 0.0f);
	//m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetTrackSpeed(0, 0.75f);

	m_nShaders = 0;
	m_ppShaders = new CShader*[m_nShaders];

	if (pHumanModel)
		delete pHumanModel;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}
#ifdef WRITE_TEXT_UI
void CScene::Build_Text_UI(Text_UI_Renderer* text_ui_renderer_ptr)
{
	text_ui_manager = new Text_UI_Manager(text_ui_renderer_ptr->m_pd2dWriteFactory, text_ui_renderer_ptr->m_pd2dDeviceContext);

	if (text_ui_manager)
	{
		std::shared_ptr<TextDesign> design_ptr = text_ui_manager->Create_Text_Design("White_Text", D2D1::ColorF(D2D1::ColorF::White, 1.0f), L"Gothic", 20.0f);
		text_ui_manager->Add_Text_Design(design_ptr);

		D2D1_RECT_F player_pos_text_area = D2D1::RectF(0.0f, 0.0f, 400.0f, 30.0f);
		D2D1_RECT_F player_normal_text_area = D2D1::RectF(0.0f, 30.0f, 400.0f, 60.0f);
		D2D1_RECT_F tile_info_text_area = D2D1::RectF(0.0f, 60.0f, 200.0f, 90.0f);

		TextBlock* player_pos_text_block_ptr = new TextBlock(design_ptr, L"Player_pos: ", player_pos_text_area);
		TextBlock* player_normal_text_block_ptr = new TextBlock(design_ptr, L"Player_normal: ", player_normal_text_area);
		TextBlock* tile_info_text_block_ptr = new TextBlock(design_ptr, L"Tile: : ", tile_info_text_area);

		text_ui_manager->Add_TextBlock(player_pos_text_block_ptr);
		text_ui_manager->Add_TextBlock(player_normal_text_block_ptr);
		text_ui_manager->Add_TextBlock(tile_info_text_block_ptr);

	}
}

std::vector<TextBlock*>* CScene::Get_Text_List()
{
	if (text_ui_manager)
		return text_ui_manager->Get_Text_Block_List();
	else
		return NULL;
}

void CScene::Update_UI()
{
	static wchar_t Player_pos_Buffer[100];
	static wchar_t Player_normal_Buffer[100];
	static wchar_t Tile_Info_Buffer[100];

	if (text_ui_manager)
	{
		XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
		int tile_n = m_pTerrain->Get_Tile(xmf3Position.x, xmf3Position.z, m_pPlayer->Get_Last_Tile());
		XMFLOAT3 tile_normal = m_pTerrain->Get_Mesh_Normal(xmf3Position.x, xmf3Position.z);

		// 버퍼에 값 포맷팅
		_stprintf_s(Player_pos_Buffer, 100, _T("Player_pos >>%.2f,%.2f,%.2f"), xmf3Position.x, xmf3Position.y, xmf3Position.z);
		_stprintf_s(Player_normal_Buffer, 100, _T("Player_normal >> %.2f,%.2f,%.2f"), tile_normal.x, tile_normal.y, tile_normal.z);
		_stprintf_s(Tile_Info_Buffer, 100, _T("Tile  >> %d"), tile_n);

		text_ui_manager->UpdateTextBlock(0, Player_pos_Buffer, NULL, NULL);
		text_ui_manager->UpdateTextBlock(1, Player_normal_Buffer, NULL, NULL);
		text_ui_manager->UpdateTextBlock(2, Tile_Info_Buffer, NULL, NULL);
	}
}

#endif

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();

	obj_manager->Clear_Object_List_All();
#ifdef WRITE_TEXT_UI
	delete text_ui_manager;
#endif

	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			m_ppShaders[i]->ReleaseShaderVariables();
			m_ppShaders[i]->ReleaseObjects();
			m_ppShaders[i]->Release();
		}
		delete[] m_ppShaders;
	}

	if (m_pTerrain) delete m_pTerrain;
	if (m_pSkyBox) delete m_pSkyBox;


	ReleaseShaderVariables();

	if (m_pLights) delete[] m_pLights;
}

ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[10];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 6; //t6: gtxtAlbedoTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 7; //t7: gtxtSpecularTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 8; //t8: gtxtNormalTexture
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 9; //t9: gtxtMetallicTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 10; //t10: gtxtEmissionTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 11; //t11: gtxtEmissionTexture
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 12; //t12: gtxtEmissionTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 13; //t13: gtxtSkyBoxTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 1; //t1: gtxtTerrainBaseTexture
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[9].NumDescriptors = 1;
	pd3dDescriptorRanges[9].BaseShaderRegister = 2; //t2: gtxtTerrainDetailTexture
	pd3dDescriptorRanges[9].RegisterSpace = 0;
	pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[16];

	pd3dRootParameters[PARAMETER_CAMERA_CBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[PARAMETER_CAMERA_CBV].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[PARAMETER_CAMERA_CBV].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[PARAMETER_CAMERA_CBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 33;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[PARAMETER_SKYBOX_TEXTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[PARAMETER_SKYBOX_TEXTURE].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[PARAMETER_SKYBOX_TEXTURE].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
	pd3dRootParameters[PARAMETER_SKYBOX_TEXTURE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[PARAMETER_TERRAIN_BASE_TEXTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[PARAMETER_TERRAIN_BASE_TEXTURE].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[PARAMETER_TERRAIN_BASE_TEXTURE].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
	pd3dRootParameters[PARAMETER_TERRAIN_BASE_TEXTURE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[PARAMETER_TERRAIN_DETAIL_TEXTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[PARAMETER_TERRAIN_DETAIL_TEXTURE].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[PARAMETER_TERRAIN_DETAIL_TEXTURE].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
	pd3dRootParameters[PARAMETER_TERRAIN_DETAIL_TEXTURE].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[PARAMETER_BONE_OFFSET].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[PARAMETER_BONE_OFFSET].Descriptor.ShaderRegister = 7; //Skinned Bone Offsets
	pd3dRootParameters[PARAMETER_BONE_OFFSET].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[PARAMETER_BONE_OFFSET].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[PARAMETER_BONE_TRANSFORM].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[PARAMETER_BONE_TRANSFORM].Descriptor.ShaderRegister = 8; //Skinned Bone Transforms
	pd3dRootParameters[PARAMETER_BONE_TRANSFORM].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[PARAMETER_BONE_TRANSFORM].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[PARAMETER_OOBB_CUBE_CBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[PARAMETER_OOBB_CUBE_CBV].Descriptor.ShaderRegister = 6; //Bounding Box Renderer
	pd3dRootParameters[PARAMETER_OOBB_CUBE_CBV].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[PARAMETER_OOBB_CUBE_CBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
}

void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();

	for (int i = 0; i < m_nShaders; i++) 
	if(m_ppShaders[i] != NULL)
		m_ppShaders[i]->ReleaseUploadBuffers();
	
	std::vector<std::shared_ptr<CGameObject>>* skinned_obj_container = obj_manager->Get_Object_List(Object_Type::skinned);
	std::vector<std::shared_ptr<CGameObject>>* non_skinned_obj_container = obj_manager->Get_Object_List(Object_Type::non_skinned);

	for (std::shared_ptr<CGameObject> obj_ptr : *skinned_obj_container)
		obj_ptr->ReleaseUploadBuffers();
	
	for (std::shared_ptr<CGameObject> obj_ptr : *non_skinned_obj_container)
		obj_ptr->ReleaseUploadBuffers();

}

void CScene::CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferViews(ID3D12Device *pd3dDevice, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorNextHandle;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		m_d3dCbvCPUDescriptorNextHandle.ptr = m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_d3dCbvCPUDescriptorNextHandle);
		m_d3dCbvGPUDescriptorNextHandle.ptr = m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	}
	return(d3dCbvGPUDescriptorHandle);
}

void CScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetResource(i);
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			pTexture->SetGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int j = 0; j < nRootParameters; j++) pTexture->SetRootParameterIndex(j, nRootParameterStartIndex + j);
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case '1':
		{
			m_pTerrain->FindFrame("tile map - 0")->Set_Active(true);
			m_pTerrain->FindFrame("tile map - 5")->Set_Active(false);
			m_pTerrain->FindFrame("tile map - 10")->Set_Active(false);
			m_pTerrain->FindFrame("tile map - 15")->Set_Active(false);
		}		break;

		case '2':
		{
			m_pTerrain->FindFrame("tile map - 0")->Set_Active(false);
			m_pTerrain->FindFrame("tile map - 5")->Set_Active(true);
			m_pTerrain->FindFrame("tile map - 10")->Set_Active(false);
			m_pTerrain->FindFrame("tile map - 15")->Set_Active(false);
		}		break;

		case '3':
		{
			m_pTerrain->FindFrame("tile map - 0")->Set_Active(false);
			m_pTerrain->FindFrame("tile map - 5")->Set_Active(false);
			m_pTerrain->FindFrame("tile map - 10")->Set_Active(true);
			m_pTerrain->FindFrame("tile map - 15")->Set_Active(false);
		}		break;

		case '4':
		{
			m_pTerrain->FindFrame("tile map - 0")->Set_Active(false);
			m_pTerrain->FindFrame("tile map - 5")->Set_Active(false);
			m_pTerrain->FindFrame("tile map - 10")->Set_Active(false);
			m_pTerrain->FindFrame("tile map - 15")->Set_Active(true);
		}		break;

		case '5':
		{
			m_pTerrain->FindFrame("tile map - 0")->Set_Active(false);
			m_pTerrain->FindFrame("tile map - 5")->Set_Active(false);
			m_pTerrain->FindFrame("tile map - 10")->Set_Active(false);
			m_pTerrain->FindFrame("tile map - 15")->Set_Active(false);
		}		break;

		case '6':
		{
			m_pTerrain->FindFrame("tile map - 0")->Set_Active(true);
			m_pTerrain->FindFrame("tile map - 5")->Set_Active(true);
			m_pTerrain->FindFrame("tile map - 10")->Set_Active(true);
			m_pTerrain->FindFrame("tile map - 15")->Set_Active(true);
		}		break;

		case '7':
		{
			m_pTerrain->FindFrame("tile map - 9")->Set_Active(false);
			m_pTerrain->FindFrame("tile map - 14")->Set_Active(false);
			m_pTerrain->FindFrame("tile map - 19")->Set_Active(false);
		}		break;

		case '8':
		{
			m_pTerrain->FindFrame("tile map - 9")->Set_Active(true);
			m_pTerrain->FindFrame("tile map - 14")->Set_Active(true);
			m_pTerrain->FindFrame("tile map - 19")->Set_Active(true);
		}		break;

		case VK_SPACE:
		{
			XMFLOAT3 pos = m_pPlayer->GetPosition();
			m_pTerrain->Get_Tile(pos.x, pos.z);
		}	break;

		case 'Z':
		{
			m_pPlayer->Animate_test();
			m_pPlayer->FallingTimer_Reset();
		}		break;

		default:
			break;
		}
	default:
		break;
	}
	return(false);
}

bool CScene::ProcessInput(UCHAR *pKeysBuffer)
{
	bool bKeyProcessed = false;

	// W, A, S, D 키 입력 처리
	if (pKeysBuffer[0x57] & 0xF0) // W 키 확인
	{
		DebugOutput("W key is pressed\n");
		bKeyProcessed = true;
	}

	if (pKeysBuffer[0x41] & 0xF0) // A 키 확인
	{
		DebugOutput("A key is pressed\n");
		bKeyProcessed = true;
	}

	if (pKeysBuffer[0x53] & 0xF0) // S 키 확인
	{
		DebugOutput("S key is pressed\n");
		bKeyProcessed = true;
	}

	if (pKeysBuffer[0x44] & 0xF0) // D 키 확인
	{
		DebugOutput("D key is pressed\n");
		bKeyProcessed = true;
	}


	// 하나 이상의 키가 처리됬으면 true, 아니면 false 
	// - true = 프레임워크에서 추가적 동작 x
	// - false = 프레임워크에서 추가적 동작 o
	// return bKeyProcessed; 
	return false; // 프레임 워크에서 플레이어 키 입력 필요

}

void CScene::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;

	//obj_manager.Animate_Objects_All(fTimeElapsed);
	
	for (int i = 0; i < m_nShaders; i++) 
		if (m_ppShaders[i]) 
			m_ppShaders[i]->AnimateObjects(fTimeElapsed);

	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Position.y += 10.0f;
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();
	}

}

void CScene::Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{

	if (m_pd3dGraphicsRootSignature) 
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	if (m_pd3dCbvSrvDescriptorHeap) 
		pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights

//	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	obj_manager->Animate_Objects(Object_Type::skinned, m_fElapsedTime);
	obj_manager->Render_Objects_All(pd3dCommandList, pCamera);
	

	for (int i = 0; i < m_nShaders; i++) 
		if (m_ppShaders[i]) 
			m_ppShaders[i]->Render_Objects(pd3dCommandList, pCamera);


	// 테스트용 - 터레인 객체 컨테이너와, 스킨 메시 객체 컨테이너에 OBB_Drawer 동시 적용
	// 테스트를 위해 터레인 객체를 임시 shared_ptr로 해서, 
	// 함수가 끝나면 터레인 객체가 제거되고 있음 -> 오류 발생 
#ifdef RENDER_OBB
	static std::shared_ptr<CHeightMapTerrain> test_ptr(m_pTerrain);
	static vector<shared_ptr<CGameObject>> temp_list{ test_ptr };
	vector<shared_ptr<CGameObject>>* temp_list_2 = obj_manager->Get_Object_List(Object_Type::skinned);
	
	temp_list.insert(temp_list.end(), temp_list_2->begin(), temp_list_2->end());

	obj_manager->Update_OBB_Drawer(pd3dDevice, pd3dCommandList, temp_list);
	obj_manager->Render_OBB_Drawer(pd3dCommandList, pCamera);
#endif
}

