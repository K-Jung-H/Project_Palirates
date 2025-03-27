
cbuffer Frame_Info : register(b0)
{
    float gfCurrentTime; 
    float gfElapsedTime; 

    float gfSecondsPerFirework; 
    int gnFlareParticlesToEmit; 
    int gnMaxFlareType2Particles; 
    float3 gf3Gravity; 

};

struct Material_Info
{
    float4 gAlbedoColor;

    float gRoughness;
    float gMetallic;
    float gSpecular_intensity;
    float gEmissive_intensity;
};

cbuffer cbGameObjectInfo : register(b1)
{
    matrix gmtxGameObject : packoffset(c0);
    Material_Info material_info : packoffset(c4);
    uint gnTexturesMask : packoffset(c7);

};

cbuffer cbCameraInfo : register(b2)
{
	matrix					gmtxView : packoffset(c0);
	matrix					gmtxProjection : packoffset(c4);
	float3					gvCameraPosition : packoffset(c8);
};

cbuffer cb_Prev_CameraInfo : register(b3)
{
    matrix gmtx_Ptrev_ViewProj : packoffset(c0);
};


//=========================================================

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

Texture2D gtxtAlbedoTexture : register(t0);
Texture2D gtxtSpecularTexture : register(t1);
Texture2D gtxtNormalTexture : register(t2);
Texture2D gtxtMetallicTexture : register(t3);
Texture2D gtxtEmissionTexture : register(t4);

Texture2D gtxtTerrainBaseTexture : register(t5);
Texture2D gtxtTerrainDetailTexture : register(t6);
TextureCube gtxtSkyCubeTexture : register(t7);


SamplerState gssWrap : register(s0);
SamplerState gssClamp : register(s1);

//===========================================================

struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
    float4 Albedo_Color : SV_TARGET0;
    float4 world_Position : SV_TARGET1;
    float4 world_Normal_and_Camera_Distance : SV_TARGET2;
    float4 Material_Light_Info : SV_TARGET3;
    float2 Velocity : SV_TARGET4;

};


struct VS_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_STANDARD_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
    
    // For Motion_Vector
    float2 screenUV : TEXCOORD1; 
    float2 prevScreenUV : TEXCOORD2; 
};

//===========================================================

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;

    float4 worldPos = mul(float4(input.position, 1.0f), gmtxGameObject);
    output.positionW = worldPos.xyz;

    float4 currClip = mul(mul(worldPos, gmtxView), gmtxProjection);
    output.position = currClip;

    float4 prevClip = mul(worldPos, gmtx_Ptrev_ViewProj);

    output.screenUV = currClip.xy / currClip.w * 0.5f + 0.5f;
    output.prevScreenUV = prevClip.xy / prevClip.w * 0.5f + 0.5f;

    
    output.normalW = mul(input.normal, (float3x3) gmtxGameObject);
    output.tangentW = mul(input.tangent, (float3x3) gmtxGameObject);
    output.bitangentW = mul(input.bitangent, (float3x3) gmtxGameObject);
    output.uv = input.uv;

    return output;
}

//===========================================================

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSStandard(VS_STANDARD_OUTPUT input)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
    output.Albedo_Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    output.world_Position = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.world_Normal_and_Camera_Distance = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.Material_Light_Info = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.Velocity = float2(0.0f, 0.0f);
    
    float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
        cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
    float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_SPECULAR_MAP)
        cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
    float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
        cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
    float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_METALLIC_MAP)
        cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
    float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_EMISSION_MAP)
        cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);

    float3 normalW;
    float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
    {
        float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
        float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
        normalW = normalize(mul(vNormal, TBN));
    }
    else
    {
        normalW = normalize(input.normalW);
    }
    
    
    output.Albedo_Color = cColor;
    
    output.world_Position = float4(input.positionW, 1.0f);
    output.world_Normal_and_Camera_Distance.xyz = normalW;
    output.world_Normal_and_Camera_Distance.w = distance(input.positionW, gvCameraPosition);

    output.Material_Light_Info = float4(material_info.gRoughness, material_info.gMetallic, material_info.gSpecular_intensity, material_info.gEmissive_intensity);
    output.Velocity = input.screenUV - input.prevScreenUV;
    
    
    return (output);

}


//===========================================================

struct VS_STANDARD_INPUT_INSTANCE
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
	
    float4x4 instance_worldMatrix : WORLDMATRIX;
};

VS_STANDARD_OUTPUT VSStandard_INSTANCE(VS_STANDARD_INPUT_INSTANCE input)
{
    VS_STANDARD_OUTPUT output;
    
    float4 worldPos = mul(float4(input.position, 1.0f), input.instance_worldMatrix);
    output.positionW = worldPos.xyz;

    float4 currClip = mul(mul(worldPos, gmtxView), gmtxProjection);
    output.position = currClip;

    float4 prevClip = mul(worldPos, gmtx_Ptrev_ViewProj);

    output.screenUV = currClip.xy / currClip.w * 0.5f + 0.5f;
    output.prevScreenUV = prevClip.xy / prevClip.w * 0.5f + 0.5f;
    

    
    output.normalW = mul(input.normal, (float3x3) input.instance_worldMatrix);
    output.tangentW = mul(input.tangent, (float3x3) input.instance_worldMatrix);
    output.bitangentW = mul(input.bitangent, (float3x3) input.instance_worldMatrix);
    output.uv = input.uv;
		

    return (output);
}

