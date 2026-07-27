# ortho

A Max/MSP external that generates **invented language** — pseudo-words that
hold the shape and internal consistency of a language without belonging to any
existing one.

Every seed names a different language — more than four billion of them. Go
looking and you will find tongues that are clipped and consonantal, others that
are long and vowel-heavy, others that sound like they have grammar. Pick the
one you want.

Determinism is what makes that worth doing. A seed does not merely start a
language, it *is* the language: the same number gives you the same tongue on
every machine, in every session, forever. Find one you like and it keeps — in
your patch, in a collaborator's, in the JavaScript reference, in the
openFrameworks addon. The language is not reimplemented per host; it is
shared.

    [ortho @seed 12345]

`tokens 20` produces:

    mit'zw mhymafu ten'v cu xkqip hzjry hclakub un fizywaz xylpj'q
    zmreh cibykame'k wu nupurumi aryjanmh munamute hbe mwif acmfc mjicoxy

## Install

Download the latest release, unzip it, and put the `ortho` folder in:

    ~/Documents/Max 9/Packages/

Quit Max completely and relaunch — packages are only scanned at launch. Then
type `ortho` into a new object box.

macOS only for now. The external is a signed universal binary (Intel and Apple
Silicon). Nothing here is deliberately mac-only, but the Windows path is
untested.

## Messages

| message | what it does |
|---|---|
| `bang` | one paragraph |
| `tokens N` | exactly N words, no punctuation — the conformance path |
| `page N` | N paragraphs |
| `section` | a new cast of names and topics; same language |
| `cleardials` | zero all seven dials and the preset |

## Outlets

**Left** — the words, one symbol per atom. Walk them with `[zl iter 1]`, store
them with `[coll]`.

**Right** — why each word appeared, one integer per word, aligned position for
position with the left outlet:

| value | meaning |
|---|---|
| 0 | freshly generated |
| 1 | function word, document scope |
| 2 | topic — the section's subject |
| 3 | name — the section's identities |
| 4 | member of a recurring phrase |

Colour by source, spawn on a name, treat recurrence differently from fresh
text. The classification is normative and identical across every ortho host.

## The seven dials

Each is a float from 0 to 1, default 0. At zero, every word is freshly
generated. Raise a dial and that kind of word starts recurring.

| dial | effect |
|---|---|
| `@phrases` | multi-word phrases recur whole and in order |
| `@function_words` | grammatical glue recurs, document scope |
| `@topics` | the section's subject recurs |
| `@names` | the section's identities recur, capitalised |
| `@commas` | commas, anchored to function words |
| `@quotation` | direct speech, anchored to a name in the cast |
| `@scare_quotes` | a single term held at arm's length |

The first four affect every path. The last three add punctuation and are
ignored by `tokens`.

`@preset` sets all seven at tuned proportions. Dials you set by hand are never
overwritten by it, so order does not matter — `[ortho @preset 0.5 @names 0.9]`
and `[ortho @names 0.9 @preset 0.5]` are identical.

## Finding a language

`ortho` has no random mode, on purpose: the object's whole contract is that a
seed always names the same tongue. Randomness belongs outside it, so bring your
own.

    [random 1000000]
          |
    [prepend seed]
          |
       [ortho]

Bang it until something sounds right, then read the seed off `[ortho]`'s
inspector and type it into the object box. From then on that language is
pinned, saved with the patch, and identical everywhere.

Valid seeds run 0 to 4294967295. `[random 1000000]` explores a sliver of that
and is still a million languages.

Two different controls, worth not confusing:

- **a new seed** changes the language — different sounds, different word shapes
- **`section`** keeps the language and changes what it is about — new names,
  new topics, same tongue

For most patches `section` is the one you reach for while it plays, and the
seed is what you settle before you start.

Bare `[ortho]` is seed 0. That is a real language like any other, not a null
state — and it means a patch reopens speaking whatever it was written in.

## Shaping

`@max_letters` (8), `@max_words` (12), and `@sentences` (4) are **not** dials.
They do not change the language, only how much of it comes out. Kept separate
in the inspector and the docs for that reason.

**They are ceilings on a random draw, not targets.** Word length varies below
`@max_letters` rather than sitting at it — that variation is deliberate, since
prose with uniform word lengths reads as a list.

The paragraph path draws twice: once for a per-sentence ceiling below your
value, then once per word below that. So `bang` and `page` produce noticeably
shorter words than `tokens` at the same setting. With `@max_letters 8` a
paragraph averages two or three letters, which is short enough that seeds start
to resemble each other.

If paragraphs read as too clipped, raise `@max_letters` well above the length
you want — 20 or more is reasonable. Or use `tokens N`, which draws once and
shows a language's character most clearly.

## Punctuation and message boxes

With the punctuation dials above zero, punctuation is attached to the word it
belongs to — a token can arrive as one symbol carrying a comma.

That symbol travels safely through `zl`, `coll`, `route`, and anything else
programmatic. It is only re-read if it lands in a message box, where Max treats
a comma as a message separator.

Punctuation is deliberately not stripped: it is part of the language, and every
other ortho host keeps it. Use `tokens` when you need output guaranteed free
of it.

## Build from source

You need Xcode command-line tools and CMake.

    git clone --recursive https://github.com/leeMeredith/ortho-max.git
    cd ortho-max
    cmake -G Xcode -B build
    cmake --build build --config Release
    codesign --force --deep -s - externals/ortho.mxo
    lipo -info externals/ortho.mxo/Contents/MacOS/ortho

That last command should report `x86_64 arm64`.

The `--recursive` matters: the Max SDK is a submodule. The language kernel is
**not** — it is vendored into `source/projects/ortho/ortho-kernel/`, so a plain
clone or a ZIP download builds without further steps. `KERNEL_VERSION` in that
folder names the upstream commit it was copied from.

Codesigning is not optional on Apple Silicon. An unsigned external fails to
load with a misleading "no such object".

## Conformance

Output is proven identical to the JavaScript reference, not assumed. The kernel
ships golden vectors; this external has been diffed against them through its
own full output path — words and source classifications both — for seeds 0 and
12345, bare and at preset 0.5.

To check a build yourself: create a fresh `[ortho @seed 12345]`, send
`tokens 20` as its first message, and compare against the reference oracle.
Every word drawn advances the generator, so the object must be freshly created
or you are comparing a different point in the same stream.

## The family

- **[ortho](https://github.com/leeMeredith/ortho)** — JavaScript reference
  implementation, `SPEC.md`, and the golden vectors. The authority.
- **[ortho-kernel](https://github.com/leeMeredith/ortho-kernel)** — the shared
  host-neutral C engine, and `HOSTS.md`, which pins what the hosts must agree
  on beyond the language itself.
- **ofxOrtho** — the openFrameworks addon.

Built from
[max-cmake-template](https://github.com/leeMeredith/max-cmake-template), which
documents the Max SDK and CMake setup this package uses.

## License

MIT. Copyright (c) 2026 JD Clemon Lee Meredith.
