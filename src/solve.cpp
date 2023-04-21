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

#include "io.h"

#include <stdio.h>
#include <inttypes.h>

bool solve_cross_check(state *pState)
{
  static bool print_before_after = false;

  if (print_before_after)
  {
    puts("\nBefore:");
    print_state(pState);
  }

  bool changed = false;

  uint16_t moreThanOncePerHLine[9];
  uint32_t moreThanOncePerHLineTriple[9];
  uint32_t moreThanOncePerVLineTriple[3];

  uint16_t digitAllowedMask = s_all;

  // Vertical Line Setup.
  {
    for (size_t hb = 0; hb < 3; hb++)
    {
      uint32_t atLeastOnce = 0;
      uint32_t moreThanOnce = 0;

      for (size_t l = 0; l < 9; l++)
      {
        const uint32_t triple = pState->x[l * 3 + hb];

        moreThanOnce |= (atLeastOnce & triple);
        atLeastOnce |= triple;
      }

      moreThanOncePerVLineTriple[hb] = moreThanOnce;
    }
  }

  // Horizontal Line Setup.
  {
    uint16_t bit = 1;

    for (size_t l = 0; l < 9; l++, bit <<= 1)
    {
      if (pState->lineH & bit)
        continue;

      uint32_t atLeastOnce = 0;
      uint32_t moreThanOnce = 0;

      for (size_t hb = 0; hb < 3; hb++)
      {
        const uint32_t triple = pState->x[l * 3 + hb];

        moreThanOnce |= (atLeastOnce & triple);
        atLeastOnce |= triple;
      }

      moreThanOncePerHLineTriple[l] = moreThanOnce;

      const uint32_t t1 = (atLeastOnce >> 10);
      const uint32_t t2 = (atLeastOnce >> 20);
      moreThanOnce |= (atLeastOnce & t1) | (moreThanOnce >> 10);
      atLeastOnce |= t1;
      moreThanOnce |= (atLeastOnce & t2) | (moreThanOnce >> 20);

      moreThanOncePerHLine[l] = (uint16_t)moreThanOnce & s_all;
    }
  }

  for (size_t hblock = 0; hblock < 3; hblock++)
  {
    for (uint32_t vlineOffset = 0; vlineOffset < 21; vlineOffset += 10)
    {
      uint32_t vlineTriplePacked[3];
      memset(&vlineTriplePacked, 0, sizeof(vlineTriplePacked));

      // Turn hb into triples.
      {
        size_t l = 0;

        for (size_t vblock = 0; vblock < 3; vblock++)
          for (size_t sl_offset = 0; sl_offset < 21; sl_offset += 10, l++)
            vlineTriplePacked[vblock] |= (((pState->x[l * 3 + hblock] >> vlineOffset) & s_all) << sl_offset);
      }

      const uint32_t currentVLineMoreThanOnce = (moreThanOncePerVLineTriple[hblock] >> vlineOffset) & digitAllowedMask;
      uint32_t digitCandidateMask = currentVLineMoreThanOnce >> 1;
      uint32_t currentDigit = 0;

      while (digitCandidateMask)
      {
        DWORD bitIndex;
        BitScanForward(&bitIndex, digitCandidateMask);
        currentDigit += (bitIndex + 1);
        digitCandidateMask >>= (bitIndex + 1);

        const uint32_t digitMaskTriple0 = 1 << currentDigit;
        const uint32_t digitMaskTriple1 = digitMaskTriple0 << 10;
        const uint32_t digitMaskTriple2 = digitMaskTriple0 << 20;
        const uint32_t digitMaskAllTriples = digitMaskTriple0 | digitMaskTriple1 | digitMaskTriple2;

        uint8_t digitsPerHLineCount[9];
        uint16_t digitsPerHLineCount_HLineMasks[9];
        uint16_t digitsInHLineBitMask[9];
        memset(&digitsPerHLineCount, 0, sizeof(digitsPerHLineCount));
        memset(&digitsPerHLineCount_HLineMasks, 0, sizeof(digitsPerHLineCount_HLineMasks));
        memset(&digitsInHLineBitMask, 0, sizeof(digitsInHLineBitMask));

        // for each hline, how many match this digit.
        {
          uint32_t hline_bit = 1;

          for (size_t hline = 0; hline < 9; hline++, hline_bit <<= 1)
          {
            if (pState->lineH & hline_bit)
              continue;

            if (moreThanOncePerHLine[hline] & currentVLineMoreThanOnce)
            {
              size_t digitsinCurrentHLine = 0;

              for (size_t hb = 0; hb < 3; hb++)
              {
                const uint32_t triple = pState->x[hline * 3 + hb];
                digitsinCurrentHLine += __popcnt64(triple & digitMaskAllTriples);
                digitsInHLineBitMask[hline] |= (!!(triple & digitMaskTriple0) | (!!(triple & digitMaskTriple1) << 1) | (!!(triple & digitMaskTriple2) << 2)) << (hb * 3);
              }

              digitsPerHLineCount[digitsinCurrentHLine]++;
              digitsPerHLineCount_HLineMasks[digitsinCurrentHLine] |= hline_bit;
            }
          }

          DEBUG_ASSERT(digitsPerHLineCount[0] == 0);
        }

        uint8_t involvedDigitsCount = 0;
        uint16_t involvedHLinePositionMask = 0;

        for (size_t i = 2; i < 9; i++) // with 1, 9 there'd be nothing to eliminate. 0 would be invalid anyways.
        {
          involvedDigitsCount += digitsPerHLineCount[i];
          involvedHLinePositionMask |= digitsPerHLineCount_HLineMasks[i];

          if (involvedDigitsCount < i) // There can now technically be n with x >= 2 same.
            continue;

          uint16_t involvedHLinePositionMaskRemaining = involvedHLinePositionMask;
          size_t currentHLineIndex = (size_t)-1;

          while (involvedHLinePositionMaskRemaining)
          {
            BitScanForward(&bitIndex, involvedHLinePositionMaskRemaining);
            currentHLineIndex += (bitIndex + 1);
            involvedHLinePositionMaskRemaining >>= (bitIndex + 1);

            if (__popcnt64(involvedHLinePositionMaskRemaining) + 1 < i)
              break; // not enough matches left to find i matching.

            if (__popcnt64(digitsInHLineBitMask[currentHLineIndex]) != i)
              continue; // this one doesn't actually contain i, so we better find a better one to compare with.

            uint16_t possibleMatchingHLinePositionsMask = involvedHLinePositionMaskRemaining;
            size_t comparedHLineIndex = currentHLineIndex;

            const uint32_t currentHLineDigitBitMask = digitsInHLineBitMask[currentHLineIndex];
            uint32_t matchingHLinesBitMask = 1 << currentHLineIndex;

            while (possibleMatchingHLinePositionsMask)
            {
              BitScanForward(&bitIndex, possibleMatchingHLinePositionsMask);
              comparedHLineIndex += (bitIndex + 1);
              possibleMatchingHLinePositionsMask >>= (bitIndex + 1);

              const size_t currentWithComparedHLineMatches = __popcnt64(digitsInHLineBitMask[comparedHLineIndex] & currentHLineDigitBitMask);
              const size_t availablePossibilitiesPerComparedLine = __popcnt64(digitsInHLineBitMask[comparedHLineIndex]);

              matchingHLinesBitMask |= ((!!(currentWithComparedHLineMatches == availablePossibilitiesPerComparedLine)) << comparedHLineIndex);
            }

            if (__popcnt64(matchingHLinesBitMask) >= i) // we found >= i matching lines!
            {
              uint32_t matchedHLineDigitsRemaining = currentHLineDigitBitMask;
              size_t currentMatchedHLineIndex = (size_t)-1;

              const uint32_t currentDigitMask = (1 << currentDigit) * (1 | (1 << 10) | (1 << 20));

              // Purge everything else in all matching lines.
              while (matchedHLineDigitsRemaining)
              {
                BitScanForward(&bitIndex, matchedHLineDigitsRemaining);
                currentMatchedHLineIndex += (bitIndex + 1);
                matchedHLineDigitsRemaining >>= (bitIndex + 1);

                const size_t currentMatchedHLineOffset = currentMatchedHLineIndex / 3;
                const size_t currentMatchedHLineTripleOffset = currentMatchedHLineIndex % 3;
                const uint32_t currentMatchedHLineTripleMask = s_all << (10 * currentMatchedHLineTripleOffset);
                const uint32_t digitInMatchedHLineTripleMask = ~(currentDigitMask & currentMatchedHLineTripleMask);

                uint32_t nonContainedVLinesRemainingBitMask = (~matchingHLinesBitMask & 0b111111111); // these are lines!
                uint32_t nonContainedVLineIndex = (uint32_t)-1;

                while (nonContainedVLinesRemainingBitMask)
                {
                  BitScanForward(&bitIndex, nonContainedVLinesRemainingBitMask);
                  nonContainedVLineIndex += (bitIndex + 1);
                  nonContainedVLinesRemainingBitMask >>= (bitIndex + 1);

                  const uint32_t triple = pState->x[nonContainedVLineIndex * 3 + currentMatchedHLineOffset];
                  const uint32_t newTriple = triple & digitInMatchedHLineTripleMask;

                  changed |= newTriple != triple;

                  pState->x[nonContainedVLineIndex * 3 + currentMatchedHLineOffset] = newTriple;
                }
              }

              digitAllowedMask &= ~(uint16_t)(1 << currentDigit);

              goto next_candidate_vline;
            }
          }
        }

      next_candidate_vline:;
      }
    }
  }

  for (size_t hline = 0; hline < 9; hline++)
  {
    const uint32_t currentHLineMoreThanOnce = moreThanOncePerHLine[hline] & digitAllowedMask;
    uint32_t digitCandidateMask = currentHLineMoreThanOnce >> 1;
    uint32_t currentDigit = 0;

    while (digitCandidateMask)
    {
      DWORD bitIndex;
      BitScanForward(&bitIndex, digitCandidateMask);
      currentDigit += (bitIndex + 1);
      digitCandidateMask >>= (bitIndex + 1);

      const uint32_t digitMaskTriple0 = 1 << currentDigit;
      const uint32_t digitMaskTriple1 = digitMaskTriple0 << 10;
      const uint32_t digitMaskTriple2 = digitMaskTriple0 << 20;
      const uint32_t digitMaskAllTriples = digitMaskTriple0 | digitMaskTriple1 | digitMaskTriple2;

      uint8_t digitsPerVLineCount[9];
      uint16_t digitsPerVLineCount_VLineMasks[9];
      uint16_t digitsInVLineBitMask[9];
      memset(&digitsPerVLineCount, 0, sizeof(digitsPerVLineCount));
      memset(&digitsPerVLineCount_VLineMasks, 0, sizeof(digitsPerVLineCount_VLineMasks));
      memset(&digitsInVLineBitMask, 0, sizeof(digitsInVLineBitMask));

      // for each vline, how many match this digit.
      {
        for (size_t hb = 0; hb < 3; hb++)
        {
          if (moreThanOncePerVLineTriple[hb] & currentHLineMoreThanOnce)
          {
            size_t digitsInCurrentVLine_0 = 0;
            size_t digitsInCurrentVLine_1 = 0;
            size_t digitsInCurrentVLine_2 = 0;

            for (size_t vl = 0; vl < 9; vl++)
            {
              const uint32_t triple = pState->x[vl * 3 + hb];
              const uint32_t tripleDigits = triple & digitMaskAllTriples;
              const uint32_t tripleDigits_0 = tripleDigits & s_all;
              const uint32_t tripleDigits_1 = (tripleDigits >> 10) & s_all;
              const uint32_t tripleDigits_2 = (tripleDigits >> 20) & s_all;

              digitsInCurrentVLine_0 += __popcnt64(tripleDigits_0);
              digitsInCurrentVLine_1 += __popcnt64(tripleDigits_1);
              digitsInCurrentVLine_2 += __popcnt64(tripleDigits_2);

              digitsInVLineBitMask[hb * 3 + 0] |= !!(tripleDigits_0) << vl;
              digitsInVLineBitMask[hb * 3 + 1] |= !!(tripleDigits_1) << vl;
              digitsInVLineBitMask[hb * 3 + 2] |= !!(tripleDigits_2) << vl;
            }

            digitsPerVLineCount[digitsInCurrentVLine_0]++;
            digitsPerVLineCount[digitsInCurrentVLine_1]++;
            digitsPerVLineCount[digitsInCurrentVLine_2]++;

            digitsPerVLineCount_VLineMasks[digitsInCurrentVLine_0] |= (1 << (hb * 3 + 0));
            digitsPerVLineCount_VLineMasks[digitsInCurrentVLine_1] |= (1 << (hb * 3 + 1));
            digitsPerVLineCount_VLineMasks[digitsInCurrentVLine_2] |= (1 << (hb * 3 + 2));
          }
        }

        DEBUG_ASSERT(digitsPerVLineCount[0] == 0);
      }

      uint8_t involvedDigitsCount = 0;
      uint16_t involvedVLinePositionMask = 0;

      for (size_t i = 2; i < 9; i++) // with 1, 9 there'd be nothing to eliminate. 0 would be invalid anyways.
      {
        involvedDigitsCount += digitsPerVLineCount[i];
        involvedVLinePositionMask |= digitsPerVLineCount_VLineMasks[i];

        if (involvedDigitsCount < i) // There can now technically be n with x >= 2 same.
          continue;

        uint16_t involvedVLinePositionMaskRemaining = involvedVLinePositionMask;
        size_t currentVLineIndex = (size_t)-1;

        while (involvedVLinePositionMaskRemaining)
        {
          BitScanForward(&bitIndex, involvedVLinePositionMaskRemaining);
          currentVLineIndex += (bitIndex + 1);
          involvedVLinePositionMaskRemaining >>= (bitIndex + 1);

          if (__popcnt64(involvedVLinePositionMaskRemaining) + 1 < i)
            break; // not enough matches left to find i matching.

          if (__popcnt64(digitsInVLineBitMask[currentVLineIndex]) != i)
            continue; // this one doesn't actually contain i, so we better find a better one to compare with.

          uint16_t possibleMatchingVLinePositionsMask = involvedVLinePositionMaskRemaining;
          size_t comparedVLineIndex = currentVLineIndex;

          const uint32_t currentVLineDigitBitMask = digitsInVLineBitMask[currentVLineIndex];
          uint32_t matchingVLinesBitMask = 1 << currentVLineIndex;

          while (possibleMatchingVLinePositionsMask)
          {
            BitScanForward(&bitIndex, possibleMatchingVLinePositionsMask);
            comparedVLineIndex += (bitIndex + 1);
            possibleMatchingVLinePositionsMask >>= (bitIndex + 1);

            const size_t currentWithComparedVLineMatches = __popcnt64(digitsInVLineBitMask[comparedVLineIndex] & currentVLineDigitBitMask);
            const size_t availablePossibilitiesPerComparedLine = __popcnt64(digitsInVLineBitMask[comparedVLineIndex]);

            matchingVLinesBitMask |= ((!!(currentWithComparedVLineMatches == availablePossibilitiesPerComparedLine)) << comparedVLineIndex);
          }

          if (__popcnt64(matchingVLinesBitMask) >= i) // we found >= i matching lines!
          {
            uint32_t matchedVLineDigitsRemaining = currentVLineDigitBitMask;
            size_t currentMatchedVLineIndex = (size_t)-1;

            const uint32_t currentDigitInverseTripleMask = (~(1 << currentDigit) & (s_all | s_done)) * (1 | (1 << 10) | (1 << 20));

            const uint32_t hlineDigitBits_0 = ((matchingVLinesBitMask & 0b1) | ((matchingVLinesBitMask & 0b10) << (10 - 1)) | ((matchingVLinesBitMask & 0b100) << (20 - 2))) << currentDigit;
            const uint32_t hlineDigitBits_1 = (((matchingVLinesBitMask & 0b1000) >> 3) | ((matchingVLinesBitMask & 0b10000) << (10 - 4)) | ((matchingVLinesBitMask & 0b100000) << (20 - 5))) << currentDigit;
            const uint32_t hlineDigitBits_2 = (((matchingVLinesBitMask & 0b1000000) >> 6) | ((matchingVLinesBitMask & 0b10000000) << (10 - 7)) | ((matchingVLinesBitMask & 0b100000000) << (20 - 8))) << currentDigit;

            const uint32_t hlineMask_0 = currentDigitInverseTripleMask | hlineDigitBits_0;
            const uint32_t hlineMask_1 = currentDigitInverseTripleMask | hlineDigitBits_1;
            const uint32_t hlineMask_2 = currentDigitInverseTripleMask | hlineDigitBits_2;

            // Purge everything else in all matching lines.
            while (matchedVLineDigitsRemaining)
            {
              BitScanForward(&bitIndex, matchedVLineDigitsRemaining);
              currentMatchedVLineIndex += (bitIndex + 1);
              matchedVLineDigitsRemaining >>= (bitIndex + 1);

              const uint32_t triple0 = pState->x[currentMatchedVLineIndex * 3 + 0];
              const uint32_t triple1 = pState->x[currentMatchedVLineIndex * 3 + 1];
              const uint32_t triple2 = pState->x[currentMatchedVLineIndex * 3 + 2];

              const uint32_t newTriple0 = triple0 & hlineMask_0;
              const uint32_t newTriple1 = triple1 & hlineMask_1;
              const uint32_t newTriple2 = triple2 & hlineMask_2;

              changed |= (newTriple0 != triple0) | (newTriple1 != triple1) | (newTriple2 != triple2);

              pState->x[currentMatchedVLineIndex * 3 + 0] = newTriple0;
              pState->x[currentMatchedVLineIndex * 3 + 1] = newTriple1;
              pState->x[currentMatchedVLineIndex * 3 + 2] = newTriple2;
            }

            digitAllowedMask &= ~(uint16_t)(1 << currentDigit);
          }

          goto next_candidate_hline;
        }
      }

    next_candidate_hline:;
    }
  }

  // block on hline/vline.
  {
    uint16_t bit = 1;
    uint16_t blockIndex = 0;

    for (size_t vb = 0; vb < 3; vb++)
    {
      for (size_t hb = 0; hb < 3; hb++, bit <<= 1, blockIndex++)
      {
        if (pState->blocks & bit)
          continue;

        uint32_t atLeastOnce = 0;
        uint32_t moreThanOnce = 0;

        for (size_t vt = 0; vt < 3; vt++)
        {
          const uint32_t triple = pState->x[vb * 9 + hb + vt * 3];

          moreThanOnce |= (atLeastOnce & triple);
          atLeastOnce |= triple;
        }

        const uint32_t atLeastOncePerBlockTriple = atLeastOnce;

        const uint32_t t1 = (atLeastOnce >> 10);
        const uint32_t t2 = (atLeastOnce >> 20);
        moreThanOnce |= (atLeastOnce & t1) | (moreThanOnce >> 10);
        atLeastOnce |= t1;
        moreThanOnce |= (atLeastOnce & t2) | (moreThanOnce >> 20);

        // Block on vline.
        {
          const uint32_t atLeastOncePerBlockTripleRot1 = (atLeastOncePerBlockTriple >> 10) | (atLeastOncePerBlockTriple << 20);
          const uint32_t atLeastOncePerBlockTripleRot2 = (atLeastOncePerBlockTripleRot1 >> 10) | (atLeastOncePerBlockTripleRot1 << 20);

          const uint32_t blockTripleOnVLineTriple = atLeastOncePerBlockTriple & ~atLeastOncePerBlockTripleRot1 & ~atLeastOncePerBlockTripleRot2 & moreThanOncePerVLineTriple[hb]; // we can handle digits not being "allowed", unlike earlier loops.
          const uint32_t invBlockTripleOnVlineTriple = ~blockTripleOnVLineTriple | (s_done | (s_done << 10) | (s_done << 20));

          if (blockTripleOnVLineTriple)
          {
            for (size_t vbl = 0; vbl < 3; vbl++)
            {
              if (vbl == vb)
                continue;

              for (size_t vl = 0; vl < 3; vl++)
              {
                const uint32_t triple = pState->x[(vbl * 3 + vl) * 3 + hb];
                const uint32_t newTriple = triple & invBlockTripleOnVlineTriple;

                changed |= (triple != newTriple);

                pState->x[(vbl * 3 + vl) * 3 + hb] = newTriple;
              }
            }
          }
        }
      }
    }
  }

  return changed;
}

