//
// Created by Иван Ильин on 14.01.2021.
//

#include <iostream>

#include "Engine.h"
#include "utils/Time.h"
#include "utils/ResourceManager.h"
#include "animation/Timeline.h"
#include "io/SoundController.h"

Engine::Engine() {
    Time::init();
    Timeline::init();
    ResourceManager::init();
    SoundController::init();
}

void Engine::create(int screenWidth, int screenHeight, const std::string &name, bool verticalSync, sf::Color background,
                    sf::Uint32 style) {
    _name = name;
    screen->open(screenWidth, screenHeight, name, verticalSync, background, style);

    Log::log("Engine::create(): started engine (" + std::to_string(screenWidth) + "x" + std::to_string(screenHeight) +
             ") with title '" + name + "'.");

    Time::update();

    start();
    camera->init(screenWidth, screenHeight);

    while (screen->isOpen()) {
        Time::startTimer("d all");

        screen->clear(Consts::BACKGROUND_COLOR);
        Time::update();

        Time::startTimer("d game update");
        update();
        Time::stopTimer("d game update");

        // Физика и анимации только при активном мире
        if (_updateWorld) {
            Time::startTimer("d animations");
            Timeline::update();
            Time::stopTimer("d animations");

            Time::startTimer("d collisions");
            world->update();
            Time::stopTimer("d collisions");
        }

        Time::startTimer("d projections");

        // === 3D РЕНДЕРИНГ (только если мир обновляется) ===
        if (_updateWorld) {
            if (_useOpenGL) {
                GLfloat* view = camera->glInvModel();

                screen->prepareToGlDrawMesh();  // включает GL_DEPTH_TEST и перспективу

                for (auto& it : *world) {
                    if (it.second->isVisible()) {
                        GLfloat* model = it.second->glModel();
                        GLfloat* geometry = it.second->glFloatArray();
                        screen->glDrawMesh(geometry, view, model, 3 * it.second->triangles().size());
                        delete[] model;
                    }
                }

                delete[] view;
            } else {
                // Софт-рендер
                camera->clear();
                for (auto& it : *world) {
                    camera->project(it.second);
                }
                for (auto& t : camera->sorted()) {
                    screen->drawTriangle(*t);
                }
            }
        }
        // Если _updateWorld == false — только фон

        // === 2D GUI НАД ВСЁМ ===
        screen->pushGLStates();           // Сохраняем текущее состояние OpenGL

        glDisable(GL_DEPTH_TEST);          // ← ОБЯЗАТЕЛЬНО: отключаем тест глубины
        glDisable(GL_CULL_FACE);          // на всякий случай

        Time::stopTimer("d projections");

        // Весь 2D-контент здесь
        if (Consts::SHOW_FPS_COUNTER) {
            screen->drawText(std::to_string(Time::fps()) + " fps",
                             Vec2D(static_cast<double>(screen->width()) - 100.0, 10.0), 25,
                             sf::Color(100, 100, 100));
        }

        printDebugInfo();  // один раз

        gui();             // ← прицел в игре ИЛИ меню на паузе

        screen->popGLStates();  // Восстанавливаем состояние (включая глубину для следующего кадра)

        screen->display();

        Time::stopTimer("d all");
    }
}

void Engine::exit() {
    if (screen->isOpen()) {
        screen->close();
    }
    SoundController::free();
    ResourceManager::free();
    Timeline::free();
    Time::free();

}

void Engine::printDebugInfo() const {

    if (_showDebugInfo) {
        // coordinates & fps:

        std::string text = _name + "\n\n X: " +
                           std::to_string((camera->position().x())) + "\n Y: " +
                           std::to_string((camera->position().y())) + "\n Z: " +
                           std::to_string((camera->position().z())) + "\n RY:" +
                            std::to_string(camera->angle().y()/M_PI) + "PI\n RL: " +
                            std::to_string(camera->angleLeftUpLookAt().x()/M_PI) + "PI\n\n" +
                           std::to_string(screen->width()) + "x" +
                           std::to_string(screen->height()) + "\t" +
                           std::to_string(Time::fps()) + " fps";

        if (_useOpenGL) {
            text += "\n Using OpenGL acceleration";
        } else {
            text += "\n" + std::to_string(_triPerSec) + " tris/s";
        }

        sf::Text t;

        t.setFont(*ResourceManager::loadFont(Consts::THIN_FONT));
        t.setString(text);
        t.setCharacterSize(30);
        t.setFillColor(sf::Color::Black);
        t.setPosition(static_cast<float>(screen->width()) - 400.0f, 10.0f);

        screen->drawText(t);

        // timers:
        int timerWidth = screen->width() - 100;
        float xPos = 50;
        float yPos = 300;
        int height = 50;

        double totalTime = Time::elapsedTimerSeconds("d all");
        double timeSum = 0;
        int i = 0;
        for (auto &[timerName, timer] : Time::timers()) {
            int width = timerWidth * timer.elapsedSeconds() / totalTime;

            if (timerName == "d all" || timerName[0] != 'd') {
                continue;
            }

            screen->drawTetragon(Vec2D{xPos, yPos + height * i},
                                 Vec2D{xPos + width, yPos + height * i},
                                 Vec2D{xPos + width, yPos + height + height * i},
                                 Vec2D{xPos, yPos + height + height * i},
                                 {static_cast<sf::Uint8>(255.0 * static_cast<double>(width) / timerWidth),
                                  static_cast<sf::Uint8>(255.0 * (1.0 - static_cast<double>(width) / timerWidth)),
                                  0, 100});

            std::string fps;
            if(timer.elapsedSeconds() > 0) {
                fps = std::to_string((int) (1.0 / timer.elapsedSeconds()));
            } else {
                fps = "inf";
            }

            screen->drawText(
                    timerName.substr(2, timerName.size()) + ":\t" + fps + " / s \t (" +
                    std::to_string((int) (100 * timer.elapsedSeconds() / totalTime)) + "%)",
                    Vec2D{xPos + 10, yPos + height * i + 5}, 30,
                    sf::Color(0, 0, 0, 150));

            i++;
            timeSum += timer.elapsedSeconds();
        }

        int width = timerWidth * (totalTime - timeSum) / totalTime;
        screen->drawTetragon(Vec2D{xPos, yPos + height * i},
                             Vec2D{xPos + width, yPos + height * i},
                             Vec2D{xPos + width, yPos + height + height * i},
                             Vec2D{xPos, yPos + height + height * i},
                             {static_cast<sf::Uint8>(255.0 * static_cast<double>(width) / timerWidth),
                              static_cast<sf::Uint8>(255.0 * (1.0 - static_cast<double>(width) / timerWidth)),
                              0, 100});

        std::string fps;
        if(totalTime - timeSum > 0) {
            fps = std::to_string((int) (1.0 / (totalTime - timeSum)));
        } else {
            fps = "inf";
        }

        screen->drawText("other:\t" + fps + " / s \t (" +
                         std::to_string((int) (100 * (totalTime - timeSum) / totalTime)) + "%)",
                         Vec2D{xPos + 10, yPos + height * i + 5}, 30,
                         sf::Color(0, 0, 0, 150));

    }
}
