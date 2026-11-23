@echo off
chcp 65001 >nul
echo ========================================
echo        F++ Kurulum (fpp setup)
echo ========================================
echo.

set "INSTALL_DIR=%LOCALAPPDATA%\fpp"
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

echo [1/3] Kopyalaniyor: fpp.exe (Yorumlayici)...
copy /Y "bin\fpp.exe" "%INSTALL_DIR%\fpp.exe" >nul
if %errorlevel% neq 0 (
    echo [HATA] bin\fpp.exe bulunamadi!
    pause
    exit /b 1
)

echo [2/3] Kopyalaniyor: fppc.exe (Sifirdan x86-64 PE Derleyici)...
copy /Y "bin\fppc.exe" "%INSTALL_DIR%\fppc.exe" >nul

echo [3/3] Kopyalaniyor: fppedit.exe (Terminal Editor)...
copy /Y "bin\fppedit.exe" "%INSTALL_DIR%\fppedit.exe" >nul

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
echo Kurulum tamamlandi! (Hicbir C derleyicisi gerekmedi)
echo Yeni bir terminal acip su sekilde kullanabilirsin:
echo   fpp test.fpp
echo   fppc merhaba.fpp merhaba.exe
echo   fppedit test.fpp
echo ========================================
pause
