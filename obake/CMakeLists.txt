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
Python3_add_library(core MODULE WITH_SOABI
    core.cpp
    type_system.cpp
    utils.cpp
    docstrings.cpp
    # Polynomials.
    expose_polynomials.cpp
    expose_polynomials_double.cpp
    expose_polynomials_rational.cpp
    expose_polynomials_real128.cpp
    expose_polynomials_real.cpp
    # Power series.
    expose_power_series.cpp
    expose_power_series_double.cpp
    expose_power_series_rational.cpp
    expose_power_series_real128.cpp
    expose_power_series_real.cpp
)

target_link_libraries(core PRIVATE obake::obake)
target_include_directories(core SYSTEM PRIVATE "${pybind11_INCLUDE_DIR}" ${Boost_INCLUDE_DIRS})
target_compile_definitions(core PRIVATE "${pybind11_DEFINITIONS}")
target_compile_options(core PRIVATE
    "$<$<CONFIG:Debug>:${OBAKE_PY_CXX_FLAGS_DEBUG}>"
    "$<$<CONFIG:Release>:${OBAKE_PY_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:RelWithDebInfo>:${OBAKE_PY_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:MinSizeRel>:${OBAKE_PY_CXX_FLAGS_RELEASE}>"
)
set_target_properties(core PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(core PROPERTIES VISIBILITY_INLINES_HIDDEN TRUE)
target_compile_features(core PRIVATE cxx_std_20)
set_property(TARGET core PROPERTY CXX_EXTENSIONS NO)

if (OBAKE_PY_ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _OBAKE_PY_IPO_RESULT OUTPUT _OBAKE_PY_IPO_OUTPUT)
    if (_OBAKE_PY_IPO_RESULT)
        message(STATUS "IPO requested and supported, enabling.")
        set_property(TARGET core PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "IPO requested, but it is not supported by the compiler:\n${_OBAKE_PY_IPO_OUTPUT}")
    endif()
    unset(_OBAKE_PY_IPO_RESULT)
    unset(_OBAKE_PY_IPO_OUTPUT)
endif()

# Installation setup.
if(OBAKE_PY_INSTALL_PATH STREQUAL "")
    message(STATUS "obake.py will be installed in the default location: ${Python3_SITEARCH}")
    set(_OBAKE_PY_INSTALL_DIR "${Python3_SITEARCH}/obake")
else()
    message(STATUS "obake.py will be installed in the custom location: ${OBAKE_PY_INSTALL_PATH}")
    set(_OBAKE_PY_INSTALL_DIR "${OBAKE_PY_INSTALL_PATH}/obake")
endif()

# Install the core module.
install(TARGETS core
    RUNTIME DESTINATION ${_OBAKE_PY_INSTALL_DIR}
    LIBRARY DESTINATION ${_OBAKE_PY_INSTALL_DIR}
)

# Add the Python files.
install(FILES ${OBAKE_PY_PYTHON_FILES} "${CMAKE_CURRENT_BINARY_DIR}/_version.py"
    DESTINATION ${_OBAKE_PY_INSTALL_DIR})

unset(_OBAKE_PY_INSTALL_DIR)
