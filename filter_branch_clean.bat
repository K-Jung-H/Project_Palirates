@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

:: ? 설정
set BRANCH_NAME=snowmourne_server_main

echo ?? 브랜치 이동 중: %BRANCH_NAME%
git checkout %BRANCH_NAME%

:: ? 제거 대상 경로들
set TARGETS=.vs AutoPCH *.ipch *.VC.db

:: ? 제거 처리 시작
echo ?? Git 인덱스에서 대용량 파일 제거 중...

for %%T in (%TARGETS%) do (
    echo → 제거 시도: %%T
    git rm --cached -r -f %%T 2>nul
)

:: ? .gitmodules 제거
if exist .gitmodules (
    echo ?? .gitmodules 제거
    git rm .gitmodules
)

:: ? .gitignore 업데이트
echo ??? .gitignore 정리 중...
(
    echo .vs/
    echo AutoPCH/
    echo *.ipch
    echo *.VC.db
) >> .gitignore

:: ? 변경 커밋
echo ?? 변경 커밋 중...
git add .gitignore
git commit -m "Removed cached large files and updated .gitignore"

:: ? 강제 푸시
echo ?? GitHub에 강제 푸시 중...
git push origin %BRANCH_NAME% --force

echo ? 완료! 브랜치 정리 및 푸시 성공!
pause
