#include "solve.h"

#include <Windows.h>

bool solve_vlines(state *pState)
{
  bool changed = false;

  for (size_t hb = 0; hb < 3; hb++)
  {
    uint32_t doneMask = 0;
    uint32_t atLeastOnce = 0;
    uint32_t moreThanOnce = 0;

    for (size_t l = 0; l < 9; l++)
    {
      const uint32_t triple = pState->x[l * 3 + hb];
      const uint32_t mask = (triple & (s_done | (s_done << 10) | (s_done << 20))) * s_all;

      doneMask |= (triple & mask);
      moreThanOnce |= (atLeastOnce & triple);
      atLeastOnce |= triple;
    }

    const uint32_t exactlyOnce = (atLeastOnce & ~moreThanOnce) & ~(s_done | (s_done << 10) | (s_done << 20));

    for (size_t l = 0; l < 9; l++)
    {
      const uint32_t triple = pState->x[l * 3 + hb];

      // Remove values that already occur in this line.
      const uint32_t mask0 = ((~triple) & (s_done | (s_done << 10) | (s_done << 20))) * s_all;
      const uint32_t new0 = triple & ~(mask0 & doneMask);

      // Select values that only occur once in this line.
      const uint32_t once = triple & exactlyOnce;
      const uint32_t _0 = !(once & s_all) * s_all;
      const uint32_t _1 = !(once & (s_all << 10)) * (s_all << 10);
      const uint32_t _2 = !(once & (s_all << 20)) * (s_all << 20);
      const uint32_t mask1 = ((once | _0) | (_1 | _2)) | (s_done | (s_done << 10) | (s_done << 20));
      const uint32_t new1 = new0 & mask1;

      changed |= (new1 != triple);

      pState->x[l * 3 + hb] = new1;
    }
  }

  return changed;
}

bool check_vlines(state *pState)
{
  for (size_t hb = 0; hb < 3; hb++)
  {
    uint32_t found = 0;

    for (size_t l = 0; l < 9; l++)
    {
      const uint32_t triple = pState->x[l * 3 + hb];
      const uint32_t mask = (triple & (s_done | (s_done << 10) | (s_done << 20))) * s_all;
      const uint32_t maskedTriple = triple & mask;

      if (maskedTriple & found)
        return false;

      found |= maskedTriple;
    }
  }

  return true;
}

bool solve_hlines(state *pState)
{
  bool changed = false;

  uint16_t bit = 1;

  for (size_t l = 0; l < 9; l++, bit <<= 1)
  {
    if (pState->lineH & bit)
      continue;

    uint32_t doneMask = 0;
    uint32_t atLeastOnce = 0;
    uint32_t moreThanOnce = 0;

    for (size_t hb = 0; hb < 3; hb++)
    {
      const uint32_t triple = pState->x[l * 3 + hb];
      const uint32_t mask = (triple & (s_done | (s_done << 10) | (s_done << 20))) * s_all;

      doneMask |= (triple & mask);
      moreThanOnce |= (atLeastOnce & triple);
      atLeastOnce |= triple;
    }

    // Not sure which option is faster, but the second one feels smarter.
    // Option A:
    // doneMask |= ((doneMask >> 10) | (doneMask >> 20));
    // const uint32_t foundMask = doneMask | (doneMask << 10) | (doneMask << 20);

    // Option B:
    const uint32_t b1 = (doneMask >> 10) & s_all;
    const uint32_t b2 = (doneMask >> 20) & s_all;
    doneMask = ((doneMask & s_all) | b1 | b2);
    const uint32_t foundMask = doneMask * (1 | (1 << 10) | (1 << 20));

    const uint32_t t1 = (atLeastOnce >> 10);
    const uint32_t t2 = (atLeastOnce >> 20);
    moreThanOnce |= (atLeastOnce & t1) | (moreThanOnce >> 10);
    atLeastOnce |= t1;
    moreThanOnce |= (atLeastOnce & t2) | (moreThanOnce >> 20);
    atLeastOnce |= t2;
    atLeastOnce &= s_all;

    const uint32_t exactlyOnce = (atLeastOnce & ~moreThanOnce) * (1 | (1 << 10) | (1 << 20));

    for (size_t hb = 0; hb < 3; hb++)
    {
      const uint32_t triple = pState->x[l * 3 + hb];

      // Remove values that already occur in this line.
      const uint32_t mask0 = ((~triple) & (s_done | (s_done << 10) | (s_done << 20))) * s_all;
      const uint32_t new0 = (triple & ~(mask0 & foundMask));

      // Select values that only occur once in this line.
      const uint32_t once = triple & exactlyOnce;
      const uint32_t _0 = !(once & s_all) * s_all;
      const uint32_t _1 = !(once & (s_all << 10)) * (s_all << 10);
      const uint32_t _2 = !(once & (s_all << 20)) * (s_all << 20);
      const uint32_t mask1 = ((once | _0) | (_1 | _2)) | (s_done | (s_done << 10) | (s_done << 20));
      const uint32_t new1 = new0 & mask1;

      changed |= (new1 != triple);

      pState->x[l * 3 + hb] = new1;
    }

    pState->lineH |= ((!!(doneMask == s_all)) << l);
  }

  return changed;
}

