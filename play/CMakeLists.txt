add_executable(preprocess preprocess.cpp)
target_link_libraries(preprocess ancl)

add_executable(ancl-cli ancl.cpp)
target_link_libraries(ancl-cli ancl)
