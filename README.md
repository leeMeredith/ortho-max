# max-cmake-template

A **blank, heavily-documented starter external for Max 9**, built with CMake and the Max SDK.

It is deliberately the most boring object possible: a two-number calculator. The point is not what it *does* — the point is that it shows you every part of a real Max external (inlets, outlets, methods, attributes, save/restore, assist tooltips) **and how to structure one across two files**: the glue that talks to Max, and a separate engine that does the real work and knows nothing about Max. Every non-obvious line has a comment explaining *why that call and not the obvious one*. Replace the engine with your own idea and you have your own object.

If you've been held back by not knowing the right SDK calls, the build incantation, or why your object silently won't load — this is meant to get you unstuck.

It's part of a small family of Max externals I build under a `b`-prefix convention. If this helps you, take a look at [bbox](https://github.com/leeMeredith/bbox) for a real, finished UI object that grew out of the same scaffolding.

## What it does

`[template]` has two inlets and two outlets.

- **Left inlet** (hot): a number, a `bang`, or a list `a b`. Sending here computes and outputs.
- **Right inlet** (cold): the second number. Stored only — does not trigger output. (The classic Max hot/cold idiom.)
- **Left outlet**: the result, as a float.
- **Right outlet**: the full picture as a list `a b result`, which you can `[unpack 0. 0. 0.]` downstream — the same pattern bbox uses to report its coordinates on the right.

The operation is an attribute:

```
[template @op 0]   ← add (default)
[template @op 1]   ← subtract
[template @op 2]   ← multiply
[template @op 3]   ← divide (guards against divide-by-zero)
```

Because `@op` is saved, the patch remembers your choice when you reopen it.

The math is done in plain C inside the engine (`simple_math.c`), not here in the Max glue — and not by leaning on a Max `[+]` object. The whole exercise is learning how numbers get **in**, get **processed**, and get **out**.

## Platform support

**macOS: verified.** Built and loaded in Max 9 on Apple Silicon, as a universal
(`x86_64 arm64`) binary.

**Windows: untested.** Nothing here is deliberately mac-only and the Max SDK
supports both platforms, but I have not built this on Windows, so I am not
claiming it works. If you try it, a report either way is welcome — open an
issue.

## Build (macOS)

You need Xcode command-line tools and CMake. Run these one line at a time.

```
git clone --recursive https://github.com/leeMeredith/max-cmake-template.git
```
```
cd max-cmake-template
```
```
cmake -G Xcode -B build
```
```
cmake --build build --config Release
```

Then verify — **"BUILD SUCCEEDED" alone does not mean Max will load it**:

```
lipo -info externals/template.mxo/Contents/MacOS/template
```
```
codesign -v externals/template.mxo 2>&1; echo "exit: $?"
```

You want `x86_64 arm64` from the first and `exit: 0` from the second. Then put the folder in `~/Documents/Max 9/Packages/` (or symlink it), fully quit Max — it only scans packages at launch — and relaunch. Type `template` into a new object box.

**[BUILD.md](BUILD.md) has the full procedure**, including six SDK behaviors that each produce a misleading symptom rather than a clear error: where the SDK scripts must be included, why folder depth matters, where the object's name actually comes from, the `if(APPLE)` trap that silently yields an Intel-only binary, cached architecture, and ad-hoc codesigning. Read it before you clone this for a new object.

### The three things that silently break a Max external

1. **Unsigned binary.** On Apple Silicon, Max refuses to load an external that isn't validly codesigned, and says "no such object" instead. The SDK ad-hoc signs during the build now, but verify with `codesign -v` every time.
2. **Class-name vs filename.** Max identifies your object by the string in `class_new("template")`, **not** the `.mxo` filename. Rename the file but not that string and it won't be found.
3. **Package-name collision.** The `"name"` field in `package-info.json` must be unique. Two packages sharing a name and Max's loader drops one — invisibly.

All three produce the same "no such object" message, which is why the verification commands matter more than the build's own success report.

## Make it your own

1. Copy the whole folder, rename it.
2. Rename `source/projects/template/` to `source/projects/<newname>/` — **that folder name becomes the object's name**.
3. Rename `template.c` to `<newname>.c`, and update the source list in the inner `CMakeLists.txt` and `add_subdirectory()` in the top-level one.
4. Change `class_new("template")` to `class_new("<newname>")`, and the `"name"` field in `package-info.json`.
5. Rewrite `simple_math.c` / `simple_math.h` with your own logic — your struct, your `compute`. Keep it Max-free. Then adjust the few call sites in the glue and add/remove inlets/outlets to match.
6. `rm -rf build externals`, then build and verify.

The full checklist is at the end of [BUILD.md](BUILD.md).

## File map

```
max-cmake-template/
├── CMakeLists.txt                      top-level: sets architecture, descends into source/
├── package-info.json                   the manifest Max reads at launch
├── max-sdk-base/                       the SDK, as a git submodule
├── externals/                          created by the build — the .mxo lands here
└── source/
    └── projects/
        └── template/
            ├── CMakeLists.txt          builds the .mxo from BOTH .c files below
            ├── template.c             ← the GLUE: talks to Max (inlets/outlets/attrs)
            ├── simple_math.c          ← the ENGINE: does the math, knows nothing of Max
            └── simple_math.h          ← the engine's struct + function list
```

Two things about that layout are load-bearing, not stylistic:

- **The object's name comes from its folder name.** `source/projects/template/` produces `template.mxo`, and `${PROJECT_NAME}` in the CMakeLists picks it up automatically. To make `myobj`, rename the folder.
- **The three-level depth `source/<category>/<object>/` is required.** The SDK's post-build script uses hardcoded `../../../` paths to find the package root. Flatten it to `source/template/` and the build compiles and links fine, then fails while copying bundle metadata.

### The two-file split (the most important idea here)

`template.c` is the **glue** — it talks to Max. `simple_math.c` / `.h` is the **engine** — it does the actual work and contains no Max code at all (no `ext.h`, no atoms, no outlets). The glue owns an engine struct and calls into it; the engine never reaches back.

This is the same structure my [bbox](https://github.com/leeMeredith/bbox) object uses, where the engine is `xeno_follower`. The payoff:

- You can test the engine without launching Max.
- You can reuse the engine in another object — or another program entirely.
- When a result is wrong you look in the engine; when an inlet misbehaves you look in the glue. The bug tells you which file it's in.

To make your own object, you mostly rewrite `simple_math.c` and change a few call sites in `template.c`.

## License

MIT. Copyright (c) 2026 JD Clemon Lee Meredith. Do anything you like with it — that's the point.
