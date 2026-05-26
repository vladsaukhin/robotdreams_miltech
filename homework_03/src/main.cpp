#include <cmath>
#include <cstddef>
#include <string>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <format>
#include <vector>

#include <nlohmann/json.hpp>

namespace Utils {

struct Coord {
  double x{};
  double y{};

  constexpr Coord& operator+=(const Coord& other) noexcept
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  constexpr Coord& operator-=(const Coord& other) noexcept
  {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  constexpr Coord& operator*=(double scalar) noexcept
  {
    x *= scalar;
    y *= scalar;
    return *this;
  }

  constexpr Coord& operator/=(double scalar) noexcept
  {
    x /= scalar;
    y /= scalar;
    return *this;
  }

  constexpr auto operator<=>(const Coord&) const noexcept = default;

  // Length / normalization
  double Length() const noexcept { return std::sqrt(x * x + y * y); }

  Coord Normalized() const noexcept
  {
    double len = Length();
    if (len == 0.0f)
      return {0.0f, 0.0f};
    return {x / len, y / len};
  }
};

using ListOfCoords = std::vector<Coord>;

inline std::ostream& operator<<(std::ostream& os, const Coord& c)
{
  os << "(" << c.x << ", " << c.y << ")";
  return os;
}

constexpr Coord operator+(Coord l, const Coord& r) noexcept
{
  l += r;
  return l;
}

constexpr Coord operator-(Coord l, const Coord& r) noexcept
{
  l -= r;
  return l;
}

constexpr Coord operator*(Coord l, double scalar) noexcept
{
  l *= scalar;
  return l;
}

constexpr Coord operator*(double scalar, Coord r) noexcept
{
  r *= scalar;
  return r;
}

constexpr Coord operator/(Coord l, double scalar) noexcept
{
  l /= scalar;
  return l;
}

double NormalizeAngle360(double angle)  // [0, 2π)
{
  double a = std::fmod(angle, 2.0f * M_PI);
  if (a < 0.0f) {
    a += 2.0f * M_PI;
  }
  return a;
}

double NormalizeAngle180(double angle)  // (-π, π]
{
  constexpr double TWO_PI = 2.0f * M_PI;
  while (angle > M_PI) {
    angle -= TWO_PI;
  }
  while (angle < -M_PI) {
    angle += TWO_PI;
  }
  return angle;
}

double AngleDiff(double from, double to)
{
  return NormalizeAngle180(to - from);
}

double GetDistance(const Coord& a, const Coord& b)
{
  return (b - a).Length();
}

}  // namespace Utils

namespace Params {

struct AmmoParams {
  double mass{};
  double drag{};
  double lift{};
};

struct DroneConfig {
  Utils::Coord startPos{};    // початкова позиція (x, y)
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

class IConfigLoader {
public:
  virtual ~IConfigLoader() = default;

public:
  virtual bool Load(std::string_view dataFolderPath) = 0;

  virtual const DroneConfig& GetConfig() const = 0;
  virtual const AmmoParams& GetAmmoParams() const = 0;
};

class JsonConfigLoader : public IConfigLoader {
public:
  JsonConfigLoader() = default;

private:
  JsonConfigLoader(const JsonConfigLoader&) = delete;
  JsonConfigLoader& operator=(const JsonConfigLoader&) = delete;

  JsonConfigLoader(JsonConfigLoader&&) = delete;
  JsonConfigLoader& operator=(JsonConfigLoader&&) = delete;

public:
  bool Load(std::string_view dataFolderPath) override { return readConfig(dataFolderPath) && readAmmo(dataFolderPath) && validateConfig(); }
  const DroneConfig& GetConfig() const override { return m_config; };
  const AmmoParams& GetAmmoParams() const override { return m_config.ammoParams; };

private:
  bool readConfig(std::string_view dataFolderPath)
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

      m_config.arrayTimeStep = j.at("targetArrayTimeStep").get<double>();
    }
    catch (const std::exception& e) {
      std::cerr << "Config error: " << e.what() << '\n';
      return false;
    }

    return true;
  }

  bool readAmmo(std::string_view dataFolderPath)
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

