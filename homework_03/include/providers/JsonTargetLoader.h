#pragma once

#include "interfaces/ITargetLoader.h"

class JsonTargetLoader : public ITargetLoader {
public:
  JsonTargetLoader() = default;

  JsonTargetLoader(JsonTargetLoader&&) = default;
  JsonTargetLoader& operator=(JsonTargetLoader&&) = default;

private:
  JsonTargetLoader(const JsonTargetLoader&) = delete;
  JsonTargetLoader& operator=(const JsonTargetLoader&) = delete;

public:
  bool Load(std::string_view dataFolderPath) override { return readTargets(dataFolderPath); }

  size_t GetTargetCount() const override { return m_targetCount; }
  size_t GetTargetTimeStepsCount() const override { return m_targetTimeStepsCount; }

  const ListOfCoords& GetTargetTimes(size_t targetIdx) const override { return m_targetsInTime.at(targetIdx); }
  const Coord& GetTargetCoordByTime(size_t targetIdx, size_t timeIdx) const override { return m_targetsInTime.at(targetIdx).at(timeIdx); }

private:
  bool readTargets(std::string_view dataFolderPath);

private:
  size_t m_targetCount{};
  size_t m_targetTimeStepsCount{};
  std::vector<ListOfCoords> m_targetsInTime{};
};