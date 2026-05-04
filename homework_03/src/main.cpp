#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Utils {

class MyString {
public:
  MyString() = default;

  explicit MyString(const char* str)
  {
    if (!str) {
      return;
    }

    m_size = std::strlen(str);
    m_data = new char[m_size + 1];

    std::memcpy(m_data, str, m_size + 1);  // including '\0'
  }

  explicit MyString(std::size_t size)
    : m_data(new char[size + 1]{})
    , m_size(size)
  {
  }

  ~MyString() { delete[] m_data; }

  MyString(const MyString&) = delete;
  MyString& operator=(const MyString&) = delete;

  MyString(MyString&& other) noexcept
    : m_data(other.m_data)
    , m_size(other.m_size)
  {
    other.m_data = nullptr;
    other.m_size = 0;
  }

  MyString& operator=(MyString&& other) noexcept
  {
    if (this != &other) {
      delete[] m_data;

      m_data = other.m_data;
      m_size = other.m_size;

      other.m_data = nullptr;
      other.m_size = 0;
    }

    return *this;
  }

  char* Data() noexcept { return m_data; }
  const char* CStr() const noexcept { return m_data ? m_data : ""; }

  std::size_t Size() const noexcept { return m_size; }

  char& operator[](std::size_t index) noexcept { return m_data[index]; }
  const char& operator[](std::size_t index) const noexcept { return m_data[index]; }

private:
  char* m_data{};
  std::size_t m_size{};
};

template <typename T>
class MyArray {
public:
  MyArray() noexcept = default;

  explicit MyArray(std::size_t size)
    : m_ptr(size ? new T[size]{} : nullptr)
    , m_size(size)
  {
  }

  explicit MyArray(T* ptr, std::size_t size) noexcept
    : m_ptr(ptr)
    , m_size(size)
  {
  }

  ~MyArray() { delete[] m_ptr; }

  MyArray(const MyArray&) = delete;
  MyArray& operator=(const MyArray&) = delete;

  MyArray(MyArray&& other) noexcept
    : m_ptr(other.m_ptr)
    , m_size(other.m_size)
  {
    other.m_ptr = nullptr;
    other.m_size = 0;
  }

  MyArray& operator=(MyArray&& other) noexcept
  {
    if (this != &other) {
      delete[] m_ptr;

      m_ptr = other.m_ptr;
      m_size = other.m_size;

      other.m_ptr = nullptr;
      other.m_size = 0;
    }

    return *this;
  }

  T& operator[](std::size_t index) noexcept { return m_ptr[index]; }

  const T& operator[](std::size_t index) const noexcept { return m_ptr[index]; }

  T* Get() noexcept { return m_ptr; }

  const T* Get() const noexcept { return m_ptr; }

  std::size_t Size() const noexcept { return m_size; }

  explicit operator bool() const noexcept { return m_ptr != nullptr; }

  T* Release() noexcept
  {
    T* ptr = m_ptr;
    m_ptr = nullptr;
    m_size = 0;
    return ptr;
  }

  void Reset(T* ptr = nullptr, std::size_t size = 0) noexcept
  {
    if (m_ptr != ptr) {
      delete[] m_ptr;
      m_ptr = ptr;
      m_size = size;
    }
  }

private:
  T* m_ptr{nullptr};
  std::size_t m_size{};
};

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

using ListOfCoords = MyArray<Coord>;

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
  double a = NormalizeAngle360(angle);
  if (a > M_PI) {
    a -= 2.0f * M_PI;
  }
  return a;
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
  Utils::Coord startPos;    // початкова позиція (x, y)
  double altitude;          // висота
  double initialDir;        // початковий напрямок (рад)
  double attackSpeed;       // швидкість атаки (м/с)
  double accelerationPath;  // шлях розгону (м)
  double arrayTimeStep;     // крок часу масиву цілей
  double simTimeStep;       // крок симуляції
  double hitRadius;         // радіус влучення
  double angularSpeed;      // кутова швидкість (рад/с)
  double turnThreshold;     // поріг повороту (рад)

  Utils::MyString ammoName{};  // обрані боєприпаси
  AmmoParams ammoParams{};
};

