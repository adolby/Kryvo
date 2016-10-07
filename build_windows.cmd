echo on

SET project_dir="%cd%"

echo Set up environment...
set PATH=%QT%\bin\;C:\Qt\Tools\QtCreator\bin\;C:\Qt\QtIFW2.0.1\bin\;%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %PLATFORM%

echo Building Kryvos...
qmake -spec win32-msvc2015 CONFIG+=x86_64 CONFIG-=debug CONFIG+=release
nmake

echo Building tests...
cd %project_dir%\tests\
qmake -spec win32-msvc2015 CONFIG+=x86_64 CONFIG-=debug CONFIG+=release
nmake

echo Copying test data...
mkdir %project_dir%\build\win\msvc\x86_64\release\test\
cd %project_dir%\build\win\msvc\x86_64\release\test\
dir
cp %project_dir%\tests\data\test-data.zip test-data.zip
7z x test-data.zip

echo Running tests...
CryptoTests

echo Packaging...
cd %project_dir%\build\windows\msvc\x86_64\release\
windeployqt Kryvos\Kryvos.exe

rd /s /q Kryvos\moc\
rd /s /q Kryvos\obj\
rd /s /q Kryvos\qrc\

echo Copying files for archival...
copy "%project_dir%\Release Notes" "Kryvos\Release Notes.txt"
copy "%project_dir%\README.md" "Kryvos\README.md"
copy "%project_dir%\LICENSE" "Kryvos\LICENSE.txt"
copy "%project_dir%\Botan License" "Kryvos\Botan License.txt"
copy ".%project_dir%\Qt License" "Kryvos\Qt License.txt"
mkdir %project_dir%\Kryvos\themes\
copy "%project_dir%\resources\stylesheets\kryvos.qss" "Kryvos\themes\kryvos.qss"

echo Copying files for installer...
mkdir %project_dir%\installer\windows\x86_64\packages\com.kryvosproject.kryvos\data\
robocopy Kryvos\ %project_dir%\installer\windows\x86_64\packages\com.kryvosproject.kryvos\data\ /E

echo Packaging portable archive...
7z a kryvos_%TAG_NAME%_windows_x86_64_portable.zip Kryvos

echo Creating installer...
cd %project_dir%\installer\windows\x86_64\
binarycreator.exe --offline-only -c config\config.xml -p packages kryvos_%TAG_NAME%_windows_x86_64_installer.exe
