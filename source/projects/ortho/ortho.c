/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 JD Clemon Lee Meredith
 */

/**
 * ============================================================================
 * ortho.c — Max glue for the ortho invented-language kernel
 * ============================================================================
 *
 * THIS FILE IS THE GLUE. It talks to Max — inlet, outlets, attributes — and
 * hands everything else to ortho-kernel, which knows nothing about Max.
 *
 * DESIGN RULE (same as ofxOrtho): this file performs TRANSLATION, not
 * INTERPRETATION.
 *
 *     t_atom list  <->  ortho_token[]  <->  kernel        GOOD
 *     "if topics > .6 then behave differently"            BAD
 *
 * If this file ever starts getting smarter than the kernel, that is a bug.
 *
 * OBJECT SURFACE
 *   bang         one paragraph
 *   tokens N     exactly N tokens, punctuation-free (the conformance path)
 *   page N       N paragraphs
 *   section      mint a fresh cast of names/topics/phrases (same language)
 *   cleardials   forget every explicit dial and zero all seven
 *
 *   left outlet   the words, one symbol per atom
 *   right outlet  the matching ORTHO_SRC_* value for each word
 *
 * A NOTE ON PUNCTUATION AND MAX
 *   The readable paths (bang, page) bake punctuation into the token text, so
 *   a token can arrive as "Lxzxe," — one symbol containing a comma. That is
 *   correct and matches every other host; the punctuation is part of the
 *   language. It travels safely through zl, coll, route and anything else
 *   programmatic. It only misbehaves if a human pastes it into a MESSAGE BOX,
 *   where Max reads the comma as a message separator. The `tokens` message is
 *   punctuation-free by contract and is the safe path when that matters.
 *   We do NOT strip or escape: mangling here would break parity with the JS
 *   reference and ofxOrtho for no real gain.
 *
 * MEMORY
 *   The kernel never allocates. Neither do we: both buffers are embedded in
 *   the object struct, so an instance is one allocation and no teardown.
 * ============================================================================
 */

#include "ext.h"
#include "ext_obex.h"
#include "ext_obex_util.h"

#include "ortho.h"   /* the vendored kernel; include path comes from CMake */

/* Largest run of tokens one message can emit. Both buffers are this long. */
#define ORTHOMAX_CAP 1024

/* Indices into the explicit[] mark array — one per dial, same order and same
 * frozen vocabulary as ortho_dials and ofxOrtho. */
enum {
    DIAL_PHRASES = 0,
    DIAL_FUNCTION_WORDS,
    DIAL_TOPICS,
    DIAL_NAMES,
    DIAL_COMMAS,
    DIAL_QUOTATION,
    DIAL_SCARE_QUOTES,
    DIAL_COUNT
};

/* ===========================================================================
   THE OBJECT STRUCT

   Note x->dials: the dials live HERE, not in x->engine. ortho.h says to treat
   ortho_t's fields as private, and ortho_set_dials() clamps on the way in. If
   attributes wrote straight into engine.dials they would bypass that clamp and
   the engine would lose authority over its own state. So Max writes here, and
   we push the whole struct into the engine before every generation call. The
   header guarantees that push rebuilds nothing and consumes no PRNG draws, so
   it cannot perturb conformance.

   x->explicit_set marks which dials the user set BY HAND. @preset fills only
   the unmarked ones, so [ortho @preset 0.5 @names 0.9] and
   [ortho @names 0.9 @preset 0.5] land on identical values. Same design as
   ofxOrtho, for the same reason: attribute order in an object box is not
   something a user should have to think about.
   =========================================================================== */
