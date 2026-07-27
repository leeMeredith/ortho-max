# Building a Max external with CMake

Everything here was verified by actually building this package on macOS
(Apple Silicon, Xcode 26, Max 9). The "gotchas" section is not hypothetical —
each one is a real failure that happened during the first build, along with the
misleading symptom it produced.

---

## One-time setup

The SDK is a git submodule. If you cloned this repo without `--recursive`, or
you are starting a fresh package, you need it before anything will configure.

```
git clone --recursive https://github.com/leeMeredith/max-cmake-template.git
```

If you forgot `--recursive`:

```
git submodule update --init --recursive
```

Starting a brand new package from scratch instead:

```
git init
```
```
git submodule add https://github.com/Cycling74/max-sdk-base.git
```

Confirm the file CMake actually looks for. If this does not print a path,
configuring will fail:

```
ls max-sdk-base/script/max-pretarget.cmake
```

---

## Build

Run these one line at a time. Do not paste them with comments attached — see
"Shell gotcha" at the bottom.

```
cd ~/Documents/max-cmake-template
```
```
rm -rf build externals
```
```
cmake -G Xcode -B build
```
```
cmake --build build --config Release
```

Configure should end with `-- Generating done` and
`-- Build files have been written to:`. The build should end with
`** BUILD SUCCEEDED **`.

---

## Verify (do not skip)

**"BUILD SUCCEEDED" does not mean the external will load.** This package once
built successfully while producing a binary Max refused to open. These two
commands are the real test.

```
lipo -info externals/template.mxo/Contents/MacOS/template
```

Expect: `Architectures in the fat file: ... are: x86_64 arm64`

If it says `Non-fat file: ... is architecture: x86_64`, the external is
Intel-only and will **not** load in Max on Apple Silicon. Fix the architecture
setting (see gotcha 4), then `rm -rf build` and reconfigure.

```
codesign -v externals/template.mxo 2>&1; echo "exit: $?"
```

Expect: `exit: 0`

The SDK ad-hoc signs during the build, so this normally passes on its own. If
it does not, sign manually and re-verify:

```
codesign --force --deep -s - externals/template.mxo
```

---

## Install into Max

Max only scans its Packages folder at launch, and **it does not follow
symlinks**. Cycling '74's docs are explicit that installing a package means
copying it in. A symlinked package sits there looking correct in Finder — arrow
badge and all — while Max never sees it, and the object comes up as
`no such object`.

(Confusingly, symlinking the *entire* Packages folder to another location does
work, and is a common trick for syncing packages across machines. It's linking
one package *inside* Packages that fails. That distinction is easy to miss.)

First confirm which Max folder your installation actually uses — do not assume
the version:

```
ls -d ~/Documents/Max*
```

Then copy the package in:

```
cp -R ~/Documents/max-cmake-template ~/Documents/Max\ 9/Packages/
```

Confirm the binary actually arrived. This is the check that matters, since an
incomplete copy fails the same silent way:

```
ls ~/Documents/Max\ 9/Packages/max-cmake-template/externals/template.mxo/Contents/MacOS/
```

That must print `template`. Then **fully quit Max** (Cmd-Q, confirm it's gone
from the dock) and relaunch.

Test: new object box, type `template`. Number box into each inlet, `[print]` on
each outlet. Send `3` to the right inlet, then `5` to the left. Expect `8` from
the left outlet and `3. 5. 8.` from the right.

### The rebuild-and-install cycle

Because Max reads the copy and not your working folder, **every rebuild needs a
re-copy**. Skip it and Max keeps loading the previous binary — the object works,
your changes just don't appear, which is a genuinely confusing way to lose an
hour.

Make it a single step:

```
cd ~/Documents/max-cmake-template && cmake --build build --config Release && rm -rf ~/Documents/Max\ 9/Packages/max-cmake-template && cp -R . ~/Documents/Max\ 9/Packages/max-cmake-template
```

Worth saving as a shell alias. Adjust the Max version folder to match yours.

---

## The seven gotchas

These are ordered by how much time each one cost. Every one of them produced a
confusing symptom rather than a clear error.

### 1. The SDK scripts belong in the external's folder, never the top level

`max-pretarget.cmake` derives a name from the directory it is included from and
calls `project()` with it. `max-posttarget.cmake` then looks for a target named
`${PROJECT_NAME}`.

Include pretarget at the top level and `PROJECT_NAME` becomes your *package*
folder name, so posttarget hunts for a target that does not exist:

