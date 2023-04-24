echo on

echo "Setting up environment..."

SET PROJECT_DIR="%SOURCE_DIR%"
SET QT_PATH="%Qt6_DIR%"

SET PATH=%QT_PATH%\bin\;%IQTA_TOOLS%\QtInstallerFramework\4.5\bin;%PATH%
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" %ARCH%

cd %PROJECT_DIR%

echo "Building Kryvo..."
qmake -makefile -spec win32-msvc CONFIG+=x86_64 CONFIG-=debug CONFIG+=release
nmake

REM echo "Copying Qt dependencies to test app..."
REM cd "%PROJECT_DIR%\build\windows\msvc\x86_64\release\test\"
REM windeployqt tests.exe

REM echo "Copying plugins for test app..."
REM mkdir "%PROJECT_DIR\build\windows\msvc\x86_64\release\test\plugins\cryptography\botan\"
REM cd "%PROJECT_DIR%\build\windows\msvc\x86_64\release\test\plugins\cryptography\botan\"

REM echo "Copying test data archive..."
REM cd "%PROJECT_DIR%\build\windows\msvc\x86_64\release\test\"
REM copy "%PROJECT_DIR%\src\tests\data\test-data.zip" test-data.zip

REM echo "Extracting test data..."
REM 7z.exe e -aoa test-data.zip

REM echo "Running tests..."
REM tests.exe

REM echo "Copy plugins to app..."
REM mkdir "%PROJECT_DIR%\build\windows\msvc\x86_64\release\widgets\plugins\cryptography\botan\"
REM cd "%PROJECT_DIR%\build\windows\msvc\x86_64\release\widgets\plugins\cryptography\botan\"

echo "Packaging..."

echo "Copying app to packaging directory..."
robocopy "%PROJECT_DIR%\build\windows\msvc\x86_64\release\widgets" "%PROJECT_DIR%\build\windows\msvc\x86_64\release\Kryvo" /E

cd "%PROJECT_DIR%\build\windows\msvc\x86_64\release\"
windeployqt Kryvo\Kryvo.exe

rd /s /q Kryvo\moc\
rd /s /q Kryvo\obj\
rd /s /q Kryvo\qrc\

echo "Copying files for archival..."
copy "%PROJECT_DIR%\Release Notes" "Kryvo\Release Notes.txt"
copy "%PROJECT_DIR%\README.md" "Kryvo\README.md"
copy "%PROJECT_DIR%\LICENSE" "Kryvo\LICENSE.txt"
copy "%PROJECT_DIR%\Botan License" "Kryvo\Botan License.txt"
copy "%PROJECT_DIR%\Qt License" "Kryvo\Qt License.txt"
mkdir "%PROJECT_DIR%\build\windows\msvc\x86_64\release\Kryvo\themes"
copy "%PROJECT_DIR%\resources\stylesheets\kryvo.qss" "Kryvo\themes\kryvo.qss"

copy "%IQTA_TOOLS%\OpenSSLv3\Win_x64\bin\libcrypto-3-x64.dll" "Kryvo\libcrypto-3-x64.dll"
copy "%IQTA_TOOLS%\OpenSSLv3\Win_x64\bin\libssl-3-x64.dll" "Kryvo\libssl-3-x64.dll"

echo "Copying files for installer..."
mkdir "%PROJECT_DIR%\installer\windows\x86_64\packages\app.kryvo\data\"
robocopy Kryvo\ "%PROJECT_DIR%\installer\windows\x86_64\packages\app.kryvo\data" /E

echo "Packaging portable archive..."
7z a -aoa kryvo_%KRYVO_VERSION%_windows_x86_64_portable.zip Kryvo

echo "Creating installer..."
cd "%PROJECT_DIR%\installer\windows\x86_64\"
binarycreator --offline-only -c config\config.xml -p packages kryvo_%KRYVO_VERSION%_windows_x86_64_installer.exe
