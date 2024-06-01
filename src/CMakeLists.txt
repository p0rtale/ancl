add_executable(ancl-cli main.cpp)
target_link_libraries(ancl-cli ancl)

add_executable(debug debug.cpp)
target_link_libraries(debug ancl)
