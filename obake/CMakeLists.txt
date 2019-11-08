# Configure the version file.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/_version.py.in" "${CMAKE_CURRENT_BINARY_DIR}/_version.py" @ONLY)

# The list of obake.py's Python files.
set(OBAKE_PY_PYTHON_FILES __init__.py test.py)

# Copy the python files in the current binary dir,
# so that we can import obake from the build dir.
# NOTE: importing from the build dir will work
# only on single-configuration generators.
foreach(OBAKE_PY_PYTHON_FILE ${OBAKE_PY_PYTHON_FILES})
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${OBAKE_PY_PYTHON_FILE}"
        "${CMAKE_CURRENT_BINARY_DIR}/${OBAKE_PY_PYTHON_FILE}" COPYONLY)
endforeach()

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
    "$<$<CONFIG:Debug>:${OBAKE_PY_CXX_FLAGS_DEBUG}>"
    "$<$<CONFIG:Release>:${OBAKE_PY_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:RelWithDebInfo>:${OBAKE_PY_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:MinSizeRel>:${OBAKE_PY_CXX_FLAGS_RELEASE}>"
)
set_target_properties(core PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(core PROPERTIES VISIBILITY_INLINES_HIDDEN TRUE)
if(NOT CMAKE_CXX_STANDARD)
    # The user did not provide the CMAKE_CXX_STANDARD variable,
    # go with the default (C++17).
    message(STATUS "Setting the C++ standard version to the default value (17).")
    set_property(TARGET core PROPERTY CXX_STANDARD 17)
else()
    message(STATUS "Using the manually-specified value for the C++ standard version (${CMAKE_CXX_STANDARD}).")
endif()
set_property(TARGET core PROPERTY CXX_STANDARD_REQUIRED YES)
set_property(TARGET core PROPERTY CXX_EXTENSIONS NO)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.9.0")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _OBAKE_PY_IPO_RESULT OUTPUT _OBAKE_PY_IPO_OUTPUT)

    if (_OBAKE_PY_IPO_RESULT)
        if (OBAKE_PY_DISABLE_IPO)
            message(STATUS "IPO is supported, but it has been disabled for this build.")
        else()
            message(STATUS "IPO is supported, enabling.")
            # NOTE: it does not seem like we can use generator
            # expressions in this property, unfortunately.
            set_property(TARGET core PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        endif()
    else()
        message(STATUS "IPO is not supported: ${_OBAKE_PY_IPO_OUTPUT}")
    endif()

    unset(_OBAKE_PY_IPO_RESULT)
    unset(_OBAKE_PY_IPO_OUTPUT)
endif()

# Setup the installation path.
set(OBAKE_PY_INSTALL_PATH "${YACMA_PYTHON_MODULES_INSTALL_PATH}/obake")

# Install the core module.
install(TARGETS core
    RUNTIME DESTINATION ${OBAKE_PY_INSTALL_PATH}
    LIBRARY DESTINATION ${OBAKE_PY_INSTALL_PATH}
)

# Add the Python files.
install(FILES ${OBAKE_PY_PYTHON_FILES} "${CMAKE_CURRENT_BINARY_DIR}/_version.py"
    DESTINATION ${OBAKE_PY_INSTALL_PATH})
