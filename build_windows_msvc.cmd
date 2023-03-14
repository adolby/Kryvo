echo on

echo "Setting up environment..."

SET project_dir="%SOURCE_DIR%"
SET qt_path="%Qt6_DIR%"
SET qt_tools=%IQTA_TOOLS%
SET tag_name="%TAG_NAME%"
SET PATH=%qt_path%\bin\;%qt_tools%\QtInstallerFramework\4.5\bin;%PATH%
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" %ARCH%

cd %project_dir%

echo "Building Kryvo..."
qmake -makefile -spec win32-msvc CONFIG+=x86_64 CONFIG-=debug CONFIG+=release
nmake

REM echo "Copying Qt dependencies to test app..."
REM cd "%project_dir%\build\windows\msvc\x86_64\release\test\"
REM windeployqt tests.exe

REM echo "Copying plugins for test app..."
REM mkdir "%project_dir\build\windows\msvc\x86_64\release\test\plugins\cryptography\botan\"
REM cd "%project_dir%\build\windows\msvc\x86_64\release\test\plugins\cryptography\botan\"

REM echo "Copying test data archive..."
REM cd "%project_dir%\build\windows\msvc\x86_64\release\test\"
REM copy "%project_dir%\src\tests\data\test-data.zip" test-data.zip

REM echo "Extracting test data..."
REM 7z.exe e -aoa test-data.zip

REM echo "Running tests..."
REM tests.exe

REM echo "Copy plugins to app..."
REM mkdir "%project_dir%\build\windows\msvc\x86_64\release\widgets\plugins\cryptography\botan\"
REM cd "%project_dir%\build\windows\msvc\x86_64\release\widgets\plugins\cryptography\botan\"

echo "Packaging..."

echo "Copying app to packaging directory..."
robocopy "%project_dir%\build\windows\msvc\x86_64\release\widgets" "%project_dir%\build\windows\msvc\x86_64\release\Kryvo" /E

cd "%project_dir%\build\windows\msvc\x86_64\release\"
windeployqt Kryvo\Kryvo.exe

rd /s /q Kryvo\moc\
rd /s /q Kryvo\obj\
rd /s /q Kryvo\qrc\

echo "Copying files for archival..."
copy "%project_dir%\Release Notes" "Kryvo\Release Notes.txt"
copy "%project_dir%\README.md" "Kryvo\README.md"
copy "%project_dir%\LICENSE" "Kryvo\LICENSE.txt"
copy "%project_dir%\Botan License" "Kryvo\Botan License.txt"
copy "%project_dir%\Qt License" "Kryvo\Qt License.txt"
mkdir "%project_dir%\build\windows\msvc\x86_64\release\Kryvo\themes"
copy "%project_dir%\resources\stylesheets\kryvo.qss" "Kryvo\themes\kryvo.qss"

echo "Copying files for installer..."
mkdir "%project_dir%\installer\windows\x86_64\packages\app.kryvo\data\"
robocopy Kryvo\ "%project_dir%\installer\windows\x86_64\packages\app.kryvo\data" /E

echo "Packaging portable archive..."
7z a -aoa kryvo_%tag_name%_windows_x86_64_portable.zip Kryvo

echo "Creating installer..."
cd "%project_dir%\installer\windows\x86_64\"
binarycreator --offline-only -c config\config.xml -p packages "%project_dir%\installer\windows\x86_64\kryvo_%tag_name%_windows_x86_64_installer.exe"
