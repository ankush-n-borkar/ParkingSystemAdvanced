@echo off
echo   Compiling Parking System - SERVER

if not exist data mkdir data
if not exist build mkdir build

gcc -Wall -Wextra -std=c99 -g src/main.c src/utils/utils.c src/utils/queue.c src/core/graph.c src/core/floor.c src/core/parking_system.c src/algorithms/pathfinding.c src/algorithms/recommendation.c src/io/file_manager.c src/analytics/analytics.c src/io/json_export.c src/server/http_server.c -o parking_system.exe -lws2_32

if %errorlevel% equ 0 (
    echo.
    echo   Compilation Successful!
    echo.
    parking_system.exe
) else (
    echo.
    echo   Compilation Failed!
)

pause