bool check_hlines(state *pState)
{
  constexpr uint32_t m0 = s_all;
  constexpr uint32_t m1 = s_all << 10;
  constexpr uint32_t m2 = s_all << 20;

  size_t bit = 1;

  for (size_t l = 0; l < 9; l++, bit <<= 1)
  {
    if (pState->lineH & bit)
      continue;

    uint32_t found = 0;

    for (size_t hb = 0; hb < 3; hb++)
    {
      const uint32_t triple = pState->x[l * 3 + hb];
      const uint32_t mask = (triple & (s_done | (s_done << 10) | (s_done << 20))) * s_all;
      const uint32_t maskedTriple = triple & mask;
      const uint32_t t0 = maskedTriple & m0;
      const uint32_t t1 = (maskedTriple & m1) >> 10;
      const uint32_t t2 = (maskedTriple & m2) >> 20;

      if ((t0 & t1) || (t0 & t2) || (t1 & t2))
        return false;

      const uint32_t combined = (t0 | t1 | t2);

      if (combined & found)
        return false;

      found |= combined;
    }
  }

  return true;
}

bool solve_blocks(state *pState)
{
  bool changed = false;

  uint16_t bit = 1;
  uint16_t blockIndex = 0;

  for (size_t vb = 0; vb < 3; vb++)
  {
    for (size_t hb = 0; hb < 3; hb++, bit <<= 1, blockIndex++)
    {
      if (pState->blocks & bit)
        continue;

      uint32_t doneMask = 0;
      uint32_t atLeastOnce = 0;
      uint32_t moreThanOnce = 0;

      for (size_t vt = 0; vt < 3; vt++)
      {
        const uint32_t triple = pState->x[vb * 9 + hb + vt * 3];
        const uint32_t mask = (triple & (s_done | (s_done << 10) | (s_done << 20))) * s_all;

        doneMask |= (triple & mask);
        moreThanOnce |= (atLeastOnce & triple);
        atLeastOnce |= triple;
      }

      // Not sure which option is faster, but the second one feels smarter.
      // Option A:
      // doneMask |= ((doneMask >> 10) | (doneMask >> 20));
      // const uint32_t foundMask = doneMask | (doneMask << 10) | (doneMask << 20);

      // Option B:
      const uint32_t b1 = (doneMask >> 10) & s_all;
      const uint32_t b2 = (doneMask >> 20) & s_all;
      doneMask = ((doneMask & s_all) | b1 | b2);
      const uint32_t foundMask = doneMask * (1 | (1 << 10) | (1 << 20));

      const uint32_t t1 = (atLeastOnce >> 10);
      const uint32_t t2 = (atLeastOnce >> 20);
      moreThanOnce |= (atLeastOnce & t1) | (moreThanOnce >> 10);
      atLeastOnce |= t1;
      moreThanOnce |= (atLeastOnce & t2) | (moreThanOnce >> 20);
      atLeastOnce |= t2;
      atLeastOnce &= s_all;

      const uint32_t exactlyOnce = (atLeastOnce & ~moreThanOnce) * (1 | (1 << 10) | (1 << 20));

      for (size_t vt = 0; vt < 3; vt++)
      {
        const uint32_t triple = pState->x[vb * 9 + hb + vt * 3];

        // Remove values that already occur in this block.
        const uint32_t mask0 = ((~triple) & (s_done | (s_done << 10) | (s_done << 20))) * s_all;
        const uint32_t new0 = (triple & ~(mask0 & foundMask));

        // Select values that only occur once in this block.
        const uint32_t once = triple & exactlyOnce;
        const uint32_t _0 = !(once & s_all) * s_all;
        const uint32_t _1 = !(once & (s_all << 10)) * (s_all << 10);
        const uint32_t _2 = !(once & (s_all << 20)) * (s_all << 20);
        const uint32_t mask1 = ((once | _0) | (_1 | _2)) | (s_done | (s_done << 10) | (s_done << 20));
        const uint32_t new1 = new0 & mask1;

        changed |= (new1 != triple);

        pState->x[vb * 9 + hb + vt * 3] = new1;
      }

      pState->blocks |= ((!!(doneMask == s_all)) << blockIndex);
    }
  }

  return changed;
}

