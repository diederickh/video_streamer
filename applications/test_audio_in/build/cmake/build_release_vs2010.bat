set d="%CD%\build.release"
if not exist %d% (
   mkdir %d%
)

cd %d%
cmake -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 10" ..
cmake --build . --target install --config Release
cd %d%\..\

