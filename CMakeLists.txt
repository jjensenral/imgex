cmake_minimum_required(VERSION 3.11)
project(imgex VERSION 0.01 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Gui REQUIRED)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(fmt)

add_executable(imgex
  src/main.cc
  src/image.cc
  src/decor.cc
  src/session.cc
  src/transform.cc
  src/xwin.cc src/qbox.hh)

target_compile_features(imgex PUBLIC cxx_std_20)
# Doesn't seem to be a compile feature in cmake yet?
add_compile_options(imgex PUBLIC -fconcepts)
target_link_libraries(imgex Qt5::Gui)
target_link_libraries(imgex fmt::fmt)