typedef struct _ortho {
    t_object    ob;              /* MUST BE FIRST */

    ortho_t     engine;          /* THE KERNEL, embedded by value */
    ortho_dials dials;           /* Max-facing dial state (see above) */

    char        explicit_set[DIAL_COUNT]; /* 1 = user set this dial by hand */
    double      preset;          /* @preset — one-knob onramp */

    long        seed;            /* @seed — names the language */
    long        max_letters;     /* @max_letters — matches ofxOrtho default 8  */
    long        max_words;       /* @max_words   — matches ofxOrtho default 12 */
    long        sentences;       /* @sentences   — no ofxOrtho counterpart     */

    ortho_token tokbuf[ORTHOMAX_CAP];  /* caller-owned buffer for the kernel */
    t_atom      atoms [ORTHOMAX_CAP];  /* scratch for building outlet lists   */

    void       *outlet_source;   /* outlet 1 (right): ORTHO_SRC_* per word */
    void       *outlet_words;    /* outlet 0 (left) : the words             */
} t_ortho;

static t_class  *ortho_class = NULL;
static t_symbol *ps_list     = NULL;

/* --- forward declarations ------------------------------------------------ */
void *orthomax_new       (t_symbol *s, long argc, t_atom *argv);
void  orthomax_free      (t_ortho *x);
void  orthomax_assist    (t_ortho *x, void *b, long m, long a, char *s);

void  orthomax_bang      (t_ortho *x);
void  orthomax_tokens    (t_ortho *x, t_symbol *s, long argc, t_atom *argv);
void  orthomax_page      (t_ortho *x, t_symbol *s, long argc, t_atom *argv);
void  orthomax_section   (t_ortho *x);
void  orthomax_cleardials(t_ortho *x);

t_max_err orthomax_seed_set  (t_ortho *x, void *attr, long argc, t_atom *argv);
t_max_err orthomax_preset_set(t_ortho *x, void *attr, long argc, t_atom *argv);

t_max_err orthomax_phrases_set       (t_ortho *x, void *attr, long argc, t_atom *argv);
t_max_err orthomax_function_words_set(t_ortho *x, void *attr, long argc, t_atom *argv);
t_max_err orthomax_topics_set        (t_ortho *x, void *attr, long argc, t_atom *argv);
t_max_err orthomax_names_set         (t_ortho *x, void *attr, long argc, t_atom *argv);
t_max_err orthomax_commas_set        (t_ortho *x, void *attr, long argc, t_atom *argv);
t_max_err orthomax_quotation_set     (t_ortho *x, void *attr, long argc, t_atom *argv);
t_max_err orthomax_scare_quotes_set  (t_ortho *x, void *attr, long argc, t_atom *argv);

/* ===========================================================================
   EXT_MAIN
   =========================================================================== */
