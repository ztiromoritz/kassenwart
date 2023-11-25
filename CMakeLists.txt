cmake_minimum_required(VERSION 3.10)
project(kassenwart C)

# for the lsp
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


file (GLOB _all_libs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.c main.c)

add_executable(kassenwart ${_all_libs})


# https://github.com/nicowilliams/inplace/issues/6
# asprintf needed this
set(_GNU_SOURCE 1)

# ncurses
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

#lua
add_subdirectory(external/lua)

target_link_libraries(kassenwart 
	PUBLIC m
	PRIVATE lua::lib
	${CURSES_LIBRARIES}
	)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

## debugging and learning
## kilo / https://viewsourcecode.org/snaptoken/kilo
add_executable(editor src/trie.c src/input.c src/utils.c src/raw.c editor.c)
add_executable(input-test src/trie.c src/input.c src/utils.c src/raw.c input-test.c)
add_executable(trie-test src/trie.c trie-test.c)

## Experiment with reading and writing key strokes
add_executable(readlog scratchpad/readlog.c)

