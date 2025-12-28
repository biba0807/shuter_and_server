cmake_minimum_required(VERSION 3.17)
project(shooter)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# --- 1. СПИСКИ ФАЙЛОВ ---

# Общие файлы для сервера и клиента
set(COMMON_SOURCES
        network/ShooterServer.cpp
        network/ShooterServer.h
        network/ShooterMsgType.h
        network/ShooterMsgType.cpp
        engine/network/ServerUDP.cpp
        engine/network/ServerUDP.h
        engine/network/UDPConnection.cpp
        engine/network/UDPConnection.h
        engine/network/UDPSocket.cpp
        engine/network/UDPSocket.h
        engine/network/ReliableMsg.cpp
        engine/network/ReliableMsg.h
        engine/network/MsgType.cpp
        engine/network/MsgType.h
        ShooterConsts.h
        player/Player.cpp
        player/Player.h
        weapon/Weapon.cpp
        weapon/Weapon.h
        engine/World.cpp
        engine/World.h
        engine/Object.cpp
        engine/Object.h
        engine/physics/RigidBody.cpp
        engine/physics/RigidBody.h
        engine/physics/HitBox.cpp
        engine/physics/HitBox.h
        engine/math/Vec4D.cpp
        engine/math/Vec4D.h
        engine/math/Vec3D.cpp
        engine/math/Vec3D.h
        engine/math/Vec2D.cpp
        engine/math/Vec2D.h
        engine/math/Matrix4x4.cpp
        engine/math/Matrix4x4.h
        engine/math/Plane.cpp
        engine/math/Plane.h
        engine/Triangle.cpp
        engine/Triangle.h
        engine/Mesh.cpp
        engine/Mesh.h
        engine/utils/Log.cpp
        engine/utils/Log.h
        engine/utils/Time.cpp
        engine/utils/Time.h
        engine/utils/Timer.cpp
        engine/utils/Timer.h
        engine/utils/ResourceManager.cpp
        engine/utils/ResourceManager.h
        engine/io/SoundController.cpp
        engine/io/SoundController.h
        engine/animation/Timeline.cpp
        engine/animation/Timeline.h
        engine/animation/Animation.cpp
        engine/animation/Animation.h
        engine/animation/Interpolation.cpp
        engine/animation/Interpolation.h
)

# Клиентские файлы
set(CLIENT_SOURCES
        Source.cpp
        Shooter.cpp
        Shooter.h
        network/ShooterClient.cpp
        network/ShooterClient.h
        engine/network/ClientUDP.cpp
        engine/network/ClientUDP.h
        player/PlayerController.cpp
        player/PlayerController.h
        engine/Engine.cpp
        engine/Engine.h
        engine/Camera.cpp
        engine/Camera.h
        engine/io/Screen.cpp
        engine/io/Screen.h
        engine/io/Keyboard.cpp
        engine/io/Keyboard.h
        engine/io/Mouse.cpp
        engine/io/Mouse.h
        engine/gui/Window.cpp
        engine/gui/Window.h
        engine/gui/Button.cpp
        engine/gui/Button.h
)

# --- 2. ПОИСК SFML ---

if(NOT APPLE AND NOT UNIX)
    set(SFML_DIR "C:/Libraries/SFML/lib/cmake/SFML")
    set(SFML_STATIC_LIBRARIES TRUE)
endif()

find_package(SFML 2.5.1 COMPONENTS graphics audio network window system REQUIRED)

# --- 3. СБОРКА ИГРЫ (shooter) ---

add_executable(shooter ${COMMON_SOURCES} ${CLIENT_SOURCES})

target_link_libraries(shooter
        sfml-graphics
        sfml-audio
        sfml-network
        sfml-window
        sfml-system
)

if (APPLE)
    target_link_libraries(shooter "-framework OpenGL")
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    target_link_libraries(shooter ${OPENGL_LIBRARIES})
else()
    # Для MinGW на Windows оставляем только OpenGL. GLUT удален.
    target_link_libraries(shooter opengl32)
endif()

# --- 4. СБОРКА СЕРВЕРА (shooter_server) ---

add_executable(shooter_server ServerMain.cpp ${COMMON_SOURCES})

target_link_libraries(shooter_server
        sfml-network
        sfml-graphics
        sfml-audio
        sfml-system
)

target_compile_definitions(shooter_server PRIVATE HEADLESS_SERVER)

# --- 5. НАСТРОЙКА ПУТЕЙ (MinGW / Linux) ---

if(WIN32)
    # Эти флаги заставляют MinGW включить стандартные библиотеки внутрь EXE
    target_link_options(shooter PRIVATE -static -static-libgcc -static-libstdc++)
    target_link_options(shooter_server PRIVATE -static -static-libgcc -static-libstdc++)
endif()

if(UNIX)
    # Настройка для Ubuntu, чтобы сервер искал библиотеки в папке lib
    set_target_properties(shooter shooter_server PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
endif()