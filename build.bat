@echo off
cls
g++ main.cpp -o main.exe -luser32 -lgdi32 -lopengl32 -lgdiplus -lShlwapi -ldwmapi -lstdc++fs -static -static-libgcc -static-libstdc++
main.exe
