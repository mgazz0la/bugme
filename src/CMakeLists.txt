add_library(register INTERFACE)
target_include_directories(register INTERFACE .)

add_library(display INTERFACE)
target_include_directories(display INTERFACE .)

add_library(log log.cc)
target_include_directories(log PUBLIC .)

add_library(mmu mmu.cc)
target_include_directories(mmu PUBLIC .)
target_link_libraries(mmu LINK_PRIVATE cpu log)

add_library(timer timer.cc)
target_include_directories(timer PUBLIC .)
target_link_libraries(timer LINK_PRIVATE register log)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
add_library(sdl_display sdl_display.cc)
target_include_directories(sdl_display PUBLIC .)
target_link_libraries(sdl_display ${SDL2_LIBRARY} display log)

add_library(ppu ppu.cc)
target_include_directories(ppu PUBLIC .)
target_link_libraries(ppu LINK_PRIVATE display log)

add_library(cpu cpu.cc opcode.cc)
target_include_directories(cpu PUBLIC .)
target_link_libraries(cpu LINK_PRIVATE display log mmu ppu timer register)


add_library(gbccore gbc.cc)
target_include_directories(gbccore PUBLIC .)
target_link_libraries(gbccore LINK_PRIVATE display cpu log mmu timer ppu)

add_executable(gbc main.cc)
target_link_libraries(gbc LINK_PRIVATE gbccore sdl_display)
install(TARGETS gbc DESTINATION bin)