void ext_main(void *r)
{
    t_class *c;

    ps_list = gensym("list");

    c = class_new("ortho",
                  (method)orthomax_new,
                  (method)orthomax_free,
                  sizeof(t_ortho),
                  0L,
                  A_GIMME, 0);

    class_addmethod(c, (method)orthomax_bang,       "bang",       A_NOTHING, 0);
    class_addmethod(c, (method)orthomax_tokens,     "tokens",     A_GIMME,   0);
    class_addmethod(c, (method)orthomax_page,       "page",       A_GIMME,   0);
    class_addmethod(c, (method)orthomax_section,    "section",    A_NOTHING, 0);
    class_addmethod(c, (method)orthomax_cleardials, "cleardials", A_NOTHING, 0);
    class_addmethod(c, (method)orthomax_assist,     "assist",     A_CANT,    0);

    /* --- @seed: names the language ---------------------------------------
     * Custom setter, because changing the seed must RE-MINT the substrate. */
    CLASS_ATTR_LONG        (c, "seed", 0, t_ortho, seed);
    CLASS_ATTR_LABEL       (c, "seed", 0, "Seed (names the language)");
    CLASS_ATTR_ACCESSORS   (c, "seed", NULL, orthomax_seed_set);
    CLASS_ATTR_DEFAULT_SAVE(c, "seed", 0, "0");

    /* --- the seven dials -------------------------------------------------
     * Frozen vocabulary, shared verbatim with the JS reference and ofxOrtho.
     * Each is a double in [0,1] defaulting to 0.
     *
     * Every dial has a custom setter so it can mark itself explicit. Clamping
     * happens in that setter as plain C — a custom setter may or may not still
     * run a CLASS_ATTR_FILTER_CLIP filter, and rather than depend on macro
     * behavior I have not verified, the clamp lives in code we can read.
     * ortho_set_dials() clamps again inside the kernel regardless. */
    CLASS_ATTR_DOUBLE      (c, "phrases", 0, t_ortho, dials.phrases);
    CLASS_ATTR_LABEL       (c, "phrases", 0, "Phrase recurrence");
    CLASS_ATTR_ACCESSORS   (c, "phrases", NULL, orthomax_phrases_set);
    CLASS_ATTR_DEFAULT_SAVE(c, "phrases", 0, "0.");

    CLASS_ATTR_DOUBLE      (c, "function_words", 0, t_ortho, dials.function_words);
    CLASS_ATTR_LABEL       (c, "function_words", 0, "Function-word recurrence");
    CLASS_ATTR_ACCESSORS   (c, "function_words", NULL, orthomax_function_words_set);
    CLASS_ATTR_DEFAULT_SAVE(c, "function_words", 0, "0.");

    CLASS_ATTR_DOUBLE      (c, "topics", 0, t_ortho, dials.topics);
    CLASS_ATTR_LABEL       (c, "topics", 0, "Topic recurrence (the section's subject)");
    CLASS_ATTR_ACCESSORS   (c, "topics", NULL, orthomax_topics_set);
    CLASS_ATTR_DEFAULT_SAVE(c, "topics", 0, "0.");

    CLASS_ATTR_DOUBLE      (c, "names", 0, t_ortho, dials.names);
    CLASS_ATTR_LABEL       (c, "names", 0, "Name recurrence (the section's identities)");
    CLASS_ATTR_ACCESSORS   (c, "names", NULL, orthomax_names_set);
    CLASS_ATTR_DEFAULT_SAVE(c, "names", 0, "0.");

    CLASS_ATTR_DOUBLE      (c, "commas", 0, t_ortho, dials.commas);
    CLASS_ATTR_LABEL       (c, "commas", 0, "Comma pacing");
    CLASS_ATTR_ACCESSORS   (c, "commas", NULL, orthomax_commas_set);
    CLASS_ATTR_DEFAULT_SAVE(c, "commas", 0, "0.");

    CLASS_ATTR_DOUBLE      (c, "quotation", 0, t_ortho, dials.quotation);
    CLASS_ATTR_LABEL       (c, "quotation", 0, "Direct speech");
    CLASS_ATTR_ACCESSORS   (c, "quotation", NULL, orthomax_quotation_set);
    CLASS_ATTR_DEFAULT_SAVE(c, "quotation", 0, "0.");

    CLASS_ATTR_DOUBLE      (c, "scare_quotes", 0, t_ortho, dials.scare_quotes);
    CLASS_ATTR_LABEL       (c, "scare_quotes", 0, "Scare quotes");
    CLASS_ATTR_ACCESSORS   (c, "scare_quotes", NULL, orthomax_scare_quotes_set);
    CLASS_ATTR_DEFAULT_SAVE(c, "scare_quotes", 0, "0.");

    /* --- @preset: one-knob onramp ----------------------------------------
     * Fills only the dials NOT set by hand, using the tuned proportions in
     * ortho_dials_preset(). Glue-level convenience, NOT part of the language
     * definition — it consumes no PRNG draws. */
    CLASS_ATTR_DOUBLE      (c, "preset", 0, t_ortho, preset);
    CLASS_ATTR_LABEL       (c, "preset", 0, "Preset (fills unset dials)");
    CLASS_ATTR_ACCESSORS   (c, "preset", NULL, orthomax_preset_set);
    CLASS_ATTR_DEFAULT_SAVE(c, "preset", 0, "0.");

    /* --- shaping ---------------------------------------------------------
     * NOT dials. Not part of the language definition, not in the frozen
     * vocabulary — they only shape how much comes out. Kept in their own group
     * here and in the docs so the distinction stays visible. Defaults for the
     * first two match ofxOrtho's; @sentences has no ofxOrtho counterpart
     * (there numSentences is a required argument), so 4 is a Max choice that
     * belongs in HOSTS.md. These clamp in C at the call site. */
    CLASS_ATTR_LONG        (c, "max_letters", 0, t_ortho, max_letters);
    CLASS_ATTR_LABEL       (c, "max_letters", 0, "Longest word");
    CLASS_ATTR_DEFAULT_SAVE(c, "max_letters", 0, "8");

    CLASS_ATTR_LONG        (c, "max_words", 0, t_ortho, max_words);
    CLASS_ATTR_LABEL       (c, "max_words", 0, "Longest sentence");
    CLASS_ATTR_DEFAULT_SAVE(c, "max_words", 0, "12");

    CLASS_ATTR_LONG        (c, "sentences", 0, t_ortho, sentences);
    CLASS_ATTR_LABEL       (c, "sentences", 0, "Sentences per paragraph");
    CLASS_ATTR_DEFAULT_SAVE(c, "sentences", 0, "4");

    class_register(CLASS_BOX, c);
    ortho_class = c;
}

