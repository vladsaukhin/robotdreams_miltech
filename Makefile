BUILD_DIR := build/debug
JOBS := 8

.PHONY: configure build format lint test quality clean

configure:
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

build: configure
	cmake --build $(BUILD_DIR) --parallel $(JOBS)

# extend here if you want to include another HW sub project
PROJECTS := homework_06

format:
	find $(PROJECTS) \
		\( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) \
		-print0 | xargs -0 clang-format -i
	find $(PROJECTS) \
		\( -name "CMakeLists.txt" -o -name "*.cmake" \) \
		-print0 | xargs -0 cmake-format -i

# extend here if you want to include another HW sub project
PROJECT_REGEX := .*homework_06/.*

lint: build
	run-clang-tidy -p $(BUILD_DIR) -j $(JOBS) $(PROJECT_REGEX)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

quality: format lint test

clean:
	rm -rf build

rebuild: clean build