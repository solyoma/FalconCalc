set(PROJECT_NAME FalconCalc)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "about.h"
    "calculate.h"
    "clipboard.h"
    "defines.h"
    "graphictext.h"
    "help.h"
    "histopt.h"
    "history.h"
    "LongNumber.h"
    "mainForm.h.h"
    "resource.h"
    "SmartString.h"
    "stdafx_lc.h"
    "TStringlist.h"
    "variables.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "about.cpp"
    "calculate.cpp"
    "graphictext.cpp"
    "help.cpp"
    "histopt.cpp"
    "history.cpp"
    "FalconCalc.cpp"
    "LongNumber.cpp"
    "mainForm.h.cpp"
    "SmartString.cpp"
    "stdafx_lc.cpp"
    "TStringList.cpp"
    "variables.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h>"
)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
################################################################################
# Includes for CMake from *.props
################################################################################
use_props(${PROJECT_NAME} Debug             "Zoli.cmake")
use_props(${PROJECT_NAME} DebugLibraryBuild "Zoli.cmake")
use_props(${PROJECT_NAME} LibraryBuild      "Zoli.cmake")
use_props(${PROJECT_NAME} Release           "Zoli.cmake")

set(ROOT_NAMESPACE FalconCalc)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_LIBRARYBUILD      "TRUE"
    INTERPROCEDURAL_OPTIMIZATION_RELEASE           "TRUE"
)
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
    $<$<CONFIG:Debug>:
        MultiThreadedDebugDLL
    >
    $<$<CONFIG:DebugLibraryBuild>:
        MultiThreadedDebugDLL
    >
    $<$<CONFIG:LibraryBuild>:
        MultiThreaded
    >
    $<$<CONFIG:Release>:
        MultiThreaded
    >
    $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:DebugLibraryBuild>,$<CONFIG:LibraryBuild>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
)
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "$<$<CONFIG:LibraryBuild>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/../mycalc++"
    ">"
    "$<$<CONFIG:Release>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/../mycalc++"
    ">"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../zoli/nlib/ablakok;"
    "${CMAKE_CURRENT_SOURCE_DIR}/."
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG"
    ">"
    "$<$<CONFIG:DebugLibraryBuild>:"
        "_DEBUG"
    ">"
    "$<$<CONFIG:LibraryBuild>:"
        "NDEBUG"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "_CRT_SECURE_NO_WARNINGS;"
    "_N_LIB;"
    "UNICODE;"
    "_UNICODE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /GL-;
            /Od;
            /ZI;
            /Y-
        >
        $<$<CONFIG:DebugLibraryBuild>:
            /GL-;
            /Od;
            /ZI;
            /Y-
        >
        $<$<CONFIG:LibraryBuild>:
            /O2;
            /Oi;
            /Gy;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
        >
        $<$<CONFIG:Release>:
            /O2;
            /Oi;
            /Gy;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
        >
        /W3;
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /DEBUG;
            /INCREMENTAL
        >
        $<$<CONFIG:DebugLibraryBuild>:
            /DEBUG;
            /INCREMENTAL
        >
        $<$<CONFIG:LibraryBuild>:
            /OPT:REF;
            /OPT:ICF;
            /INCREMENTAL:NO
        >
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF;
            /INCREMENTAL:NO
        >
        /SUBSYSTEM:WINDOWS
    )
endif()

################################################################################
# Dependencies
################################################################################
add_dependencies(${PROJECT_NAME}
    ablakok
)

set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "$<$<CONFIG:Debug>:"
        "debugablakok"
    ">"
    "$<$<CONFIG:DebugLibraryBuild>:"
        "debugablakok"
    ">"
    "$<$<CONFIG:LibraryBuild>:"
        "ablakok"
    ">"
    "$<$<CONFIG:Release>:"
        "ablakok"
    ">"
    "ComCtl32;"
    "GdiPlus;"
    "Uxtheme;"
    "urlmon"
)
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

target_link_directories(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "$ENV{LibraryPath};"
        "$ENV{AdditionalLibraryDirectories}"
    ">"
    "$<$<CONFIG:DebugLibraryBuild>:"
        "$ENV{LibraryPath};"
        "$ENV{AdditionalLibraryDirectories}"
    ">"
)



