/* ===========================================================================
   HELPERS
   =========================================================================== */

static long orthomax_clamp(long v, long lo, long hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static double orthomax_clamp01(double v)
{
    if (v < 0.0) return 0.0;
    if (v > 1.0) return 1.0;
    return v;
}

/* Recompute the dials from @preset, leaving hand-set dials alone.
 *
 * Explicit dials already hold the user's value in x->dials, so we only need to
 * overwrite the unmarked ones. That is what makes attribute order irrelevant:
 * preset-then-dial ends up in the same place as dial-then-preset. */
static void orthomax_recompute(t_ortho *x)
{
    ortho_dials p;

    ortho_dials_preset(&p, x->preset);

    if (!x->explicit_set[DIAL_PHRASES])        x->dials.phrases        = p.phrases;
    if (!x->explicit_set[DIAL_FUNCTION_WORDS]) x->dials.function_words = p.function_words;
    if (!x->explicit_set[DIAL_TOPICS])         x->dials.topics         = p.topics;
    if (!x->explicit_set[DIAL_NAMES])          x->dials.names          = p.names;
    if (!x->explicit_set[DIAL_COMMAS])         x->dials.commas         = p.commas;
    if (!x->explicit_set[DIAL_QUOTATION])      x->dials.quotation      = p.quotation;
    if (!x->explicit_set[DIAL_SCARE_QUOTES])   x->dials.scare_quotes   = p.scare_quotes;
}

/* Push the Max-side dials into the engine. Free per ortho.h: no substrate
 * rebuild, no PRNG draws. Called before every generation so attribute order
 * can never leave the engine stale. */
static void orthomax_sync(t_ortho *x)
{
    ortho_set_dials(&x->engine, &x->dials);
}

/* Fire both outlets from the first n tokens in x->tokbuf.
 *
 * Right outlet first, left last — Max output is depth-first, so sending the
 * source classification first means anything reacting to the words already has
 * the matching sources in hand. Same ordering rule the template uses.
 *
 * One atom buffer serves both lists because the sends are sequential. */
static void orthomax_emit(t_ortho *x, int n)
{
    int i;

    if (n <= 0) return;

    for (i = 0; i < n; i++)
        atom_setlong(x->atoms + i, (long)x->tokbuf[i].source);
    outlet_list(x->outlet_source, ps_list, (short)n, x->atoms);

    for (i = 0; i < n; i++)
        atom_setsym(x->atoms + i, gensym(x->tokbuf[i].text));
    outlet_list(x->outlet_words, ps_list, (short)n, x->atoms);
}

/* ===========================================================================
   CONSTRUCTOR
   =========================================================================== */
void *orthomax_new(t_symbol *s, long argc, t_atom *argv)
{
    t_ortho *x = (t_ortho *)object_alloc(ortho_class);
    int i;

    if (!x) return NULL;

    /* Engine state first, before anything can change it. */
    ortho_dials_clear(&x->dials);
    for (i = 0; i < DIAL_COUNT; i++) x->explicit_set[i] = 0;

    x->preset      = 0.0;
    x->seed        = 0;
    x->max_letters = 8;
    x->max_words   = 12;
    x->sentences   = 4;

    ortho_init(&x->engine, 0, &x->dials);

    /* Outlets are created right-to-left, so the rightmost is declared first. */
    x->outlet_source = outlet_new((t_object *)x, NULL);
    x->outlet_words  = outlet_new((t_object *)x, NULL);

    /* Applies typed-in @attributes AND saved ones on patch load. Any order
     * lands correct: @seed re-mints on arrival, dials mark themselves and
     * recompute, and the next generation call syncs whatever resulted. */
    attr_args_process(x, argc, argv);

    return x;
}

/* ===========================================================================
   DESTRUCTOR
   The kernel allocates nothing and both buffers are embedded, so there is
   genuinely nothing to release. One inlet means no proxy either.
   =========================================================================== */
void orthomax_free(t_ortho *x)
{
}

/* ===========================================================================
   MESSAGES
   =========================================================================== */

void orthomax_bang(t_ortho *x)
{
    int n;

    orthomax_sync(x);
    n = ortho_paragraph(&x->engine,
                        (int)orthomax_clamp(x->sentences,   1, 64),
                        (int)orthomax_clamp(x->max_words,   1, 64),
                        (int)orthomax_clamp(x->max_letters, 1, 32),
                        x->tokbuf, (size_t)ORTHOMAX_CAP);
    orthomax_emit(x, n);
}

void orthomax_tokens(t_ortho *x, t_symbol *s, long argc, t_atom *argv)
{
    long want;
    int  n;

    want = argc ? atom_getlong(argv) : 0;
    if (want <= 0) {
        object_warn((t_object *)x, "tokens: needs a positive count");
        return;
    }
    if (want > ORTHOMAX_CAP) {
        object_warn((t_object *)x, "tokens: %ld exceeds capacity, using %d",
                    want, ORTHOMAX_CAP);
        want = ORTHOMAX_CAP;
    }

    orthomax_sync(x);
    n = ortho_tokens(&x->engine,
                     (int)want,
                     (int)orthomax_clamp(x->max_letters, 1, 32),
                     x->tokbuf, (size_t)ORTHOMAX_CAP);
    orthomax_emit(x, n);
}

void orthomax_page(t_ortho *x, t_symbol *s, long argc, t_atom *argv)
{
    long want;
    long i;
    int  written = 0;
    int  got;
    int  hit_cap = 0;

    want = argc ? atom_getlong(argv) : 0;
    if (want <= 0) {
        object_warn((t_object *)x, "page: needs a positive paragraph count");
        return;
    }

    orthomax_sync(x);

    for (i = 0; i < want; i++) {
        if (written >= ORTHOMAX_CAP) { hit_cap = 1; break; }

        got = ortho_paragraph(&x->engine,
                              (int)orthomax_clamp(x->sentences,   1, 64),
                              (int)orthomax_clamp(x->max_words,   1, 64),
                              (int)orthomax_clamp(x->max_letters, 1, 32),
                              x->tokbuf + written,
                              (size_t)(ORTHOMAX_CAP - written));
        if (got <= 0) { hit_cap = 1; break; }
        written += got;
    }

    if (hit_cap)
        object_warn((t_object *)x,
                    "page: stopped at %d tokens (capacity %d) after %ld of %ld paragraphs",
                    written, ORTHOMAX_CAP, i, want);

    orthomax_emit(x, written);
}

void orthomax_section(t_ortho *x)
{
    orthomax_sync(x);
    ortho_new_section(&x->engine);
}

/* Forget every explicit mark, zero all seven dials, AND zero @preset — the
 * ofxOrtho clearDials() equivalent, which does the same three things.
 *
 * Clearing the preset is not optional. If it stayed live, the very next dial
 * change would call recompute and every unmarked dial would refill from it,
 * silently undoing the clear. */
void orthomax_cleardials(t_ortho *x)
{
    int i;
    x->preset = 0.0;
    ortho_dials_clear(&x->dials);
    for (i = 0; i < DIAL_COUNT; i++) x->explicit_set[i] = 0;
}

/* ===========================================================================
   ATTRIBUTE SETTERS
   =========================================================================== */

/* The seed IS the language, so setting it re-mints the substrate. Dials are
 * passed from our own copy, which lives outside ortho_t — so nothing is being
 * read out of the struct while it is overwritten. */
t_max_err orthomax_seed_set(t_ortho *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        long v = atom_getlong(argv);
        if (v < 0)           v = 0;
        if (v > 4294967295L) v = 4294967295L;
        x->seed = v;
        ortho_init(&x->engine, (uint32_t)x->seed, &x->dials);
    }
    return MAX_ERR_NONE;
}

