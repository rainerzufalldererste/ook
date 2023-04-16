#include "ook.h"

#include <stdio.h>
#include <inttypes.h>

#include <Windows.h>

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

void inspect_triple_value_internal(const uint32_t value)
{
  if (value & s_done)
    fputs("(done) ", stdout);

  for (uint8_t i = 1; i <= 9; i++)
  {
    if (value & (1 << i))
      fputc('0' + i, stdout);
    else
      fputc('.', stdout);
  }
}

void inspect_triple_value(const uint32_t value)
{
  inspect_triple_value_internal(value);
  puts("");
}

void inspect_triple(const uint32_t triple)
{
  size_t tripleIndex = 0;

  fputs("[ ", stdout);

  for (size_t offset = 0; offset < 21; offset += 10, tripleIndex++)
  {
    if (offset > 0)
      fputs("  |  ", stdout);

    printf("%" PRIu64 ": ", tripleIndex);
    inspect_triple_value_internal(triple >> offset);
  }

  puts(" ]");
}

void inspect_bits(const uint64_t value)
{
  uint64_t v = value;

  for (size_t i = 0; i < 64; i++, v >>= 1)
  {
    if (v == 0)
    {
      fputc('/', stdout);
      break;
    }

    if (v & 1)
      printf("%" PRIX64 " ", i);
    else
      fputs(". ", stdout);
  }

  puts("");
}
