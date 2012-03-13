INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src
  ${VLE_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS})

LINK_DIRECTORIES(
  ${VLE_LIBRARY_DIRS}
  ${Boost_LIBRARY_DIRS})

ADD_LIBRARY(logistics SHARED ArrivalGenerator.cpp Container.hpp Decision.cpp
  Dispatch.cpp Split.cpp Transit.cpp Transport.hpp TransportGenerator.cpp)

TARGET_LINK_LIBRARIES(logistics
  ${VLE_LIBRARIES}
  ${Boost_LIBRARIES})

INSTALL(TARGETS logistics
  RUNTIME DESTINATION lib
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