t_max_err orthomax_preset_set(t_ortho *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        x->preset = orthomax_clamp01(atom_getfloat(argv));
        orthomax_recompute(x);
    }
    return MAX_ERR_NONE;
}

/* Shared body for the seven dial setters: clamp, store, mark explicit, then
 * recompute so any unmarked dials still track @preset. */
static t_max_err orthomax_dial_set(t_ortho *x, int which, double *field,
                                   long argc, t_atom *argv)
{
    if (argc && argv) {
        *field = orthomax_clamp01(atom_getfloat(argv));
        x->explicit_set[which] = 1;
        orthomax_recompute(x);
    }
    return MAX_ERR_NONE;
}

t_max_err orthomax_phrases_set(t_ortho *x, void *attr, long argc, t_atom *argv)
{ return orthomax_dial_set(x, DIAL_PHRASES, &x->dials.phrases, argc, argv); }

t_max_err orthomax_function_words_set(t_ortho *x, void *attr, long argc, t_atom *argv)
{ return orthomax_dial_set(x, DIAL_FUNCTION_WORDS, &x->dials.function_words, argc, argv); }

t_max_err orthomax_topics_set(t_ortho *x, void *attr, long argc, t_atom *argv)
{ return orthomax_dial_set(x, DIAL_TOPICS, &x->dials.topics, argc, argv); }

