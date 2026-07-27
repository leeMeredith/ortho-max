/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 JD Clemon Lee Meredith
 */

/**
 * ============================================================================
 * simple_math.h — the "engine" half of the template external
 * ============================================================================
 *
 * THIS FILE KNOWS NOTHING ABOUT MAX.
 *
 *   Notice there is no #include "ext.h" here. No t_object, no atoms, no
 *   outlets. This file is plain portable C. You could drop simple_math.c +
 *   simple_math.h into a totally unrelated program and it would compile and
 *   run exactly the same.
 *
 * WHY SPLIT THE LOGIC OUT INTO ITS OWN FILE?
 *   This is the single most useful habit in this whole template, and it's the
 *   same structure my [bbox] object uses (there the engine is called
 *   xeno_follower.c / .h). The idea:
 *
 *       template.c   = the GLUE. Talks to Max: inlets, outlets, attributes.
 *                       Knows nothing about HOW the math works.
 *       simple_math.c = the ENGINE. Does the actual work. Knows nothing
 *                       about Max.
 *
 *   Keeping them apart means:
 *     - You can test the engine without launching Max.
 *     - You can reuse the engine in another object (or another program).
 *     - When something breaks you instantly know which half to look in:
 *       a wrong number is an engine bug; a wrong inlet is a glue bug.
 *
 * HOW YOU MAKE THIS YOUR OWN
 *   Replace t_simple_math and the three functions below with whatever your
 *   object computes. As long as your engine stays Max-free, template.c only
 *   needs tiny changes to call it.
 *
 *   The include guard (#ifndef SIMPLE_MATH_H ...) stops this header being
 *   pasted in twice if multiple .c files include it. Always use one.
 * ============================================================================
 */

#ifndef SIMPLE_MATH_H
#define SIMPLE_MATH_H

/* ---------------------------------------------------------------------------
 * The operations this little engine knows how to do.
 * template.c exposes these to the user as the @op attribute, but the NAMES
 * and NUMBERS live here with the engine, because the engine is what acts on
 * them. The glue just passes one of these values in.
 * ------------------------------------------------------------------------- */
enum {
    SIMPLE_MATH_ADD      = 0,   /* a + b */
    SIMPLE_MATH_SUBTRACT = 1,   /* a - b */
    SIMPLE_MATH_MULTIPLY = 2,   /* a * b */
    SIMPLE_MATH_DIVIDE   = 3    /* a / b (safely handled) */
};

/* ---------------------------------------------------------------------------
 * The engine's state.
 *
 * For a calculator the "state" is tiny: the two operands, which operation to
 * perform, and the most recent result. A bigger engine (like the motion
 * follower in bbox) would carry far more here — velocities, frame counters,
 * thresholds — but the principle is identical: ALL the data the engine needs
 * lives in ONE struct that the glue owns.
 *
 * t_ prefix is the Max convention for "this is a type". We follow it even
 * though this struct never touches Max, so it sits comfortably alongside the
 * Max types in template.c.
 * ------------------------------------------------------------------------- */
typedef struct _simple_math {
    double a;          /* left operand  */
    double b;          /* right operand */
    long   op;         /* one of SIMPLE_MATH_* above */
    double last_result;/* result of the most recent compute() — handy for a
                        * bang to re-send without recomputing, and for
                        * inspecting state while debugging */

    char   error;      /* 0 = fine, 1 = the last compute hit a problem
                        * (e.g. divide by zero). The glue can read this and
                        * warn the user. The engine NEVER prints anything
                        * itself — printing is Max's job, so it stays in the
                        * glue. The engine only reports "something was off". */
} t_simple_math;

/* ---------------------------------------------------------------------------
 * The engine's public functions. Three is plenty.
 * ------------------------------------------------------------------------- */

/* Set sane starting values. Call once when the object is created, the same
 * way bbox calls xeno_follower_init(). */
void   simple_math_init   (t_simple_math *m);

/* Do the math. Reads a, b, and op from the struct; writes last_result and
 * error; returns the result too (so the caller can use it inline). */
double simple_math_compute(t_simple_math *m);

/* Convenience setters. These exist so template.c never has to reach inside
 * the struct and poke fields directly — it asks the engine to change its own
 * state. That keeps the engine in charge of its own data, which matters more
 * as engines get more complex. */
void   simple_math_set_a  (t_simple_math *m, double a);
void   simple_math_set_b  (t_simple_math *m, double b);
void   simple_math_set_op (t_simple_math *m, long op);

#endif /* SIMPLE_MATH_H */
