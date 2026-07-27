/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 JD Clemon Lee Meredith
 */

/**
 * ============================================================================
 * ortho.c — A blank, heavily-documented starter external for Max 9
 * ============================================================================
 *
 * WHAT THIS IS
 *   The smallest *complete* Max external that still shows you every part
 *   you will actually need in a real object:
 *
 *       - multiple inlets        (left = a, right = b)
 *       - multiple outlets       (result, and a full list you can unpack)
 *       - methods for int/float/bang/list
 *       - an attribute you can set in the object box or inspector
 *       - attribute SAVE / RESTORE so the patch remembers its settings
 *       - assist strings (the tooltips you see when you hover inlets)
 *       - a SEPARATE ENGINE FILE that does the real work and knows nothing
 *         about Max  (simple_math.c / simple_math.h)
 *
 *   The "behavior" on purpose is the most boring thing imaginable: it takes
 *   two numbers and combines them (add / subtract / multiply / divide).
 *   A pocket calculator.
 *
 *   THIS FILE IS THE "GLUE." It talks to Max — inlets, outlets, attributes —
 *   and hands the numbers to the ENGINE (simple_math) to actually crunch.
 *   That two-file split is the most important thing to learn here. It's the
 *   same structure my [bbox] object uses, where the engine is xeno_follower.
 *
 *   When you build your own object, you rewrite simple_math.c with your own
 *   idea and adjust the few call sites here. Everything else is scaffolding
 *   you'd have written anyway.
 *
 * WHY A "PLAIN BOX" AND NOT A UI OBJECT
 *   Max has two broad kinds of external:
 *     1. a normal object box  [ortho]      <- THIS FILE
 *     2. a UI object you draw  (panels, dials, my [bbox] / [bcircle])
 *   UI objects drag in painting, mouse handling, views, zoom math — a lot
 *   of extra surface area. For learning the SDK you want #1. Start here,
 *   graduate to UI later.
 *
 * HOW TO RENAME THIS FOR YOUR OWN OBJECT
 *   Pick a name (say "myobj") and replace every "ortho" token with it,
 *   in this .c file, both CMakeLists.txt files, package-info.json, and the
 *   doc/help filenames. The class name passed to class_new() is what Max
 *   actually uses to find your object — the filename alone is NOT enough.
 *   (See the README "Make it your own" section.) The engine files
 *   (simple_math.*) can keep their names or be renamed to suit; they're
 *   generic and carry over either way.
 *
 * BUILD (macOS, Apple Silicon or Intel)
 *   cmake -G Xcode -B build
 *   cmake --build build --config Release
 *   codesign --force --deep -s - externals/ortho.mxo   <- MANDATORY on
 *                                                              Apple Silicon,
 *                                                              or Max silently
 *                                                              refuses to load.
 *
 * Build against: max-sdk-base, Max 9.
 * ============================================================================
 */

#include "ext.h"          /* the core Max API: objects, atoms, outlets   */
#include "ext_obex.h"     /* the "obex" object system: classes + attrs   */
#include "ext_obex_util.h"/* convenience macros for declaring attributes */

#include "simple_math.h"  /* OUR ENGINE — the Max-free calculator logic. */

/* ===========================================================================
   THE OBJECT STRUCT
   This is your object's memory. One of these is allocated per [ortho]
   in the patch. The FIRST member MUST be t_object — Max casts your pointer
   to t_object* constantly, so the object header has to sit at offset 0.
   (For UI objects the first member is t_jbox instead — same rule, fancier
   header.)

   Notice the engine lives INSIDE the object as a plain member: t_simple_math
   math. The Max object OWNS the engine. This is exactly how bbox carries its
   t_xeno_follower. The glue holds the engine and calls into it; the engine
   never reaches back out.
   =========================================================================== */
