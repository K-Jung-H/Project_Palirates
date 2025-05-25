#define FRAME_BUFFER_WIDTH 840.0f
#define FRAME_BUFFER_HEIGHT 480.0f


cbuffer Frame_Info : register(b0)
{
    float gfCurrentTime; 
    float gfElapsedTime; 
};

struct Material_Info
{
    float4 gAlbedoColor;
    uint light_material_ID;
    uint Outline_Color_ID;
    uint Blur_Mask;
    uint padding0;
};

cbuffer cbGameObjectInfo : register(b1)
{
    matrix gmtxGameObject : packoffset(c0); // 16개 (c0 ~ c3)
    Material_Info material_info : packoffset(c4); // 8개 (c4 ~ c5)
    float3 gObjectVelocity : packoffset(c6); // 3개 (c6.xyz)
    uint gnTexturesMask : packoffset(c6.w); // 1개 (c6.w)
};

cbuffer cbCameraInfo : register(b2)
{
    matrix gmtxView : packoffset(c0);
    matrix gmtxProjection : packoffset(c4);
    matrix gmtxInverseView : packoffset(c8);
    float3 gvCameraPosition : packoffset(c12);
};

cbuffer cb_Prev_CameraInfo : register(b3)
{
    matrix gmtx_Prev_ViewProj : packoffset(c0);
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
    float4 Velocity_Mask_Obj_Id : SV_TARGET3;

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

    float2 velocity : TEXCOORD1; // Velocity for Motion_Vector
};

//===========================================================

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;

    // 월드 공간 위치
    float4 worldPos = mul(float4(input.position, 1.0f), gmtxGameObject);
    output.positionW = worldPos.xyz;

    // 현재 클립 위치 (카메라 이동 포함)
    float4 clipCurr = mul(mul(worldPos, gmtxView), gmtxProjection);
    output.position = clipCurr;
    float2 currNDC = clipCurr.xy / clipCurr.w;

    // 이전 프레임 카메라에서 본 위치 (같은 worldPos)
    float4 clipPrevCam = mul(worldPos, gmtx_Prev_ViewProj);
    float2 prevNDCCam = clipPrevCam.xy / clipPrevCam.w;

    float2 camVelocity = currNDC - prevNDCCam;

    // 객체 속도 → 방향 벡터 (w = 0)
    float4 velocityClip = mul(mul(float4(gObjectVelocity, 0.0f), gmtxView), gmtxProjection);
    float2 objVelocity = velocityClip.xy / clipCurr.w;

    // 블렌딩 가중치 (속도 큰 쪽 중심)
    float lenCam = length(camVelocity);
    float lenObj = length(objVelocity);
    float weight = lenObj / (lenObj + lenCam + 1e-5);

    float2 blendedVelocity = lerp(camVelocity, objVelocity, weight);

    output.velocity = blendedVelocity;

    // 기타 속성
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
    output.Albedo_Color = float4(1.0f, 0.0f, 0.0f, 0.0f);
    output.world_Position = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.world_Normal_and_Camera_Distance = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.Velocity_Mask_Obj_Id = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
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
    
    
    output.Albedo_Color.xyz = cColor.xyz;
    output.Albedo_Color.a = (float) (material_info.light_material_ID) / 255.0f;
    
    output.world_Position = float4(input.positionW, 1.0f);
    output.world_Normal_and_Camera_Distance.xyz = normalW;
    output.world_Normal_and_Camera_Distance.w = distance(input.positionW, gvCameraPosition);
    
    output.Velocity_Mask_Obj_Id.xy = input.velocity.xy;
    output.Velocity_Mask_Obj_Id.z = material_info.Blur_Mask; // mask
    output.Velocity_Mask_Obj_Id.w = material_info.Outline_Color_ID; // outline_id
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

    float2 currUV = currClip.xy / currClip.w * 0.5f + 0.5f;

    float4 prevClip = mul(worldPos, gmtx_Prev_ViewProj);
    float2 prevUV = prevClip.xy / prevClip.w * 0.5f + 0.5f;

    float2 camVelocityUV = currUV - prevUV;
    float2 camVelocityPx = camVelocityUV * float2(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
    
    // 고정된 객체는 카메라의 이동 반대 방향으로 블러링되야 자연스러움
    output.velocity = -camVelocityPx; 
    
    output.normalW = mul(input.normal, (float3x3) input.instance_worldMatrix);
    output.tangentW = mul(input.tangent, (float3x3) input.instance_worldMatrix);
    output.bitangentW = mul(input.bitangent, (float3x3) input.instance_worldMatrix);
    output.uv = input.uv;

    return output;
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


// 픽셀 좌표계 기반

VS_STANDARD_OUTPUT VS_SkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;

    // 스키닝 적용
    float4x4 mtxVertexToBoneWorld = (float4x4) 0.0f;
    for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
    {
        mtxVertexToBoneWorld += input.weights[i] *
            mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
    }

    float4 worldPos = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld);
    output.positionW = worldPos.xyz;

    // 클립 공간 위치 계산
    float4 clipCurr = mul(mul(worldPos, gmtxView), gmtxProjection);
    output.position = clipCurr;

    // 객체 이동에 의한 velocity (뷰-투영 후 클립 → NDC → 픽셀)
    float4 velocityClip = mul(mul(float4(gObjectVelocity, 0.0f), gmtxView), gmtxProjection);
    float2 objVelocityNDC = velocityClip.xy / clipCurr.w;
    float2 objVelocityPx = objVelocityNDC * 0.5f * float2(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
    
    output.velocity = objVelocityPx;

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
    output.Albedo_Color = float4(1.0f, 0.0f, 0.0f, 0.0f);
    output.world_Position = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.world_Normal_and_Camera_Distance = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.Velocity_Mask_Obj_Id = float4(0.0f, 0.0f, 0.0f, 20.0f);
    
    float3 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0).xyz;
    float3 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1).xyz;
    
    //output.Albedo_Color = input.color * saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));
    output.Albedo_Color.xyz = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));
    output.Albedo_Color.a = (float) (material_info.light_material_ID) / 255.0f;
    
    output.world_Position = float4(input.positionW, 1.0f);
    output.world_Normal_and_Camera_Distance.xyz = float3(0.0f, 1.0f, 0.0f);
    output.world_Normal_and_Camera_Distance.w = distance(input.positionW, gvCameraPosition);
    
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
    output.Albedo_Color = float4(1.0f, 0.0f, 0.0f, 0.0f);
    output.world_Position = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.world_Normal_and_Camera_Distance = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.Velocity_Mask_Obj_Id = float4(0.0f, 0.0f, 0.0f, 20.0f);
    
    output.Albedo_Color.xyz = input.color.xyz;
    output.Albedo_Color.a = (float) (material_info.light_material_ID) / 255.0f;
    
    output.world_Position = float4(input.positionW, 1.0f);
    output.world_Normal_and_Camera_Distance.xyz = float3(0.0f, 1.0f, 0.0f);
    output.world_Normal_and_Camera_Distance.w = distance(input.positionW, gvCameraPosition);

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

float4 PS_BoundingBox(VS_OBB_OUTPUT input) : SV_TARGET
{
    float4 cColor = input.color;
        
    return (cColor);
}

struct VS_UI_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_UI_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_UI_OUTPUT VS_UI(VS_UI_INPUT input)
{
    VS_UI_OUTPUT output;
    output.position = float4(input.position.xy, 0.0f, 1.0f); 
    output.uv = input.uv;
    return output;
}

float4 PS_UI(VS_UI_OUTPUT input) : SV_TARGET
{
    return gtxtAlbedoTexture.Sample(gssClamp, input.uv);
}