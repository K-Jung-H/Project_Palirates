#include "stdafx.h"
#include "Effect.h"
#include "Shader.h"
#include "Scene.h"

//=====================================================================================

Sprite_Object::Sprite_Object()
{
	sprite_info = {};

	if (!sprite_material)
	{
		sprite_material = make_shared<CMaterial>(1);

		if (Sprite_Effect_Manager::sprite_shader)
			sprite_material->SetShader(Sprite_Effect_Manager::sprite_shader);
	}
}

void Sprite_Object::Set_BaseTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* filename)
{
	if (filename == NULL)
		return;

	shared_ptr<CTexture> sprite_texture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);
	sprite_texture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, filename, RESOURCE_TEXTURE2D, 0);

	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, sprite_texture.get(), 0, ROOT_PARAMETER_TRANSPARENT_ALBEDO_TEXTURE_INDEX);

	sprite_material->SetTexture(sprite_texture, 0);
}

void Sprite_Object::Set_BaseTexture(shared_ptr<CTexture> new_texture)
{	
	sprite_material->SetTexture(new_texture, 0);
}


void Sprite_Object::Animate(float fTimeElapsed)
{
	TimeElapsed += fTimeElapsed;

	if (life_type == Sprite_Effect_Lifetime::OneShot)
	{
		int currentFrame = static_cast<int>(TimeElapsed / sprite_info.frameTime);
		int totalFrames = static_cast<int>(sprite_info.totalFrames);

		if (currentFrame >= totalFrames)
			Set_Active(false);
	}

}


void Sprite_Object::Update_Sprite_Info(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_TRANSPARENT_SPRITE_INFO_INDEX, 4, &sprite_info, 0);

}

void Sprite_Object::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pMesh)
	{
		auto billboard_mesh = dynamic_pointer_cast<Billboard_Mesh>(m_pMesh);

		if (sprite_material && sprite_material->m_pShader)
		{
			if(billboard_mesh)
				sprite_material->m_pShader->Setting_Render(pd3dCommandList, 1);
			else
				sprite_material->m_pShader->Setting_Render(pd3dCommandList, 0);

			sprite_material->UpdateShaderVariable(pd3dCommandList);
		}

		Update_Sprite_Info(pd3dCommandList);
		UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);


		m_pMesh->Render(pd3dCommandList, 0);
	}
}

void Sprite_Object::Instance_Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VERTEX_BUFFER_VIEW instance_buffer, int instance_num)
{
	if (m_pMesh)
	{
		auto billboard_mesh = dynamic_pointer_cast<Billboard_Mesh>(m_pMesh);

		if (sprite_material && sprite_material->m_pShader)
		{
			if (billboard_mesh)
				sprite_material->m_pShader->Setting_Render(pd3dCommandList, 1);
			else
				sprite_material->m_pShader->Setting_Render(pd3dCommandList, 0);

			sprite_material->UpdateShaderVariable(pd3dCommandList);
		}


		Update_Sprite_Info(pd3dCommandList);


		m_pMesh->Instancing_Render(pd3dCommandList, instance_buffer, instance_num);
	}
}


void Sprite_Object::Reset()
{
	TimeElapsed = 0.0f;
	Set_Active(true);
}


//=====================================================================================

Aura_Object::Aura_Object(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float bottom_radius, float top_radius, float height)
	: Sprite_Object()
{
	m_pMesh = make_shared<Frustum_Ring_Shape_Mesh>(pd3dDevice, pd3dCommandList, bottom_radius, top_radius, height, 64);


}

void Aura_Object::Animate(float fTimeElapsed)
{
	Sprite_Object::Animate(fTimeElapsed);
<<<<<<< HEAD
=======

	if (m_pTargetObject)
	{
		XMFLOAT3 target_pos = m_pTargetObject->GetPosition();
		SetPosition(target_pos);
	}
>>>>>>> server_0628
}

void Aura_Object::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (sprite_material && sprite_material->m_pShader)
	{
		sprite_material->m_pShader->Setting_Render(pd3dCommandList, 0);
		sprite_material->UpdateShaderVariable(pd3dCommandList);
	}

	Sprite_Object::Render(pd3dCommandList, pCamera);
}

//=============================================================

