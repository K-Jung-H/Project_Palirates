# Project_Palirates
한국공대 2025년도 졸업작품


기존 프레임 워크를 C에서 C++ 방식으로 변경 중
------------------------------------------------------------------------------------------------
- 객체를 shared_ptr로 관리
- 컨테이너 vector로 변경
- 문자열 string_view 활용

- 파티클 처리 방식:
	- 1. 입자들을 각 설정된 값에 따라 업데이트 -> 입자들의 배열을 UAV로 연결하여 CS 단계로 전달하고, CS단계에서 동일한 연산을 병렬 처리
	- 2. 입자들의 배열을 렌더링 -> GS, SO 단계를 거쳐 일부 입자들이 소멸되고, 새로운 입자들이 배열에 추가됨
	- 3. 입자들의 배열을 렌더링 -> 업데이트된 입자들의 정보가 담긴 배열을 다시 CBV로 연결하여, 그래픽스 파이프라인에서 인스턴싱 기반 렌더링 처리

주요 사항
------------------------------------------------------------------------------------------------


현재 진행 내용:
===================================================================
- Scene_Manager
- OBB_Drawer
- Text_UI_Render
- Text_UI_Manager
- Tile_Map
- Particle_Manager 


작업 목표:
------------------------------------------------------------------------------------------------
G 버퍼 타입 변경하기
{
	float4 Albedo_Texture : SV_TARGET0;
	float4 view_Normal : SV_TARGET1;
	float view_Depth : SV_TARGET2;
	float Camera_Distance : SV_TARGET3;
	float Material_ID : SV_TARGET4;
}

사용하는 재질 ID를 렌더 타겟에 저장하고, PostRender 과정에서 ID 기반으로 재질을 찾아 적용할 것

즉, 재질을 객체의 자식 변수가 아니라, 전부 컨테이너에 담아서 관리하고 PostRender에서 배열로 연결하여, ID로 사용할 재질을 찾아 적용해야 함

재질을 관리하는 컨테이너부터 만들기 -> Material_Manager 만들기

GameObject에 저장되는 Material 구조체는 유지

기존 속성은 유지하되, 
- shader
- texture 

빛을 반사하는 속성
	UINT							m_nType = 0x00;
	float							m_fGlossiness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;
	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
속성들을 별도의 구조체로 저장하고, 메니저를 통해 관리하도록 설정하기

CMaterial에서는 ID를 통해 매니저에서 읽어올 것.
매니저는 씬 메니저의 자식 객체로 설정하고, 씬에서는 포인터를 통해 연결되도록 하여, 씬마다 중복되는 재질을 방지하기


진행 상황
------------------------------------------------------------------------------------------------
메니저 구현 완료, 중복 재질 방지되는 것 확인, Material 정보 저장한 Structured_Buffer 생성

To do:
-> 셰이더에서 렌더 타겟에 Material ID 저장하게 하기
-> RTV에 사용할 GXDI 포맷 변경하게 하기
-> 프레임워크에 메니저에서 Structured_Buffer 를 셰이더 변수에 연결하는 동작 추가하기
-> Light 함수 Post에서 호출하여, Deffered_Render 테스트하기




