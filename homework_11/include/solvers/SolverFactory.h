#pragma once

#include "interfaces/IBallisticSolver.h"

enum class SolverType { ANALYTICAL };

IBallisticSolverPtr CreateSolver(SolverType type);