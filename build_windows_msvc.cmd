echo on

SET project_dir="%cd%"

echo "Setting up environment..."
set PATH=%QTPATH%\bin\;C:\Qt\QtIFW-3.0.1\bin\;%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" %PLATFORM%

echo "Building Kryvo..."
qmake -spec win32-msvc CONFIG+=x86_64 CONFIG-=debug CONFIG+=release
nmake

echo "Copying Qt dependencies to test app..."
cd "%project_dir%\build\windows\msvc\x86_64\release\test\"
windeployqt tests.exe

echo "Copying plugins for test app..."
mkdir "%project_dir\build\windows\msvc\x86_64\release\test\plugins\cryptography\botan\"
cd "%project_dir%\build\windows\msvc\x86_64\release\test\plugins\cryptography\botan\"

echo "Copying test data archive..."
cd "%project_dir%\build\windows\msvc\x86_64\release\test\"
copy "%project_dir%\src\tests\data\test-data.zip" test-data.zip

echo "Extracting test data..."
7z.exe e test-data.zip

echo "Running tests..."
tests.exe

echo "Copy plugins to app..."
mkdir "%project_dir%\build\windows\msvc\x86_64\release\widgets\plugins\cryptography\botan\"
cd "%project_dir%\build\windows\msvc\x86_64\release\widgets\plugins\cryptography\botan\"

echo "Packaging..."

echo "Copying app to packaging directory..."
robocopy "%project_dir%\build\windows\msvc\x86_64\release\widgets\" "%project_dir%\build\windows\msvc\x86_64\release\Kryvo\" /E

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
mkdir "%project_dir%\Kryvo\themes\"
copy "%project_dir%\resources\stylesheets\kryvo.qss" "Kryvo\themes\kryvo.qss"

echo "Copying files for installer..."
mkdir "%project_dir%\installer\windows\x86_64\packages\app.kryvo\data\"
robocopy Kryvo\ "%project_dir%\installer\windows\x86_64\packages\app.kryvo\data\" /E

echo "Packaging portable archive..."
7z.exe a kryvo_%TAG_NAME%_windows_x86_64_portable.zip Kryvo

echo "Creating installer..."
cd "%project_dir%\installer\windows\x86_64\"
binarycreator.exe --offline-only -c config\config.xml -p packages kryvo_%TAG_NAME%_windows_x86_64_installer.exe
