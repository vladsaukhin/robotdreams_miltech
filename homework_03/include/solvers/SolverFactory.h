#pragma once

#include "interfaces/IBallisticSolver.h"

enum class SolverType { ANALYTICAL, TABLE };

IBallisticSolverPtr CreateSolver(SolverType type);