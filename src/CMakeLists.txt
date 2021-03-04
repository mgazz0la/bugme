add_library(register INTERFACE)
target_include_directories(register INTERFACE .)

add_library(cpu INTERFACE)
target_include_directories(register INTERFACE .)
target_link_libraries(cpu INTERFACE register)

add_executable(gbc main.cc)
target_link_libraries(gbc LINK_PRIVATE cpu)
install(TARGETS gbc DESTINATION bin)
