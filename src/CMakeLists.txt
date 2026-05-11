set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB source_files *.cpp *.h *.hpp gpx/*.cpp gpx/*.h gpx/*.hpp poi/*.cpp poi/*.h poi/*.hpp)

set(CURRENT_LIB_NAME my_gpx_lib)

add_library(${CURRENT_LIB_NAME} SHARED ${source_files})

# target_include_directories(${CURRENT_LIB_NAME} PRIVATE ${LARDOS_EXTERNAL_INCLUDE_DIRS} ${LARDOS_INCLUDE_DIR})
target_link_libraries(${CURRENT_LIB_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${CURRENT_LIB_NAME} PRIVATE tinyxml2::tinyxml2)
target_include_directories(${CURRENT_LIB_NAME} PRIVATE ${RAPIDCSV_INCLUDE_DIRS})
target_link_libraries(${CURRENT_LIB_NAME} PRIVATE httplib::httplib OpenSSL::SSL OpenSSL::Crypto)
# Source - https://stackoverflow.com/a/77275052
# Posted by baziorek
# Retrieved 2026-04-11, License - CC BY-SA 4.0
target_link_libraries(${CURRENT_LIB_NAME} PRIVATE $<IF:$<TARGET_EXISTS:SOCI::soci_core>,SOCI::soci_core,SOCI::soci_core_static>)
target_link_libraries(${CURRENT_LIB_NAME} PRIVATE $<IF:$<TARGET_EXISTS:SOCI::soci_sqlite3>,SOCI::soci_sqlite3,SOCI::soci_sqlite3_static>)


set_target_properties(${CURRENT_LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_compile_definitions(${CURRENT_LIB_NAME}
    PRIVATE MY_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)
list(APPEND GPX_LIBRARIES ${CURRENT_LIB_NAME})
set(GPX_LIBRARIES ${GPX_LIBRARIES} PARENT_SCOPE)