  bool validateConfig()
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
    return isValidResult;
  }

private:
  DroneConfig m_config{};
};

enum class ConfigLoaderType { JSON_FILE };

IConfigLoader* CreateLoader(ConfigLoaderType type)
{
  switch (type) {
    case ConfigLoaderType::JSON_FILE:
      return new JsonConfigLoader();
    default:
      throw std::out_of_range(std::format("CreateLoader factory cannot create a Loader for type {}",
                                          static_cast<std::underlying_type_t<ConfigLoaderType>>(type)));
  }
}

}  // namespace Params

namespace TargetsParams {

using TargetsInTime = std::vector<Utils::ListOfCoords>;

class ITargetLoader {
public:
  virtual ~ITargetLoader() = default;

public:
  virtual bool Load(std::string_view dataFolderPath) = 0;

  virtual size_t GetTargetCount() const = 0;
  virtual size_t GetTargetTimeStepsCount() const = 0;

  virtual const Utils::ListOfCoords& GetTargetTimes(size_t targetIdx) const = 0;
  virtual const Utils::Coord& GetTargetCoordByTime(size_t targetIdx, size_t timeIdx) const = 0;
};

class JsonTargetLoader : public ITargetLoader {
public:
  JsonTargetLoader() = default;

private:
  JsonTargetLoader(const JsonTargetLoader&) = delete;
  JsonTargetLoader& operator=(const JsonTargetLoader&) = delete;

  JsonTargetLoader(JsonTargetLoader&&) = delete;
  JsonTargetLoader& operator=(JsonTargetLoader&&) = delete;

public:
  bool Load(std::string_view dataFolderPath) override { return readTargets(dataFolderPath); }

  size_t GetTargetCount() const override { return m_targetCount; }
  size_t GetTargetTimeStepsCount() const override { return m_targetTimeStepsCount; }

  const Utils::ListOfCoords& GetTargetTimes(size_t targetIdx) const override { return m_targetsInTime.at(targetIdx); }
  const Utils::Coord& GetTargetCoordByTime(size_t targetIdx, size_t timeIdx) const override
  {
    return m_targetsInTime.at(targetIdx).at(timeIdx);
  }

private:
  bool readTargets(std::string_view dataFolderPath)
  {
    const auto targetsPath = dataFolderPath.data() + std::string("/targets.json");
    std::ifstream input(targetsPath);
    if (!input) {
      std::cerr << std::format("Can't open {}", targetsPath) << std::endl;
      return false;
    }

    try {
      nlohmann::json j = nlohmann::json::parse(input);

      if (!j.is_object()) {
        throw std::runtime_error("Root must be an object");
      }

      m_targetCount = j.at("targetCount").get<size_t>();
      m_targetTimeStepsCount = j.at("timeSteps").get<size_t>();

      const auto& targets = j.at("targets");

      if (!targets.is_array()) {
        throw std::runtime_error("'targets' must be array");
      }

      if (targets.size() != m_targetCount) {
        throw std::runtime_error("targets.size != targetCount");
      }

      m_targetsInTime.reserve(m_targetCount);  // preallocate memory for targets

      for (size_t i = 0; i < targets.size(); ++i) {
        const auto& target = targets.at(i);

        // validate
        if (!target.is_object()) {
          throw std::runtime_error("target must be object");
        }

        const auto& positions = target.at("positions");

        if (!positions.is_array()) {
          throw std::runtime_error("positions must be array");
        }

        if (positions.size() != m_targetTimeStepsCount) {
          throw std::runtime_error("positions.size != timeSteps");
        }

        // preallocate memory for target positions in time

        m_targetsInTime.emplace_back(Utils::ListOfCoords(m_targetTimeStepsCount));
        auto targetPositionsIter = m_targetsInTime.back();

        // read
        for (size_t k = 0; k < positions.size(); ++k) {
          const auto& position = positions.at(k);

          if (!position.is_object()) {
            throw std::runtime_error("position must be object");
          }

          const double x = position.at("x").get<double>();
          const double y = position.at("y").get<double>();

          if (x < 0 || y < 0) {
            throw std::runtime_error("Coords must be non-negative");
          }

          targetPositionsIter.push_back({x, y});
        }
      }
    }
    catch (const std::exception& e) {
      std::cerr << "JsonTargetProvider error: " << e.what() << '\n';
      return false;
    }

    return true;
  }

private:
  size_t m_targetCount{};
  size_t m_targetTimeStepsCount{};
  TargetsInTime m_targetsInTime{};
};

enum class TargetLoaderType { JSON_FILE };

ITargetLoader* CreateTargetLoader(TargetLoaderType type)
{
  switch (type) {
    case TargetLoaderType::JSON_FILE:
      return new JsonTargetLoader();
    default:
      throw std::out_of_range(std::format("CreateTargetLoader factory cannot create a Loader for type {}",
                                          static_cast<std::underlying_type_t<TargetLoaderType>>(type)));
  }
}

}  // namespace TargetsParams

