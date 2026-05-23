#include "ballistics/ballistics.h"
#include "ballistics/input_params.h"

#include <gtest/gtest.h>
#include <optional>
#include <source_location>

namespace {

constexpr double gEps{1e-2};

void ballisticsSolutionScenario(const InputParams& params,
                                const Coord& expectedFirePoint,
                                const std::optional<Coord>& expectedManeuverPoint,
                                std::source_location location = std::source_location::current())
{
  SCOPED_TRACE(std::string("Scenario: ") + location.function_name() + ":" + std::to_string(location.line()));

  auto ammoParamsOpt = GetAmmoParams(params.ammo_name);
  ASSERT_TRUE(ammoParamsOpt) << "Expected valid ammo parameters for ammo: " << params.ammo_name;

  auto timeOfFlightOpt = GetTimeOfFlight(*ammoParamsOpt, params.altitude, params.attackSpeed);
  ASSERT_TRUE(timeOfFlightOpt) << "Expected valid time of flight";

  auto horizontalFlightDistanceOpt = GetHorizontalFlightDistance(*ammoParamsOpt, params.attackSpeed, *timeOfFlightOpt);
  ASSERT_TRUE(horizontalFlightDistanceOpt) << "Expected valid horizontal flight distance";

  auto solution = GetBallistics(params, *timeOfFlightOpt, *horizontalFlightDistanceOpt);

  ASSERT_NEAR(solution.firePoint.x, expectedFirePoint.x, gEps) << "Fire point X does not match expected value";
  ASSERT_NEAR(solution.firePoint.y, expectedFirePoint.y, gEps) << "Fire point Y does not match expected value";

  if (expectedManeuverPoint) {
    ASSERT_TRUE(solution.maneuverPoint) << "Expected a valid maneuver point";
    ASSERT_NEAR(solution.maneuverPoint.value().x, expectedManeuverPoint->x, gEps) << "Maneuver point X does not match expected value";
    ASSERT_NEAR(solution.maneuverPoint.value().y, expectedManeuverPoint->y, gEps) << "Maneuver point Y does not match expected value";
  }
  else {
    ASSERT_FALSE(solution.maneuverPoint) << "Expected no maneuver point";
  }
}

}  // namespace

TEST(Ballistics, GetTimeOfFlightTest)
{
  const AmmoParams ammo{"TestAmmo", 0.5f, 0.1f, 0.0f};
  const double altitude = 100.0f;
  const double speed = 300.0f;

  auto timeOfFlightOpt = GetTimeOfFlight(ammo, altitude, speed);

  EXPECT_TRUE(timeOfFlightOpt) << "Expected valid time of flight";
  if (timeOfFlightOpt) {
    EXPECT_GT(*timeOfFlightOpt, 0.0f) << "Expected positive time of flight";
  }
}

TEST(Ballistics, GetHorizontalFlightDistanceTest)
{
  const AmmoParams ammo{"TestAmmo", 0.5f, 0.1f, 0.0f};
  const double timeOfFlight = 2.0f;
  const double speed = 300.0f;

  auto horizontalFlightDistanceOpt = GetHorizontalFlightDistance(ammo, timeOfFlight, speed);

  EXPECT_TRUE(horizontalFlightDistanceOpt) << "Expected valid horizontal flight distance";
  if (horizontalFlightDistanceOpt) {
    EXPECT_GT(*horizontalFlightDistanceOpt, 0.0f) << "Expected positive horizontal flight distance";
  }
}

TEST(Ballistics, GetBallisticsVOG17Test)
{
  const auto file_path = std::string(REAL_DATA_DIR) + "/VOG17.txt";
  auto paramsOpt = ParseInputParams(file_path);
  ASSERT_TRUE(paramsOpt) << "Expected valid input parameters from file: " << file_path;

  ballisticsSolutionScenario(*paramsOpt, {173.759, 173.759}, Coord{166.688, 166.688});
}

TEST(Ballistics, GetBallisticsM67Test)
{
  const auto file_path = std::string(REAL_DATA_DIR) + "/M67.txt";
  auto paramsOpt = ParseInputParams(file_path);
  ASSERT_TRUE(paramsOpt) << "Expected valid input parameters from file: " << file_path;

  ballisticsSolutionScenario(*paramsOpt, {490.496, 179.496}, Coord{478.496, 167.496});
}

TEST(Ballistics, GetBallisticsRKG3Test)
{
  const auto file_path = std::string(REAL_DATA_DIR) + "/RKG-3.txt";
  auto paramsOpt = ParseInputParams(file_path);
  ASSERT_TRUE(paramsOpt) << "Expected valid input parameters from file: " << file_path;

  ballisticsSolutionScenario(*paramsOpt, {513.085, 202.085}, Coord{504.6, 193.6});
}

TEST(Ballistics, GetBallisticsGlidingVOGTest)
{
  const auto file_path = std::string(REAL_DATA_DIR) + "/GLIDING-VOG.txt";
  auto paramsOpt = ParseInputParams(file_path);
  ASSERT_TRUE(paramsOpt) << "Expected valid input parameters from file: " << file_path;

  ballisticsSolutionScenario(*paramsOpt, {242.711, 242.711}, std::nullopt);
}

TEST(Ballistics, GetBallisticsGlidingRKGTest)
{
  const auto file_path = std::string(REAL_DATA_DIR) + "/GLIDING-RKG.txt";
  auto paramsOpt = ParseInputParams(file_path);
  ASSERT_TRUE(paramsOpt) << "Expected valid input parameters from file: " << file_path;

  ballisticsSolutionScenario(*paramsOpt, {966.534, 404.519}, std::nullopt);
}
