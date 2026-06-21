cmake_minimum_required(VERSION 3.20)

project(homework3 CXX)

# Код домашньої роботи збирається з тим самим стандартом, що й лекційні приклади.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(homework3)

target_sources(
    homework3
    PRIVATE include/Coords.hpp
            include/Utils.h
            include/DroneConfig.h
            include/DroneAbstractions.h
            include/MissionProcessor.h
            include/interfaces/IConfigLoader.h
            include/interfaces/ITargetLoader.h
            include/interfaces/IBallisticSolver.h
            include/interfaces/ILogger.h
            include/interfaces/IDroneState.h
            include/interfaces/DroneStateContext.h
            include/config/JsonConfigLoader.h
            include/config/ConfigFactory.h
            include/providers/JsonTargetLoader.h
            include/providers/ProviderFactory.h
            include/solvers/AnalyticalSolver.h
            include/solvers/SolverFactory.h
            include/loggers/JsonLogger.h
            include/loggers/LoggerFactory.h
            include/droneStates/StateAccelerating.h
            include/droneStates/StateDecelerating.h
            include/droneStates/StateMoving.h
            include/droneStates/StateStopped.h
            include/droneStates/StateTurning.h
            src/Utils.cpp
            src/MissionProcessor.cpp
            src/config/JsonConfigLoader.cpp
            src/config/ConfigFactory.cpp
            src/providers/JsonTargetLoader.cpp
            src/providers/ProviderFactory.cpp
            src/solvers/AnalyticalSolver.cpp
            src/solvers/SolverFactory.cpp
            src/loggers/JsonLogger.cpp
            src/loggers/LoggerFactory.cpp
            src/droneStates/StateAccelerating.cpp
            src/droneStates/StateDecelerating.cpp
            src/droneStates/StateMoving.cpp
            src/droneStates/StateStopped.cpp
            src/droneStates/StateTurning.cpp
            src/main.cpp)

target_include_directories(homework3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_options(homework3 PRIVATE -Wall -Wextra -pedantic)

find_package(nlohmann_json REQUIRED)

target_link_libraries(homework3 PRIVATE nlohmann_json::nlohmann_json)
