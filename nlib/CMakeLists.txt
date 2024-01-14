set(PROJECT_NAME ablakok)

################################################################################
# Source groups
################################################################################
set(Designer_Forms
#    "designer/designer.cpp"
#    "designer/designer.h"
#    "designer/designerdialogs.cpp"
#    "designer/designerdialogs.h"
#    "designer/designeroptions.cpp"
#    "designer/designeroptions.h"
#    "designer/projectsettings.cpp"
#    "designer/projectsettings.h"
)
source_group("Designer Forms" FILES ${Designer_Forms})

set(Designer_Header_Files
#    "designer/designercontrols.h"
#    "designer/designerform.h"
#    "designer/designermenu.h"
#    "designer/designio.h"
#    "designer/designproperties.h"
#    "designer/designregistry.h"
#    "designer/property_buttons.h"
#    "designer/property_canvas.h"
#    "designer/property_controlbase.h"
#    "designer/property_dialog.h"
#    "designer/property_generalcontrol.h"
#    "designer/property_graphiccontrol.h"
#    "designer/property_gridbase.h"
#    "designer/property_imagelist.h"
#    "designer/property_images.h"
#    "designer/property_menu.h"
#    "designer/property_objectbase.h"
#    "designer/property_screen.h"
#    "designer/property_syscontrol.h"
#    "designer/property_toolbars.h"
#    "designer/resource.h"
#    "designer/resunzip.h"
#    "designer/serializer.h"
#    "zlib/zlib.h"
)
source_group("Designer Header Files" FILES ${Designer_Header_Files})

set(Designer_Source_Files
#    "designer/designercontrols.cpp"
#    "designer/designerform.cpp"
#    "designer/designermenu.cpp"
#    "designer/designio.cpp"
#    "designer/designproperties.cpp"
#    "designer/designregistry.cpp"
#    "designer/resunzip.cpp"
#    "designer/serializer.cpp"
#    "designer/stdafx.cpp"
)
source_group("Designer Source Files" FILES ${Designer_Source_Files})

set(Header_Files
    "application.h"
    "buttons.h"
    "canvas.h"
    "comdata.h"
    "controlbase.h"
    "dialog.h"
    "events.h"
    "filestream.h"
    "general.h"
    "generalcontrol.h"
    "genericfile.h"
    "graphiccontrol.h"
    "gridbase.h"
    "helperwindows.h"
    "imagelist.h"
    "images.h"
    "inifile.h"
    "kbd64.h"
    "menu.h"
    "objectbase.h"
    "registry.h"
    "screen.h"
    "sparse_list.h"
    "stdafx_zoli.h"
    "syscontrol.h"
    "themes.h"
#    "toolbars.h"
    "utility.h"
    "wexception.h"
    "windowfunctions.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "application.cpp"
    "buttons.cpp"
    "canvas.cpp"
    "comdata.cpp"
    "controlbase.cpp"
    "dialog.cpp"
    "filestream.cpp"
    "general.cpp"
    "generalcontrol.cpp"
    "graphiccontrol.cpp"
    "gridbase.cpp"
    "helperwindows.cpp"
    "imagelist.cpp"
    "images.cpp"
    "inifile.cpp"
    "main.cpp"
    "menu.cpp"
    "objectbase.cpp"
    "registry.cpp"
    "screen.cpp"
    "syscontrol.cpp"
    "themes.cpp"
#    "toolbars.cpp"
    "utility.cpp"
    "wexception.cpp"
    "windowfunctions.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Designer_Forms}
    ${Designer_Header_Files}
    ${Designer_Source_Files}
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE ablakok)

