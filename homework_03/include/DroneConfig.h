#pragma once

#include <string>

#include "Coords.hpp"

struct AmmoParams {
  double mass{};
  double drag{};
  double lift{};
};

struct DroneConfig {
  Coord startPos{};           // початкова позиція (x, y)
  double altitude{};          // висота
  double initialDir{};        // початковий напрямок (рад)
  double attackSpeed{};       // швидкість атаки (м/с)
  double accelerationPath{};  // шлях розгону (м)
  double arrayTimeStep{};     // крок часу масиву цілей
  double simTimeStep{};       // крок симуляції
  double hitRadius{};         // радіус влучення
  double angularSpeed{};      // кутова швидкість (рад/с)
  double turnThreshold{};     // поріг повороту (рад)

  std::string ammoName{};  // обрані боєприпаси
  AmmoParams ammoParams{};
};