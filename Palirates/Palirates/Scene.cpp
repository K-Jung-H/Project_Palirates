//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"


//=============================================================================================

CScene::CScene()
{
	//if (m_pDescriptorHeap == NULL)
	//	m_pDescriptorHeap = new CDescriptorHeap();

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
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(250.0f, 50.0f, 250.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights[1].m_bEnable = true;
	m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights[1].m_fRange = 500.0f;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[1].m_fFalloff = 8.0f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));


	m_pLights[2].m_bEnable = false;
	m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);
//	m_pLights[2].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, -1.0f);


	m_pLights[3].m_bEnable = false;
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

	m_pLights[4].m_bEnable = false;
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

//	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 76); //SuperCobra(17), Gunship(2), Player:Mi24(1), Angrybot()

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature); 

	BuildDefaultLightsAndMaterials();

#ifdef RENDER_PARTICLE
	particle_manager = new Particle_Manager(pd3dDevice, pd3dCommandList);
	particle_manager->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
#endif


	obj_manager = new Object_Manager();
#ifdef RENDER_OBB
	obj_manager->Create_OBB_Drawer(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
#endif

//	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);


	XMFLOAT3 xmf3Scale(10.0f, 0.0f, 10.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.3f, 0.0f, 0.0f);
	m_pTerrain = make_shared<CHeightMapTerrain>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 0, 0, 257, 257, xmf3Scale, xmf4Color, 8, 3);
	m_pTerrain->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	obj_manager->Set_Terrain_Object(m_pTerrain);


	CLoadedModelInfo* pGargoyleModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Anubis_lp.bin", NULL);
	CLoadedModelInfo* pGargoyleModel2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Medusa_LP_Human.bin", NULL);
	CLoadedModelInfo* pGargoyleModel3 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Gargoyle_LP.bin", NULL);
	CLoadedModelInfo* pGargoyleModel4 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Seaman_v12.bin", NULL);
	CLoadedModelInfo* pGargoyleModel5 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Wench_v12.bin", NULL);
	CLoadedModelInfo* pGargoyleModel6 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/First_Mate_v12.bin", NULL);


	string obj_name_1 = "test_obj_name_1";
	string obj_name_2 = "test_obj_name_2";
	string obj_name_3 = "test_obj_name_3";
	string obj_name_4 = "test_palyer2";
	string obj_name_5 = "test_palyer3";
	string obj_name_6 = "test_palyer4";


	std::string_view name_view = obj_name_1;
	std::shared_ptr<CMonsterObject> humanObject_1 = std::make_shared<CMonsterObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGargoyleModel, 5);
	humanObject_1->SetPosition(0.0f, m_pTerrain->Get_Mesh_Height(0.0f, 0.0f), 0.0f);
	humanObject_1->SetScale(30.0f, 30.0f, 30.0f);
	humanObject_1->Set_Name(obj_name_1);
	humanObject_1->test_num = 1;
	obj_manager->Add_Object(humanObject_1, Object_Type::skinned);
	
	//====================================================
	// 테스트용 코드	
