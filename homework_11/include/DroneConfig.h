#pragma once

#include <string>

#include "Coords.hpp"

struct AmmoParams {
  double mass{};
  double drag{};
  double lift{};
};

struct DroneConfig {
  Coord startPos{};              // початкова позиція (x, y)
  double altitude{};             // висота
  double initialDir{};           // початковий напрямок (рад)
  double attackSpeed{};          // швидкість атаки (м/с)
  double accelerationPath{};     // шлях розгону (м)
  double arrayTimeStep{};        // крок часу масиву цілей
  double simTimeStep{};          // крок симуляції
  double hitRadius{};            // радіус влучення
  double angularSpeed{};         // кутова швидкість (рад/с)
  double turnThreshold{};        // поріг повороту (рад)
  double targetTimeStep{0.05};   // крок оновлення фізики дрона
  double physicsTimeStep{0.01};  //
  double timeScale{10.0};  // прискорення часу: потік інтегрує крок dt, а спить dt / timeScale реального часу

  std::string ammoName{};  // обрані боєприпаси
  AmmoParams ammoParams{};
};