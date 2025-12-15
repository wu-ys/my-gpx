set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB source_files *.cpp *.h *.hpp)

set(CURRENT_LIB_NAME my_gpx_lib)

add_library(${CURRENT_LIB_NAME} SHARED ${source_files})

# target_include_directories(${CURRENT_LIB_NAME} PRIVATE ${LARDOS_EXTERNAL_INCLUDE_DIRS} ${LARDOS_INCLUDE_DIR})
target_link_libraries(${CURRENT_LIB_NAME} PRIVATE tinyxml2::tinyxml2)
target_include_directories(${CURRENT_LIB_NAME} PRIVATE ${RAPIDCSV_INCLUDE_DIRS})
target_link_libraries(${CURRENT_LIB_NAME} PRIVATE httplib::httplib OpenSSL::SSL OpenSSL::Crypto)

set_target_properties(${CURRENT_LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_compile_definitions(${CURRENT_LIB_NAME}
    PRIVATE MY_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)
list(APPEND GPX_LIBRARIES ${CURRENT_LIB_NAME})
set(GPX_LIBRARIES ${GPX_LIBRARIES} PARENT_SCOPE)