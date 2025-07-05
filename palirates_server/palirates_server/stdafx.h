#pragma once

#define WIN32_LEAN_AND_MEAN   
#define NOMINMAX              

#include <windows.h> 
#include <wrl.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")


#include <Mmsystem.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <mutex>
//#include <DirectXMath.h>
#include "DX_Setter.h"

using namespace std;

enum Scene_Type
{
	Lobby,
	Board,
	Stage_1,
	Stage_2,
	Test,
	etc,
	None,
};

enum KeyIndex
{
    KEY_INDEX_W = 0,
    KEY_INDEX_S = 1,
    KEY_INDEX_A = 2,
    KEY_INDEX_D = 3,
    KEY_INDEX_Q = 4,
    KEY_INDEX_E = 5,
    KEY_INDEX_SHIFT = 6,
    KEY_INDEX_ENTER = 7
};

// 실제 플래그 enum
enum InputFlags : uint32_t
{
    INPUT_NONE = 0,
    INPUT_W = 1 << KEY_INDEX_W,
    INPUT_S = 1 << KEY_INDEX_S,
    INPUT_A = 1 << KEY_INDEX_A,
    INPUT_D = 1 << KEY_INDEX_D,
    INPUT_Q = 1 << KEY_INDEX_Q,
    INPUT_E = 1 << KEY_INDEX_E,
    INPUT_SHIFT = 1 << KEY_INDEX_SHIFT,
    INPUT_ENTER = 1 << KEY_INDEX_ENTER
};