void Sprite_Effect_Instance_Info::Create_Instance_Data_ShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT bufferSize = sizeof(Sprite_Effect_Instance_Data) * MAX_EFFECT_NUM;
	bufferSize = (bufferSize + 255) & ~255;

	Instance_data = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, bufferSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	Instance_data->Map(0, NULL, (void**)&Mapped_Instance_data);

	m_d3dInstancingBufferView.BufferLocation = Instance_data->GetGPUVirtualAddress();
	m_d3dInstancingBufferView.StrideInBytes = sizeof(Sprite_Effect_Instance_Data);
	m_d3dInstancingBufferView.SizeInBytes = bufferSize;
}

void Sprite_Effect_Instance_Info::Update_Instance_Data(std::vector<std::shared_ptr<Sprite_Object>> obj_list)
{
	int new_instancing_num = 0;
	for (std::shared_ptr<Sprite_Object> sprite_obj : obj_list)
	{
		if (sprite_obj->Get_Active())
		{
			Mapped_Instance_data[new_instancing_num].position = sprite_obj->GetPosition();
			Mapped_Instance_data[new_instancing_num].elapsed_time = sprite_obj->Get_Elapsed_Time();
			Mapped_Instance_data[new_instancing_num].scale_value = sprite_obj->Get_Scale_Value();
			Mapped_Instance_data[new_instancing_num].blending_color = { 0,0,0 };

			++new_instancing_num;
		};


	}
	

	Instancing_num = new_instancing_num;
}

void Sprite_Effect_Instance_Info::Release_Instance_Data_ShaderVariables()
{
	if (Instance_data) Instance_data->Unmap(0, NULL);
	if (Instance_data) Instance_data->Release();
}

//=============================================================
Sprite_Shader* Sprite_Effect_Manager::sprite_shader = NULL;


void Sprite_Effect_Manager::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, shared_ptr<ID3D12RootSignature> pd3dGraphicsRootSignature)
{
	if (sprite_shader == NULL)
	{
		sprite_shader = new Sprite_Shader();
		sprite_shader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		sprite_shader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}
}


Sprite_Effect_Manager::Sprite_Effect_Manager(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	Init_Format(pd3dDevice, pd3dCommandList);

	Create_Instance_Data_ShaderVariables(pd3dDevice, pd3dCommandList);
}

void Sprite_Effect_Manager::Init_Format(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	Init_Sprite_Format(pd3dDevice, pd3dCommandList);
	Init_Mesh_Format(pd3dDevice, pd3dCommandList);


	//---------------------------------------------------------------------------

	Sprite_Effect_Format hit_1_effect_format;

	hit_1_effect_format.mesh = mesh_map["Billboard"];
	hit_1_effect_format.spriteInfo = sprite_texture_map["hit_1_effect"].first;
	hit_1_effect_format.texture = sprite_texture_map["hit_1_effect"].second;
	hit_1_effect_format.lifetime = Sprite_Effect_Lifetime::OneShot;

	sprite_format_map[Sprite_Effect_Type::Hit_1] = hit_1_effect_format;



	Sprite_Effect_Format hit_2_effect_format;

	hit_2_effect_format.mesh = mesh_map["Billboard"];
	hit_2_effect_format.spriteInfo = sprite_texture_map["hit_2_effect"].first;
	hit_2_effect_format.texture = sprite_texture_map["hit_2_effect"].second;
	hit_2_effect_format.lifetime = Sprite_Effect_Lifetime::OneShot;

	sprite_format_map[Sprite_Effect_Type::Hit_2] = hit_2_effect_format;
	
}

