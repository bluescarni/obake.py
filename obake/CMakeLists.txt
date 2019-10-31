# Configure the version file.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/_version.py.in" "${CMAKE_CURRENT_BINARY_DIR}/_version.py" @ONLY)

# Copy the python files in the current binary dir,
# so that we can import obake from the build dir.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/__init__.py" "${CMAKE_CURRENT_BINARY_DIR}/__init__.py" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test.py" "${CMAKE_CURRENT_BINARY_DIR}/test.py" COPYONLY)

# Core module.
YACMA_PYTHON_MODULE(core
    core.cpp
    type_system.cpp
    utils.cpp
    docstrings.cpp
    expose_polynomials.cpp
    expose_polynomials_double.cpp
    expose_polynomials_integer.cpp
    expose_polynomials_rational.cpp
    expose_polynomials_real128.cpp
    expose_polynomials_real.cpp
)

target_link_libraries(core PRIVATE obake::obake)
target_include_directories(core PRIVATE "${pybind11_INCLUDE_DIR}" ${Boost_INCLUDE_DIRS})
target_compile_definitions(core PRIVATE "${pybind11_DEFINITIONS}")
if(WIN32)
    # NOTE: the imported obake target already brings
    # the necessary Boost libraries into the link chain.
    # No need to use the Boost pragmas for auto-linking.
    target_compile_definitions(core PRIVATE "BOOST_ALL_NO_LIB")
endif()
target_compile_options(core PRIVATE
    "$<$<CONFIG:DEBUG>:${OBAKE_PY_CXX_FLAGS_DEBUG}>"
    "$<$<CONFIG:RELEASE>:${OBAKE_PY_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:RelWithDebInfo>:${OBAKE_PY_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:MinSizeRel>:${OBAKE_PY_CXX_FLAGS_RELEASE}>"
)
set_target_properties(core PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(core PROPERTIES VISIBILITY_INLINES_HIDDEN TRUE)
set_property(TARGET core PROPERTY CXX_STANDARD 17)
set_property(TARGET core PROPERTY CXX_STANDARD_REQUIRED YES)
set_property(TARGET core PROPERTY CXX_EXTENSIONS NO)