################################################################################
# Target name
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    TARGET_NAME_DEBUG             "debug${PROJECT_NAME}"
    TARGET_NAME_DEBUGLIBRARYBUILD "debug${PROJECT_NAME}"
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
    $<$<CONFIG:LibraryBuild>:
        MultiThreaded
    >
    $<$<CONFIG:Release>:
        MultiThreaded
    >
    $<$<NOT:$<OR:$<CONFIG:LibraryBuild>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
)
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "DEBUG"
    ">"
    "$<$<CONFIG:DebugLibraryBuild>:"
        "DEBUG"
    ">"
    "$<$<CONFIG:LibraryBuild>:"
        "NDEBUG"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "LIBBUILD;"
    "UNICODE;"
    "_UNICODE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /Od
        >
        $<$<CONFIG:DebugLibraryBuild>:
            /Od
        >
        $<$<CONFIG:LibraryBuild>:
            /O2;
            /Oi;
            /Gy
        >
        $<$<CONFIG:Release>:
            /O2;
            /Oi;
            /Gy
        >
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:LibraryBuild>:
            /OPT:REF;
            /OPT:ICF
        >
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /MACHINE:X86;
        /SUBSYSTEM:WINDOWS
    )
endif()

target_link_directories(${PROJECT_NAME} PUBLIC
    "$<$<CONFIG:Debug>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/zlibdebug"
    ">"
    "$<$<CONFIG:DebugLibraryBuild>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/zlibdebug"
    ">"
    "$<$<CONFIG:LibraryBuild>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/zlib"
    ">"
    "$<$<CONFIG:Release>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/zlib"
    ">"
)



























set(PROJECT_NAME ablakok)

################################################################################
# Source groups
################################################################################
set(Designer_Forms
#    "designer/designer.cpp"
#    "designer/designer.h"
#    "designer/designerdialogs.cpp"
#    "designer/designerdialogs.h"
#    "designer/designeroptions.cpp"
#    "designer/designeroptions.h"
#    "designer/projectsettings.cpp"
#    "designer/projectsettings.h"
)
source_group("Designer Forms" FILES ${Designer_Forms})

set(Designer_Header_Files
#    "designer/designercontrols.h"
#    "designer/designerform.h"
#    "designer/designermenu.h"
#    "designer/designio.h"
#    "designer/designproperties.h"
#    "designer/designregistry.h"
#    "designer/property_buttons.h"
#    "designer/property_canvas.h"
#    "designer/property_controlbase.h"
#    "designer/property_dialog.h"
#    "designer/property_generalcontrol.h"
#    "designer/property_graphiccontrol.h"
#    "designer/property_gridbase.h"
#    "designer/property_imagelist.h"
#    "designer/property_images.h"
#    "designer/property_menu.h"
#    "designer/property_objectbase.h"
#    "designer/property_screen.h"
#    "designer/property_syscontrol.h"
#    "designer/property_toolbars.h"
#    "designer/resource.h"
#    "designer/resunzip.h"
#    "designer/serializer.h"
#    "zlib/zlib.h"
)
source_group("Designer Header Files" FILES ${Designer_Header_Files})

set(Designer_Source_Files
#    "designer/designercontrols.cpp"
#    "designer/designerform.cpp"
#    "designer/designermenu.cpp"
#    "designer/designio.cpp"
#    "designer/designproperties.cpp"
#    "designer/designregistry.cpp"
#    "designer/resunzip.cpp"
#    "designer/serializer.cpp"
#    "designer/stdafx.cpp"
)
source_group("Designer Source Files" FILES ${Designer_Source_Files})

