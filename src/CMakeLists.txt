add_library(register INTERFACE)
target_include_directories(register INTERFACE .)

add_library(cpu cpu.cc opcode.cc)
target_include_directories(cpu PUBLIC .)
target_link_libraries(cpu LINK_PRIVATE register mmu)

add_library(mmu mmu.cc)
target_include_directories(mmu PUBLIC .)
target_link_libraries(mmu LINK_PRIVATE cpu)

add_library(gbccore gbc.cc)
target_include_directories(gbccore PUBLIC .)
target_link_libraries(gbccore LINK_PRIVATE cpu mmu)

add_executable(gbc main.cc)
target_link_libraries(gbc LINK_PRIVATE gbccore)
install(TARGETS gbc DESTINATION bin)
