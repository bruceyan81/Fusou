cmake_minimum_required(VERSION 3.20)

project(Fusou LANGUAGES CXX)

# Fusou は Windows / MSVC 専用のコンソールゲームとして構成する
if(NOT WIN32)
    message(FATAL_ERROR "Fusou supports Windows only.")
endif()

if(NOT MSVC)
    message(FATAL_ERROR "Fusou supports MSVC only.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# FUSOU_STAGE はゲーム内の開発用機能を切り替えるための設定
set(FUSOU_STAGE "Dev" CACHE STRING "Build stage: Dev or Shipping")
set_property(CACHE FUSOU_STAGE PROPERTY STRINGS Dev Shipping)

add_library(fusou_config INTERFACE)

target_compile_definitions(fusou_config INTERFACE
    NOMINMAX
)

if(FUSOU_STAGE STREQUAL "Shipping")
    target_compile_definitions(fusou_config INTERFACE
        FUSOU_DEV=0
        FUSOU_SHIPPING=1
    )
elseif(FUSOU_STAGE STREQUAL "Dev")
    target_compile_definitions(fusou_config INTERFACE
        FUSOU_DEV=1
        FUSOU_SHIPPING=0
    )
else()
    message(FATAL_ERROR "FUSOU_STAGE must be 'Dev' or 'Shipping' (got: ${FUSOU_STAGE})")
endif()

find_program(FUSOU_POWERSHELL_EXECUTABLE
    NAMES powershell pwsh
    REQUIRED
)

# 生成ヘッダーは src 配下と同じ include 形式で参照できるようにする
# そのため、ビルドディレクトリ内にも src/Core/... の構造を作る
set(FUSOU_GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(FUSOU_GENERATED_TILE_IDS_HEADER
    "${FUSOU_GENERATED_INCLUDE_DIR}/src/Core/World/TileIdsGenerated.h"
)
set(FUSOU_GENERATED_TILE_PALETTE_HEADER
    "${FUSOU_GENERATED_INCLUDE_DIR}/src/Core/Presentation/TilePaletteGenerated.h"
)
set(FUSOU_TILE_PALETTE_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Palette/TilePalette.json"
)
set(FUSOU_TILE_PALETTE_GENERATOR
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/GenerateTilePalette.ps1"
)

add_custom_command(
    OUTPUT
        "${FUSOU_GENERATED_TILE_IDS_HEADER}"
        "${FUSOU_GENERATED_TILE_PALETTE_HEADER}"
    COMMAND "${FUSOU_POWERSHELL_EXECUTABLE}"
            -NoProfile
            -ExecutionPolicy Bypass
            -File "${FUSOU_TILE_PALETTE_GENERATOR}"
            -Target Cpp
            -TileIdsPath "${FUSOU_GENERATED_TILE_IDS_HEADER}"
            -TilePalettePath "${FUSOU_GENERATED_TILE_PALETTE_HEADER}"
    DEPENDS
        "${FUSOU_TILE_PALETTE_GENERATOR}"
        "${FUSOU_TILE_PALETTE_SOURCE}"
    COMMENT "Generating Fusou tile palette headers"
    VERBATIM
)

add_custom_target(FusouGeneratedPalette
    DEPENDS
        "${FUSOU_GENERATED_TILE_IDS_HEADER}"
        "${FUSOU_GENERATED_TILE_PALETTE_HEADER}"
)

set(FUSOU_GAME_SOURCES
    "src/Fusou.cpp"

    "src/App/Assets/MapJsonLoader.cpp"

    "src/App/Composition/AppComposition.cpp"

    "src/App/Runtime/GameApp.cpp"

    "src/Core/Game/CameraFollow.cpp"
    "src/Core/Game/EnemyAi.cpp"
    "src/Core/Game/EnemySim.cpp"
    "src/Core/Game/GameScene.cpp"
    "src/Core/Game/GameTick.cpp"
    "src/Core/Game/GoalWaveEffect.cpp"
    "src/Core/Game/LadderSim.cpp"
    "src/Core/Game/GoalActivation.cpp"
    "src/Core/Game/LightState.cpp"
    "src/Core/Game/LightSystem.cpp"
    "src/Core/Game/PlayerContact.cpp"
    "src/Core/Game/PlayerMoveRules.cpp"

    "src/Core/Loop/FrameClock.cpp"
    "src/Core/Loop/GameLoop.cpp"
    "src/Core/Loop/RuntimeCoordinator.cpp"

    "src/Core/Presentation/Presenter.cpp"

    "src/Core/UI/UI.cpp"

    "src/Core/World/Camera.cpp"
    "src/Core/World/TileMap.cpp"

    "src/Platform/Console/ConsoleSession.cpp"

    "src/Platform/Win32Console/Win32ConsoleApi.cpp"
    "src/Platform/Win32Console/Win32ConsoleInputAdapter.cpp"
    "src/Platform/Win32Console/Win32KeyboardState.cpp"
    "src/Platform/Win32Console/Win32ConsoleRenderAdapter.cpp"

    "src/Platform/Clock/SteadyClockAdapter.cpp"
)

add_library(FusouGame STATIC ${FUSOU_GAME_SOURCES})

target_compile_options(FusouGame PRIVATE
    /permissive-
    /W4
    /EHsc
    /utf-8
)

add_dependencies(FusouGame FusouGeneratedPalette)

target_include_directories(FusouGame
    PUBLIC
        "${FUSOU_GENERATED_INCLUDE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/utfcpp-4.0.9/source"
)

target_link_libraries(FusouGame
    PUBLIC
        fusou_config
)

add_executable(Fusou
    "src/Main.cpp"
    "Resources/Resources.rc"
)

target_compile_options(Fusou PRIVATE
    /permissive-
    /W4
    /EHsc
    /utf-8
)

target_link_libraries(Fusou
    PRIVATE
        FusouGame
)

set(FUSOU_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Assets")

set(FUSOU_INTRO_JSON_SRC "${FUSOU_ASSETS_DIR}/MapAssets/Intro.json")
set(FUSOU_MAP_JSON_SRC "${FUSOU_ASSETS_DIR}/MapAssets/Map.json")
set(FUSOU_MAP_BACKGROUND_JSON_SRC "${FUSOU_ASSETS_DIR}/MapAssets/MapBackground.json")
set(FUSOU_DIED_JSON_SRC "${FUSOU_ASSETS_DIR}/MapAssets/Died.json")
set(FUSOU_FIN_JSON_SRC "${FUSOU_ASSETS_DIR}/MapAssets/Fin.json")
set(FUSOU_PLAYER_JSON_SRC "${FUSOU_ASSETS_DIR}/ActorAssets/Player.json")

add_custom_command(TARGET Fusou POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${FUSOU_INTRO_JSON_SRC}"
            "$<TARGET_FILE_DIR:Fusou>/Intro.json"

    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${FUSOU_MAP_JSON_SRC}"
            "$<TARGET_FILE_DIR:Fusou>/Map.json"

    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${FUSOU_MAP_BACKGROUND_JSON_SRC}"
            "$<TARGET_FILE_DIR:Fusou>/MapBackground.json"

    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${FUSOU_DIED_JSON_SRC}"
            "$<TARGET_FILE_DIR:Fusou>/Died.json"

    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${FUSOU_FIN_JSON_SRC}"
            "$<TARGET_FILE_DIR:Fusou>/Fin.json"

    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${FUSOU_PLAYER_JSON_SRC}"
            "$<TARGET_FILE_DIR:Fusou>/Player.json"
    VERBATIM
)
