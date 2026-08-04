# Releasing ortho-max

The full sequence for a spec bump. Every step here has cost time when skipped.

## 1. Re-vendor the kernel

The kernel is a FLAT COPY, not a submodule — a clone or ZIP download builds
with no extra steps. Push ortho-kernel FIRST, or KERNEL_VERSION records a
commit hash that does not exist remotely.

    cd ~/Documents/ortho-kernel && git push && git rev-parse HEAD

    cd ~/Documents/Max\ 9/Packages/ortho
    rm -rf source/projects/ortho/ortho-kernel
    rsync -a --exclude build --exclude .git \
      ~/Documents/ortho-kernel/ source/projects/ortho/ortho-kernel/

Then verify the copy landed. This is not ceremony: rsync failing partway
leaves a mix of old and new source that still COMPILES. If the struct changed,
a stale header against fresh source produces garbage rather than an error.

    diff -r ~/Documents/ortho-kernel/src source/projects/ortho/ortho-kernel/src
    diff -r ~/Documents/ortho-kernel/include source/projects/ortho/ortho-kernel/include

Both must be silent.

## 2. KERNEL_VERSION

Lives INSIDE the vendored tree, at
`source/projects/ortho/ortho-kernel/KERNEL_VERSION`, so the `rm -rf` above
deletes it. Write it AFTER the rsync, never before. Vendor-only — upstream
does not carry this file. Records the FULL 40-character hash.

    cat > source/projects/ortho/ortho-kernel/KERNEL_VERSION << 'INNER'
    ortho-kernel vendored copy
    upstream: https://github.com/leeMeredith/ortho-kernel
    commit:   <full hash from git rev-parse HEAD>
    spec:     <N.0> · vectors <vN>

    Do not edit these files here. Edit upstream, re-vendor, re-run conformance.
    INNER

## 3. Build

    rm -rf build externals
    cmake -G Xcode -B build
    cmake --build build --config Release

## 4. Verify the binary

"BUILD SUCCEEDED" is NOT sufficient. Silent Intel-only binaries are a real
failure mode.

    lipo -info externals/ortho.mxo/Contents/MacOS/ortho   # want: x86_64 arm64
    codesign --force --deep -s - externals/ortho.mxo
    codesign -v externals/ortho.mxo; echo "exit: $?"      # want: exit: 0

## 5. Verify against the vectors, inside Max

Cannot be scripted. Protocol matters:

1. FULLY QUIT Max (Cmd-Q, gone from the dock) — not just close the patcher.
2. Relaunch.
3. New patcher, fresh `[ortho @seed 12345]`.
4. `tokens 20` as the FIRST message to that object. Nothing before it.

Any message that draws — `word`, `sentence`, `paragraph`, another `tokens` —
advances the PRNG, and the next output will not match. A `sentence` or
`paragraph` message returns the READABLE path: capitalised, terminal marks.
If the output has capitals and `!`, the wrong message was sent.

Expected:

    cd ~/Documents/ortho
    head -20 test/vectors/vN/seed_12345_bare.txt | cut -f2 | tr '\n' ' '; echo

## 6. Commit, tag, push

`build/` and `externals/` are gitignored; `git add -A` is safe.

    git add -A
    git commit -m "Re-vendor kernel at <short hash> (spec N.0, vectors vN): <what changed>"
    git push
    git tag vN.0.0
    git push origin vN.0.0

Tag convention here is `vN.0.0`. The reference repo uses `spec-N.0`.
ortho-kernel is untagged by convention — it is referenced by hash.

## 7. Build the release zip

README tells users to unzip and drop the `ortho` FOLDER into their Packages
directory, so the zip must contain a folder named `ortho`.

Ships: externals, help, docs, package-info.json, README.md, LICENSE.
Does NOT ship: build, source, max-sdk-base — those are for building.

    cd /tmp && rm -rf ortho-release && mkdir -p ortho-release/ortho
    cd ~/Documents/Max\ 9/Packages/ortho
    cp -R externals help docs package-info.json README.md LICENSE \
      /tmp/ortho-release/ortho/

Re-verify inside the COPY — this is what users get, and cp can strip
signatures:

    cd /tmp/ortho-release
    lipo -info ortho/externals/ortho.mxo/Contents/MacOS/ortho
    codesign -v ortho/externals/ortho.mxo; echo "exit: $?"

Then zip. `-y` preserves symlinks rather than following them:

    zip -r -y ortho-vN.0.0.zip ortho

## 8. Check the help file for stale claims

`help/ortho.maxhelp` is a patcher file full of coordinates, so grepping for a
version number gives false positives from geometry like `[ 20.0, 375.0, ... ]`.
Look at the matches, do not trust the count.

    grep -o '[^"]*N\.0[^"]*' help/ortho.maxhelp | head

README.md deliberately pins no spec or vector version — those facts live in
KERNEL_VERSION so the README cannot drift. Keep it that way.

## 9. Attach the zip to the GitHub release

The tag alone gives source at that point. The release zip is what users
without Xcode and CMake can actually run.