void Sprite_Effect_Manager::Init_Sprite_Format(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	shared_ptr<CTexture> hit_1_sprite_texture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);
	hit_1_sprite_texture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Effect/hit_1.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, hit_1_sprite_texture.get(), 0, ROOT_PARAMETER_TRANSPARENT_ALBEDO_TEXTURE_INDEX);

	SpriteInfo hit_1_sprite_info{ 4,4, 16, 0.05f };
	sprite_texture_map["hit_1_effect"] = make_pair(hit_1_sprite_info, hit_1_sprite_texture);

	//---------------------------------------------------------------------------

	shared_ptr<CTexture> hit_2_sprite_texture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);
	hit_2_sprite_texture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Effect/hit_2.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, hit_2_sprite_texture.get(), 0, ROOT_PARAMETER_TRANSPARENT_ALBEDO_TEXTURE_INDEX);

	SpriteInfo hit_2_sprite_info{ 4,4, 16, 0.05f };
	sprite_texture_map["hit_2_effect"] = make_pair(hit_2_sprite_info, hit_2_sprite_texture);

	//---------------------------------------------------------------------------

	shared_ptr<CTexture> hit_3_sprite_texture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);
	hit_3_sprite_texture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Effect/hit_3.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, hit_3_sprite_texture.get(), 0, ROOT_PARAMETER_TRANSPARENT_ALBEDO_TEXTURE_INDEX);

	SpriteInfo hit_3_sprite_info{ 4,4, 16, 0.05f };
	sprite_texture_map["hit_3_effect"] = make_pair(hit_3_sprite_info, hit_3_sprite_texture);

	//---------------------------------------------------------------------------

	shared_ptr<CTexture> hit_4_sprite_texture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);
	hit_4_sprite_texture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Effect/hit_4.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, hit_4_sprite_texture.get(), 0, ROOT_PARAMETER_TRANSPARENT_ALBEDO_TEXTURE_INDEX);

	SpriteInfo hit_4_sprite_info{ 4,4, 16, 0.05f };
	sprite_texture_map["hit_4_effect"] = make_pair(hit_4_sprite_info, hit_4_sprite_texture);

	//---------------------------------------------------------------------------

	shared_ptr<CTexture> hit_5_sprite_texture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1, 0, 0, 1, 0, 0);
	hit_5_sprite_texture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Effect/hit_5.dds", RESOURCE_TEXTURE2D, 0);
	CDescriptor_Heap::CreateGraphicsShaderResourceViews(pd3dDevice, hit_5_sprite_texture.get(), 0, ROOT_PARAMETER_TRANSPARENT_ALBEDO_TEXTURE_INDEX);

	SpriteInfo hit_5_sprite_info{ 4,4, 16, 0.05f };
	sprite_texture_map["hit_5_effect"] = make_pair(hit_5_sprite_info, hit_5_sprite_texture);
}

void Sprite_Effect_Manager::Init_Mesh_Format(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	shared_ptr<Billboard_Mesh> billboard_mesh = make_shared<Billboard_Mesh>(pd3dDevice, pd3dCommandList, 100);

	mesh_map["Billboard"] = billboard_mesh;
}


void Sprite_Effect_Manager::Create_Instance_Data_ShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	std::array<Sprite_Effect_Type, 3> all_Effect_Types =
	{
	Sprite_Effect_Type::Hit_1,
	Sprite_Effect_Type::Hit_2,
	Sprite_Effect_Type::etc
	};

	for (auto type : all_Effect_Types)
	{
		Sprite_Effect_Instance_Info type_instance_info {};
		
		type_instance_info.Create_Instance_Data_ShaderVariables(pd3dDevice, pd3dCommandList);

		instance_info_map[type] = type_instance_info;
	}

}


shared_ptr<Sprite_Object> Sprite_Effect_Manager::Recycle_Effect(Sprite_Effect_Type type, Sprite_Effect_Lifetime style)
{
	shared_ptr<Sprite_Object> re_using_sprite_obj = NULL;

	auto it = effect_object_map.find(type);
	if (it == effect_object_map.end())
		return re_using_sprite_obj;

	const std::vector<std::shared_ptr<Sprite_Object>>& obj_list = it->second;

	for (shared_ptr<Sprite_Object> sprite_obj : obj_list)
	{
		if(sprite_obj->Get_Sprite_Life_Style() == style && sprite_obj->Get_Active() == false)
		{
			sprite_obj->Reset();
			re_using_sprite_obj = sprite_obj;
			break;
		}
	}

	return re_using_sprite_obj;

}

shared_ptr<Sprite_Object> Sprite_Effect_Manager::Add_Effect(Sprite_Effect_Type type, XMFLOAT3 effect_position)
{
	auto it = sprite_format_map.find(type);
	if (it == sprite_format_map.end()) 
		return NULL;
	

	const Sprite_Effect_Format& format = it->second;

	shared_ptr<Sprite_Object> sprite_obj = Recycle_Effect(type, format.lifetime);

	if (sprite_obj != NULL) {
		sprite_obj->SetPosition(effect_position);
	}
	else if (sprite_obj == NULL) 
	{
		sprite_obj = make_shared<Sprite_Object>();
		sprite_obj->SetPosition(effect_position);

		sprite_obj->SetMesh(format.mesh);
		sprite_obj->Set_Sprite_Info(format.spriteInfo);
		sprite_obj->Set_BaseTexture(format.texture);

		sprite_obj->Set_Life_Style(format.lifetime);
		sprite_obj->Set_Sprite_Effect_Type(type);

		effect_object_map[type].push_back(sprite_obj);

	}



	return sprite_obj;
}


