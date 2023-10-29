@echo off

echo "Setting up Moth UI prerequesites for VS build..."

pushd ..\..\external

rmdir fmt\build /s /q
mkdir fmt\build
cd fmt\build

cmake .. && cmake --build . --config Release && cmake --install . --config Release --prefix=installed
if %errorlevel% neq 0 exit /b %errorlevel%
cd ..\..

rmdir freetype\build /s /q
mkdir freetype\build
cd freetype\build
cmake .. && cmake --build . --config Release && cmake --install . --config Release --prefix=installed
if %errorlevel% neq 0 exit /b %errorlevel%
cd ..\..

rmdir glfw\build /s /q
mkdir glfw\build
cd glfw\build
cmake .. && cmake --build . --config Release && cmake --install . --config Release --prefix=installed
if %errorlevel% neq 0 exit /b %errorlevel%
cd ..\..

rmdir harfbuzz-icu-freetype\build /s /q
mkdir harfbuzz-icu-freetype\build
cd harfbuzz-icu-freetype\build
cmake .. && cmake --build . --config Release && cmake --install . --config Release --prefix=installed
if %errorlevel% neq 0 exit /b %errorlevel%
cd ..\..

rmdir spdlog\build /s /q
mkdir spdlog\build
cd spdlog\build
cmake .. && cmake --build . --config Release && cmake --install . --config Release --prefix=installed
if %errorlevel% neq 0 exit /b %errorlevel%
cd ..\..

popd

if not exist "moth_ui.props" (
	echo f | xcopy moth_ui.props.example moth_ui.props /y
)
