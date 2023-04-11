#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

#include <Windows.h>

#define OOK_ARRAYSIZE_C_STYLE(arrayName) (sizeof(arrayName) / sizeof(arrayName[0]))

#ifdef OOK_FORCE_ARRAYSIZE_C_STYLE
#define OOK_ARRAYSIZE(arrayName) OOK_ARRAYSIZE_C_STYLE(arrayName)
#else
template <typename T, size_t TCount>
inline constexpr size_t OOK_ARRAYSIZE(const T(&)[TCount]) { return TCount; }
#endif

#ifdef _DEBUG
#define DEBUG_ONLY_DEBUG_BREAK() __debugbreak()
#define DEBUG_ASSERT(conditional)  do { if (!(conditional)) __debugbreak(); } while (0)
#else
#define DEBUG_ONLY_DEBUG_BREAK() do { } while (0)
#define DEBUG_ASSERT(conditional) do { } while (0)
#endif

#define ERROR_IF(conditional, error) do { if ((conditional)) { puts(error); DEBUG_ONLY_DEBUG_BREAK(); return EXIT_FAILURE; } } while (0)

HANDLE _console = nullptr;

inline void _init_console()
{
  if (_console == nullptr)
    _console = GetStdHandle(STD_OUTPUT_HANDLE);
}

inline void _reset_console_color()
{
  fflush(stdout);

  _init_console();

  SetConsoleTextAttribute(_console, 7);
}

inline void _set_console_color(const int16_t fg, const int16_t bg)
{
  fflush(stdout);

  _init_console();

  SetConsoleTextAttribute(_console, (WORD)(fg | (bg << 8)));
}

inline uint64_t _get_ticks()
{
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);

  return now.QuadPart;
}

uint64_t _ticks_to_ns(const uint64_t ticks)
{
  LARGE_INTEGER freq;
  QueryPerformanceFrequency(&freq);

  return (ticks * 1000 * 1000 * 1000) / freq.QuadPart;
}

enum : uint32_t
{
  s_done = 1,
  s_1 = 1 << 1,
  s_2 = 1 << 2,
  s_3 = 1 << 3,
  s_4 = 1 << 4,
  s_5 = 1 << 5,
  s_6 = 1 << 6,
  s_7 = 1 << 7,
  s_8 = 1 << 8,
  s_9 = 1 << 9,
  s_all = s_1 | s_2 | s_3 | s_4 | s_5 | s_6 | s_7 | s_8 | s_9,
  s_initRow = s_all | (s_all << 10) | (s_all << 20),
};

struct state
{
  uint32_t x[3 * 9];
  uint16_t blocks, lineH;
};

void init(state *pState)
{
  pState->blocks = 0;
  pState->lineH = 0;

  for (size_t i = 0; i < OOK_ARRAYSIZE(pState->x); i++)
    pState->x[i] = s_initRow;
}

bool parse(state *pState, const char *in, const size_t size)
{
  size_t i = 0;

  for (size_t vb = 0; vb < 3; vb++)
  {
    for (size_t l = 0; l < 3; l++)
    {
      for (size_t hb = 0; hb < 3; hb++)
      {
        constexpr uint32_t mask = s_all | s_done;
        uint32_t offset = 0;

        for (size_t n = 0; n < 3; n++, offset += 10)
        {
          ERROR_IF(i >= size, "Unexpected end of file.");

          if (in[i] == ' ' || in[i] == '.' || in[i] == 'X')
          {
            i++;
            continue;
          }
          else if (hb == 2 && (in[i] == '\r' || in[i] == '\n'))
          {
            break;
          }

          ERROR_IF(in[i] < '1' || in[i] > '9', "Unexpected symbol in input file.");

          pState->x[(vb * 3 + l) * 3 + hb] &= ~(mask << offset);
          pState->x[(vb * 3 + l) * 3 + hb] |= ((s_done | (1 << (in[i] - '0'))) << offset);

          i++;
        }

        if (vb == 2 && l == 2 && hb == 2)
          return false;

        ERROR_IF(i >= size, "Unexpected end of file.");

        if (in[i] == '|' || in[i] == ',' || in[i] == '/')
          i++;
      }

      ERROR_IF(i >= size, "Unexpected end of file.");

      if (in[i] == '\r' || in[i] == '\n')
        i++;
      else
        ERROR_IF(false, "Unexpected symbol. Expected line end.");

      ERROR_IF(i >= size, "Unexpected end of file.");

      if (in[i] == '\r' || in[i] == '\n')
        i++;
    }

    for (size_t j = 0; j < 9 + 2; j++)
    {
      ERROR_IF(i >= size, "Unexpected end of file.");

      if (in[i] == '-' || in[i] == '=')
        i++;
      else if (i % 4 == 3 && (in[i] == '|' || in[i] == ',' || in[i] == '/'))
        i++;
      else
        break;
    }

    ERROR_IF(i >= size, "Unexpected end of file.");

    if (in[i] == '\r' || in[i] == '\n')
      i++;

    ERROR_IF(i >= size, "Unexpected end of file.");

    if (in[i] == '\r' || in[i] == '\n')
      i++;
  }

  return false;
}

