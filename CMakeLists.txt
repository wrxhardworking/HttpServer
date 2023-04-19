cmake_minimum_required(VERSION 3.0.0)


project(HttpServer VERSION 0.1.1)


set(CMAKE_CXX_STANDARD 17)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})


set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})


file(GLOB SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)


add_library(${PROJECT_NAME} ${SRC})


target_link_libraries(
        ${PROJECT_NAME}
        pthread
        ${CMAKE_THREAD_LIBS_INIT}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)


add_executable(main main.cpp )


message(${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(
        main
        ${PROJECT_NAME}
)