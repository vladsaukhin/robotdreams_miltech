#include "ballistics/input_params.h"

#include <gtest/gtest.h>
#include <string_view>

TEST(InputParams, GetAmmoParamsTest)
{
  const std::vector<std::string> expectedTrue{"VOG-17", "M67", "RKG-3", "GLIDING-VOG", "GLIDING-RKG"};

  const std::vector<std::string> expectedFalse{"UnknownAmmo", "", "vog-17", "M-67"};

  for (const auto& ammoName : expectedTrue) {
    auto ammoOpt = GetAmmoParams(ammoName);

    EXPECT_TRUE(ammoOpt) << "Expected valid ammo: " << ammoName;
    if (ammoOpt) {
      EXPECT_EQ(ammoOpt->name, ammoName);
    }
  }

  for (const auto& ammoName : expectedFalse) {
    auto ammoOpt = GetAmmoParams(ammoName);

    EXPECT_FALSE(ammoOpt) << "Expected invalid ammo: " << ammoName;
  }
}

TEST(InputParams, ParseInputParamsTest)
{
  const auto validFilePath = std::string(TEST_DATA_DIR) + "/valid_input.txt";
  auto validInputOpt = ParseInputParams(validFilePath);
  EXPECT_TRUE(validInputOpt) << "Expected valid input parameters from file: " << validFilePath;

  const auto invalidFilePath = std::string(TEST_DATA_DIR) + "/invalid_input.txt";
  auto invalidInputOpt = ParseInputParams(invalidFilePath);
  EXPECT_FALSE(invalidInputOpt) << "Expected invalid input parameters from file: " << invalidFilePath;
}
