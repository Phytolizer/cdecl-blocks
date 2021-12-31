cmake_minimum_required(VERSION 3.12...3.23)

project(
  cdecl
  LANGUAGES C
  VERSION 0.1.0
  DESCRIPTION "Describe a C declaration in English"
  HOMEPAGE_URL "https://github.com/Phytolizer/cdecl-blocks"
)

add_library(vec src/vec.c)
target_include_directories(vec PUBLIC inc)
add_executable(cdecl src/cdecl.c src/cdlex.c src/cdgram.c)
target_include_directories(cdecl PUBLIC inc)
target_link_libraries(cdecl PUBLIC vec)
