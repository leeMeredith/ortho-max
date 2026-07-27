# ============================================================================
# Top-level CMakeLists.txt for the max-cmake-template package
# ============================================================================
#
# This file does almost nothing on purpose. It sets the build architecture,
# then hands off to the folder that actually builds the external.
#
# IMPORTANT: the SDK scripts (max-pretarget / max-posttarget) are NOT included
# here. They belong in the external's own folder — see the note in
# source/projects/template/CMakeLists.txt for why. Including them here breaks
# the build.
# ============================================================================

cmake_minimum_required(VERSION 3.19)

# Build a UNIVERSAL binary (Intel + Apple Silicon).
#
# WHY THIS MATTERS: Max 9 on Apple Silicon runs natively as arm64. An
# Intel-only external will NOT load there — and the symptom is the same
# misleading "no such object" you get from a bad name or a missing signature.
#
# TWO TRAPS, BOTH OF WHICH THIS BLOCK AVOIDS:
#
# 1. max-pretarget.cmake sets CMAKE_OSX_ARCHITECTURES to x86_64 if it is still
#    empty when the SDK runs. So we must set it first, or we silently get an
#    Intel-only build that still reports "BUILD SUCCEEDED".
#
# 2. Do NOT guard this with if(APPLE). The APPLE variable does not exist until
#    project() has run, so an if(APPLE) test up here is always false and the
#    block quietly does nothing. CMAKE_OSX_ARCHITECTURES is simply ignored on
#    non-Apple platforms, so no guard is needed.
#
# CACHE + FORCE because architecture is baked into the configured build tree.
# If you change it, delete build/ and re-run cmake; editing it alone won't
# take effect on an already-configured tree.
#
# Verify after building with:
#   lipo -info externals/template.mxo/Contents/MacOS/template
#   -> expect: x86_64 arm64
if (NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64"
        CACHE STRING "Build architectures for macOS" FORCE)
endif ()

# A name for the overall package/workspace. This is NOT the object name —
# the object is named by its source folder (see below).
project(ortho_package)

# Build the actual object.
#
# The path depth here matters: the SDK expects source/<category>/<object>/.
# The FOLDER NAME of the last component becomes the object's name.
add_subdirectory(source/projects/ortho)