#pragma once

#include "interfaces/IBallisticSolver.h"
#include "CommonSolver.h"

class TableSolver : public IBallisticSolver, public CommonSolver {
public:
  TableSolver();

  TableSolver(TableSolver&&) = default;
  TableSolver& operator=(TableSolver&&) = default;

  ~TableSolver();

private:
  TableSolver(const TableSolver&) = delete;
  TableSolver& operator=(const TableSolver&) = delete;

public:
  Target Solve(const BallisticsSolverContext&) override;

private:
  bool solveCommonBallistics(const DroneConfig&) override;

private:
  struct BallisticTable;
  std::unique_ptr<BallisticTable> m_ballisticTable;
};