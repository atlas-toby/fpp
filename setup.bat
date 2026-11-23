@echo off
chcp 65001 >nul
echo ========================================
echo        F++ Kurulum (fpp setup)
echo ========================================
echo.

where gcc >nul 2>nul
if %errorlevel% neq 0 (
    echo [HATA] gcc bulunamadi! Lutfen MinGW-W64 (gcc) kurup tekrar deneyin.
    pause
    exit /b 1
)

set "INSTALL_DIR=%LOCALAPPDATA%\fpp"
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

echo [1/3] Derleniyor: fpp.exe (Yorumlayici)...
gcc main.c -o "%INSTALL_DIR%\fpp.exe" -lwinmm -lshell32
if %errorlevel% neq 0 exit /b 1

echo [2/3] Derleniyor: fppc.exe (Sifirdan x86-64 PE Derleyici)...
gcc fppc.c -o "%INSTALL_DIR%\fppc.exe"
if %errorlevel% neq 0 exit /b 1

echo [3/3] Derleniyor: fppedit.exe (Terminal Editor)...
gcc fppedit.c -o "%INSTALL_DIR%\fppedit.exe"
if %errorlevel% neq 0 exit /b 1

echo.
echo PATH'e ekleniyor: %INSTALL_DIR%
powershell -NoProfile -Command "$p = [Environment]::GetEnvironmentVariable('Path', 'User'); if ($p -notlike '*%INSTALL_DIR%*') { [Environment]::SetEnvironmentVariable('Path', $p + ';%INSTALL_DIR%', 'User') }"

if exist "%USERPROFILE%\vimfiles" (
    if not exist "%USERPROFILE%\vimfiles\syntax" mkdir "%USERPROFILE%\vimfiles\syntax"
    copy /Y "vim\fpp.vim" "%USERPROFILE%\vimfiles\syntax\fpp.vim" >nul 2>nul
    echo [+] Vim F++ syntax dosyasi guncellendi.
)

echo.
echo ========================================
echo Kurulum tamamlandi!
echo Yeni bir terminal acip su sekilde kullanabilirsin:
echo   fpp test.fpp
echo   fppc merhaba.fpp merhaba.exe
echo   fppedit test.fpp
echo ========================================
pause