typedef struct _ortho {
    t_object       ob;            /* MUST BE FIRST — the Max object header   */

    t_simple_math  math;          /* THE ENGINE (see simple_math.h)          */

    void          *proxy_b;       /* the right inlet (inlet 1)               */
    long           proxy_inletnum;/* which inlet a message arrived on        */

    void          *outlet_result; /* outlet 0 (left):  the computed number   */
    void          *outlet_list;   /* outlet 1 (right): "a b result" as a list*/
} t_ortho;

/* ===========================================================================
   GLOBAL CLASS POINTER
   Every instance shares one class definition. Created once in ext_main().
   =========================================================================== */
static t_class *orthomax_class = NULL;

/* ===========================================================================
   CACHED SYMBOLS
   gensym() interns a string into a t_symbol. Doing it once at load time and
   reusing the pointer is faster than calling gensym() in a hot path.
   =========================================================================== */
static t_symbol *ps_list = NULL;   /* gensym("list") */

/* ===========================================================================
   FORWARD DECLARATIONS
   =========================================================================== */
void *orthomax_new   (t_symbol *s, long argc, t_atom *argv);
void  orthomax_free  (t_ortho *x);
void  orthomax_assist(t_ortho *x, void *b, long m, long a, char *s);

void  orthomax_int   (t_ortho *x, long n);
void  orthomax_float (t_ortho *x, double f);
void  orthomax_bang  (t_ortho *x);
void  orthomax_list  (t_ortho *x, t_symbol *s, long argc, t_atom *argv);

void  orthomax_output(t_ortho *x);   /* fires the two outlets           */

/*
 * Attribute setter for @op. We use a custom setter (instead of letting Max
 * write the field directly) so the new value passes through the engine's own
 * clamp via simple_math_set_op(). This keeps the engine the single authority
 * over its own state — the same reason bbox routes @follow through a setter.
 */
t_max_err orthomax_op_set(t_ortho *x, void *attr, long argc, t_atom *argv);

/* ===========================================================================
   EXT_MAIN — runs ONCE when Max first loads the external
   =========================================================================== */
void ext_main(void *r)
{
    t_class *c;

    ps_list = gensym("list");

    /*
     * class_new() — the heart of it.
     *   "ortho"      : the name Max types into the object box. THIS, not
     *                      the filename, is the real identity of your object.
     *   orthomax_new    : constructor, called per instance.
     *   orthomax_free   : destructor.
     *   sizeof(t_ortho): how much memory each instance needs (this now
     *                        includes the embedded engine struct).
     *   0L               : menu function (UI only) — none here.
     *   A_GIMME          : "give me the raw argument list" so we can read
     *                      typed-in arguments ourselves in new().
     */
    c = class_new("ortho",
                  (method)orthomax_new,
                  (method)orthomax_free,
                  sizeof(t_ortho),
                  0L,
                  A_GIMME, 0);

    /* --- METHODS: messages this object understands --- */
    class_addmethod(c, (method)orthomax_int,    "int",    A_LONG,   0);
    class_addmethod(c, (method)orthomax_float,  "float",  A_FLOAT,  0);
    class_addmethod(c, (method)orthomax_bang,   "bang",   A_NOTHING,0);
    class_addmethod(c, (method)orthomax_list,   "list",   A_GIMME,  0);
    class_addmethod(c, (method)orthomax_assist, "assist", A_CANT,   0);

    /* --- ATTRIBUTE: @op --- */
    /*
     * The field physically lives inside the engine: math.op. The macro can
     * point straight at it via the nested path (t_ortho, math.op). But we
     * also attach a custom setter so writes go through simple_math_set_op()
     * and get clamped. Order of operations when the user sets @op:
     *   user types @op 9  ->  orthomax_op_set runs  ->  simple_math_set_op
     *   clamps 9 down to 3 (Divide)  ->  stored safely in math.op.
     *
     * CLASS_ATTR_SAVE is the SAVE/RESTORE bit: with it, the chosen op is
     * written into the patcher file and restored on reload (applied by
     * attr_args_process in new()). Without it, @op resets every load.
     */
    CLASS_ATTR_LONG     (c, "op", 0, t_ortho, math.op);
    CLASS_ATTR_LABEL    (c, "op", 0, "Operation");
    CLASS_ATTR_ENUMINDEX(c, "op", 0, "Add Subtract Multiply Divide");
    CLASS_ATTR_ACCESSORS(c, "op", NULL, orthomax_op_set);  /* custom setter  */
    CLASS_ATTR_SAVE     (c, "op", 0);                        /* persist        */
    CLASS_ATTR_DEFAULT  (c, "op", 0, "0");                   /* default = Add  */

    /*
     * IMPORTANT — ENUMINDEX SUPPLIES LABELS, NOT PARSING.
     *
     * The four words above are only what the Inspector menu displays. The
     * attribute's actual value is a long, so it is set by INDEX:
     *
     *     op 2            <- multiply         [ortho @op 2]
     *     op multiply     <- does NOT work
     *
     * Sending a word is not an error and produces no warning. atom_getlong()
     * simply returns 0 for a symbol, so every misspelled or symbolic value
     * silently becomes 0 (Add). The symptom is an object that appears to
     * ignore the message — the first operation works and the others seem
     * dead, because they are all setting the same value.
     *
     * If you want your object to accept words, you have to parse them
     * yourself in the setter: check atom_gettype(argv) for A_SYM and compare
     * the symbol against your names before falling back to atom_getlong().
     */

    class_register(CLASS_BOX, c);
    orthomax_class = c;
}

