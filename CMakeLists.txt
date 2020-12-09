cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
SET(CMAKE_VERBOSE_MAKEFILE on)
project(gifmd5 LANGUAGES C)
add_executable(
gifmd5 main.c
darray.c
lzw_bits.c
lzw_table.c
lzw.c
)
