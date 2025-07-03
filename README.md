# Project\_Palirates

한국공대 2025년도 졸업작품



## C++ 방식 최대한 활용할 것

* 객체를 shared\_ptr로 관리
* 컨테이너 vector로 변경
* 문자열 string\_view 활용



## 주의 사항

* 커밋 할때, 파일의 인코딩 형식 확인할 것
* UTF-16 계열인 경우, UTF-8로 변환 할 것 // 깃에서 파일이 바이너리로 읽히게 되는 원인
* 사용할 조명이 많아질 경우, Deffered Lighting 최적화 할 것 // 테셀레이션을 이용하여, 조명 영역을 구하고, 영역에 포함되는 픽셀에서만 조명 연산하는 방식
* Object::Render 에서 계층 구조 따라 갈때, Active에 의한 동작 전환이 이상해보임 - 검토 필요



# 현재 진행 내용:

* Scene\_Manager
* OBB\_Drawer
* Text\_UI\_Render
* Text\_UI\_Manager
* Tile\_Map
* Particle\_Manager
* Light\_Material\_Manager
* OBB\_Render
* OBB\_CollisionManager
* OBB\_Manager

## 작업 목표:

서버 작성하기



## 진행 상황:

* 플레이어 애니메이션 동기화 되고 있는것 확인



문제점:

* 플레이어 위치 반영 안되고 있는 중 // Look벡터는 제대로 되고 있음
* 모델 동기화 필요 --> 이건 패킷에 모델 번호를 매번 보내게 해야 할 듯



## 할 일

* 플레이어 생성 소멸 동기화 동작 추가할 것



서버에 충돌 검사 추가하기

