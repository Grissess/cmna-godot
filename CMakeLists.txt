cmake_minimum_required(VERSION 3.18...3.22)

project(cmna-godot
	VERSION 0.1
	DESCRIPTION "A Godot binding for cmna"
	LANGUAGES C)

find_package(cmna REQUIRED)
find_package(LAPACK REQUIRED)  # FIXME: actually a dependency

add_subdirectory(src)
