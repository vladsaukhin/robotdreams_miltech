BUILD_DIR := build/debug

.PHONY: configure build format lint test quality clean

configure:
	cmake --preset debug

build: configure
	cmake --build --preset debug

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