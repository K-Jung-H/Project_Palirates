//--------------------------------------------------------------------------------------
#define MAX_LIGHTS			16 
#define MAX_MATERIALS		16 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

#define _WITH_LOCAL_VIEWER_HIGHLIGHTING
#define _WITH_THETA_PHI_CONES
//#define _WITH_REFLECT

struct LIGHT
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular;
	float3					m_vPosition;
	float 					m_fFalloff;
	float3					m_vDirection;
	float 					m_fTheta; //cos(m_fTheta)
	float3					m_vAttenuation;
	float					m_fPhi; //cos(m_fPhi)
	bool					m_bEnable;
	int 					m_nType;
	float					m_fRange;
	float					padding;
};

struct MATERIAL
{
    float4 m_cAmbient;
    float4 m_cDiffuse;
    float4 m_cSpecular;
    float4 m_cEmissive;
    
    float m_fGlossiness;
    float m_fSmoothness;
    float m_fSpecularHighlight;
    float m_fMetallic;
    float m_fGlossyReflection;
};



cbuffer cbLights : register(b1)
{
	LIGHT					gLights[MAX_LIGHTS];
	float4					gcGlobalAmbientLight;
	int						gnLights;
};


StructuredBuffer<MATERIAL> Material_Info : register(t5);

//===================================================================

float4 ComputeDiffuseSpecular(float3 vToLight, float3 vNormal, float3 vToCamera, int Material_ID, int Light_ID)
{
    MATERIAL material = Material_Info[Material_ID]; 
    LIGHT light = gLights[Light_ID];

    float fDiffuseFactor = max(dot(vToLight, vNormal), 0.0f);
    float fSpecularFactor = 0.0f;

    if (fDiffuseFactor > 0.0f && material.m_cSpecular.a > 0.0f)
    {
#ifdef _WITH_REFLECT
        float3 vReflect = reflect(-vToLight, vNormal);
        fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), material.m_cSpecular.a);
#else
        float3 vHalf = normalize(vToCamera + vToLight);
        fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), material.m_cSpecular.a);
#endif
    }

    return (light.m_cAmbient * material.m_cAmbient) +
           (light.m_cDiffuse * fDiffuseFactor * material.m_cDiffuse) +
           (light.m_cSpecular * fSpecularFactor * material.m_cSpecular);
}



float4 DirectionalLight(int Light_ID, float3 vNormal, float3 vToCamera, int Material_ID)
{
    float3 vToLight = -gLights[Light_ID].m_vDirection;
    return ComputeDiffuseSpecular(vToLight, vNormal, vToCamera, Material_ID, Light_ID);
}



float4 PointLight(int Light_ID, float3 vPosition, float3 vNormal, float3 vToCamera, int Material_ID)
{
    float3 vToLight = gLights[Light_ID].m_vPosition - vPosition;
    float fDistance = length(vToLight);
    
    if (fDistance <= gLights[Light_ID].m_fRange)
    {
        vToLight /= fDistance;
        float fAttenuation = 1.0f / dot(gLights[Light_ID].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));
        return ComputeDiffuseSpecular(vToLight, vNormal, vToCamera, Material_ID, Light_ID) * fAttenuation;
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}



float4 SpotLight(int Light_ID, float3 vPosition, float3 vNormal, float3 vToCamera, int Material_ID)
{
    float3 vToLight = gLights[Light_ID].m_vPosition - vPosition;
    float fDistance = length(vToLight);
    
    if (fDistance <= gLights[Light_ID].m_fRange)
    {
        vToLight /= fDistance;
        float fAttenuation = 1.0f / dot(gLights[Light_ID].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));
        float fSpotFactor = pow(max(dot(-vToLight, gLights[Light_ID].m_vDirection), 0.0f), gLights[Light_ID].m_fFalloff);

        return ComputeDiffuseSpecular(vToLight, vNormal, vToCamera, Material_ID, Light_ID) * fAttenuation * fSpotFactor;
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}


float4 Lighting(float3 wPosition, float3 wNormal, float3 camera_pos, int Material_ID)
{
    float3 vToCamera = normalize(camera_pos.xyz - wPosition);
    float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);


    for (int i = 0; i < gnLights; i++)
    {
        if (gLights[i].m_bEnable)
        {
            if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
            {
                cColor += DirectionalLight(i, wNormal, vToCamera, Material_ID);
            }
            else if (gLights[i].m_nType == POINT_LIGHT)
            {
                cColor += PointLight(i, wPosition, wNormal, vToCamera, Material_ID);
            }
            else if (gLights[i].m_nType == SPOT_LIGHT)
            {
                cColor += SpotLight(i, wPosition, wNormal, vToCamera, Material_ID);
            }
        }
    }

    MATERIAL material = Material_Info[Material_ID];
    cColor += (gcGlobalAmbientLight * material.m_cAmbient);
    cColor.a = material.m_cDiffuse.a;

    return cColor;
}