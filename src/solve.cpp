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
  bool changed = false;

  puts("Cross Check:");
  print_state(pState);

  uint16_t block_more_than_once[9];
  uint32_t block_more_than_once_triple[9];
  uint16_t hline_more_than_once[9];
  uint32_t hline_more_than_once_triple[9];
  uint32_t vline_more_than_once_triple[3];

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

      vline_more_than_once_triple[hb] = moreThanOnce;
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

      hline_more_than_once_triple[l] = moreThanOnce;

      const uint32_t t1 = (atLeastOnce >> 10);
      const uint32_t t2 = (atLeastOnce >> 20);
      moreThanOnce |= (atLeastOnce & t1) | (moreThanOnce >> 10);
      atLeastOnce |= t1;
      moreThanOnce |= (atLeastOnce & t2) | (moreThanOnce >> 20);

      hline_more_than_once[l] = (uint16_t)moreThanOnce & s_all;
    }
  }

  // Block Setup.
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

        block_more_than_once_triple[blockIndex] = moreThanOnce;

        const uint32_t t1 = (atLeastOnce >> 10);
        const uint32_t t2 = (atLeastOnce >> 20);
        moreThanOnce |= (atLeastOnce & t1) | (moreThanOnce >> 10);
        atLeastOnce |= t1;
        moreThanOnce |= (atLeastOnce & t2) | (moreThanOnce >> 20);

        block_more_than_once[blockIndex] = (uint16_t)moreThanOnce & s_all;
      }
    }
  }

  for (size_t hblock = 0; hblock < 3; hblock++)
  {
    for (uint32_t vline_offset = 0; vline_offset < 21; vline_offset += 10)
    {
      uint32_t vline_as_triple[3];
      memset(&vline_as_triple, 0, sizeof(vline_as_triple));

      // Turn vline into triples.
      {
        size_t l = 0;

        for (size_t vblock = 0; vblock < 3; vblock++)
          for (size_t sl_offset = 0; sl_offset < 21; sl_offset += 10, l++)
            vline_as_triple[vblock] |= (((pState->x[l * 3 + hblock] >> vline_offset) & s_all) << sl_offset);
      }

      puts("vline_as_triple");
      inspect_triple(vline_as_triple[0]);
      inspect_triple(vline_as_triple[1]);
      inspect_triple(vline_as_triple[2]);

      const uint32_t vline_more_than_once = (vline_more_than_once_triple[hblock] >> vline_offset) & digitAllowedMask;
      uint32_t candidates = vline_more_than_once >> 1;
      uint32_t currentDigit = 0;

      puts("vline_more_than_once");
      inspect_triple_value(vline_more_than_once);

      while (candidates)
      {
        DWORD bitIndex;
        BitScanForward(&bitIndex, candidates);
        currentDigit += (bitIndex + 1);
        candidates >>= (bitIndex + 1);

        printf("\tcurrentDigit: %" PRIu32 "\n", currentDigit);

        const uint32_t single_mask_0 = 1 << currentDigit;
        const uint32_t single_mask_1 = single_mask_0 << 10;
        const uint32_t single_mask_2 = single_mask_0 << 20;
        const uint32_t triple_mask = single_mask_0 | single_mask_1 | single_mask_2;

        uint8_t count[9];
        uint16_t count_mask[9];
        uint16_t line_mask[9];
        memset(&count, 0, sizeof(count));
        memset(&count_mask, 0, sizeof(count_mask));
        memset(&line_mask, 0, sizeof(line_mask));

        // for each hline, how many match this digit.
        {
          uint32_t hline_bit = 1;

          for (size_t hline = 0; hline < 9; hline++, hline_bit <<= 1)
          {
            if (pState->lineH & hline_bit)
              continue;

            if (hline_more_than_once[hline] & vline_more_than_once)
            {
              size_t line_count = 0;

              for (size_t hb = 0; hb < 3; hb++)
              {
                const uint32_t triple = pState->x[hline * 3 + hb];
                line_count += __popcnt64(triple & triple_mask);
                line_mask[hline] |= (!!(triple & single_mask_0) | (!!(triple & single_mask_1) << 1) | (!!(triple & single_mask_2) << 2)) << (hb * 3);
              }

              count[line_count]++;
              count_mask[line_count] |= hline_bit;
            }
          }

          for (size_t i = 0; i < 9; i++)
          {
            printf("\t\tcount %" PRIu64 ": %" PRIu8 "\n", i, count[i]);
            
            printf("\t\tcount_mask %" PRIu64 ": ", i);
            inspect_bits(count_mask[i]);

            printf("\t\tline_mask %" PRIu64 ": ", i);
            inspect_bits(line_mask[i]);
          }

          puts("");

          DEBUG_ASSERT(count[0] == 0);
        }

        uint8_t runningCount = 0;
        uint16_t runningMask = 0;

        for (size_t i = 2; i < 9; i++) // with 1, 9 there'd be nothing to eliminate. 0 would be invalid anyways.
        {
          runningCount += count[i];
          runningMask |= count_mask[i];

          if (runningCount < i) // There can now technically be n with x >= 2 same.
            continue;

          uint16_t runningMaskRemaining = runningMask;
          size_t lineIndex = (size_t)-1;

          while (runningMaskRemaining)
          {
            BitScanForward(&bitIndex, runningMaskRemaining);
            lineIndex += (bitIndex + 1);
            runningMaskRemaining >>= (bitIndex + 1);

            if (__popcnt64(runningMaskRemaining) + 1 < i)
              break; // not enough matches left to find i matching.

            if (__popcnt64(line_mask[lineIndex]) != i)
              continue; // this one doesn't actually contain i, so we better find a better one to compare with.

            printf("\t\tlineIndex %" PRIu64 " (looking for %" PRIu64 " %" PRIu32 "s, there should be %" PRIu64 " others)\n", lineIndex, i, currentDigit, __popcnt64(runningMaskRemaining));
            
            uint16_t runningMaskPair = runningMaskRemaining;
            size_t pairLineIndex = lineIndex;

            const uint32_t line0 = line_mask[lineIndex];
            uint32_t matchingLines = 1 << lineIndex;

            while (runningMaskPair)
            {
              BitScanForward(&bitIndex, runningMaskPair);
              pairLineIndex += (bitIndex + 1);
              runningMaskPair >>= (bitIndex + 1);

              const size_t matchCount = __popcnt64(line_mask[pairLineIndex] & line0);
              const size_t totalCount = __popcnt64(line_mask[pairLineIndex]);

              matchingLines |= ((!!(matchCount == totalCount)) << pairLineIndex);
            }

            if (__popcnt64(matchingLines) >= i) // we found >= i matching lines!
            {
              fputs("\t\tmatchingLines: ", stdout);
              inspect_bits(matchingLines);
              fputs("\t\ttouched (ln0): ", stdout);
              inspect_bits(line0);

              uint32_t touchedLeft = line0;
              size_t touchedIndex = (size_t)-1;

              const uint32_t currentDigitMask = (1 << currentDigit) * (1 | (1 << 10) | (1 << 20));

              // Purge everything else in all matching lines.
              while (touchedLeft)
              {
                BitScanForward(&bitIndex, touchedLeft);
                touchedIndex += (bitIndex + 1);
                touchedLeft >>= (bitIndex + 1);

                printf("\t\t\tmatchingLineIndex: %" PRIu64 "\n", touchedIndex);

                const size_t touchedOffsetIndex = touchedIndex / 3;
                const size_t touchedLineOffsetTripleValue = touchedIndex % 3;
                const uint32_t touchedLineTripleMask = s_all << (10 * touchedLineOffsetTripleValue);
                const uint32_t digitTripleMask = ~(currentDigitMask & touchedLineTripleMask);

                fputs("\t\t\tdigitTripleMask: ", stdout);
                inspect_triple(digitTripleMask);

                size_t lineBit = 1;

                for (size_t l = 0; l < 9; l++, lineBit <<= 1)
                {
                  if (matchingLines & lineBit)
                    continue;

                  const uint32_t lineTriple = pState->x[l * 3 + touchedOffsetIndex];
                  const uint32_t newValue = lineTriple & digitTripleMask;

                  printf("\t\t\t\tline %" PRIu64 " | before: ", l);
                  inspect_triple(lineTriple);

                  printf("\t\t\t\tline %" PRIu64 " | after:  ", l);
                  inspect_triple(newValue);

                  changed |= newValue != lineTriple;

                  pState->x[l * 3 + touchedOffsetIndex] = newValue;
                }
              }

              puts("\nAfter:");
              print_state(pState);

              digitAllowedMask &= ~(uint16_t)(1 << currentDigit);

              goto next_candidate;
            }
          }
        }

      next_candidate:;
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