void print(const state *pState)
{
  for (size_t vb = 0; vb < 3; vb++)
  {
    for (size_t l = 0; l < 3; l++)
    {
      for (size_t hb = 0; hb < 3; hb++)
      {
        uint32_t offset = 0;

        for (size_t n = 0; n < 3; n++, offset += 10)
        {
#ifdef _DEBUG
          const uint32_t v = (pState->x[(vb * 3 + l) * 3 + hb] >> offset) & (s_all | s_done);
#else
          const uint32_t v = (pState->x[(vb * 3 + l) * 3 + hb] >> offset);
#endif
          
          if (v & s_done)
          {
            DWORD index = 0;
            BitScanForward(&index, v & ~s_done);

            fputc('0' + index, stdout);
          }
          else
          {
            fputc(' ', stdout);
          }
        }

        if (hb < 2)
          fputc('|', stdout);
      }

      fputc('\n', stdout);
    }

    if (vb < 2)
    {
      for (size_t j = 0; j < 9 + 2; j++)
        fputc('-', stdout);

      fputc('\n', stdout);
    }
  }
}

void print_state(const state *pState)
{
  for (size_t vb = 0; vb < 3; vb++)
  {
    for (size_t l = 0; l < 3; l++)
    {
      for (uint32_t sl = 0; sl < 3; sl++)
      {
        if (pState->lineH & (1 << (vb * 3 + l)))
          fputs("$ ", stdout);
        else
          fputs("  ", stdout);

        for (size_t hb = 0; hb < 3; hb++)
        {
          uint32_t offset = 0;

          for (size_t n = 0; n < 3; n++, offset += 10)
          {
            const uint32_t v = (pState->x[(vb * 3 + l) * 3 + hb] >> offset);

            if (n > 0)
              fputs(" . ", stdout);

            _set_console_color(v & s_done ? 15 : 8, 2);

            for (uint32_t i = 0; i < 3; i++)
            {
              if (1 & (v >> (1 + i + sl * 3)))
                fputc('0' + (1 + i + sl * 3), stdout);
              else
                fputc(' ', stdout);
            }

            _reset_console_color();
          }

          if (hb < 2)
            fputs(" | ", stdout);
        }

        if (sl < 2)
          fputc('\n', stdout);
      }

      if (l < 2)
        fputs("\n  . . : . . : . . | . . : . . : . . | . . : . . : . .\n", stdout);
      else
        fputc('\n', stdout);
    }

    if (vb < 2)
      fputs("- ----:-----:-----|-----:-----:-----|-----:-----:----\n", stdout);
  }
}

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

int32_t main(const int32_t argc, char **ppArgv)
{
  ERROR_IF(argc <= 1, "Usage: ook <filename>");

  state s;
  init(&s);

  // Read File.
  {
    FILE *pFile = fopen(ppArgv[1], "rb");
    
    ERROR_IF(pFile == nullptr, "Failed to open file.");

    char data[(3 * 3 + 4) * 9 + (9 + 2) * 3];
    const size_t sizeRead = fread(data, 1, sizeof(data), pFile);
    
    fclose(pFile);

    ERROR_IF(parse(&s, data, sizeRead), "Failed to parse file.");
  }

  print(&s);

  // Simple Solve.
  {
    fputs("Simple Solve...", stdout);

    const uint64_t before = _get_ticks();
    simple_solve(&s);
    const uint64_t after = _get_ticks();

    printf(" (Completed in %9.6f ms)\n", (after - before) * 1e-6f);

    print_state(&s);
  }

  ERROR_IF(!check(&s), "No solutions found. (No Valid Options)");
  ERROR_IF(!check_blocks(&s), "No solutions found. (Invalid Block)");
  ERROR_IF(!check_vlines(&s), "No solutions found. (Invalid Vertical Line)");
  ERROR_IF(!check_hlines(&s), "No solutions found. (Invalid Horizontal Line)");

  // Recursively Guess.
  if (s.blocks != 0b111111111)
  {
    fputs("Guessing...", stdout);
  
    const uint64_t before = _get_ticks();
    
    size_t guesses = 0, total = 0;
    ERROR_IF(!recursive_guess(&s, &guesses, &total), "Failed to solve by guessing.");
    
    const uint64_t after = _get_ticks();
  
    printf(" (Completed in %9.6f ms | %" PRIu64 " consecutive guesses needed | %" PRIu64 " total)\n", (after - before) * 1e-6f, guesses, total);
  
    print(&s);
  }

  return EXIT_SUCCESS;
}