//	humanObject_1->m_pSkinnedAnimationController->Bone_Info();
	/*CGameObject* test_obj  = humanObject_1->FindFrame("MiddleFinger3_R");
	CGameObject* test_obj2 = humanObject_1->FindFrame("Shoulder_R");

	CGameObject* test_obj123 = humanObject_1->FindFrame("Head");
	CGameObject* test_obj2123 = humanObject_1->FindFrame("Feet");

	test_obj->Add_Collider(0.0f);
	test_obj2->Add_Collider(10.0f);*/

	//====================================================

	name_view = obj_name_2;
	std::shared_ptr<CMonsterObject> humanObject_2 = std::make_shared<CMonsterObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGargoyleModel2, 5);
	humanObject_2->SetPosition(10.0f, m_pTerrain->Get_Mesh_Height(10.0f, 10.0f), 10.0f);
	humanObject_2->SetScale(10.0f, 10.0f, 10.0f);
	humanObject_2->Set_Name(obj_name_2);
	humanObject_2->test_num = 2;
	obj_manager->Add_Object(humanObject_2, Object_Type::skinned);

	name_view = obj_name_3;
	std::shared_ptr<CMonsterObject> humanObject_3 = std::make_shared<CMonsterObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGargoyleModel3, 5);
	humanObject_3->SetPosition(10.0f, m_pTerrain->Get_Mesh_Height(10.0f, 0.0f), 0.0f);
	humanObject_3->SetScale(15.0f, 15.0f, 15.0f);
	humanObject_3->SetRotationAxis(XMFLOAT3(1.0f, 0.0f, 0.0f));
	XMFLOAT3 tt = { 0.0f, 1.0f, 0.0f };
	humanObject_3->Rotate(&tt, 90.0f);
	humanObject_3->Set_Name(obj_name_3);
	humanObject_3->test_num = 3;
	obj_manager->Add_Object(humanObject_3, Object_Type::skinned);

	name_view = obj_name_4;
	std::shared_ptr<CMultiPlayerObject> humanObject_4 = std::make_shared<CMultiPlayerObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGargoyleModel4, 12);
	humanObject_4->SetPosition(5.0f, m_pTerrain->Get_Mesh_Height(5.0f, 10.0f), 10.0f);
	humanObject_4->Set_Name(obj_name_4);
	humanObject_4->test_num = 4;
	obj_manager->Add_Object(humanObject_4, Object_Type::skinned);

	name_view = obj_name_5;
	std::shared_ptr<CMultiPlayerObject> humanObject_5 = std::make_shared<CMultiPlayerObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGargoyleModel5, 12);
	humanObject_5->SetPosition(15.0f, m_pTerrain->Get_Mesh_Height(15.0f, 20.0f), 20.0f);
	humanObject_5->Set_Name(obj_name_5);
	humanObject_5->test_num = 5;
	obj_manager->Add_Object(humanObject_5, Object_Type::skinned);

	name_view = obj_name_4;
	std::shared_ptr<CMultiPlayerObject> humanObject_6 = std::make_shared<CMultiPlayerObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGargoyleModel6, 12);
	humanObject_6->SetPosition(20.0f, m_pTerrain->Get_Mesh_Height(20.0f, 10.0f), 10.0f);
	humanObject_6->Set_Name(obj_name_6);
	humanObject_6->test_num = 6;
	obj_manager->Add_Object(humanObject_6, Object_Type::skinned);

	//=====================================================
