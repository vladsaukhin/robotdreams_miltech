#include "config/JsonConfigLoader.h"

#include <format>
#include <fstream>

#include <nlohmann/json.hpp>

bool JsonConfigLoader::readConfig(std::string_view dataFolderPath)
{
  const auto confPath = dataFolderPath.data() + std::string("/config.json");
  std::ifstream input(confPath);
  if (!input) {
    std::cerr << std::format("Can't open {}", confPath) << std::endl;
    return false;
  }

  try {
    nlohmann::json j = nlohmann::json::parse(input);

    if (!j.is_object()) {
      throw std::runtime_error("Config root must be an object");
    }

    const auto& drone = j.at("drone");

    const auto& pos = drone.at("position");
    m_config.startPos.x = pos.at("x").get<double>();
    m_config.startPos.y = pos.at("y").get<double>();

    m_config.altitude = drone.at("altitude").get<double>();
    m_config.initialDir = drone.at("initialDirection").get<double>();
    m_config.attackSpeed = drone.at("attackSpeed").get<double>();
    m_config.accelerationPath = drone.at("accelerationPath").get<double>();
    m_config.angularSpeed = drone.at("angularSpeed").get<double>();
    m_config.turnThreshold = drone.at("turnThreshold").get<double>();

    m_config.ammoName = j.at("ammo").get<std::string>().c_str();

    const auto& sim = j.at("simulation");
    m_config.simTimeStep = sim.at("timeStep").get<double>();
    m_config.hitRadius = sim.at("hitRadius").get<double>();
    if (sim.contains("targetTimeStep")) {
      m_config.targetTimeStep = sim.at("targetTimeStep").get<double>();
    }
    if (sim.contains("physicsTimeStep")) {
      m_config.physicsTimeStep = sim.at("physicsTimeStep").get<double>();
    }
    if (sim.contains("timeScale")) {
      m_config.timeScale = sim.at("timeScale").get<double>();
    }

    m_config.arrayTimeStep = j.at("targetArrayTimeStep").get<double>();
  }
  catch (const std::exception& e) {
    std::cerr << "Config error: " << e.what() << '\n';
    return false;
  }

  return true;
}

bool JsonConfigLoader::readAmmo(std::string_view dataFolderPath)
{
  const auto ammoPath = dataFolderPath.data() + std::string("/ammo.json");
  std::ifstream input(ammoPath);
  if (!input) {
    std::cerr << std::format("Can't open {}", ammoPath) << std::endl;
    return false;
  }

  try {
    nlohmann::json j = nlohmann::json::parse(input);

    if (!j.is_array()) {
      throw std::runtime_error("Ammo root must be an array");
    }

    const size_t ammoCount = j.size();
    for (size_t i = 0; i < ammoCount; ++i) {
      if (m_config.ammoName == j[i].at("name").get<std::string>()) {
        m_config.ammoParams.mass = j[i].at("mass").get<double>();
        m_config.ammoParams.drag = j[i].at("drag").get<double>();
        m_config.ammoParams.lift = j[i].at("lift").get<double>();
        return true;
      }
    }
  }
  catch (const std::exception& e) {
    std::cerr << "Ammo error: " << e.what() << '\n';
    return false;
  }

  return false;
}

bool JsonConfigLoader::validateConfig()
{
  bool isValidResult = true;
  if (m_config.altitude < 0.0f || m_config.startPos.x < 0.0f || m_config.startPos.y < 0.0f) {
    std::cerr << "Drons coordinates must be non-negative" << std::endl;
    isValidResult &= false;
  }
  if (m_config.attackSpeed <= 0.0f) {
    std::cerr << "attackSpeed must be greater than 0." << std::endl;
    isValidResult &= false;
  }
  if (m_config.accelerationPath < 0.0f) {
    std::cerr << "AccelerationPath must be non-negative." << std::endl;
    isValidResult &= false;
  }
  if (m_config.arrayTimeStep <= 0.0f) {
    std::cerr << "ArrayTimeStep must be greater than 0." << std::endl;
    isValidResult &= false;
  }
  if (m_config.simTimeStep <= 0.0f) {
    std::cerr << "SimTimeStep must be greater than 0." << std::endl;
    isValidResult &= false;
  }
  if (m_config.simTimeStep > m_config.arrayTimeStep) {
    std::cerr << "SimTimeStep must be less than or equal to ArrayTimeStep." << std::endl;
    isValidResult &= false;
  }
  if (m_config.hitRadius < 0.0f) {
    std::cerr << "HitRadius must be non-negative.";
    isValidResult &= false;
  }
  if (m_config.angularSpeed <= 0.0f) {
    std::cerr << "AngularSpeed must be greater than 0.";
    isValidResult &= false;
  }
  if (m_config.turnThreshold < 0.0f) {
    std::cerr << "TurnThreshold must be non-negative.";
    isValidResult &= false;
  }
  if (m_config.targetTimeStep < 0.0f) {
    std::cerr << "TargetTimeStep must be non-negative.";
    isValidResult &= false;
  }
  if (m_config.physicsTimeStep < 0.0f) {
    std::cerr << "PhysicsTimeStep must be non-negative.";
    isValidResult &= false;
  }
  if (m_config.timeScale < 0.0f) {
    std::cerr << "TimeScale must be non-negative.";
    isValidResult &= false;
  }
  if (m_config.physicsTimeStep >= m_config.simTimeStep) {
    std::cerr << "PhysicsTimeStep must be less than SimTimeStep.";
    isValidResult &= false;
  }
  return isValidResult;
}