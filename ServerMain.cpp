#include "network/ShooterServer.h"
#include "engine/utils/Time.h"
#include "engine/utils/Log.h"
#include "ShooterConsts.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    Log::log("Server starting...");

    // Инициализация системного времени
    Time::init();

    // Создаем "мир" для просчета серверной логики (позиции, бонусы)
    auto world = std::make_shared<World>();

    // ВАЖНО: Если сервер должен проверять попадания через rayCast,
    // нужно загрузить коллизии карты (без текстур)
    // world->loadMap(ShooterConsts::MAP_OBJ, Vec3D{5, 5, 5});

    auto server = std::make_shared<ShooterServer>();

    // Читаем порт из файла или используем стандартный
    sf::Uint16 port = 54000;
    server->start(port);

    if (server->isWorking()) {
        server->generateBonuses();
        std::cout << "Shooter Server is running on port " << port << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;
    } else {
        std::cerr << "Failed to start server!" << std::endl;
        return 1;
    }

    // Главный цикл сервера
    while (server->isWorking()) {
        Time::update();

        // Обновление сетевых пакетов и логики бонусов
        server->update();

        // Обновление состояний (респаун бонусов и т.д.)
        server->updateInfo();

        // Ограничиваем нагрузку на CPU (примерно 100 тиков в секунду)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}