include_directories(.)
add_subdirectory(ppu)
add_subdirectory(cpu)

add_library(log log.cc)

add_library(options options.cc)
target_link_libraries(options LINK_PRIVATE log)

add_library(memory memory.cc)
target_link_libraries(memory LINK_PRIVATE log)

add_library(cartridge cartridge.cc)
target_link_libraries(cartridge LINK_PRIVATE log)

#add_library(debug debug.cc)
#target_link_libraries(debug LINK_PRIVATE log)

add_library(timer timer.cc)
target_link_libraries(timer LINK_PRIVATE log)

add_library(joypad joypad.cc)
target_link_libraries(joypad LINK_PRIVATE log)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
add_library(sdl_display sdl_display.cc)
target_link_libraries(sdl_display ${SDL2_LIBRARY} log)

add_library(bugmecore gbc.cc)
target_link_libraries(bugmecore LINK_PRIVATE ${SDL2_LIBRARY} cartridge cpu log memory timer ppu)

add_executable(bugme main.cc)
target_link_libraries(bugme LINK_PRIVATE bugmecore sdl_display options)
install(TARGETS bugme DESTINATION bin)
