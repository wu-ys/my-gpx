set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB source_files *.cpp)

# if (LARDOS_CUDA AND NOT LARDOS_TORCH)
#     file(GLOB source_files *.cpp *.cu)
# else (LARDOS_CUDA AND NOT LARDOS_TORCH)
#     file(GLOB source_files *.cpp)
# endif()

foreach(term ${source_files})
    file(RELATIVE_PATH short_name ${CMAKE_CURRENT_SOURCE_DIR} ${term})

    string(LENGTH ${short_name} namelength)
    math(EXPR namelength4 "${namelength}-4")
    string(SUBSTRING ${short_name} 0 ${namelength4} new_short_name)
    add_executable(${new_short_name})
    target_sources(${new_short_name} PUBLIC ${short_name})

    target_include_directories(${new_short_name} PRIVATE ${GPX_INCLUDE_DIR})
    target_link_libraries(${new_short_name} PRIVATE ${GPX_LIBRARIES} tinyxml2::tinyxml2)
    # if (LARDOS_CUDA)
    #     target_link_libraries(${new_short_name} PRIVATE ${CUDA_LIBRARIES} ${CUBLAS_LIB} ${CUSPARSE_LIB} ${CUSOLVER_LIB})
    # endif()

    message("${new_short_name}")

endforeach()