//==================================================================

#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			256

cbuffer cbBoneOffsets : register(b4)
{
	float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b5)
{
	float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_SKINNED_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	int4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
};

VS_STANDARD_OUTPUT VS_SkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;

    float4x4 mtxVertexToBoneWorld = (float4x4) 0.0f;
    for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
    {
        mtxVertexToBoneWorld += input.weights[i] *
            mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
    }

    float4 worldPos = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld);
    output.positionW = worldPos.xyz;

    float4 viewPos = mul(worldPos, gmtxView);
    float4 currClip = mul(viewPos, gmtxProjection);
    output.position = currClip;

    float4 prevClip = mul(worldPos, gmtx_Ptrev_ViewProj);

    output.screenUV = currClip.xy / currClip.w * 0.5f + 0.5f;
    output.prevScreenUV = prevClip.xy / prevClip.w * 0.5f + 0.5f;

    
    
    output.normalW = mul(input.normal, (float3x3) mtxVertexToBoneWorld).xyz;
    output.tangentW = mul(input.tangent, (float3x3) mtxVertexToBoneWorld).xyz;
    output.bitangentW = mul(input.bitangent, (float3x3) mtxVertexToBoneWorld).xyz;

    output.uv = input.uv;

    return output;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
    float3 positionW : POSITION;

	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
    
};

VS_TERRAIN_OUTPUT VSTerrain_Solid(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;
    output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
    float4 positionV = mul(float4(output.positionW, 1.0f), gmtxView);
    output.position = mul(positionV, gmtxProjection);
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;
    
	return(output);
}


PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSTerrain_Solid(VS_TERRAIN_OUTPUT input)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
    output.Albedo_Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    output.world_Position = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.world_Normal_and_Camera_Distance = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.Material_Light_Info = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.Velocity = float2(0.0f, 0.0f);
    
    float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
    float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);
    
    output.Albedo_Color = input.color * saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));
    
    output.world_Position = float4(input.positionW, 1.0f);
    output.world_Normal_and_Camera_Distance.xyz = float3(0.0f, 1.0f, 0.0f);
    output.world_Normal_and_Camera_Distance.w = distance(input.positionW, gvCameraPosition);

    output.Material_Light_Info = float4(material_info.gRoughness, 0.0f, material_info.gSpecular_intensity, material_info.gEmissive_intensity);
    
    return (output);
}

VS_TERRAIN_OUTPUT VSTerrain_Wireframe(VS_TERRAIN_INPUT input)
{
    VS_TERRAIN_OUTPUT output;
    input.position.y -= 1.0f;
    
    output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
    float4 positionV = mul(float4(output.positionW, 1.0f), gmtxView);
    output.position = mul(positionV, gmtxProjection);
    
    output.color = input.color;
    output.uv0 = input.uv0;
    output.uv1 = input.uv1;

    return (output);
}

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSTerrain_Wireframe(VS_TERRAIN_OUTPUT input)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
    output.Albedo_Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    output.world_Position = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.world_Normal_and_Camera_Distance = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.Material_Light_Info = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.Velocity = float2(0.0f, 0.0f);
    
    output.Albedo_Color = input.color;
    
    output.world_Position = float4(input.positionW, 1.0f);
    output.world_Normal_and_Camera_Distance.xyz = float3(0.0f, 1.0f, 0.0f);
    output.world_Normal_and_Camera_Distance.w = distance(input.positionW, gvCameraPosition);

    output.Material_Light_Info = float4(material_info.gRoughness, 0.0f, material_info.gSpecular_intensity, material_info.gEmissive_intensity);

    return (output);
}


//=============================================================

struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float3	positionL : POSITION;
	float4	position : SV_POSITION;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
	VS_SKYBOX_CUBEMAP_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.positionL = input.position;

	return(output);
}


// SkyBox don't need DefferedRendering
float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, input.positionL);

    return (cColor);
}

struct VS_OBB_INPUT
{
    float3 position : POSITION; 
    float4 color : COLOR; 
    
    float4x4 obb_worldMatrix : WORLDMATRIX; // 월드 변환 행렬
    float4 instanceColor : INSTANCECOLOR; // 색상
};

struct VS_OBB_OUTPUT
{
    float4 position : SV_POSITION; 
    float4 color : COLOR; 
};

VS_OBB_OUTPUT VS_BoundingBox(VS_OBB_INPUT input)
{
    VS_OBB_OUTPUT output;
	output.position = mul(mul(mul(float4(input.position, 1.0f), input.obb_worldMatrix), gmtxView), gmtxProjection);
    output.color = input.instanceColor;
	
    return (output);
}


// OBB don't need DefferedRendering
PS_MULTIPLE_RENDER_TARGETS_OUTPUT PS_BoundingBox(VS_OBB_OUTPUT input)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
    output.Albedo_Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    output.world_Position = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.world_Normal_and_Camera_Distance = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.Material_Light_Info = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.Velocity = float2(0.0f, 0.0f);
    
    return (output);
}


//float4 PS_BoundingBox(VS_OBB_OUTPUT input) : SV_TARGET
//{
//    float4 cColor = input.color;
        
//    return (cColor);
//}