bool ReadAmmo(std::string_view dataFolderPath, const char* ammoName, AmmoParams& ammo)
{
  std::ifstream input(std::string(dataFolderPath) + "/ammo.json");
  if (!input) {
    std::cerr << "Can't open ammo.json" << std::endl;
    return false;
  }

  try {
    json j = json::parse(input);

    if (!j.is_array()) {
      throw std::runtime_error("Root must be an array");
    }

    const size_t ammoCount = j.size();
    for (size_t i = 0; i < ammoCount; ++i) {
      if (std::strcmp(ammoName, j[i].at("name").get<std::string>().c_str()) == 0) {
        ammo.mass = j[i].at("mass").get<double>();
        ammo.drag = j[i].at("drag").get<double>();
        ammo.lift = j[i].at("lift").get<double>();
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

bool ValidateConfig(const DroneConfig& conf)
{
  bool isValidResult = true;
  if (conf.altitude < 0.0f || conf.startPos.x < 0.0f || conf.startPos.y < 0.0f) {
    std::cerr << "Drons coordinates must be non-negative" << std::endl;
    isValidResult &= false;
  }
  if (conf.attackSpeed <= 0.0f) {
    std::cerr << "attackSpeed must be greater than 0." << std::endl;
    isValidResult &= false;
  }
  if (conf.accelerationPath < 0.0f) {
    std::cerr << "AccelerationPath must be non-negative." << std::endl;
    isValidResult &= false;
  }
  if (conf.arrayTimeStep <= 0.0f) {
    std::cerr << "ArrayTimeStep must be greater than 0." << std::endl;
    isValidResult &= false;
  }
  if (conf.simTimeStep <= 0.0f) {
    std::cerr << "SimTimeStep must be greater than 0." << std::endl;
    isValidResult &= false;
  }
  if (conf.simTimeStep > conf.arrayTimeStep) {
    std::cerr << "SimTimeStep must be less than or equal to ArrayTimeStep." << std::endl;
    isValidResult &= false;
  }
  if (conf.hitRadius < 0.0f) {
    std::cerr << "HitRadius must be non-negative.";
    isValidResult &= false;
  }
  if (conf.angularSpeed <= 0.0f) {
    std::cerr << "AngularSpeed must be greater than 0.";
    isValidResult &= false;
  }
  if (conf.turnThreshold < 0.0f) {
    std::cerr << "TurnThreshold must be non-negative.";
    isValidResult &= false;
  }
  return isValidResult;
}

bool ReadSimulationParams(std::string_view dataFolderPath, DroneConfig& conf)
{
  std::ifstream input(std::string(dataFolderPath) + "/config.json");
  if (!input) {
    std::cerr << "Can't open config.json" << std::endl;
    return false;
  }

  try {
    json j = json::parse(input);

    if (!j.is_object()) {
      throw std::runtime_error("Root must be an object");
    }

    const auto& drone = j.at("drone");

    const auto& pos = drone.at("position");
    conf.startPos.x = pos.at("x").get<double>();
    conf.startPos.y = pos.at("y").get<double>();

    conf.altitude = drone.at("altitude").get<double>();
    conf.initialDir = drone.at("initialDirection").get<double>();
    conf.attackSpeed = drone.at("attackSpeed").get<double>();
    conf.accelerationPath = drone.at("accelerationPath").get<double>();
    conf.angularSpeed = drone.at("angularSpeed").get<double>();
    conf.turnThreshold = drone.at("turnThreshold").get<double>();

    conf.ammoName = Utils::MyString(j.at("ammo").get<std::string>().c_str());

    const auto& sim = j.at("simulation");
    conf.simTimeStep = sim.at("timeStep").get<double>();
    conf.hitRadius = sim.at("hitRadius").get<double>();

    conf.arrayTimeStep = j.at("targetArrayTimeStep").get<double>();
  }
  catch (const std::exception& e) {
    std::cerr << "Config error: " << e.what() << '\n';
    return false;
  }

  if (!ReadAmmo(dataFolderPath, conf.ammoName.CStr(), conf.ammoParams)) {
    return false;
  }

  return ValidateConfig(conf);
}

}  // namespace Params

namespace TargetsParams {

using TargetsInTime = Utils::MyArray<Utils::ListOfCoords>;

// allocate memory inside
bool LoadTargets(std::string_view dataFolderPath, TargetsInTime& targetsInTime)
{
  std::ifstream input(std::string(dataFolderPath) + "/targets.json");
  if (!input) {
    std::cerr << "Can't open targets.json" << std::endl;
    return false;
  }

  try {
    json j = json::parse(input);

    if (!j.is_object()) {
      throw std::runtime_error("Root must be an object");
    }

    const size_t targetCount = j.at("targetCount").get<size_t>();
    const size_t timeSteps = j.at("timeSteps").get<size_t>();

    const auto& targets = j.at("targets");

    if (!targets.is_array()) {
      throw std::runtime_error("'targets' must be array");
    }

    if (targets.size() != targetCount) {
      throw std::runtime_error("targets.size != targetCount");
    }

    targetsInTime = TargetsInTime(targetCount);

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

      if (positions.size() != timeSteps) {
        throw std::runtime_error("positions.size != timeSteps");
      }

      // allocate
      targetsInTime[i] = Utils::ListOfCoords(timeSteps);

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

        targetsInTime[i][k] = {x, y};
      }
    }
  }
  catch (const std::exception& e) {
    std::cerr << "Ammo error: " << e.what() << '\n';
    return false;
  }

  return true;
}

void PrintTargets(const TargetsInTime& data)
{
  const size_t targetCount = data.Size();

  for (size_t i = 0; i < targetCount; ++i) {
    const size_t timeSteps = data[i].Size();

    std::cout << "Target #" << i << ":\n";

    for (size_t t = 0; t < timeSteps; ++t) {
      std::cout << "  t=" << t << data[i][t] << '\n';
    }
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

struct Ballistics {
  double timeOfFlight{};
  double horizontalFlightDistance{};
};

bool GetTimeOfFlight(const Params::AmmoParams& ammo, double zd, double speed, double& timeOfFlight)
{
  constexpr double g{9.81f};
  const double V0 = speed;
  const double sq_m = std::pow(ammo.mass, 2);
  const double sq_d = std::pow(ammo.drag, 2);

  const double a = ammo.drag * g * ammo.mass - 2 * sq_d * ammo.lift * V0;
  const double b = -3 * g * sq_m + 3 * ammo.drag * ammo.lift * ammo.mass * V0;
  const double c = 6 * sq_m * zd;

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
  const double t = 2 * std::sqrt(-p / 3.0f) * std::cos((fi + 4 * M_PI) / 3.0f) - b / (3 * a);

  if (t < 0) {
    std::cerr << "timeOfFlight < 0" << std::endl;
    return false;
  }

  timeOfFlight = t;
  return true;
}

bool GetHorizontalFlightDistance(const Params::AmmoParams& ammo, double timeOfFlight, double speed, double& horizontalFlightDistance)
{
  constexpr double g{9.81f};
  const double V0 = speed;
  const double sq_m = std::pow(ammo.mass, 2);
  const double sq_d = std::pow(ammo.drag, 2);
  const double cu_d = std::pow(ammo.drag, 3);
  const double sq_l = std::pow(ammo.lift, 2);
  const double cu_l = std::pow(ammo.lift, 3);

  const double h_part1 = V0 * timeOfFlight;
  const double h_part2 = std::pow(timeOfFlight, 2) * ammo.drag * V0 / (2 * ammo.mass);
  const double h_part3 = std::pow(timeOfFlight, 3) * (6 * ammo.drag * g * ammo.lift * ammo.mass - 6 * sq_d * (sq_l - 1) * V0) / (36 * sq_m);

  // clang-format off
   const double h_part4 = std::pow(timeOfFlight, 4)
      * (-6 * sq_d * g * ammo.lift * (1 + sq_l + sq_l * sq_l) * ammo.mass
         + 3 * cu_d * sq_l * (1 + sq_l) * V0
         + 6 * cu_d * sq_l * sq_l * (1 + sq_l) * V0)
      / (36 * std::pow(1 + sq_l, 2) * std::pow(ammo.mass, 3));

   const double h_part5 = std::pow(timeOfFlight, 5)
      * (3 * cu_d * g * cu_l * ammo.mass
         - 3 * sq_d * sq_d * sq_l * (1 + sq_l) * V0)
      / (36 * (1 + sq_l) * sq_m * sq_m);
  // clang-format on

  const double h = h_part1 - h_part2 + h_part3 + h_part4 + h_part5;

  if (h < 0) {
    std::cerr << "horizontalFlightDistance < 0" << std::endl;
    return false;
  }

  horizontalFlightDistance = h;
  return true;
}

bool GetBallistics(const Params::DroneConfig& conf, Ballistics& ballistics)
{
  if (!Calculation::GetTimeOfFlight(conf.ammoParams, conf.altitude, conf.attackSpeed, ballistics.timeOfFlight)) {
    return false;
  }

  return Calculation::GetHorizontalFlightDistance(
    conf.ammoParams, ballistics.timeOfFlight, conf.attackSpeed, ballistics.horizontalFlightDistance);
}

double GetAcceleration(const Params::DroneConfig& conf)
{
  return std::pow(conf.attackSpeed, 2) / (2.0f * conf.accelerationPath);
}

double GetStopTime(const Drone& drone, const Params::DroneConfig& conf)
{
  switch (drone.state) {
    case STOPPED:
      return 0.0f;

    case ACCELERATING:
    case MOVING:
    case DECELERATING:
      return drone.speed / GetAcceleration(conf);

    case TURNING:
      return drone.turnRemaining;

    default:
      return 0.0f;
  }
}

Utils::Coord GetInterpolatedTarget(const TargetsParams::TargetsInTime& targetsInTime, size_t targetIdx, double arrayTimeStep, double time)
{
  const double samplePos = time / arrayTimeStep;
  const int rawIdx = static_cast<int>(std::floor(samplePos));
  const int idx = rawIdx % 60;
  const int next = (idx + 1) % 60;
  const double frac = samplePos - std::floor(samplePos);

  const double x = targetsInTime[targetIdx][idx].x + (targetsInTime[targetIdx][next].x - targetsInTime[targetIdx][idx].x) * frac;
  const double y = targetsInTime[targetIdx][idx].y + (targetsInTime[targetIdx][next].y - targetsInTime[targetIdx][idx].y) * frac;
  return {x, y};
}

Utils::Coord GetTargetVelocity(size_t targetIdx,
                               const Params::DroneConfig& conf,
                               const TargetsParams::TargetsInTime& targetsInTime,
                               double currentTime)
{
  const double dt = conf.simTimeStep;
  const auto p0 = GetInterpolatedTarget(targetsInTime, targetIdx, conf.arrayTimeStep, currentTime);
  const auto p1 = GetInterpolatedTarget(targetsInTime, targetIdx, conf.arrayTimeStep, currentTime + dt);
  return {(p1.x - p0.x) / dt, (p1.y - p0.y) / dt};
}

Utils::Coord GetFirePoint(const Utils::Coord& dronPos, const Utils::Coord& targetPos, const Calculation::Ballistics& ballistics)
{
  const Utils::Coord delta = targetPos - dronPos;
  const double distanceToTarget = delta.Length();
  const double ratio = (distanceToTarget - ballistics.horizontalFlightDistance) / distanceToTarget;

  return dronPos + (targetPos - dronPos) * ratio;
}

double ComputeTravelTime(double distance, double acceleration, double currentSpeed, double maxSpeed)
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

Utils::Coord GetAimPoint(const Drone& drone, const Ballistics& ballistics)
{
  Utils::Coord dir{std::cos(drone.diraction), std::sin(drone.diraction)};

  return drone.position + dir * ballistics.horizontalFlightDistance;
}

Calculation::Target GetTarget(int targetIdx,
                              const Params::DroneConfig& conf,
                              const Calculation::Drone& drone,
                              const Calculation::Ballistics& ballistics,
                              const TargetsParams::TargetsInTime& targetsInTime,
                              double currentTime)
{
  Calculation::Target target{.idx = targetIdx};

  const double acceleration = GetAcceleration(conf);
  const auto targetVelocity = GetTargetVelocity(target.idx, conf, targetsInTime, currentTime);

  // get current position from targets file
  const auto currentPos = GetInterpolatedTarget(targetsInTime, target.idx, conf.arrayTimeStep, currentTime);

  // get fire point based on current target position
  const auto currentFirePoint = GetFirePoint(drone.position, currentPos, ballistics);

  double totalTime =
    ComputeTravelTime((currentFirePoint - drone.position).Length(), acceleration, drone.speed, conf.attackSpeed) + ballistics.timeOfFlight;

  const auto predictedPos = currentPos + (targetVelocity * totalTime);

  const auto predictedFirePoint = GetFirePoint(drone.position, predictedPos, ballistics);

  // get time and position based on predicted coordinates

  target.totalTime = ComputeTravelTime((predictedFirePoint - drone.position).Length(), acceleration, drone.speed, conf.attackSpeed) +
                     ballistics.timeOfFlight;

  target.releasePoint = predictedFirePoint;
  target.predictedPosition = predictedPos;

  target.aimPoint = GetAimPoint(drone, ballistics);

  return target;
}

bool SelectBestTarget(Calculation::Target& bestTarget,
                      const Params::DroneConfig& conf,
                      const Calculation::Drone& drone,
                      const TargetsParams::TargetsInTime& targetsInTime,
                      double currentTime)
{
  bool found = false;

  Calculation::Ballistics ballistics;
  if (!Calculation::GetBallistics(conf, ballistics)) {
    std::cerr << "Cannot calculate balistics.\n";
    return false;
  }

  const int targetsCount = static_cast<int>(targetsInTime.Size());
  for (int targetID = 0; targetID < targetsCount; ++targetID) {
    auto target = GetTarget(targetID, conf, drone, ballistics, targetsInTime, currentTime);

    if (drone.currentTarget != UNDEFINED_TARGET_ID && drone.currentTarget != targetID) {
      target.totalTime += GetStopTime(drone, conf);
    }

    if (target.totalTime < bestTarget.totalTime) {
      bestTarget = std::move(target);
      found = true;
    }
  }
  return found;
}

double GetAccelerationTime(const Params::DroneConfig& conf)
{
  return 2.0f * conf.accelerationPath / conf.attackSpeed;
}

void ChangeDronePosition(Calculation::Drone& drone, double dt)
{
  const Utils::Coord positionToAdd = {std::cos(drone.diraction) * drone.speed * dt, std::sin(drone.diraction) * drone.speed * dt};
  drone.position += positionToAdd;
}

void AdjustDroneStateToTarget(Calculation::Drone& drone, const Calculation::Target& target, const Params::DroneConfig& conf)
{
  drone.currentTarget = target.idx;
  const auto targetDir = target.releasePoint - drone.position;
  drone.targetDir = Utils::NormalizeAngle180(std::atan2(targetDir.y, targetDir.x));

  const double deltaAngle = std::fabs(Utils::AngleDiff(drone.diraction, drone.targetDir));

  if (deltaAngle > conf.turnThreshold) {
    if (drone.state == DroneState::MOVING || drone.state == DroneState::ACCELERATING) {
      // plan deceleration
      drone.state = DECELERATING;
    }
    else if (drone.state == DroneState::STOPPED) {
      // plan turning
      drone.state = TURNING;
      drone.turnRemaining = deltaAngle / conf.angularSpeed;
    }
  }
  else {
    // change diraction without stopping
    drone.diraction = drone.targetDir;

    if (drone.speed < conf.attackSpeed - gEps) {
      drone.state = ACCELERATING;
    }
    else {
      drone.state = MOVING;
    }
  }
}

void MoveDrone(Calculation::Drone& drone, const Params::DroneConfig& conf)
{
  const double a = GetAcceleration(conf);

  const double dt = conf.simTimeStep;

  switch (drone.state) {
    case DroneState::STOPPED: {
      drone.speed = 0.0f;
      break;
    }
    case DroneState::ACCELERATING: {
      drone.speed += a * dt;

      if (drone.speed >= conf.attackSpeed) {
        drone.speed = conf.attackSpeed;
        drone.state = MOVING;
      }

      ChangeDronePosition(drone, dt);
      break;
    }
    case DroneState::DECELERATING: {
      drone.speed -= a * dt;

      if (drone.speed <= gEps) {
        drone.speed = 0.0f;
        drone.state = STOPPED;
      }

      ChangeDronePosition(drone, dt);
      break;
    }
    case DroneState::TURNING: {
      drone.speed = 0.0f;

      const double deltaAngle = Utils::AngleDiff(drone.diraction, drone.targetDir);
      const double deltaAngleAbs = std::fabs(deltaAngle);

      const double maxTurn = conf.angularSpeed * dt;

      if (deltaAngleAbs <= maxTurn + gEps) {
        drone.diraction = drone.targetDir;
        drone.turnRemaining = 0.0f;
        drone.state = ACCELERATING;
      }
      else {
        const double turnStep = (deltaAngle > 0.0f ? maxTurn : -maxTurn);
        drone.diraction = Utils::NormalizeAngle180(drone.diraction + turnStep);
        drone.turnRemaining = (deltaAngleAbs - maxTurn) / conf.angularSpeed;
      }
      break;
    }
    case DroneState::MOVING: {
      ChangeDronePosition(drone, dt);
      break;
    }
  }
}

}  // namespace Calculation

namespace LogUtils {

struct SimStep {
  Utils::Coord pos;               // позиція дрона
  double direction;               // напрямок (рад)
  Calculation::DroneState state;  // стан автомата (0-4)
  int targetIdx;                  // індекс поточної цілі
  Utils::Coord dropPoint;         // точка скиду (куди летить дрон)
  Utils::Coord aimPoint;          // куди впаде бомба (якщо скинути зараз)
  Utils::Coord predictedTarget;   // прогнозована позиція цілі
};

using SimSteps = Utils::MyArray<SimStep>;

void RecordStep(SimSteps& simSteps, int idx, const Calculation::Drone& drone, const Calculation::Target& target)
{
  simSteps[idx].pos = drone.position;
  simSteps[idx].direction = drone.diraction;
  simSteps[idx].state = drone.state;
  simSteps[idx].targetIdx = drone.currentTarget;

  simSteps[idx].dropPoint = target.releasePoint;
  simSteps[idx].aimPoint = target.aimPoint;
  simSteps[idx].predictedTarget = target.predictedPosition;
}

bool WriteSimLog(std::string_view dataFolderPath, const SimSteps& log, size_t lastStepIdx)
{
  if (lastStepIdx > log.Size()) {
    std::cerr << "lastStepIdx is greater than log size.\n";
    return false;
  }

  std::ofstream output(std::string(dataFolderPath) + "/simulation.json");
  if (!output) {
    std::cerr << "Cannot open simulation.json file for writing.\n";
    return false;
  }

  try {
    json out;
    out["totalSteps"] = lastStepIdx;
    out["steps"] = json::array();
    for (size_t i = 0; i < lastStepIdx; ++i) {
      const auto& logStep = log[i];
      json step;
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
    std::cerr << "Simulation log: " << e.what() << '\n';
    return false;
  }

  return true;
}

}  // namespace LogUtils

int main(int argc, char** argv)
{
  // The program expects exactly one argument: a path to data folder
  if (argc != 2) {
    std::cerr << "can't start without path to data folder\n";
    return 1;
  }

  Params::DroneConfig conf;

  TargetsParams::TargetsInTime targetInTime;

  if (!Params::ReadSimulationParams(argv[1], conf) || !TargetsParams::LoadTargets(argv[1], targetInTime)) {
    return 1;
  }

  LogUtils::SimSteps log(Calculation::MAX_STEPS);

  Calculation::Drone drone{
    .position = conf.startPos,
    .diraction = conf.initialDir,
    .state = Calculation::STOPPED,
  };

  size_t step{0};
  double currentTime{};

  while (step < Calculation::MAX_STEPS) {
    Calculation::Target bestTarget{};
    if (!SelectBestTarget(bestTarget, conf, drone, targetInTime, currentTime)) {
      std::cerr << "No valid target solution at step " << step << std::endl;
      return 1;
    }

    AdjustDroneStateToTarget(drone, bestTarget, conf);

    MoveDrone(drone, conf);

    LogUtils::RecordStep(log, step, drone, bestTarget);

    // release point
    if ((bestTarget.releasePoint - drone.position).Length() <= conf.hitRadius) {
      break;
    }

    // move
    currentTime += conf.simTimeStep;
    step += 1;
  }

  if (step == Calculation::MAX_STEPS) {
    std::cerr << "Simulation exceeded " << Calculation::MAX_STEPS << " steps.\n";
    return 1;
  }

  if (!LogUtils::WriteSimLog(argv[1], log, step)) {
    return 1;
  }

  return 0;
}
