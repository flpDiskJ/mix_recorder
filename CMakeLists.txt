cmake_minimum_required(VERSION 3.8)
project(mix_rec)

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)

set(SRC ${CMAKE_SOURCE_DIR}/src)

add_executable(mix_rec
	${SRC}/main.cpp
)
target_include_directories(
	mix_rec PUBLIC
	${SDL2_INCLUDE_DIRS}
)
target_link_libraries(
	mix_rec PUBLIC
	${SDL2_LIBRARIES}
)

install(TARGETS mix_rec
  COMPONENT linapp
  RUNTIME DESTINATION "/bin/"
  LIBRARY DESTINATION "/lib/"
  DESTINATION "/bin/"
  )

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "KK")
INCLUDE(CPack)
