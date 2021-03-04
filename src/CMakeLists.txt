add_library(register INTERFACE)
target_include_directories(register INTERFACE .)

add_library(cpu cpu.cc)
target_include_directories(cpu PUBLIC .)
target_link_libraries(cpu LINK_PRIVATE register)

add_executable(gbc main.cc)
target_link_libraries(gbc LINK_PRIVATE cpu)
install(TARGETS gbc DESTINATION bin)