```
CMake Error at max-sdk-base/script/max-posttarget.cmake:11 (set_property):
  set_property could not find TARGET max-cmake-template.
```

The correct shape, all inside `source/<category>/<object>/CMakeLists.txt`:

```cmake
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../max-sdk-base/script/max-pretarget.cmake)
add_library(${PROJECT_NAME} MODULE myobj.c my_engine.c)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../max-sdk-base/script/max-posttarget.cmake)
```

Use `${PROJECT_NAME}` in `add_library`, not a hard-coded name.

### 2. Folder depth must be `source/<category>/<object>/`

`max-posttarget.cmake` uses hardcoded `../../../` paths to locate the package
root. At that depth it lands correctly. One level shallower and it escapes
above your package. The build compiles and links fine, then dies while copying
bundle metadata:

```
cp: .../source/template/../../../externals/template.mxo/Contents/PkgInfo:
    No such file or directory
```

`projects` is just a category name; Cycling '74's own packages use it.

### 3. The object's name comes from its folder name

Not from anything you type in the CMakeLists. `source/projects/template/`
produces `template.mxo`. To build `ortho`, the folder must be
`source/projects/ortho/`.

Separately, the name Max looks up when you type it into an object box is the
string in `class_new("template")` in the `.c` file — **not** the `.mxo`
filename. Rename the folder and forget the `class_new` string and the object
will not be found.

### 4. Set the architecture before `project()`, and do not guard it with `if(APPLE)`

`max-pretarget.cmake` sets `CMAKE_OSX_ARCHITECTURES` to `x86_64` if it is still
empty when the SDK runs. So you must set it first, or you silently get an
Intel-only build that still reports success.

The subtle part: `APPLE` does not exist until `project()` has run. An
`if(APPLE)` test above that line is *always false*, so the block quietly does
nothing — which is exactly how the Intel-only build slipped through. No guard
is needed; the variable is ignored on other platforms.

```cmake
cmake_minimum_required(VERSION 3.19)

if (NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64"
        CACHE STRING "Build architectures for macOS" FORCE)
endif ()

project(my_package)
add_subdirectory(source/projects/myobj)
```

### 5. Architecture is baked into the configured build tree

It is a cached variable. Editing the CMakeLists alone will not change an
already-configured tree. Any architecture change needs:

```
rm -rf build
```

then reconfigure. The same is true after a failed configure: a stale
`CMakeCache.txt` will keep reproducing the old error even once the source is
fixed. When something makes no sense, delete `build` first.

### 6. Ad-hoc codesigning is mandatory on Apple Silicon

An unsigned external produces `no such object` in the Max Console — the same
message you get from a wrong class name or a missing package. Three unrelated
causes, one symptom, which is why `codesign -v` belongs in every build cycle
rather than only when something looks wrong.

The SDK now signs during the build. Verify anyway.

### 7. A symlinked package is invisible to Max

Max does not follow symlinks in the Packages folder. The package appears
correctly in Finder, with an arrow badge suggesting it's wired up, and Max
simply never scans it — producing `no such object` for a binary that is
perfectly good.

That makes **four** distinct causes for the same message: unsigned binary,
wrong class name, package-name collision, and symlinked package. This is why
verifying with commands beats reasoning from the Console text, which cannot
tell them apart.

Copy the package in. See the install section above.

---

## Shell gotcha (zsh)

Do not paste multi-line command blocks that contain comments. Interactive zsh
does not treat `#` as a comment, and parentheses are read as glob qualifiers:

```
zsh: command not found: #
zsh: unknown sort specifier
zsh: number expected
```

None of those are build failures — they are the shell choking on prose. Run one
command at a time.

---

## Cloning this template for a new object

1. Copy the package folder, rename it.
2. Rename `source/projects/template/` to `source/projects/<newname>/`.
3. Rename `template.c` to `<newname>.c`.
4. Update the source list in `source/projects/<newname>/CMakeLists.txt`.
5. Update `add_subdirectory()` in the top-level `CMakeLists.txt`.
6. Change `class_new("template")` to `class_new("<newname>")` in the `.c` file.
7. Change `"name"` in `package-info.json` — two packages sharing a name makes
   Max silently drop one of them.
8. Rewrite the engine (`simple_math.c/.h`) with your own logic, keeping it free
   of any Max headers.
9. `rm -rf build externals`, then build and verify as above.

Step 7 is easy to miss because nothing about the binary looks wrong when it is
skipped — the other package just disappears from Max.