void simple_solve(state *pState)
{
  uint8_t check_pattern = 0b111;

  while (true)
  {
    if (check_pattern & 0b001)
    {
      check_pattern &= 0b110;

      if (solve_vlines(pState))
      {
        recalc_done(pState);
        check_pattern = 0b110;
      }
    }

    if (check_pattern & 0b010)
    {
      check_pattern &= 0b101;

      if (solve_hlines(pState))
      {
        recalc_done(pState);
        check_pattern = 0b101;
      }
    }

    if (check_pattern & 0b100)
    {
      check_pattern &= 0b001;

      if (solve_blocks(pState))
      {
        recalc_done(pState);
        check_pattern = 0b011;
      }
    }

    if (!check_pattern)
    {
      if (pState->blocks == 0b111111111 || pState->lineH == 0b111111111)
        break;

      if (solve_cross_check(pState))
      {
        recalc_done(pState);
        check_pattern = 0b111;
      }
      else
      {
        break;
      }
    }
  }
}

bool checked_solve(state *pState)
{
  uint8_t check_pattern = 0b111;

  while (check_pattern)
  {
    if (check_pattern & 0b001)
    {
      check_pattern &= 0b110;

      if (solve_vlines(pState))
      {
        recalc_done(pState);

        if (!check_blocks(pState) || !check_hlines(pState) || !check(pState))
          return false;

        check_pattern = 0b110;
      }
    }

    if (check_pattern & 0b010)
    {
      check_pattern &= 0b101;

      if (solve_hlines(pState))
      {
        recalc_done(pState);

        if (!check_blocks(pState) || !check_vlines(pState) || !check(pState))
          return false;

        check_pattern = 0b101;
      }
    }

    if (check_pattern & 0b100)
    {
      check_pattern &= 0b011;

      if (solve_blocks(pState))
      {
        recalc_done(pState);

        if (!check_vlines(pState) || !check_hlines(pState) || !check(pState))
          return false;

        check_pattern = 0b011;
      }
    }

    if (!check_pattern)
    {
      if (pState->blocks == 0b111111111 || pState->lineH == 0b111111111)
        break;

      if (solve_cross_check(pState))
      {
        recalc_done(pState);

        if (!check_vlines(pState) || !check_hlines(pState) || !check(pState))
          return false;

        check_pattern = 0b111;
      }
      else
      {
        break;
      }
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
