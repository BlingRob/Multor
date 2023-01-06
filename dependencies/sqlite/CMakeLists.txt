cmake_minimum_required (VERSION 3.8)
project("SQLLite")

add_library(${PROJECT_NAME} STATIC
        "sqlite3.h" 
        "sqlite3.c"
        "sqlite3ext.h"
	"fileio.c"
	"test_windirent.h"
	"test_windirent.c") 

#target_include_directories(${PROJECT_NAME} PUBLIC "include")
add_executable("shell" "sqlite3.h" "shell.c" "sqlite3.c")