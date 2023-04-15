#pragma once

#include "ook.h"

bool parse(state *pState, const char *in, const size_t size);

void print(const state *pState);
void print_state(const state *pState);