#ifdef LOAD_SCENE
	// Load Scene

	CLoadedModelInfo* Test_Scene_Model = CGameObject::Load_Scene_File(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Scene/Scene_File/TST.bin", NULL);
	std::shared_ptr<CGameObject> test_scene = std::make_shared<CGameObject>();
	test_scene->Set_Name("test_scene");
	test_scene = Test_Scene_Model->m_pModelRootObject;
	test_scene->SetPosition(1300.0f, m_pTerrain->Get_Mesh_Height(1300.0f, 800.0f), 800.0f);
	test_scene->SetScale({ 5.0f,5.0f ,5.0f }, true);
	obj_manager->Add_Object(test_scene, Object_Type::fixed);
#endif
	//=====================================================

	unordered_map<std::string, Fixed_Object_Info>* temp_list_map = obj_manager->Get_Object_List_Map(Object_Type::fixed);

	// 씬에 있는 모든 fixed 객체들을 지형에 따라 재배치하기

	for (auto& [mesh_name, instance_info] : *temp_list_map)
	{
		m_pTerrain->Reset_Obj_List_Height(instance_info.fixed_obj_list);
		m_pTerrain->Reset_Obj_List_Up_Vector(instance_info.fixed_obj_list);
	}

	// 씬에 있는 모든 fixed 객체들을 타일에 맞게 분류하기
	obj_manager->Classify_Objects_By_Tile();

	Object_Manager::Reserve_Update();

	/*if (pGargoyleModel)
		delete pGargoyleModel;
	if (pGargoyleModel2)
		delete pGargoyleModel2;
	if (pGargoyleModel3)
		delete pGargoyleModel3;*/

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

#ifdef WRITE_TEXT_UI
void CScene::Build_Text_UI(Text_UI_Renderer* text_ui_renderer_ptr)
{
	text_ui_manager = new Text_UI_Manager(text_ui_renderer_ptr->m_pd2dWriteFactory, text_ui_renderer_ptr->m_pd2dDeviceContext);

	if (text_ui_manager)
	{
		std::shared_ptr<TextDesign> design_ptr = text_ui_manager->Create_Text_Design("White_Text", D2D1::ColorF(D2D1::ColorF::Black, 1.0f), L"Gothic", 20.0f);
		text_ui_manager->Add_Text_Design(design_ptr);

		D2D1_RECT_F player_pos_text_area = D2D1::RectF(0.0f, 0.0f, 400.0f, 30.0f);
		D2D1_RECT_F player_normal_text_area = D2D1::RectF(0.0f, 30.0f, 400.0f, 60.0f);
		D2D1_RECT_F tile_info_text_area = D2D1::RectF(0.0f, 60.0f, 200.0f, 90.0f);
		D2D1_RECT_F player_xz = D2D1::RectF(0.0f, 90.0f, 400.0f, 120.0f);
		D2D1_RECT_F player_state = D2D1::RectF(0.0f, 120.0f, 200.0f, 150.0f);
		D2D1_RECT_F player_Laststate = D2D1::RectF(0.0f, 150.0f, 400.0f, 180.0f);

		TextBlock* player_pos_text_block_ptr = new TextBlock(design_ptr, L"Player_pos: ", player_pos_text_area);
		TextBlock* player_normal_text_block_ptr = new TextBlock(design_ptr, L"Player_normal: ", player_normal_text_area);
		TextBlock* tile_info_text_block_ptr = new TextBlock(design_ptr, L"Tile: : ", tile_info_text_area);
		TextBlock* player_xz_ptr = new TextBlock(design_ptr, L"Player_XZ: : ", player_xz);
		TextBlock* player_state_ptr = new TextBlock(design_ptr, L"Player_state: : ", player_state);
		TextBlock* player_Laststate_ptr = new TextBlock(design_ptr, L"Player_LastState: : ", player_Laststate);

		text_ui_manager->Add_TextBlock(player_pos_text_block_ptr);
		text_ui_manager->Add_TextBlock(player_normal_text_block_ptr);
		text_ui_manager->Add_TextBlock(tile_info_text_block_ptr);
		text_ui_manager->Add_TextBlock(player_xz_ptr);
		text_ui_manager->Add_TextBlock(player_state_ptr);
		text_ui_manager->Add_TextBlock(player_Laststate_ptr);

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
	static wchar_t Player_XZ_Buffer[100];
	static wchar_t Player_state_Buffer[100];
	static wchar_t Player_Laststate_Buffer[100];

	if (text_ui_manager)
	{
		XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
		int tile_n = m_pTerrain->Get_Tile(xmf3Position.x, xmf3Position.z, m_pPlayer->Get_Last_Tile());
		XMFLOAT3 tile_normal = m_pTerrain->Get_Mesh_Normal(xmf3Position.x, xmf3Position.z);
		float player_x = m_pPlayer->GetMoveX();
		float player_z = m_pPlayer->GetMoveZ();
		State currentState = m_pPlayer->GetStateMachine()->Get_State();
		std::wstring stateStr = stateToStringMap[currentState];  
		State LastState = m_pPlayer->GetStateMachine()->Get_LastState();
		std::wstring LastStateStr = stateToStringMap[LastState];


		_stprintf_s(Player_pos_Buffer, 100, _T("Player_pos >>%.2f,%.2f,%.2f"), xmf3Position.x, xmf3Position.y, xmf3Position.z);
		_stprintf_s(Player_normal_Buffer, 100, _T("Player_normal >> %.2f,%.2f,%.2f"), tile_normal.x, tile_normal.y, tile_normal.z);
		_stprintf_s(Tile_Info_Buffer, 100, _T("Tile  >> %d"), tile_n);
		_stprintf_s(Player_XZ_Buffer, 100, _T("Player_XZ  >> %.2f,%.2f"), player_x, player_z);
		_stprintf_s(Player_state_Buffer, 100, _T("Player_state  >> %s"), stateStr.c_str());
		_stprintf_s(Player_Laststate_Buffer, 100, _T("Player_LastState  >> %s"), LastStateStr.c_str());

		text_ui_manager->UpdateTextBlock(0, Player_pos_Buffer, NULL, NULL);
		text_ui_manager->UpdateTextBlock(1, Player_normal_Buffer, NULL, NULL);
		text_ui_manager->UpdateTextBlock(2, Tile_Info_Buffer, NULL, NULL);
		text_ui_manager->UpdateTextBlock(3, Player_XZ_Buffer, NULL, NULL);
		text_ui_manager->UpdateTextBlock(4, Player_state_Buffer, NULL, NULL);
		text_ui_manager->UpdateTextBlock(5, Player_Laststate_Buffer, NULL, NULL);
	}
}

#endif

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
//	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();


	obj_manager->Clear_Object_List_All();
#ifdef WRITE_TEXT_UI
	delete text_ui_manager;
#endif

	if (Shader_list.size())
		for (std::shared_ptr<CShader> shader_ptr : Shader_list)
			shader_ptr.reset();
		
		if (m_pSkyBox) delete m_pSkyBox;


	ReleaseShaderVariables();

	if (m_pLights) delete[] m_pLights;
}

ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[9];
	{
		pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[0].NumDescriptors = 1;
		pd3dDescriptorRanges[0].BaseShaderRegister = 0; //t0: gtxtAlbedoTexture
		pd3dDescriptorRanges[0].RegisterSpace = 0;
		pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[1].NumDescriptors = 1;
		pd3dDescriptorRanges[1].BaseShaderRegister = 1; //t1: gtxtSpecularTexture
		pd3dDescriptorRanges[1].RegisterSpace = 0;
		pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[2].NumDescriptors = 1;
		pd3dDescriptorRanges[2].BaseShaderRegister = 2; //t2: gtxtNormalTexture
		pd3dDescriptorRanges[2].RegisterSpace = 0;
		pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[3].NumDescriptors = 1;
		pd3dDescriptorRanges[3].BaseShaderRegister = 3; //t3: gtxtMetallicTexture
		pd3dDescriptorRanges[3].RegisterSpace = 0;
		pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[4].NumDescriptors = 1;
		pd3dDescriptorRanges[4].BaseShaderRegister = 4; //t4: gtxtEmissionTexture
		pd3dDescriptorRanges[4].RegisterSpace = 0;
		pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[5].NumDescriptors = 1;
		pd3dDescriptorRanges[5].BaseShaderRegister = 5; //t5: gtxtTerrainBaseTexture
		pd3dDescriptorRanges[5].RegisterSpace = 0;
		pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[6].NumDescriptors = 1;
		pd3dDescriptorRanges[6].BaseShaderRegister = 6; //t6: gtxtTerrainDetailTexture
		pd3dDescriptorRanges[6].RegisterSpace = 0;
		pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[7].NumDescriptors = 1;
		pd3dDescriptorRanges[7].BaseShaderRegister = 7; //t7: gtxtSkyBoxTexture
		pd3dDescriptorRanges[7].RegisterSpace = 0;
		pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		pd3dDescriptorRanges[8].NumDescriptors = 1;
		pd3dDescriptorRanges[8].BaseShaderRegister = 8; //t8: random value for particle move
		pd3dDescriptorRanges[8].RegisterSpace = 0;
		pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		//=======================================================================
	}

	D3D12_ROOT_PARAMETER pd3dRootParameters[14];
	{
		// n = 0, b0 = Frame_Info
		pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].Descriptor.ShaderRegister = 0; //Frame_Info
		pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_FRAME_CBV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// n = 1, b1 = GameObject
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.Num32BitValues = 33;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.ShaderRegister = 1; 
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].Constants.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_GAMEOBJECT_TRANSFORM_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// n = 2, b4 = Skinned Bone Offsets
		pd3dRootParameters[ROOT_PARAMETER_BONE_OFFSET_CBV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[ROOT_PARAMETER_BONE_OFFSET_CBV_INDEX].Descriptor.ShaderRegister = 3;
		pd3dRootParameters[ROOT_PARAMETER_BONE_OFFSET_CBV_INDEX].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_BONE_OFFSET_CBV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		// n = 3, b5 = Skinned Bone Transforms
		pd3dRootParameters[ROOT_PARAMETER_BONE_TRANSFORM_CBV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[ROOT_PARAMETER_BONE_TRANSFORM_CBV_INDEX].Descriptor.ShaderRegister = 4; 
		pd3dRootParameters[ROOT_PARAMETER_BONE_TRANSFORM_CBV_INDEX].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_BONE_TRANSFORM_CBV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		// n = 4, b2 = Camera
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].Descriptor.ShaderRegister = 2; 
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].Descriptor.RegisterSpace = 0;
		pd3dRootParameters[ROOT_PARAMETER_CAMERA_CBV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


		// n = 5, t0 = Albeo_Texture
		pd3dRootParameters[ROOT_PARAMETER_ALBEDO_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_ALBEDO_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_ALBEDO_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
		pd3dRootParameters[ROOT_PARAMETER_ALBEDO_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 6, t1 = Specular_Texture
		pd3dRootParameters[ROOT_PARAMETER_SPECULAR_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_SPECULAR_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_SPECULAR_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
		pd3dRootParameters[ROOT_PARAMETER_SPECULAR_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 7, t2 = Normal_Texture
		pd3dRootParameters[ROOT_PARAMETER_NORMAL_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_NORMAL_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_NORMAL_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
		pd3dRootParameters[ROOT_PARAMETER_NORMAL_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 8, t3 = Metallic_Texture
		pd3dRootParameters[ROOT_PARAMETER_METALLIC_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_METALLIC_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_METALLIC_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
		pd3dRootParameters[ROOT_PARAMETER_METALLIC_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 9, t4 = Emission_Texture
		pd3dRootParameters[ROOT_PARAMETER_EMISSION_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_EMISSION_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_EMISSION_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
		pd3dRootParameters[ROOT_PARAMETER_EMISSION_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 10, t5 = Terrain_Base_Texture
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_BASE_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_BASE_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_BASE_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_BASE_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 11, t6 = Terrain_Base_Texture
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_DETAIL_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_DETAIL_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_DETAIL_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
		pd3dRootParameters[ROOT_PARAMETER_TERRAIN_DETAIL_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 12,  t7 = Sky_Box
		pd3dRootParameters[ROOT_PARAMETER_SKYBOX_TEXTURE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_SKYBOX_TEXTURE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_SKYBOX_TEXTURE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
		pd3dRootParameters[ROOT_PARAMETER_SKYBOX_TEXTURE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// n = 13,  t8 = RandomValue
		pd3dRootParameters[ROOT_PARAMETER_RANDOM_VALUE_SRV_INDEX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		pd3dRootParameters[ROOT_PARAMETER_RANDOM_VALUE_SRV_INDEX].DescriptorTable.NumDescriptorRanges = 1;
		pd3dRootParameters[ROOT_PARAMETER_RANDOM_VALUE_SRV_INDEX].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
		pd3dRootParameters[ROOT_PARAMETER_RANDOM_VALUE_SRV_INDEX].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

	}

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

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
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
	HRESULT hr = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);

	if (FAILED(hr))
	{

		if (pd3dErrorBlob)
		{
			OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
		}
		else
		{

			OutputDebugStringA("Failed to create root signature.\n");
		}
	}

	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256 * N
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CScene::UpdateShaderVariables_Light_Info(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_POST_LIGHT_INFO_CBV_INDEX, d3dcbLightsGpuVirtualAddress); //Lights
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

	if (Shader_list.size())
		for (std::shared_ptr<CShader> shader_ptr : Shader_list)
			shader_ptr->ReleaseUploadBuffers();

	
	std::vector<std::shared_ptr<CGameObject>>* skinned_obj_container = obj_manager->Get_Object_List(Object_Type::skinned);
	std::vector<std::shared_ptr<CGameObject>>* non_skinned_obj_container = obj_manager->Get_Object_List(Object_Type::non_skinned);

	for (std::shared_ptr<CGameObject> obj_ptr : *skinned_obj_container)
		obj_ptr->ReleaseUploadBuffers();
	
	for (std::shared_ptr<CGameObject> obj_ptr : *non_skinned_obj_container)
		obj_ptr->ReleaseUploadBuffers();

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
		case 'Q':
		{
			if (test_button)
				break;

			test_button = true;
		}	break;

		case 'Z':
		{
			m_pPlayer->GetStateMachine()->changeState(State::Knock_Down, Key_Value::None);
			m_pPlayer->SetStateElapsedTime(0.0f);
			m_pPlayer->GetSkinnedAnimationController()->m_pAnimationTracks[TRACK_KNOCK_DOWN].m_fPosition = -ANIMATION_CALLBACK_EPSILON;
		}		break;
		case 'X':
		{
			m_pPlayer->GetStateMachine()->changeState(State::Get_Up, Key_Value::None);
			m_pPlayer->SetStateElapsedTime(0.0f);
			m_pPlayer->GetSkinnedAnimationController()->m_pAnimationTracks[TRACK_GET_UP].m_fPosition = -ANIMATION_CALLBACK_EPSILON;
		}		break;
		case 'C':
		{
			auto it = obj_manager->Get_Object_List(Object_Type::skinned);
			if (it && it->size() > 5) {
				auto multiPlayerObj = std::dynamic_pointer_cast<CMultiPlayerObject>((*it)[5]);
				if (multiPlayerObj) {  
					multiPlayerObj->GetStateMachine()->changeState(State::Knock_Down, Key_Value::None);
					multiPlayerObj->SetStateElapsedTime(0.0f);
					multiPlayerObj->GetSkinnedAnimationController()->m_pAnimationTracks[TRACK_KNOCK_DOWN].m_fPosition = -ANIMATION_CALLBACK_EPSILON;
				}
			}
		}		break;
		case 'V':
		{
			auto it = obj_manager->Get_Object_List(Object_Type::skinned);
			if (it && it->size() > 5) {
				auto multiPlayerObj = std::dynamic_pointer_cast<CMultiPlayerObject>((*it)[5]);
				if (multiPlayerObj) {
					multiPlayerObj->GetStateMachine()->changeState(State::Get_Up, Key_Value::None);
					multiPlayerObj->SetStateElapsedTime(0.0f);
					multiPlayerObj->GetSkinnedAnimationController()->m_pAnimationTracks[TRACK_GET_UP].m_fPosition = -ANIMATION_CALLBACK_EPSILON;
				}
			}
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

	// W, A, S, D 
	if (pKeysBuffer[0x57] & 0xF0) // W 
	{
		//DebugOutput("W key is pressed\n");
		bKeyProcessed = true;
	}

	if (pKeysBuffer[0x41] & 0xF0) // A 
	{
		//DebugOutput("A key is pressed\n");
		bKeyProcessed = true;
	}

	if (pKeysBuffer[0x53] & 0xF0) // S 
	{
		//DebugOutput("S key is pressed\n");
		bKeyProcessed = true;
	}

	if (pKeysBuffer[0x44] & 0xF0) // D 
	{
		//DebugOutput("D key is pressed\n");
		bKeyProcessed = true;
	}



	return false; 

}

void CScene::Animate_Objects(ID3D12GraphicsCommandList *pd3dCommandList, float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;

	if (Shader_list.size())
		for (std::shared_ptr<CShader> shader_ptr : Shader_list)
			shader_ptr->AnimateObjects(fTimeElapsed);


	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Position.y += 10.0f;
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();
	}
}

void CScene::Update_Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
#ifdef RENDER_OBB

	//vector<shared_ptr<CGameObject>>* temp_list = obj_manager->Get_Object_List(Object_Type::skinned);
	//obj_manager->Update_OBB_Drawer(pd3dDevice, pd3dCommandList, *temp_list);

	unordered_map<std::string, Fixed_Object_Info>* temp_list_map = obj_manager->Get_Object_List_Map(Object_Type::fixed);
	obj_manager->Update_OBB_Drawer(pd3dDevice, pd3dCommandList, *temp_list_map);

#endif
	obj_manager->Update(pd3dDevice, pd3dCommandList);
}

void CScene::Animate_Particles(ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed)
{
#ifdef RENDER_PARTICLE
	if (particle_manager)
		particle_manager->AnimateObjects(pd3dCommandList, fTimeElapsed);

#endif
}

void CScene::Prepare_Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pd3dGraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);


//	CDescriptor_Heap::SetDescriptorHeaps(pd3dCommandList, 1);
//	pCamera->SetViewportsAndScissorRects(pd3dCommandList);


	pCamera->Update_PreRender_ShaderVariables(pd3dCommandList);

	//씬의 객체들 프러스텀 컬링
	obj_manager->Check_Culling_All(pCamera);

	// Light Update
	UpdateShaderVariables(pd3dCommandList);

	obj_manager->Animate_Objects(Object_Type::skinned, m_fElapsedTime);

}

void CScene::Render(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
//	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);


	obj_manager->Render_Objects_All(pd3dCommandList, pCamera);
	

	if (Shader_list.size())
		for (std::shared_ptr<CShader> shader_ptr : Shader_list)
			shader_ptr->Render_Objects(pd3dCommandList, pCamera);


#ifdef RENDER_PARTICLE
	if (particle_manager)
	{
		particle_manager->Render_All(pd3dCommandList, pCamera, 0);
		particle_manager->Render_All(pd3dCommandList, pCamera, 1);
	}

#endif


#ifdef RENDER_OBB
	obj_manager->Render_OBB_Drawer(pd3dCommandList, pCamera);
#endif



}

void CScene::Finalize_Frame(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
#ifdef RENDER_PARTICLE
	if (particle_manager)
		particle_manager->OnPostRender_All();

#endif
}


void Test_Scene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

//	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 76); //SuperCobra(17), Gunship(2), Player:Mi24(1), Angrybot()

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	BuildDefaultLightsAndMaterials();

	obj_manager = new Object_Manager();

#ifdef RENDER_OBB
	obj_manager->Create_OBB_Drawer(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
#endif

//	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);


	XMFLOAT3 xmf3Scale(20.0f, 10.0f, 20.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.3f, 0.0f, 0.0f);

	m_pTerrain = make_shared<CHeightMapTerrain>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 0, 0, 257, 257, xmf3Scale, xmf4Color, 8, 2);
//	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 0, 0, 257, 257, xmf3Scale, xmf4Color, 8, 2);
	m_pTerrain->SetPosition(XMFLOAT3(0.0f, -100.0f * xmf3Scale.y, 0.0f));


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
	obj_manager->Add_Object(humanObject_1, Object_Type::skinned);

	//====================================================
	// 테스트용 코드	
//	humanObject_1->m_pSkinnedAnimationController->Bone_Info();
	CGameObject* test_obj = humanObject_1->FindFrame("MiddleFinger3_R");
	CGameObject* test_obj2 = humanObject_1->FindFrame("Shoulder_R");

	CGameObject* test_obj123 = humanObject_1->FindFrame("Head");
	CGameObject* test_obj2123 = humanObject_1->FindFrame("Feet");

	test_obj->Add_Collider(0.0f);
	test_obj2->Add_Collider(10.0f);

	//====================================================

	name_view = obj_name_2;
	std::shared_ptr<CHumanObject> humanObject_2 = std::make_shared<CHumanObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pHumanModel, 1);
	humanObject_2->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	humanObject_2->SetPosition(430.0f, m_pTerrain->Get_Mesh_Height(400.0f, 700.0f), 700.0f);
	humanObject_2->SetScale(10.0f, 10.0f, 10.0f);
	humanObject_2->Set_Name(name_view);
	obj_manager->Add_Object(humanObject_2, Object_Type::skinned);


	name_view = obj_name_3;
	std::shared_ptr<CHumanObject> humanObject_3 = std::make_shared<CHumanObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pHumanModel, 1);
	humanObject_3->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
	humanObject_3->SetPosition(400.0f, m_pTerrain->Get_Mesh_Height(400.0f, 720.0f), 720.0f);
	humanObject_3->SetScale(10.0f, 10.0f, 10.0f);
	humanObject_3->Set_Name(name_view);
	obj_manager->Add_Object(humanObject_3, Object_Type::skinned);

	//=====================================================
#ifdef LOAD_SCENE
	// Load Scene

	CLoadedModelInfo* Test_Scene_Model = CGameObject::Load_Scene_File(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Scene/Scene_File/TST.bin", NULL);
	std::shared_ptr<CGameObject> test_scene = std::make_shared<CGameObject>();
	test_scene->Set_Name("test_scene");
	test_scene = Test_Scene_Model->m_pModelRootObject;
	test_scene->SetPosition(1300.0f, m_pTerrain->Get_Mesh_Height(1300.0f, 800.0f), 800.0f);
	test_scene->SetScale({ 5.0f,5.0f ,5.0f }, true);
	obj_manager->Add_Object(test_scene, Object_Type::fixed);
#endif
	//=====================================================

	Object_Manager::Reserve_Update();



	if (pHumanModel)
		delete pHumanModel;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void Weapon_Select_Scene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	//	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 76); 

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	BuildDefaultLightsAndMaterials();

	obj_manager = new Object_Manager();

#ifdef RENDER_OBB
	obj_manager->Create_OBB_Drawer(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
#endif

	//	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);


	XMFLOAT3 xmf3Scale(20.0f, 10.0f, 20.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.3f, 0.0f, 0.0f);

	m_pTerrain = make_shared<CHeightMapTerrain>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 0, 0, 257, 257, xmf3Scale, xmf4Color, 8, 2);
	//	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/HeightMap.raw"), 0, 0, 257, 257, xmf3Scale, xmf4Color, 8, 2);
	m_pTerrain->SetPosition(XMFLOAT3(0.0f, -100.0f * xmf3Scale.y, 0.0f));


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
	obj_manager->Add_Object(humanObject_1, Object_Type::skinned);

	//====================================================
	// 테스트용 코드	
//	humanObject_1->m_pSkinnedAnimationController->Bone_Info();
	CGameObject* test_obj = humanObject_1->FindFrame("MiddleFinger3_R");
	CGameObject* test_obj2 = humanObject_1->FindFrame("Shoulder_R");

	CGameObject* test_obj123 = humanObject_1->FindFrame("Head");
	CGameObject* test_obj2123 = humanObject_1->FindFrame("Feet");

	test_obj->Add_Collider(0.0f);
	test_obj2->Add_Collider(10.0f);

	//====================================================

	name_view = obj_name_2;
	std::shared_ptr<CHumanObject> humanObject_2 = std::make_shared<CHumanObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pHumanModel, 1);
	humanObject_2->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	humanObject_2->SetPosition(430.0f, m_pTerrain->Get_Mesh_Height(400.0f, 700.0f), 700.0f);
	humanObject_2->SetScale(10.0f, 10.0f, 10.0f);
	humanObject_2->Set_Name(name_view);
	obj_manager->Add_Object(humanObject_2, Object_Type::skinned);


	name_view = obj_name_3;
	std::shared_ptr<CHumanObject> humanObject_3 = std::make_shared<CHumanObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pHumanModel, 1);
	humanObject_3->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
	humanObject_3->SetPosition(400.0f, m_pTerrain->Get_Mesh_Height(400.0f, 720.0f), 720.0f);
	humanObject_3->SetScale(10.0f, 10.0f, 10.0f);
	humanObject_3->Set_Name(name_view);
	obj_manager->Add_Object(humanObject_3, Object_Type::skinned);

	//=====================================================
#ifdef LOAD_SCENE
	// Load Scene

	CLoadedModelInfo* Test_Scene_Model = CGameObject::Load_Scene_File(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Scene/Scene_File/TST.bin", NULL);
	std::shared_ptr<CGameObject> test_scene = std::make_shared<CGameObject>();
	test_scene->Set_Name("test_scene");
	test_scene = Test_Scene_Model->m_pModelRootObject;
	test_scene->SetPosition(1300.0f, m_pTerrain->Get_Mesh_Height(1300.0f, 800.0f), 800.0f);
	test_scene->SetScale({ 5.0f,5.0f ,5.0f }, true);
	obj_manager->Add_Object(test_scene, Object_Type::fixed);
#endif
	//=====================================================

	Object_Manager::Reserve_Update();



	if (pHumanModel)
		delete pHumanModel;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

