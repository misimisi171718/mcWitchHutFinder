include_directories(../lib/CppBenchmark/include)
link_directories(${PROJECT_SOURCE_DIR}/lib/CppBenchmark/bin)
add_executable(getFetureVsGetSructure getFetureVsGetSructure.cpp)
target_link_libraries(getFetureVsGetSructure cppbenchmark cubiomes)