set(PROJECT_NAME FalconCalc)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "about.h"
    "calculate.h"
    "clipboard.h"
    "defines.h"
    "graphictext.h"
    "help.h"
    "histopt.h"
    "history.h"
    "LongNumber.h"
    "mainForm.h.h"
    "resource.h"
    "SmartString.h"
    "stdafx_lc.h"
    "TStringlist.h"
    "variables.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "about.cpp"
    "calculate.cpp"
    "graphictext.cpp"
    "help.cpp"
    "histopt.cpp"
    "history.cpp"
    "FalconCalc.cpp"
    "LongNumber.cpp"
    "mainForm.h.cpp"
    "SmartString.cpp"
    "stdafx_lc.cpp"
    "TStringList.cpp"
    "variables.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h>"
)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
################################################################################
# Includes for CMake from *.props
################################################################################
use_props(${PROJECT_NAME} Debug             "Zoli.cmake")
use_props(${PROJECT_NAME} DebugLibraryBuild "Zoli.cmake")
use_props(${PROJECT_NAME} LibraryBuild      "Zoli.cmake")
use_props(${PROJECT_NAME} Release           "Zoli.cmake")

set(ROOT_NAMESPACE FalconCalc)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_LIBRARYBUILD      "TRUE"
    INTERPROCEDURAL_OPTIMIZATION_RELEASE           "TRUE"
)
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
    $<$<CONFIG:Debug>:
        MultiThreadedDebugDLL
    >
    $<$<CONFIG:DebugLibraryBuild>:
        MultiThreadedDebugDLL
    >
    $<$<CONFIG:LibraryBuild>:
        MultiThreaded
    >
    $<$<CONFIG:Release>:
        MultiThreaded
    >
    $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:DebugLibraryBuild>,$<CONFIG:LibraryBuild>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
)
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "$<$<CONFIG:LibraryBuild>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/../mycalc++"
    ">"
    "$<$<CONFIG:Release>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/../mycalc++"
    ">"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../zoli/nlib/ablakok;"
    "${CMAKE_CURRENT_SOURCE_DIR}/."
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG"
    ">"
    "$<$<CONFIG:DebugLibraryBuild>:"
        "_DEBUG"
    ">"
    "$<$<CONFIG:LibraryBuild>:"
        "NDEBUG"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "_CRT_SECURE_NO_WARNINGS;"
    "_N_LIB;"
    "UNICODE;"
    "_UNICODE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /GL-;
            /Od;
            /ZI;
            /Y-
        >
        $<$<CONFIG:DebugLibraryBuild>:
            /GL-;
            /Od;
            /ZI;
            /Y-
        >
        $<$<CONFIG:LibraryBuild>:
            /O2;
            /Oi;
            /Gy;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
        >
        $<$<CONFIG:Release>:
            /O2;
            /Oi;
            /Gy;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
        >
        /W3;
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /DEBUG;
            /INCREMENTAL
        >
        $<$<CONFIG:DebugLibraryBuild>:
            /DEBUG;
            /INCREMENTAL
        >
        $<$<CONFIG:LibraryBuild>:
            /OPT:REF;
            /OPT:ICF;
            /INCREMENTAL:NO
        >
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF;
            /INCREMENTAL:NO
        >
        /SUBSYSTEM:WINDOWS
    )
endif()

################################################################################
# Dependencies
################################################################################
add_dependencies(${PROJECT_NAME}
    ablakok
)

set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "$<$<CONFIG:Debug>:"
        "debugablakok"
    ">"
    "$<$<CONFIG:DebugLibraryBuild>:"
        "debugablakok"
    ">"
    "$<$<CONFIG:LibraryBuild>:"
        "ablakok"
    ">"
    "$<$<CONFIG:Release>:"
        "ablakok"
    ">"
    "ComCtl32;"
    "GdiPlus;"
    "Uxtheme;"
    "urlmon"
)
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

target_link_directories(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "$ENV{LibraryPath};"
        "$ENV{AdditionalLibraryDirectories}"
    ">"
    "$<$<CONFIG:DebugLibraryBuild>:"
        "$ENV{LibraryPath};"
        "$ENV{AdditionalLibraryDirectories}"
    ">"
)

