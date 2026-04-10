cd build
premake5.exe vs2022 || pause
cd ../
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" build/build_files/prueba.sln /p:Configuration=Debug || pause
prueba.exe
