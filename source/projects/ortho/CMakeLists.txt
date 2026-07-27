# ============================================================================
# source/projects/template/CMakeLists.txt — builds the actual external
# ============================================================================
#
# TWO THINGS TO UNDERSTAND ABOUT THIS FILE:
#
# 1. THE OBJECT'S NAME COMES FROM THE NAME OF THIS FOLDER.
#    max-pretarget.cmake takes the folder it is included from and calls
#    project() with that name. Because this folder is called "template",
#    PROJECT_NAME becomes "template" and the built external is template.mxo.
#    That is why add_library() below uses ${PROJECT_NAME} rather than a
#    hard-coded name, and why the SDK scripts must be included HERE and never
#    from the top-level CMakeLists.txt. Including max-pretarget.cmake at the
#    top level makes PROJECT_NAME your PACKAGE folder's name, and the build
#    then fails with "set_property could not find TARGET <package-name>".
#
# 2. THE FOLDER DEPTH MATTERS. IT MUST BE source/<category>/<object>/.
#    max-posttarget.cmake contains hardcoded relative paths of the form
#    ../../../externals to find the package root. That only lands correctly
#    when this file sits exactly three levels below the package root, i.e.
#    source/projects/template/. If you flatten this to source/template/, the
#    build compiles and links fine but then fails at the packaging step with:
#        cp: .../source/template/../../../externals/template.mxo/Contents/
#            PkgInfo: No such file or directory
#    because ../../../ escaped above the package. Keep the three-level depth.
#    ("projects" is just a category name — Cycling '74's own packages use it.)
#
# RENAMING FOR YOUR OWN OBJECT:
#   1. rename this folder     source/projects/template -> source/projects/myobj
#   2. rename the source file template.c -> myobj.c
#   3. update the add_library() source list below
#   4. update add_subdirectory() in the top-level CMakeLists.txt
#   5. change class_new("template") to class_new("myobj") in the .c file
#   You never type the object name in this file — ${PROJECT_NAME} picks it up
#   from the folder automatically.
# ============================================================================

# --- SDK pre-target setup: MUST come before add_library ---
# Sets include paths, bundle settings, compiler definitions, and calls
# project() using this folder's name. The ../../../ walks up three levels,
# from source/projects/template/ to the package root where max-sdk-base lives.
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../max-sdk-base/script/max-pretarget.cmake)

include_directories(
    "${MAX_SDK_INCLUDES}"
    "${MAX_SDK_MSP_INCLUDES}"
    "${MAX_SDK_JIT_INCLUDES}"
)

# Define the external. MODULE means "a loadable plugin bundle" — for Max that
# becomes a .mxo on macOS / .mxe64 on Windows.
#
# NOTE: this template has only been built and verified on macOS. Nothing here
# is deliberately mac-only, and the SDK supports both, but the Windows path is
# untested — treat it as unverified rather than supported.
#
# List EVERY .c file the object is built from: the glue (template.c) PLUS the
# engine (simple_math.c). Headers (.h) are NOT listed — they are pulled in by
# #include; only .c files compile. Add more .c files here as you grow.
add_library(${PROJECT_NAME} MODULE
    ortho.c
    simple_math.c
)

# --- SDK post-target setup: MUST come after add_library ---
# Applies the bundle settings, links the Max API, and copies the bundle
# metadata (Info.plist, PkgInfo) into externals/<name>.mxo at the package root.
# It expects the target to be named ${PROJECT_NAME}, which is why add_library
# above uses that variable.
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../max-sdk-base/script/max-posttarget.cmake)