set(Header_Files
    "application.h"
    "buttons.h"
    "canvas.h"
    "comdata.h"
    "controlbase.h"
    "dialog.h"
    "events.h"
    "filestream.h"
    "general.h"
    "generalcontrol.h"
    "genericfile.h"
    "graphiccontrol.h"
    "gridbase.h"
    "helperwindows.h"
    "imagelist.h"
    "images.h"
    "inifile.h"
    "kbd64.h"
    "menu.h"
    "objectbase.h"
    "registry.h"
    "screen.h"
    "sparse_list.h"
    "stdafx_zoli.h"
    "syscontrol.h"
    "themes.h"
#    "toolbars.h"
    "utility.h"
    "wexception.h"
    "windowfunctions.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "application.cpp"
    "buttons.cpp"
    "canvas.cpp"
    "comdata.cpp"
    "controlbase.cpp"
    "dialog.cpp"
    "filestream.cpp"
    "general.cpp"
    "generalcontrol.cpp"
    "graphiccontrol.cpp"
    "gridbase.cpp"
    "helperwindows.cpp"
    "imagelist.cpp"
    "images.cpp"
    "inifile.cpp"
    "main.cpp"
    "menu.cpp"
    "objectbase.cpp"
    "registry.cpp"
    "screen.cpp"
    "syscontrol.cpp"
    "themes.cpp"
#    "toolbars.cpp"
    "utility.cpp"
    "wexception.cpp"
    "windowfunctions.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Designer_Forms}
    ${Designer_Header_Files}
    ${Designer_Source_Files}
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE ablakok)

################################################################################
# Target name
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    TARGET_NAME_DEBUG             "debug${PROJECT_NAME}"
    TARGET_NAME_DEBUGLIBRARYBUILD "debug${PROJECT_NAME}"
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
    $<$<CONFIG:LibraryBuild>:
        MultiThreaded
    >
    $<$<CONFIG:Release>:
        MultiThreaded
    >
    $<$<NOT:$<OR:$<CONFIG:LibraryBuild>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
)
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "DEBUG"
    ">"
    "$<$<CONFIG:DebugLibraryBuild>:"
        "DEBUG"
    ">"
    "$<$<CONFIG:LibraryBuild>:"
        "NDEBUG"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "LIBBUILD;"
    "UNICODE;"
    "_UNICODE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /Od
        >
        $<$<CONFIG:DebugLibraryBuild>:
            /Od
        >
        $<$<CONFIG:LibraryBuild>:
            /O2;
            /Oi;
            /Gy
        >
        $<$<CONFIG:Release>:
            /O2;
            /Oi;
            /Gy
        >
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:LibraryBuild>:
            /OPT:REF;
            /OPT:ICF
        >
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /MACHINE:X86;
        /SUBSYSTEM:WINDOWS
    )
endif()

target_link_directories(${PROJECT_NAME} PUBLIC
    "$<$<CONFIG:Debug>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/zlibdebug"
    ">"
    "$<$<CONFIG:DebugLibraryBuild>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/zlibdebug"
    ">"
    "$<$<CONFIG:LibraryBuild>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/zlib"
    ">"
    "$<$<CONFIG:Release>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/zlib"
    ">"
)



























set(PROJECT_NAME ablakok)

################################################################################
# Source groups
################################################################################
set(Designer_Forms
#    "designer/designer.cpp"
#    "designer/designer.h"
#    "designer/designerdialogs.cpp"
#    "designer/designerdialogs.h"
#    "designer/designeroptions.cpp"
#    "designer/designeroptions.h"
#    "designer/projectsettings.cpp"
#    "designer/projectsettings.h"
)
source_group("Designer Forms" FILES ${Designer_Forms})

set(Designer_Header_Files
#    "designer/designercontrols.h"
#    "designer/designerform.h"
#    "designer/designermenu.h"
#    "designer/designio.h"
#    "designer/designproperties.h"
#    "designer/designregistry.h"
#    "designer/property_buttons.h"
#    "designer/property_canvas.h"
#    "designer/property_controlbase.h"
#    "designer/property_dialog.h"
#    "designer/property_generalcontrol.h"
#    "designer/property_graphiccontrol.h"
#    "designer/property_gridbase.h"
#    "designer/property_imagelist.h"
#    "designer/property_images.h"
#    "designer/property_menu.h"
#    "designer/property_objectbase.h"
#    "designer/property_screen.h"
#    "designer/property_syscontrol.h"
#    "designer/property_toolbars.h"
#    "designer/resource.h"
#    "designer/resunzip.h"
#    "designer/serializer.h"
#    "zlib/zlib.h"
)
source_group("Designer Header Files" FILES ${Designer_Header_Files})

