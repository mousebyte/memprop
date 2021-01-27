cmake_minimum_required(VERSION 3.17)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/Catch2/extras/")
project("memprop" VERSION 1.0.0 LANGUAGES CXX)
option(SIGSLOT_COMPILE_EXAMPLES "" OFF)
option(SIGSLOT_COMPILE_TESTS "" OFF)
option(MEMPROP_COMPILE_TESTS "Compile tests" ON)
add_subdirectory(lib/sigslot)

add_library(${PROJECT_NAME} INTERFACE)
add_library(Mousebyte::memprop ALIAS ${PROJECT_NAME})

include(GNUInstallDirs)
target_include_directories(${PROJECT_NAME} INTERFACE
            $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(${PROJECT_NAME} INTERFACE Pal::Sigslot)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES EXPORT_NAME memprop)

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)
configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION
  ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(EXPORT ${PROJECT_NAME}_Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE Mousebyte::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/memprop DESTINATION include)
export(TARGETS ${PROJECT_NAME}
    NAMESPACE Mousebyte::
    FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake
)
if(MEMPROP_COMPILE_TESTS)
    add_subdirectory(lib/Catch2)
    add_executable(memprop_tests tests/tests.cpp)
    target_link_libraries(memprop_tests PRIVATE Catch2::Catch2WithMain Mousebyte::memprop)
    include(CTest)
    include(Catch)
    catch_discover_tests(memprop_tests)
    add_custom_command(TARGET memprop_tests
                       POST_BUILD
                       COMMAND ctest --output-on-failure)
endif()
