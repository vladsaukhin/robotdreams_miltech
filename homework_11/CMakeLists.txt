cmake_minimum_required(VERSION 3.20)

project(homework11 CXX)

# Код домашньої роботи збирається з тим самим стандартом, що й лекційні приклади.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED IMPORTED_TARGET libgpiod)

find_package(Boost REQUIRED CONFIG COMPONENTS program_options)

add_executable(homework11)

target_sources(
    homework11
    PRIVATE include/Coords.hpp
            include/Utils.h
            include/DroneConfig.h
            include/DroneAbstractions.h
            include/MissionProcessor.h
            include/ParseArgs.hpp
            include/drone_link.h
            include/GpioOutput.h
            include/UartPort.h
            include/interfaces/IBallisticSolver.h
            include/interfaces/ILogger.h
            include/interfaces/IDroneState.h
            include/interfaces/DroneStateContext.h
            include/solvers/AnalyticalSolver.h
            include/solvers/CommonSolver.h
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
            src/GpioOutput.cpp
            src/UartPort.cpp
            src/solvers/AnalyticalSolver.cpp
            src/solvers/CommonSolver.cpp
            src/solvers/SolverFactory.cpp
            src/loggers/JsonLogger.cpp
            src/loggers/LoggerFactory.cpp
            src/droneStates/StateAccelerating.cpp
            src/droneStates/StateDecelerating.cpp
            src/droneStates/StateMoving.cpp
            src/droneStates/StateStopped.cpp
            src/droneStates/StateTurning.cpp
            src/main.cpp)

target_include_directories(homework11 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_options(homework11 PRIVATE -Wall -Wextra -pedantic)

find_package(nlohmann_json REQUIRED)

target_link_libraries(homework11 PRIVATE nlohmann_json::nlohmann_json PkgConfig::GPIOD
                                         Boost::program_options)
