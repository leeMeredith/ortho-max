/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 JD Clemon Lee Meredith
 */

/**
 * ============================================================================
 * simple_math.c — the implementation of the Max-free calculator engine
 * ============================================================================
 *
 * It includes ONLY its own header. No Max headers. That is the whole point:
 * this code has no idea it lives inside a Max object. (Compare xeno_follower.c
 * in bbox — same discipline.)
 *
 * Everything here is ordinary C you could have written in any program.
 * ============================================================================
 */

#include "simple_math.h"

/* ---------------------------------------------------------------------------
 * Initialize the engine to a known, safe starting state.
 * Always do this in your constructor so the struct never holds garbage.
 * ------------------------------------------------------------------------- */
void simple_math_init(t_simple_math *m)
{
    m->a           = 0.0;
    m->b           = 0.0;
    m->op          = SIMPLE_MATH_ADD;
    m->last_result = 0.0;
    m->error       = 0;
}

/* ---------------------------------------------------------------------------
 * The actual computation. This is the heart of the engine and the one place a
 * newcomer rewrites to make their own object.
 *
 * Note what it does NOT do: it doesn't print, doesn't send to an outlet,
 * doesn't know what an inlet is. It takes the state it was given, produces a
 * number, records whether anything went wrong, and returns. Pure and testable.
 * ------------------------------------------------------------------------- */
double simple_math_compute(t_simple_math *m)
{
    double result;

    m->error = 0;   /* assume success until proven otherwise */

    switch (m->op) {
        case SIMPLE_MATH_SUBTRACT:
            result = m->a - m->b;
            break;

        case SIMPLE_MATH_MULTIPLY:
            result = m->a * m->b;
            break;

        case SIMPLE_MATH_DIVIDE:
            if (m->b == 0.0) {
                /* Dividing by zero gives inf/nan, which spreads through a
                 * patch and corrupts everything downstream. Refuse it: flag
                 * the error and return a harmless 0. The GLUE will see the
                 * error flag and decide whether to warn the user — because
                 * deciding to talk to the user is Max's job, not ours. */
                m->error = 1;
                result   = 0.0;
            } else {
                result = m->a / m->b;
            }
            break;

        case SIMPLE_MATH_ADD:
        default:
            result = m->a + m->b;
            break;
    }

    m->last_result = result;
    return result;
}

/* ---------------------------------------------------------------------------
 * Setters. Trivial here, but they keep template.c from reaching directly
 * into the struct's fields. As an engine grows, having one controlled door
 * for each change (instead of the glue poking fields from the outside) is
 * what stops bugs from creeping in.
 *
 * simple_math_set_op clamps to the valid range so a bad value from the
 * outside can never put the engine into an unknown operation.
 * ------------------------------------------------------------------------- */
void simple_math_set_a(t_simple_math *m, double a)
{
    m->a = a;
}

void simple_math_set_b(t_simple_math *m, double b)
{
    m->b = b;
}

void simple_math_set_op(t_simple_math *m, long op)
{
    if (op < SIMPLE_MATH_ADD)    op = SIMPLE_MATH_ADD;
    if (op > SIMPLE_MATH_DIVIDE) op = SIMPLE_MATH_DIVIDE;
    m->op = op;
}
