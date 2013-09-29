@echo off

set d=%CD%

if not exist "%d%\build.release" (
   mkdir %d%\build.release
)

cd %d%\build.release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target install --config Release

cd %d%