shared_ptr<Sprite_Object> Sprite_Effect_Manager::Add_Effect(Sprite_Effect_Type type, Sprite_Effect_Lifetime life_style, XMFLOAT3 effect_position)
{
	shared_ptr<Sprite_Object> sprite_obj = Recycle_Effect(type, life_style);

	if (sprite_obj != NULL)
		sprite_obj->SetPosition(effect_position);


	if (sprite_obj == NULL)
	{
		sprite_obj = make_shared<Sprite_Object>();
		sprite_obj->SetPosition(effect_position);

		switch (type)
		{
		case Sprite_Effect_Type::Hit_1:
		{
			sprite_obj->SetMesh(mesh_map["Billboard"]);
			sprite_obj->Set_Sprite_Info(sprite_texture_map["hit_1_effect"].first);
			sprite_obj->Set_BaseTexture(sprite_texture_map["hit_1_effect"].second);
			sprite_obj->Set_Life_Style(life_style);
			sprite_obj->Set_Sprite_Effect_Type(type);
		}
		break;
		case Sprite_Effect_Type::Hit_2:
		{
			sprite_obj->SetMesh(mesh_map["Billboard"]);
			sprite_obj->Set_Sprite_Info(sprite_texture_map["hit_2_effect"].first);
			sprite_obj->Set_BaseTexture(sprite_texture_map["hit_2_effect"].second);
			sprite_obj->Set_Life_Style(life_style);
			sprite_obj->Set_Sprite_Effect_Type(type);
		}
		break;

		case Sprite_Effect_Type::etc:
		default:
			break;
		}

	}

	effect_object_map[type].push_back(sprite_obj);


	return sprite_obj;
}

void Sprite_Effect_Manager::Animate_Effects_All(float fTimeElapsed)
{
	for (pair<Sprite_Effect_Type, std::vector<std::shared_ptr<Sprite_Object>>> type_map : effect_object_map)
		Animate_Effects(fTimeElapsed, type_map.first);

}

void Sprite_Effect_Manager::Animate_Effects(float fTimeElapsed, Sprite_Effect_Type type)
{
	auto it = effect_object_map.find(type);
	if (it == effect_object_map.end())
		return;

	const std::vector<std::shared_ptr<Sprite_Object>>& obj_list = it->second;

	if (obj_list.empty())
		return;

	for (std::shared_ptr<Sprite_Object> effect_obj : obj_list)
	{
		if (effect_obj->Get_Active())
			effect_obj->Animate(fTimeElapsed);
	}
}

void Sprite_Effect_Manager::Update_Effects_All()
{
	for (pair<Sprite_Effect_Type, std::vector<std::shared_ptr<Sprite_Object>>> type_map : effect_object_map)
		Update_Effects(type_map.first);

}


void Sprite_Effect_Manager::Update_Effects(Sprite_Effect_Type type)
{
	auto it_1 = effect_object_map.find(type);
	if (it_1 == effect_object_map.end())
		return;


	auto it_2 = instance_info_map.find(type);
	if (it_2 == instance_info_map.end())
		return;
	
	std::vector<std::shared_ptr<Sprite_Object>> obj_list = effect_object_map[type];


	instance_info_map[type].Update_Instance_Data(obj_list);
}


void Sprite_Effect_Manager::Render_Effects(ID3D12GraphicsCommandList* cmdList, CCamera* pCamera, Sprite_Effect_Type type)
{
	auto it = effect_object_map.find(type);
	if (it == effect_object_map.end()) 
		return; 

	Sprite_Effect_Instance_Info instance_info = instance_info_map[type];

	if (instance_info.Instancing_num == 0)
		return;

	const auto& first_obj = effect_object_map[type].front();
	
	first_obj->Instance_Render(cmdList, instance_info.m_d3dInstancingBufferView, instance_info.Instancing_num);
}

void Sprite_Effect_Manager::Render_Effects_All(ID3D12GraphicsCommandList* cmdList, CCamera* pCamera)
{
	for (pair<Sprite_Effect_Type, std::vector<std::shared_ptr<Sprite_Object>>> type_map : effect_object_map)
		Render_Effects(cmdList, pCamera, type_map.first);
}