//namespace Matrix4x4
//{
//	inline XMFLOAT4X4 Identity()
//	{
//		XMFLOAT4X4 xmf4x4Result;
//		XMStoreFloat4x4(&xmf4x4Result, XMMatrixIdentity());
//		return(xmf4x4Result);
//	}
//
//	inline XMFLOAT4X4 Zero()
//	{
//		XMFLOAT4X4 xmf4x4Result;
//		XMStoreFloat4x4(&xmf4x4Result, XMMatrixSet(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
//		return(xmf4x4Result);
//	}
//
//	inline XMFLOAT4X4 Multiply(XMFLOAT4X4& xmmtx4x4Matrix1, XMFLOAT4X4& xmmtx4x4Matrix2)
//	{
//		XMFLOAT4X4 xmf4x4Result;
//		XMStoreFloat4x4(&xmf4x4Result, XMMatrixMultiply(XMLoadFloat4x4(&xmmtx4x4Matrix1), XMLoadFloat4x4(&xmmtx4x4Matrix2)));
//		return(xmf4x4Result);
//	}
//
//	inline XMFLOAT4X4 Scale(XMFLOAT4X4& xmf4x4Matrix, float fScale)
//	{
//		XMFLOAT4X4 xmf4x4Result;
//		XMStoreFloat4x4(&xmf4x4Result, XMLoadFloat4x4(&xmf4x4Matrix) * fScale);
//		/*
//				XMVECTOR S, R, T;
//				XMMatrixDecompose(&S, &R, &T, XMLoadFloat4x4(&xmf4x4Matrix));
//				S = XMVectorScale(S, fScale);
//				T = XMVectorScale(T, fScale);
//				R = XMVectorScale(R, fScale);
//				//R = XMQuaternionMultiply(R, XMVectorSet(0, 0, 0, fScale));
//				XMStoreFloat4x4(&xmf4x4Result, XMMatrixAffineTransformation(S, XMVectorZero(), R, T));
//		*/
//		return(xmf4x4Result);
//	}
//
//	inline XMFLOAT4X4 Add(XMFLOAT4X4& xmmtx4x4Matrix1, XMFLOAT4X4& xmmtx4x4Matrix2)
//	{
//		XMFLOAT4X4 xmf4x4Result;
//		XMStoreFloat4x4(&xmf4x4Result, XMLoadFloat4x4(&xmmtx4x4Matrix1) + XMLoadFloat4x4(&xmmtx4x4Matrix2));
//		return(xmf4x4Result);
//	}
//
//	inline XMFLOAT4X4 Multiply(XMFLOAT4X4& xmmtx4x4Matrix1, XMMATRIX& xmmtxMatrix2)
//	{
//		XMFLOAT4X4 xmf4x4Result;
//		XMStoreFloat4x4(&xmf4x4Result, XMLoadFloat4x4(&xmmtx4x4Matrix1) * xmmtxMatrix2);
//		return(xmf4x4Result);
//	}
//
//	inline XMFLOAT4X4 RotateAxis(XMFLOAT3& xmf3Axis, float fAngle)
//	{
//		XMFLOAT4X4 xmf4x4Result;
//		XMStoreFloat4x4(&xmf4x4Result, XMMatrixRotationAxis(XMLoadFloat3(&xmf3Axis), XMConvertToRadians(fAngle)));
//		return(xmf4x4Result);
//	}
//
//	inline XMFLOAT4X4 Rotate(float x, float y, float z)
//	{
//		XMFLOAT4X4 xmf4x4Result;
//		XMStoreFloat4x4(&xmf4x4Result, XMMatrixRotationRollPitchYaw(XMConvertToRadians(x), XMConvertToRadians(y), XMConvertToRadians(z)));
//		return(xmf4x4Result);
//	}
//
//	inline XMFLOAT4X4 AffineTransformation(XMFLOAT3& xmf3Scaling, XMFLOAT3& xmf3RotateOrigin, XMFLOAT3& xmf3Rotation, XMFLOAT3& xmf3Translation)
//	{
//		XMFLOAT4X4 xmf4x4Result;
//		XMStoreFloat4x4(&xmf4x4Result, XMMatrixAffineTransformation(XMLoadFloat3(&xmf3Scaling), XMLoadFloat3(&xmf3RotateOrigin), XMQuaternionRotationRollPitchYaw(XMConvertToRadians(xmf3Rotation.x), XMConvertToRadians(xmf3Rotation.y), XMConvertToRadians(xmf3Rotation.z)), XMLoadFloat3(&xmf3Translation)));
//		return(xmf4x4Result);
//	}
//
//	inline XMFLOAT4X4 Multiply(XMMATRIX& xmmtxMatrix1, XMFLOAT4X4& xmmtx4x4Matrix2)
//	{
//		XMFLOAT4X4 xmf4x4Result;
//		XMStoreFloat4x4(&xmf4x4Result, xmmtxMatrix1 * XMLoadFloat4x4(&xmmtx4x4Matrix2));
//		return(xmf4x4Result);
//	}
//
//	inline XMFLOAT4X4 Interpolate(XMFLOAT4X4& xmf4x4Matrix1, XMFLOAT4X4& xmf4x4Matrix2, float t)
//	{
//		XMFLOAT4X4 xmf4x4Result;
//		XMVECTOR S0, R0, T0, S1, R1, T1;
//		XMMatrixDecompose(&S0, &R0, &T0, XMLoadFloat4x4(&xmf4x4Matrix1));
//		XMMatrixDecompose(&S1, &R1, &T1, XMLoadFloat4x4(&xmf4x4Matrix2));
//		XMVECTOR S = XMVectorLerp(S0, S1, t);
//		XMVECTOR T = XMVectorLerp(T0, T1, t);
//		XMVECTOR R = XMQuaternionSlerp(R0, R1, t);
//		XMStoreFloat4x4(&xmf4x4Result, XMMatrixAffineTransformation(S, XMVectorZero(), R, T));
//		return(xmf4x4Result);
//	}
//
//	inline XMFLOAT4X4 Inverse(XMFLOAT4X4& xmmtx4x4Matrix)
//	{
//		XMFLOAT4X4 xmf4x4Result;
//		XMStoreFloat4x4(&xmf4x4Result, XMMatrixInverse(NULL, XMLoadFloat4x4(&xmmtx4x4Matrix)));
//		return(xmf4x4Result);
//	}
//
//	inline XMFLOAT4X4 Transpose(XMFLOAT4X4& xmmtx4x4Matrix)
//	{
//		XMFLOAT4X4 xmf4x4Result;
//		XMStoreFloat4x4(&xmf4x4Result, XMMatrixTranspose(XMLoadFloat4x4(&xmmtx4x4Matrix)));
//		return(xmf4x4Result);
//	}
//
//	inline XMFLOAT4X4 PerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ, float FarZ)
//	{
//		XMFLOAT4X4 xmf4x4Result;
//		XMStoreFloat4x4(&xmf4x4Result, XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio, NearZ, FarZ));
//		return(xmf4x4Result);
//	}
//	inline XMFLOAT4X4 LookAtLH(XMFLOAT3& xmf3EyePosition, XMFLOAT3& xmf3LookAtPosition, XMFLOAT3& xmf3UpDirection)
//	{
//		XMFLOAT4X4 xmf4x4Result;
//		XMStoreFloat4x4(&xmf4x4Result, XMMatrixLookAtLH(XMLoadFloat3(&xmf3EyePosition), XMLoadFloat3(&xmf3LookAtPosition), XMLoadFloat3(&xmf3UpDirection)));
//		return(xmf4x4Result);
//	}
//}