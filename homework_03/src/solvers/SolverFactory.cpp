#include "solvers/SolverFactory.h"

#include "solvers/AnalyticalSolver.h"
#include "solvers/TableSolver.h"

#include <format>

IBallisticSolverPtr CreateSolver(SolverType type)
{
  switch (type) {
    case SolverType::ANALYTICAL:
      return std::make_unique<AnalyticalSolver>();
    case SolverType::TABLE:
      return std::make_unique<TableSolver>();
    default:
      throw std::out_of_range(
        std::format("CreateSolver factory cannot create a Sorver for type {}", static_cast<std::underlying_type_t<SolverType>>(type)));
  }
}