/* ===========================================================================
   CONSTRUCTOR — one per [ortho] in the patch
   =========================================================================== */
void *orthomax_new(t_symbol *s, long argc, t_atom *argv)
{
    t_ortho *x = (t_ortho *)object_alloc(orthomax_class);
    if (!x) return NULL;

    /*
     * Initialize the ENGINE first, before anything can change its state.
     * This mirrors bbox calling xeno_follower_init() before jbox_new so a
     * restored attribute overrides the default rather than the other way
     * round. simple_math_init sets a/b/op/result to safe zeros.
     */
    simple_math_init(&x->math);

    /*
     * OUTLETS — created right-to-left, so declare the RIGHTMOST first.
     */
    x->outlet_list   = outlet_new((t_object *)x, NULL); /* rightmost: a b result */
    x->outlet_result = outlet_new((t_object *)x, NULL); /* leftmost : result     */

    /*
     * INLETS — the left inlet is automatic. The SECOND inlet uses a PROXY so
     * one set of methods can serve both inlets; proxy_getinlet() tells us
     * which inlet a message arrived on.
     */
    x->proxy_b = proxy_new((t_object *)x, 1, &x->proxy_inletnum);

    /*
     * APPLY ARGUMENTS + SAVED ATTRIBUTES.
     * attr_args_process() reads typed-in @attributes ([ortho @op 2])
     * AND, on patch load, applies SAVED attribute values (because @op is
     * marked CLASS_ATTR_SAVE). Because @op has a custom setter, the restored
     * value flows through simple_math_set_op() and lands clamped in the engine.
     * Call this AFTER inlets/outlets exist.
     *
     * (Plain boxes restore saved attrs through this call. UI/jbox objects use
     * a dictionary + CLASS_FLAG_NEWDICTIONARY instead — the extra wrinkle from
     * bbox. For a normal box, attr_args_process is all you need.)
     */
    attr_args_process(x, argc, argv);

    return x;
}

/* ===========================================================================
   DESTRUCTOR
   The engine is a plain embedded struct — it allocates nothing, so it needs
   no teardown. (If your engine ever allocates memory, give it a
   simple_math_free() and call it here, before freeing the proxy.) We only
   have to free the proxy inlet.
   =========================================================================== */
void orthomax_free(t_ortho *x)
{
    if (x->proxy_b) {
        object_free(x->proxy_b);
        x->proxy_b = NULL;
    }
}