set(Designer_Source_Files
#    "designer/designercontrols.cpp"
#    "designer/designerform.cpp"
#    "designer/designermenu.cpp"
#    "designer/designio.cpp"
#    "designer/designproperties.cpp"
#    "designer/designregistry.cpp"
#    "designer/resunzip.cpp"
#    "designer/serializer.cpp"
#    "designer/stdafx.cpp"
)
source_group("Designer Source Files" FILES ${Designer_Source_Files})

set(Header_Files
    "application.h"
    "buttons.h"
    "canvas.h"
    "comdata.h"
    "controlbase.h"
    "dialog.h"
    "events.h"
    "filestream.h"
    "general.h"
    "generalcontrol.h"
    "genericfile.h"
    "graphiccontrol.h"
    "gridbase.h"
    "helperwindows.h"
    "imagelist.h"
    "images.h"
    "inifile.h"
    "kbd64.h"
    "menu.h"
    "objectbase.h"
    "registry.h"
    "screen.h"
    "sparse_list.h"
    "stdafx_zoli.h"
    "syscontrol.h"
    "themes.h"
#    "toolbars.h"
    "utility.h"
    "wexception.h"
    "windowfunctions.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "application.cpp"
    "buttons.cpp"
    "canvas.cpp"
    "comdata.cpp"
    "controlbase.cpp"
    "dialog.cpp"
    "filestream.cpp"
    "general.cpp"
    "generalcontrol.cpp"
    "graphiccontrol.cpp"
    "gridbase.cpp"
    "helperwindows.cpp"
    "imagelist.cpp"
    "images.cpp"
    "inifile.cpp"
    "main.cpp"
    "menu.cpp"
    "objectbase.cpp"
    "registry.cpp"
    "screen.cpp"
    "syscontrol.cpp"
    "themes.cpp"
#    "toolbars.cpp"
    "utility.cpp"
    "wexception.cpp"
    "windowfunctions.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Designer_Forms}
    ${Designer_Header_Files}
    ${Designer_Source_Files}
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE ablakok)

################################################################################
# Target name
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    TARGET_NAME_DEBUG             "debug${PROJECT_NAME}"
    TARGET_NAME_DEBUGLIBRARYBUILD "debug${PROJECT_NAME}"
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
    $<$<CONFIG:LibraryBuild>:
        MultiThreaded
    >
    $<$<CONFIG:Release>:
        MultiThreaded
    >
    $<$<NOT:$<OR:$<CONFIG:LibraryBuild>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
)
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "DEBUG"
    ">"
    "$<$<CONFIG:DebugLibraryBuild>:"
        "DEBUG"
    ">"
    "$<$<CONFIG:LibraryBuild>:"
        "NDEBUG"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "LIBBUILD;"
    "UNICODE;"
    "_UNICODE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /Od
        >
        $<$<CONFIG:DebugLibraryBuild>:
            /Od
        >
        $<$<CONFIG:LibraryBuild>:
            /O2;
            /Oi;
            /Gy
        >
        $<$<CONFIG:Release>:
            /O2;
            /Oi;
            /Gy
        >
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:LibraryBuild>:
            /OPT:REF;
            /OPT:ICF
        >
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /MACHINE:X86;
        /SUBSYSTEM:WINDOWS
    )
endif()

target_link_directories(${PROJECT_NAME} PUBLIC
    "$<$<CONFIG:Debug>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/zlibdebug"
    ">"
    "$<$<CONFIG:DebugLibraryBuild>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/zlibdebug"
    ">"
    "$<$<CONFIG:LibraryBuild>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/zlib"
    ">"
    "$<$<CONFIG:Release>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/zlib"
    ">"
)

