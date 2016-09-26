;; Setup environment
set PATH=%QT%\bin\;C:\Qt\Tools\QtCreator\bin\;C:\Qt\QtIFW2.0.1\bin\;%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %PLATFORM%

;; Build Kryvos
cd src
qmake -spec win32-msvc2015 CONFIG+=x86_64 CONFIG-=debug CONFIG+=release
nmake

;; Package Kryvos
cd ..
mkdir installer\windows\x64\packages\com.kryvosproject.kryvos\data\
cd build\win\msvc\x86_64\release\
windeployqt Kryvos\Kryvos.exe
rd /s /q Kryvos\moc
rd /s /q Kryvos\obj
rd /s /q Kryvos\qrc
copy "..\..\..\..\..\Release Notes" "Kryvos\Release Notes.txt"
copy "..\..\..\..\..\README.md" "Kryvos\README.md"
copy "..\..\..\..\..\LICENSE" "Kryvos\LICENSE.txt"
copy "..\..\..\..\..\Botan License" "Kryvos\Botan License.txt"
copy "..\..\..\..\..\Qt License" "Kryvos\Qt License.txt"
mkdir Kryvos\themes
copy "..\..\..\..\..\src\resources\stylesheets\kryvos.qss" "Kryvos\themes\kryvos.qss"
robocopy Kryvos\ ..\..\..\..\..\installer\windows\x64\packages\com.kryvosproject.kryvos\data\ /E
7z a kryvos_%APPVEYOR_REPO_TAG_NAME%_windows_x86_64_portable.zip Kryvos
cd ..\..\..\..\..\installer\windows\x64\
binarycreator.exe --offline-only -c config\config.xml -p packages kryvos_%APPVEYOR_REPO_TAG_NAME%_windows_x86_64_installer.exe
