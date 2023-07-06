@echo off

cd build
del pokemon_sprite.exe

cls

echo Building...

g++ -std=c++17 -O2 ^
src/main.cpp ^
src/utility.cpp ^
-o pokemon_sprite.exe

echo Done
echo.

if exist pokemon_sprite.exe (
	pokemon_sprite.exe
)