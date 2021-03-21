all: compile

cmake:
	@mkdir -p build
	@cd build && cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1

compile: cmake
	@cd build && make

clean:
	@rm -rf build

.PHONY: cmake compile clean