/* ===========================================================================
   INPUT METHODS — where numbers come IN
   Left inlet is "hot" (triggers output); right inlet is "cold" (stores only).
   We never poke x->math fields directly — we ask the engine to update itself
   via its setters. The glue's job is routing, not arithmetic.
   =========================================================================== */

void orthomax_int(t_ortho *x, long n)
{
    orthomax_float(x, (double)n);   /* reuse the float path */
}

void orthomax_float(t_ortho *x, double f)
{
    long inlet = proxy_getinlet((t_object *)x);

    if (inlet == 1) {
        /* RIGHT inlet (cold): hand b to the engine, do NOT output. */
        simple_math_set_b(&x->math, f);
    } else {
        /* LEFT inlet (hot): hand a to the engine, then compute and output. */
        simple_math_set_a(&x->math, f);
        orthomax_output(x);
    }
}

void orthomax_bang(t_ortho *x)
{
    orthomax_output(x);             /* recompute with current a, b, op */
}

void orthomax_list(t_ortho *x, t_symbol *s, long argc, t_atom *argv)
{
    long inlet = proxy_getinlet((t_object *)x);

    /* atom_getfloat coerces int OR float atoms safely. */
    if (argc >= 1) simple_math_set_a(&x->math, atom_getfloat(argv));
    if (argc >= 2) simple_math_set_b(&x->math, atom_getfloat(argv + 1));

    if (inlet != 1) orthomax_output(x);  /* only the left inlet is hot */
}

/* ===========================================================================
   OUTPUT
   Ask the engine for a result, then fire BOTH outlets.

   Right outlet first, left (main) outlet last: in Max, output triggers
   downstream processing immediately, depth-first. Sending the detail list
   first means anything reacting to the result outlet already has matching
   detail data available.
   =========================================================================== */
void orthomax_output(t_ortho *x)
{
    /* THE ONE LINE THAT DOES THE WORK lives in the engine. */
    double result = simple_math_compute(&x->math);

    /*
     * The engine flags problems (like divide-by-zero) but never speaks to the
     * user — that's our job. Check the flag and warn from the glue, where the
     * Max console actually is.
     */
    if (x->math.error) {
        object_warn((t_object *)x, "divide by zero — returning 0");
    }

    /*
     * RIGHT outlet: the full picture as a list "a b result". Downstream you
     * can [unpack 0. 0. 0.] this — the same pattern [bbox] uses to report its
     * x/y on the right outlet.
     */
    t_atom out[3];
    atom_setfloat(out,     x->math.a);
    atom_setfloat(out + 1, x->math.b);
    atom_setfloat(out + 2, result);
    outlet_list(x->outlet_list, ps_list, 3, out);

    /* LEFT outlet: just the answer, as a float. */
    outlet_float(x->outlet_result, result);
}

/* ===========================================================================
   ATTRIBUTE SETTER for @op
   Routes the incoming value through the engine's clamp so the engine stays
   the single authority over its own op field. Returns MAX_ERR_NONE always;
   a bad value is clamped, not rejected.
   =========================================================================== */
t_max_err orthomax_op_set(t_ortho *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        simple_math_set_op(&x->math, atom_getlong(argv));
    }
    return MAX_ERR_NONE;
}

/* ===========================================================================
   ASSIST — inlet/outlet tooltips shown in edit mode
   =========================================================================== */
void orthomax_assist(t_ortho *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        switch (a) {
            case 0: strncpy_zero(s, "a (hot): number, bang, or list \"a b\"", 512); break;
            case 1: strncpy_zero(s, "b (cold): second number, stored only",  512); break;
        }
    } else { /* ASSIST_OUTLET */
        switch (a) {
            case 0: strncpy_zero(s, "result (float)",               512); break;
            case 1: strncpy_zero(s, "a b result (list to unpack)",  512); break;
        }
    }
}
