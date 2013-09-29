set d="%CD%\build.debug"
if not exist %d% (
   mkdir %d%
)

cd %d%
cmake -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 10" ..
cmake --build . --target install --config Debug
cd %d%\..\

