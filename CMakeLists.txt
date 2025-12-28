cmake_minimum_required(VERSION 3.17)
project(shooter)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# --- 1. Списки исходных файлов ---

# Общие файлы (Логика, Сеть, Физика, Математика)
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

# Клиентские файлы (Рендеринг, Управление, Камера)
set(CLIENT_SOURCES
        Source.cpp
        Shooter.cpp
        Shooter.h
        network/ShooterClient.cpp
        network/ShooterClient.h
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
)

# --- 2. Поиск SFML ---

if(NOT APPLE AND NOT UNIX)
    set(SFML_DIR "C:/Libraries/SFML/lib/cmake/SFML")
    set(SFML_STATIC_LIBRARIES TRUE)
endif()

find_package(SFML 2.5.1 COMPONENTS graphics audio network window system REQUIRED)

# --- 3. Создание цели: ИГРА (shooter) ---

add_executable(shooter ${COMMON_SOURCES} ${CLIENT_SOURCES})

target_link_libraries(shooter
        sfml-graphics
        sfml-audio
        sfml-network
        sfml-window
        sfml-system
)

# OpenGL для игры
if (APPLE)
    target_link_libraries(shooter "-framework OpenGL" "-framework GLUT")
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    find_package(GLUT REQUIRED)
    target_link_libraries(shooter ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
endif()

# --- 4. Создание цели: СЕРВЕР (shooter_server) ---

# Проверяем, существует ли файл ServerMain.cpp (создайте его, если еще нет)
add_executable(shooter_server ServerMain.cpp ${COMMON_SOURCES})

# Серверу всё же нужен sfml-graphics из-за использования sf::Color в COMMON_SOURCES
target_link_libraries(shooter_server
        sfml-network
        sfml-graphics
        sfml-audio
        sfml-system
)

# Макрос для отделения серверной логики в коде
target_compile_definitions(shooter_server PRIVATE HEADLESS_SERVER)