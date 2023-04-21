#pragma once

#include "ook.h"

bool solve_vlines(state *pState);
bool check_vlines(state *pState);

bool solve_hlines(state *pState);
bool check_hlines(state *pState);

bool solve_blocks(state *pState);
bool check_blocks(state *pState);

void recalc_done(state *pState);

bool check(const state *pState);

void simple_solve(state *pState);
bool checked_solve(state *pState);

void simple_solve_advanced(state *pState);
bool checked_solve_advanced(state *pState);

bool recursive_guess(state *pState, size_t *pGuesses, size_t *pTotalGuesses);
bool recursive_guess_advanced(state *pState, size_t *pGuesses, size_t *pTotalGuesses);
