#pragma once

#include "ook.h"

bool parse(state *pState, const char *in, const size_t size);

void print(const state *pState);
void print_state(const state *pState);

void inspect_triple_value(const uint32_t value);
void inspect_triple(const uint32_t triple);
void inspect_bits(const uint64_t value);