bool check_blocks(state *pState)
{
  constexpr uint32_t m0 = s_all;
  constexpr uint32_t m1 = s_all << 10;
  constexpr uint32_t m2 = s_all << 20;

  size_t bit = 1;

  for (size_t vb = 0; vb < 3; vb++)
  {
    for (size_t hb = 0; hb < 3; hb++, bit <<= 1)
    {
      if (pState->blocks & bit)
        continue;

      uint32_t found = 0;

      for (size_t vt = 0; vt < 3; vt++)
      {
        const uint32_t triple = pState->x[vb * 9 + hb + vt * 3];
        const uint32_t mask = (triple & (s_done | (s_done << 10) | (s_done << 20))) * s_all;
        const uint32_t maskedTriple = triple & mask;
        const uint32_t t0 = maskedTriple & m0;
        const uint32_t t1 = (maskedTriple & m1) >> 10;
        const uint32_t t2 = (maskedTriple & m2) >> 20;

        if ((t0 & t1) || (t0 & t2) || (t1 & t2))
          return false;

        const uint32_t combined = (t0 | t1 | t2);

        if (combined & found)
          return false;

        found |= combined;
      }
    }
  }

  return true;
}

void recalc_done(state *pState)
{
  constexpr uint32_t m0 = s_all;
  constexpr uint32_t m1 = s_all << 10;
  constexpr uint32_t m2 = s_all << 20;

  for (size_t i = 0; i < OOK_ARRAYSIZE(pState->x); i++)
  {
    const uint32_t triple = pState->x[i];
    const uint32_t t0 = !!(__popcnt64(triple & m0) == 1);
    const uint32_t t1 = !!(__popcnt64(triple & m1) == 1);
    const uint32_t t2 = !!(__popcnt64(triple & m2) == 1);

    pState->x[i] |= (t0 | (t1 << 10) | (t2 << 20));
  }
}

bool check(const state *pState)
{
  constexpr uint32_t m0 = s_all;
  constexpr uint32_t m1 = s_all << 10;
  constexpr uint32_t m2 = s_all << 20;

  for (size_t i = 0; i < OOK_ARRAYSIZE(pState->x); i++)
  {
    const uint32_t triple = pState->x[i];

    if ((__popcnt64(triple & m0) == 0) || (__popcnt64(triple & m1) == 0) || (__popcnt64(triple & m2) == 0))
      return false;
  }

  return true;
}