namespace Calculation {

constexpr size_t MAX_STEPS{10'000};
constexpr double gEps{1e-6f};
constexpr int UNDEFINED_TARGET_ID{-1};

enum DroneState : uint8_t { STOPPED, ACCELERATING, DECELERATING, TURNING, MOVING };

struct Drone {
  Utils::Coord position{};
  double diraction{};

  int currentTarget{UNDEFINED_TARGET_ID};
  double targetDir{};

  DroneState state{STOPPED};
  double speed{};
  double turnRemaining{};
};

struct Target {
  int idx{UNDEFINED_TARGET_ID};

  double totalTime{std::numeric_limits<double>::max()};
  Utils::Coord releasePoint{};

  Utils::Coord predictedPosition{};
  Utils::Coord aimPoint{};
};

struct BallisticsSolverContext {
  int targetIdx{UNDEFINED_TARGET_ID};
  const Params::IConfigLoader& conf;
  const Calculation::Drone& drone;
  const TargetsParams::ITargetLoader& targetLoader;
  double currentTime{};
  double acceleration{};
};

class IBallisticSolver {
public:
  virtual ~IBallisticSolver() = default;

public:
  virtual Target Solve(const BallisticsSolverContext&) = 0;
};

class AnalyticalSolver : public IBallisticSolver {
public:
  AnalyticalSolver() = default;

private:
  AnalyticalSolver(const AnalyticalSolver&) = delete;
  AnalyticalSolver& operator=(const AnalyticalSolver&) = delete;

  AnalyticalSolver(AnalyticalSolver&&) = delete;
  AnalyticalSolver& operator=(AnalyticalSolver&&) = delete;

public:
  Target Solve(const BallisticsSolverContext& context) override
  {
    const auto& conf = context.conf.GetConfig();
    solveCommonBallistics(conf);

    Calculation::Target target{.idx = context.targetIdx};

    const auto& drone = context.drone;

    const auto targetVelocity = getTargetVelocity(target.idx, conf, context.targetLoader, context.currentTime);

    // get current position from targets file
    const auto currentPos = getInterpolatedTarget(context.targetLoader, target.idx, conf.arrayTimeStep, context.currentTime);

    // get fire point based on current target position
    const auto currentFirePoint = getFirePoint(drone.position, currentPos);

    double totalTime =
      computeTravelTime((currentFirePoint - drone.position).Length(), context.acceleration, drone.speed, conf.attackSpeed) + m_timeOfFlight;

    const auto predictedPos = currentPos + (targetVelocity * totalTime);

    const auto predictedFirePoint = getFirePoint(drone.position, predictedPos);

    // get time and position based on predicted coordinates

    target.totalTime =
      computeTravelTime((predictedFirePoint - drone.position).Length(), context.acceleration, drone.speed, conf.attackSpeed) +
      m_timeOfFlight;

    target.releasePoint = predictedFirePoint;
    target.predictedPosition = predictedPos;

    target.aimPoint = getAimPoint(drone);

    return target;
  }

private:
  bool setTimeOfFlight(const Params::AmmoParams& ammo, double altitude, double speed)
  {
    constexpr double g{9.81f};
    const double V0 = speed;
    const double sq_m = std::pow(ammo.mass, 2);
    const double sq_d = std::pow(ammo.drag, 2);

    const double a = ammo.drag * g * ammo.mass - 2 * sq_d * ammo.lift * V0;
    const double b = -3 * g * sq_m + 3 * ammo.drag * ammo.lift * ammo.mass * V0;
    const double c = 6 * sq_m * altitude;

    const double p = -std::pow(b, 2) / (3 * std::pow(a, 2));
    const double q = 2 * std::pow(b, 3) / (27 * std::pow(a, 3)) + c / a;

    if (p >= 0) {
      std::cerr << "No real solution for time of flight" << std::endl;
      return false;
    }

    const double fi_arg = 3 * q * std::sqrt(-3.0f / p) / (2 * p);
    if (fi_arg < -1 || fi_arg > 1) {
      std::cerr << "Arccos arg has to be in the range (-1;1)" << std::endl;
      return false;
    }

    const double fi = std::acos(fi_arg);
    m_timeOfFlight = 2 * std::sqrt(-p / 3.0f) * std::cos((fi + 4 * M_PI) / 3.0f) - b / (3 * a);

    if (m_timeOfFlight < 0) {
      std::cerr << "timeOfFlight < 0" << std::endl;
      return false;
    }

    return true;
  }

  bool setHorizontalFlightDistance(const Params::AmmoParams& ammo, double speed)
  {
    constexpr double g{9.81f};
    const double V0 = speed;
    const double sq_m = std::pow(ammo.mass, 2);
    const double sq_d = std::pow(ammo.drag, 2);
    const double cu_d = std::pow(ammo.drag, 3);
    const double sq_l = std::pow(ammo.lift, 2);
    const double cu_l = std::pow(ammo.lift, 3);

    const double h_part1 = V0 * m_timeOfFlight;
    const double h_part2 = std::pow(m_timeOfFlight, 2) * ammo.drag * V0 / (2 * ammo.mass);
    const double h_part3 =
      std::pow(m_timeOfFlight, 3) * (6 * ammo.drag * g * ammo.lift * ammo.mass - 6 * sq_d * (sq_l - 1) * V0) / (36 * sq_m);

    // clang-format off
   const double h_part4 = std::pow(m_timeOfFlight, 4)
      * (-6 * sq_d * g * ammo.lift * (1 + sq_l + sq_l * sq_l) * ammo.mass
         + 3 * cu_d * sq_l * (1 + sq_l) * V0
         + 6 * cu_d * sq_l * sq_l * (1 + sq_l) * V0)
      / (36 * std::pow(1 + sq_l, 2) * std::pow(ammo.mass, 3));

   const double h_part5 = std::pow(m_timeOfFlight, 5)
      * (3 * cu_d * g * cu_l * ammo.mass
         - 3 * sq_d * sq_d * sq_l * (1 + sq_l) * V0)
      / (36 * (1 + sq_l) * sq_m * sq_m);
    // clang-format on

    m_horizontalFlightDistance = h_part1 - h_part2 + h_part3 + h_part4 + h_part5;

    if (m_horizontalFlightDistance < 0) {
      std::cerr << "horizontalFlightDistance < 0" << std::endl;
      return false;
    }

    return true;
  }

  void solveCommonBallistics(const Params::DroneConfig& conf)
  {
    if (!m_commonBallisticsSolved) {
      if (setTimeOfFlight(conf.ammoParams, conf.altitude, conf.attackSpeed) &&
          setHorizontalFlightDistance(conf.ammoParams, conf.attackSpeed)) {
        m_commonBallisticsSolved = true;
      }
      else {
        throw std::logic_error(std::format("Connot solve balistics for given ammo {}", conf.ammoName));
      }
    }
  }

  Utils::Coord getInterpolatedTarget(const TargetsParams::ITargetLoader& targetsLoader, size_t targetIdx, double arrayTimeStep, double time)
  {
    const double samplePos = time / arrayTimeStep;
    const int rawIdx = static_cast<int>(std::floor(samplePos));
    const int idx = rawIdx % targetsLoader.GetTargetTimeStepsCount();
    const int next = (idx + 1) % targetsLoader.GetTargetTimeStepsCount();
    const double frac = samplePos - std::floor(samplePos);

    const auto& targetTimes = targetsLoader.GetTargetTimes(targetIdx);
    const double x = targetTimes[idx].x + (targetTimes[next].x - targetTimes[idx].x) * frac;
    const double y = targetTimes[idx].y + (targetTimes[next].y - targetTimes[idx].y) * frac;
    return {x, y};
  }

  Utils::Coord getTargetVelocity(size_t targetIdx,
                                 const Params::DroneConfig& conf,
                                 const TargetsParams::ITargetLoader& targetsLoader,
                                 double currentTime)
  {
    const double dt = conf.simTimeStep;
    const auto p0 = getInterpolatedTarget(targetsLoader, targetIdx, conf.arrayTimeStep, currentTime);
    const auto p1 = getInterpolatedTarget(targetsLoader, targetIdx, conf.arrayTimeStep, currentTime + dt);
    return {(p1.x - p0.x) / dt, (p1.y - p0.y) / dt};
  }

  Utils::Coord getFirePoint(const Utils::Coord& dronPos, const Utils::Coord& targetPos)
  {
    const Utils::Coord delta = targetPos - dronPos;
    const double distanceToTarget = delta.Length();
    const double ratio = (distanceToTarget - m_horizontalFlightDistance) / distanceToTarget;

    return dronPos + (targetPos - dronPos) * ratio;
  }

  double computeTravelTime(double distance, double acceleration, double currentSpeed, double maxSpeed)
  {
    if (distance <= 0.0f) {
      return 0.0f;
    }

    if (currentSpeed >= maxSpeed) {
      return distance / maxSpeed;
    }

    const double distanceToMaxSpeed = (maxSpeed * maxSpeed - currentSpeed * currentSpeed) / (2.0f * acceleration);

    if (distance <= distanceToMaxSpeed) {
      return (-currentSpeed + std::sqrt(currentSpeed * currentSpeed + 2.0f * acceleration * distance)) / acceleration;
    }

    const double timeToMaxSpeed = (maxSpeed - currentSpeed) / acceleration;
    const double cruiseDistance = distance - distanceToMaxSpeed;
    const double cruiseTime = cruiseDistance / maxSpeed;

    return timeToMaxSpeed + cruiseTime;
  }

  Utils::Coord getAimPoint(const Drone& drone)
  {
    Utils::Coord dir{std::cos(drone.diraction), std::sin(drone.diraction)};

    return drone.position + dir * m_horizontalFlightDistance;
  }

private:
  bool m_commonBallisticsSolved{false};
  double m_timeOfFlight{};
  double m_horizontalFlightDistance{};
};

class ILogger {
public:
  virtual ~ILogger() = default;

public:
  virtual void RecordStep(int idx, const Calculation::Drone& drone, const Calculation::Target& target) = 0;
  virtual void DumpLog(std::string_view dataFolderPath, size_t lastStepIdx) = 0;
  virtual void Reset() = 0;
};

class JsonLogger : public ILogger {
public:
  JsonLogger() = default;

private:
  JsonLogger(const JsonLogger&) = delete;
  JsonLogger& operator=(const JsonLogger&) = delete;

  JsonLogger(JsonLogger&&) = delete;
  JsonLogger& operator=(JsonLogger&&) = delete;

public:
  void RecordStep(int idx, const Calculation::Drone& drone, const Calculation::Target& target) override
  {
    m_simSteps[idx].pos = drone.position;
    m_simSteps[idx].direction = drone.diraction;
    m_simSteps[idx].state = drone.state;
    m_simSteps[idx].targetIdx = drone.currentTarget;

    m_simSteps[idx].dropPoint = target.releasePoint;
    m_simSteps[idx].aimPoint = target.aimPoint;
    m_simSteps[idx].predictedTarget = target.predictedPosition;
  }

  void DumpLog(std::string_view dataFolderPath, size_t lastStepIdx) override
  {
    try {
      if (lastStepIdx > m_simSteps.size()) {
        throw std::runtime_error("lastStepIdx is greater than log size.");
      }

      const auto logPath = dataFolderPath.data() + std::string("/simulation.json");
      std::ofstream output(logPath);
      if (!output) {
        throw std::runtime_error("Failed to open simulation.json for writing");
      }

      nlohmann::json out;
      out["totalSteps"] = lastStepIdx;
      out["steps"] = nlohmann::json::array();
      for (const auto& logStep : m_simSteps) {
        nlohmann::json step;
        step["position"] = {{"x", logStep.pos.x}, {"y", logStep.pos.y}};
        step["direction"] = logStep.direction;
        step["state"] = static_cast<int>(logStep.state);
        step["targetIndex"] = logStep.targetIdx;
        step["dropPoint"] = {{"x", logStep.dropPoint.x}, {"y", logStep.dropPoint.y}};
        step["aimPoint"] = {{"x", logStep.aimPoint.x}, {"y", logStep.aimPoint.y}};
        step["predictedTarget"] = {{"x", logStep.predictedTarget.x}, {"y", logStep.predictedTarget.y}};
        out["steps"].push_back(std::move(step));
      }

      output << out.dump(2);
    }
    catch (const std::exception& e) {
      std::cerr << "JsonLogger: " << e.what() << '\n';
    }
  }

  void Reset() override { m_simSteps.clear(); }

private:
  struct SimStep {
    Utils::Coord pos;               // позиція дрона
    double direction;               // напрямок (рад)
    Calculation::DroneState state;  // стан автомата (0-4)
    int targetIdx;                  // індекс поточної цілі
    Utils::Coord dropPoint;         // точка скиду (куди летить дрон)
    Utils::Coord aimPoint;          // куди впаде бомба (якщо скинути зараз)
    Utils::Coord predictedTarget;   // прогнозована позиція цілі
  };

  using SimSteps = std::vector<SimStep>;

private:
  SimSteps m_simSteps{};
};

class MissionProcessor {
public:
  MissionProcessor(Params::IConfigLoader* configLoader,
                   TargetsParams::ITargetLoader* targetLoader,
                   IBallisticSolver* ballisticSolver,
                   ILogger* logger)
  {
    if (!configLoader) {
      throw std::logic_error("ConfigLoader is not initialized");
    }

    if (!targetLoader) {
      throw std::logic_error("TargetLoader is not initialized");
    }

    if (!ballisticSolver) {
      throw std::logic_error("BallisticSolver is not initialized");
    }

    if (!logger) {
      throw std::logic_error("Logger is not initialized");
    }

    m_configLoader = configLoader;
    m_targetLoader = targetLoader;
    m_ballisticSolver = ballisticSolver;
    m_logger = logger;
  }

  ~MissionProcessor()
  {
    // do not delete here, MissionProcessor is not supposed to controle lifetime of external objects
  }

private:
  MissionProcessor(const MissionProcessor&) = delete;
  MissionProcessor& operator=(const MissionProcessor&) = delete;

  MissionProcessor(MissionProcessor&&) = delete;
  MissionProcessor& operator=(MissionProcessor&&) = delete;

public:
  void Init(std::string_view dataFolderPath)
  {
    if (!m_configLoader->Load(dataFolderPath)) {
      throw std::logic_error("Cannot initialize ConfigLoader");
    }

    if (!m_targetLoader->Load(dataFolderPath)) {
      throw std::logic_error("Cannot initialize TargetLoader");
    }

    m_drone.position = m_configLoader->GetConfig().startPos;
    m_drone.diraction = m_configLoader->GetConfig().initialDir;
    m_drone.state = Calculation::STOPPED;

    m_acceleration = std::pow(m_configLoader->GetConfig().attackSpeed, 2) / (2.0f * m_configLoader->GetConfig().accelerationPath);
    m_dataFolderPath = dataFolderPath;

    m_initialized = true;
  }

  void ChangeSolver(IBallisticSolver* ballisticSolver)
  {
    if (!ballisticSolver) {
      throw std::logic_error("New BallisticSolver is not initialized");
    }
    m_ballisticSolver = ballisticSolver;
  }

  void Reset()
  {
    m_drone.position = m_configLoader->GetConfig().startPos;
    m_drone.diraction = m_configLoader->GetConfig().initialDir;
    m_drone.state = Calculation::STOPPED;
    m_currentTime = 0.0;
    m_step = 0;

    m_logger->Reset();
  }

  void ProcessMission()
  {
    if (!m_initialized) {
      throw std::logic_error("MissionProcessor is not initialized");
    }

    const auto& conf = m_configLoader->GetConfig();

    while (m_step < Calculation::MAX_STEPS) {
      Calculation::Target bestTarget{};
      m_currentProcessedTargetID = 0;

      while (hasNext()) {
        auto target = step();
        if (target.totalTime < bestTarget.totalTime) {
          bestTarget = std::move(target);
        }
      }

      adjustDroneStateToTarget(bestTarget);

      moveDrone();

      m_logger->RecordStep(m_step, m_drone, bestTarget);

      // release point
      if (m_drone.state == MOVING && (bestTarget.releasePoint - m_drone.position).Length() <= 0.25 * conf.hitRadius) {
        break;
      }

      // move
      m_currentTime += conf.simTimeStep;
      m_step += 1;
    }

    if (m_step == Calculation::MAX_STEPS) {
      throw std::runtime_error(std::format("Simulation exceeded {} steps.\n", Calculation::MAX_STEPS));
    }

    m_logger->DumpLog(m_dataFolderPath, m_step);
  }

private:
  double getStopTime() const
  {
    switch (m_drone.state) {
      case STOPPED:
        return 0.0f;

      case ACCELERATING:
      case MOVING:
      case DECELERATING:
        return m_drone.speed / m_acceleration;

      case TURNING:
        return m_drone.turnRemaining;

      default:
        return 0.0f;
    }
  }

  // required method ?
  bool hasNext() { return m_currentProcessedTargetID < static_cast<int>(m_targetLoader->GetTargetCount()); }

  // required method ?
  Target step()
  {
    BallisticsSolverContext context{.targetIdx = m_currentProcessedTargetID,
                                    .conf = *m_configLoader,
                                    .drone = m_drone,
                                    .targetLoader = *m_targetLoader,
                                    .currentTime = m_currentTime,
                                    .acceleration = m_acceleration};

    auto target = m_ballisticSolver->Solve(context);

    if (m_drone.currentTarget != UNDEFINED_TARGET_ID && m_drone.currentTarget != m_currentProcessedTargetID) {
      target.totalTime += getStopTime();
    }

    m_currentProcessedTargetID++;

    return target;
  }

  void adjustDroneStateToTarget(const Target& target)
  {
    const auto& conf = m_configLoader->GetConfig();

    m_drone.currentTarget = target.idx;
    const auto targetDir = target.releasePoint - m_drone.position;
    m_drone.targetDir = Utils::NormalizeAngle180(std::atan2(targetDir.y, targetDir.x));

    const double deltaAngle = std::fabs(Utils::AngleDiff(m_drone.diraction, m_drone.targetDir));

    if (deltaAngle > conf.turnThreshold) {
      if (m_drone.state == DroneState::MOVING || m_drone.state == DroneState::ACCELERATING) {
        m_drone.state = DECELERATING;  // plan deceleration
      }
      else if (m_drone.state == DroneState::STOPPED) {
        // plan turning
        m_drone.state = TURNING;
        m_drone.turnRemaining = deltaAngle / conf.angularSpeed;
      }
    }
    else {
      // change diraction without stopping
      m_drone.diraction = m_drone.targetDir;

      if (m_drone.speed < conf.attackSpeed - gEps) {
        m_drone.state = ACCELERATING;
      }
      else {
        m_drone.state = MOVING;
      }
    }
  }

  void changeDronePosition(double dt)
  {
    const Utils::Coord positionToAdd = {std::cos(m_drone.diraction) * m_drone.speed * dt, std::sin(m_drone.diraction) * m_drone.speed * dt};
    m_drone.position += positionToAdd;
  }

  void moveDrone()
  {
    const auto& conf = m_configLoader->GetConfig();

    const double dt = conf.simTimeStep;

    switch (m_drone.state) {
      case DroneState::STOPPED: {
        m_drone.speed = 0.0f;
        break;
      }
      case DroneState::ACCELERATING: {
        m_drone.speed += m_acceleration * dt;

        if (m_drone.speed >= conf.attackSpeed) {
          m_drone.speed = conf.attackSpeed;
          m_drone.state = MOVING;
        }

        changeDronePosition(dt);
        break;
      }
      case DroneState::DECELERATING: {
        m_drone.speed -= m_acceleration * dt;

        if (m_drone.speed <= gEps) {
          m_drone.speed = 0.0f;
          m_drone.state = STOPPED;
        }

        changeDronePosition(dt);
        break;
      }
      case DroneState::TURNING: {
        m_drone.speed = 0.0f;

        const double deltaAngle = Utils::AngleDiff(m_drone.diraction, m_drone.targetDir);
        const double deltaAngleAbs = std::fabs(deltaAngle);

        const double maxTurn = conf.angularSpeed * dt;

        if (deltaAngleAbs <= maxTurn + gEps) {
          m_drone.diraction = m_drone.targetDir;
          m_drone.turnRemaining = 0.0f;
          m_drone.state = ACCELERATING;
        }
        else {
          const double turnStep = (deltaAngle > 0.0f ? maxTurn : -maxTurn);
          m_drone.diraction = Utils::NormalizeAngle180(m_drone.diraction + turnStep);
          m_drone.turnRemaining = (deltaAngleAbs - maxTurn) / conf.angularSpeed;
        }
        break;
      }
      case DroneState::MOVING: {
        changeDronePosition(dt);
        break;
      }
    }
  }

private:
  Params::IConfigLoader* m_configLoader{};
  TargetsParams::ITargetLoader* m_targetLoader{};
  IBallisticSolver* m_ballisticSolver{};
  ILogger* m_logger{};

  bool m_initialized{false};
  std::string m_dataFolderPath{};

private:
  int m_currentProcessedTargetID{UNDEFINED_TARGET_ID};
  Drone m_drone{};
  size_t m_step{0};
  double m_currentTime{};

  double m_acceleration{};
};

enum class SolverType { ANALYTICAL };

IBallisticSolver* CreateSolver(SolverType type)
{
  switch (type) {
    case SolverType::ANALYTICAL:
      return new AnalyticalSolver();
    default:
      throw std::out_of_range(
        std::format("CreateSolver factory cannot create a Sorver for type {}", static_cast<std::underlying_type_t<SolverType>>(type)));
  }
}

enum class LoggerType { JSON_FILE };

ILogger* CreateLogger(LoggerType type)
{
  switch (type) {
    case LoggerType::JSON_FILE:
      return new JsonLogger();
    default:
      throw std::out_of_range(
        std::format("CreateLogger factory cannot create a Logger for type {}", static_cast<std::underlying_type_t<LoggerType>>(type)));
  }
}

}  // namespace Calculation

int main(int argc, char** argv)
{
  // The program expects exactly one argument: a path to data folder
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <path_to_dir_with_files>\n";
    return 1;
  }

  auto configLoader = Params::CreateLoader(Params::ConfigLoaderType::JSON_FILE);
  auto targetLoader = TargetsParams::CreateTargetLoader(TargetsParams::TargetLoaderType::JSON_FILE);
  auto solver = Calculation::CreateSolver(Calculation::SolverType::ANALYTICAL);
  auto logger = Calculation::CreateLogger(Calculation::LoggerType::JSON_FILE);

  try {
    Calculation::MissionProcessor missionProcessor(configLoader, targetLoader, solver, logger);

    missionProcessor.Init(argv[1]);

    missionProcessor.ProcessMission();
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  // clean up
  if (!configLoader) {
    delete configLoader;
  }
  if (!targetLoader) {
    delete targetLoader;
  }
  if (!solver) {
    delete solver;
  }
  if (!logger) {
    delete logger;
  }
  return 0;
}
