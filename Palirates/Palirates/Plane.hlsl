#define FRAME_BUFFER_WIDTH 840.0f
#define FRAME_BUFFER_HEIGHT 480.0f

SamplerState gssWrap : register(s0);
SamplerState gssClamp : register(s1);


cbuffer Frame_Info : register(b0)
{
    float gfCurrentTime;
    float gfElapsedTime;
};


struct Material_Info
{
    float4 gAlbedoColor;
    uint material_ID;
    uint padding0;
    uint padding1;
    uint padding2;
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
    float3 gvCameraPosition : packoffset(c8);
};

Texture2D Plane_BaseTexture : register(t0);
Texture2D Plane_DetailTexture : register(t1);
Texture2D Plane_Height_Map: register(t2);
Texture2D Plane_Normal_Map : register(t3);

struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
    float4 Albedo_Color : SV_TARGET0;
    float4 world_Position : SV_TARGET1;
    float4 world_Normal_and_Camera_Distance : SV_TARGET2;
    float4 Velocity_Mask_Obj_Id : SV_TARGET3;

};


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

VS_TERRAIN_OUTPUT VS_Plane(VS_TERRAIN_INPUT input)
{
    VS_TERRAIN_OUTPUT output;
        
    output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;

    float height = Plane_Height_Map.SampleLevel(gssWrap, input.uv0, 0.0f).x;

    height -= 0.5f;
    //if (height < 0.0f)
    //    height *= 10.0f;
    
        output.positionW.y += (height * 50.0f);
    
    float4 positionV = mul(float4(output.positionW, 1.0f), gmtxView);
    output.position = mul(positionV, gmtxProjection);
    output.color = input.color;
    output.uv0 = input.uv0;
    output.uv1 = input.uv1;
    
    return (output);
}


PS_MULTIPLE_RENDER_TARGETS_OUTPUT PS_Plane(VS_TERRAIN_OUTPUT input)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
    output.Albedo_Color = float4(1.0f, 0.0f, 0.0f, 0.0f);
    output.world_Position = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.world_Normal_and_Camera_Distance = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.Velocity_Mask_Obj_Id = float4(0.0f, 0.0f, 1.0f, 0.0f);

    float2 animatedUV1 = input.uv1 + float2(0.0, gfCurrentTime * 0.1f); // x축 흐름

    float4 cBaseTexColor = Plane_BaseTexture.Sample(gssWrap, input.uv0);
    float4 cDetailTexColor = Plane_DetailTexture.Sample(gssWrap, animatedUV1);

    output.Albedo_Color.xyz = (input.color * saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f))).xyz;
    output.Albedo_Color.a = (float) (material_info.material_ID) / 255.0f;
    
//       output.Albedo_Color = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));
    
//    output.Albedo_Color = Plane_Height_Map.Sample(gssWrap, input.uv0); // For Debug
//    output.Albedo_Color = Plane_Normal_Map.Sample(gssWrap, input.uv0); // For Debug

    output.world_Position = float4(input.positionW, 1.0f);
    
    
    float3 plane_normal = Plane_Normal_Map.Sample(gssWrap, input.uv0).xyz;

    
    output.world_Normal_and_Camera_Distance.xyz = plane_normal;
    output.world_Normal_and_Camera_Distance.w = distance(input.positionW, 0.0f);
    
    return (output);
}