void simple_solve(state *pState)
{
  bool found = true;

  while (found)
  {
    found = false;

    if (solve_vlines(pState))
    {
      recalc_done(pState);
      found = true;
    }

    if (solve_hlines(pState))
    {
      recalc_done(pState);
      found = true;
    }

    if (solve_blocks(pState))
    {
      recalc_done(pState);
      found = true;
    }
  }
}

bool checked_solve(state *pState)
{
  bool found = true;

  while (found)
  {
    found = false;

    if (solve_vlines(pState))
    {
      recalc_done(pState);

      if (!check_blocks(pState) || !check_hlines(pState) || !check(pState))
        return false;

      found = true;
    }

    if (solve_hlines(pState))
    {
      recalc_done(pState);

      if (!check_blocks(pState) || !check_vlines(pState) || !check(pState))
        return false;

      found = true;
    }

    if (solve_blocks(pState))
    {
      recalc_done(pState);

      if (!check_vlines(pState) || !check_hlines(pState) || !check(pState))
        return false;

      found = true;
    }
  }

  return true;
}

size_t find_lowest(state *pState, size_t *pSubIndex)
{
  size_t least = 11;
  size_t index = 0;
  size_t subIndex = 0;

  constexpr uint32_t m0 = s_all;
  constexpr uint32_t m1 = s_all << 10;
  constexpr uint32_t m2 = s_all << 20;

  constexpr uint32_t d0 = 1;
  constexpr uint32_t d1 = 1 << 10;
  constexpr uint32_t d2 = 1 << 20;

  for (size_t i = 0; i < OOK_ARRAYSIZE(pState->x); i++)
  {
    const uint32_t triple = pState->x[i];

    const size_t t0 = __popcnt64(triple & m0) + (!!(triple & d0) * 9);
    const size_t t1 = __popcnt64(triple & m1) + (!!(triple & d1) * 9);
    const size_t t2 = __popcnt64(triple & m2) + (!!(triple & d2) * 9);

    size_t idx = 2;
    size_t l = t2;

    if (t0 < t2)
    {
      if (t0 < t1)
      {
        idx = 0;
        l = t0;
      }
      else if (t1 < t2)
      {
        idx = 1;
        l = t1;
      }
    }
    else if (t1 < t2)
    {
      idx = 1;
      l = t1;
    }

    if (l < least)
    {
      least = l;
      index = i;
      subIndex = idx;

      if (l == 2)
        break;
    }
  }

  *pSubIndex = subIndex;

  return index;
}

bool recursive_guess(state *pState, size_t *pGuesses, size_t *pTotalGuesses)
{
  if (pState->blocks == 0b111111111)
    return true;

  size_t subIndex;
  const size_t tripleIndex = find_lowest(pState, &subIndex);
  subIndex *= 10;

  const uint32_t options = (pState->x[tripleIndex] >> subIndex) & (s_all | s_done);
  DEBUG_ASSERT(!(options & s_done));

  uint32_t offset = 0;
  uint32_t last = options >> 1;

  DWORD i;
  BitScanForward(&i, last);
  offset += (i + 1);
  last >>= (i + 1);

  // guess.
  while (true)
  {
    state s = *pState;

    s.x[tripleIndex] &= ((~(s_all << subIndex)) | (1 << (offset + subIndex)));
    s.x[tripleIndex] |= (s_done << (subIndex));

    (*pTotalGuesses)++;

    if (checked_solve(&s) && recursive_guess(&s, pGuesses, pTotalGuesses))
    {
      (*pGuesses)++;
      *pState = s;
      return true;
    }

    if (last == 0)
      break;

    BitScanForward(&i, last);
    offset += (i + 1);
    last >>= (i + 1);
  }

  return false;
}
