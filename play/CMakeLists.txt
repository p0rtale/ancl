add_executable(preprocess preprocess.cpp)
target_link_libraries(preprocess ancl)

add_executable(parse parse.cpp)
target_link_libraries(parse ancl)

add_executable(ir ir.cpp)
target_link_libraries(ir ancl)

add_executable(ancl-cli ancl.cpp)
target_link_libraries(ancl-cli ancl)
