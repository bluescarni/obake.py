# Configure the version file.
# NOTE: do it in the CMAKE_BINARY_DIR,
# will move it later.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/_version.py.in" "${CMAKE_BINARY_DIR}/_version.py" @ONLY)

# The list of obake.py's Python files.
set(OBAKE_PY_PYTHON_FILES __init__.py test.py)

# Try to understand if the generator is multi-configuration
# (e.g., MSVC).
get_property(OBAKE_PY_GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(OBAKE_PY_GENERATOR_IS_MULTI_CONFIG)
    message(STATUS "Multi-configuration generator detected.")
endif()

# Copy the python files in the current binary dir,
# so that we can import obake from the build dir.
if(OBAKE_PY_GENERATOR_IS_MULTI_CONFIG)
    foreach(OBAKE_PY_PYTHON_FILE ${OBAKE_PY_PYTHON_FILES})
        file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${OBAKE_PY_PYTHON_FILE}"
            INPUT "${CMAKE_CURRENT_SOURCE_DIR}/${OBAKE_PY_PYTHON_FILE}")
    endforeach()
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/_version.py"
        INPUT "${CMAKE_BINARY_DIR}/_version.py")
else()
    foreach(OBAKE_PY_PYTHON_FILE ${OBAKE_PY_PYTHON_FILES})
        file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${OBAKE_PY_PYTHON_FILE}"
            INPUT "${CMAKE_CURRENT_SOURCE_DIR}/${OBAKE_PY_PYTHON_FILE}")
    endforeach()
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/_version.py"
        INPUT "${CMAKE_BINARY_DIR}/_version.py")
endif()

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