t_max_err orthomax_names_set(t_ortho *x, void *attr, long argc, t_atom *argv)
{ return orthomax_dial_set(x, DIAL_NAMES, &x->dials.names, argc, argv); }

t_max_err orthomax_commas_set(t_ortho *x, void *attr, long argc, t_atom *argv)
{ return orthomax_dial_set(x, DIAL_COMMAS, &x->dials.commas, argc, argv); }

t_max_err orthomax_quotation_set(t_ortho *x, void *attr, long argc, t_atom *argv)
{ return orthomax_dial_set(x, DIAL_QUOTATION, &x->dials.quotation, argc, argv); }

t_max_err orthomax_scare_quotes_set(t_ortho *x, void *attr, long argc, t_atom *argv)
{ return orthomax_dial_set(x, DIAL_SCARE_QUOTES, &x->dials.scare_quotes, argc, argv); }

/* ===========================================================================
   ASSIST
   =========================================================================== */
void orthomax_assist(t_ortho *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        strncpy_zero(s, "bang, tokens N, page N, section, cleardials, attributes", 512);
    } else {
        switch (a) {
            case 0: strncpy_zero(s, "words (list of symbols)", 512); break;
            case 1: strncpy_zero(s, "sources (0 fresh 1 function 2 topic 3 name 4 phrase)", 512); break;
        }
    }
}
