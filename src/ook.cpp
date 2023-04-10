#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <Windows.h>

#define OOK_ARRAYSIZE_C_STYLE(arrayName) (sizeof(arrayName) / sizeof(arrayName[0]))

#ifdef OOK_FORCE_ARRAYSIZE_C_STYLE
#define OOK_ARRAYSIZE(arrayName) OOK_ARRAYSIZE_C_STYLE(arrayName)
#else
template <typename T, size_t TCount>
inline constexpr size_t OOK_ARRAYSIZE(const T(&)[TCount]) { return TCount; }
#endif

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
  s_init = s_1 | s_2 | s_3 | s_4 | s_5 | s_6 | s_7 | s_8 | s_9,
  s_initRow = s_init | (s_init << 10) | (s_init << 20),
};

#ifdef _DEBUG
#define DEBUG_ONLY_DEBUG_BREAK() __debugbreak()
#else
#define DEBUG_ONLY_DEBUG_BREAK()
#endif

#define ERROR_IF(conditional, error) do { if ((conditional)) { puts(error); DEBUG_ONLY_DEBUG_BREAK(); return EXIT_FAILURE; } } while (0)

struct state
{
  uint32_t x[3 * 9];
  uint16_t blocks, lineH, lineV;
};

void init(state *pState)
{
  pState->blocks = 0;
  pState->lineH = 0;
  pState->lineV = 0;

  for (size_t i = 0; i < OOK_ARRAYSIZE(pState->x); i++)
    pState->x[i] = s_initRow;
}

bool read(state *pState, const char *data, const size_t sizeRead)
{
  size_t i = 0;

  for (size_t vb = 0; vb < 3; vb++)
  {
    for (size_t l = 0; l < 3; l++)
    {
      for (size_t hb = 0; hb < 3; hb++)
      {
        constexpr uint32_t mask = s_initRow | s_done;
        uint32_t offset = 0;

        for (size_t n = 0; n < 3; n++, offset += 10)
        {
          ERROR_IF(i >= sizeRead, "Unexpected end of file.");

          if (data[i] == ' ' || data[i] == '.' || data[i] == 'X')
          {
            i++;
            continue;
          }
          else if (hb == 2 && (data[i] == '\r' || data[i] == '\n'))
          {
            break;
          }

          ERROR_IF(data[i] < '0' || data[i] > '9', "Unexpected symbol in input file.");

          pState->x[(vb * 3 + l) * 3 + hb] &= ~(mask << offset);
          pState->x[(vb * 3 + l) * 3 + hb] |= ((s_done | (1 << (data[i] - '0'))) << offset);

          i++;
        }

        if (vb == 2 && l == 2 && hb == 2)
          return false;

        ERROR_IF(i >= sizeRead, "Unexpected end of file.");

        if (data[i] == '|' || data[i] == ',' || data[i] == '/')
          i++;
      }

      ERROR_IF(i >= sizeRead, "Unexpected end of file.");

      if (data[i] == '\r' || data[i] == '\n')
        i++;
      else
        ERROR_IF(false, "Unexpected symbol. Expected line end.");

      ERROR_IF(i >= sizeRead, "Unexpected end of file.");

      if (data[i] == '\r' || data[i] == '\n')
        i++;
    }

    for (size_t j = 0; j < 9 + 2; j++)
    {
      ERROR_IF(i >= sizeRead, "Unexpected end of file.");

      if (data[i] == '-' || data[i] == '=')
        i++;
      else if (i % 4 == 3 && (data[i] == '|' || data[i] == ',' || data[i] == '/'))
        i++;
      else
        break;
    }

    ERROR_IF(i >= sizeRead, "Unexpected end of file.");

    if (data[i] == '\r' || data[i] == '\n')
      i++;

    ERROR_IF(i >= sizeRead, "Unexpected end of file.");

    if (data[i] == '\r' || data[i] == '\n')
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
          const uint32_t v = (pState->x[(vb * 3 + l) * 3 + hb] >> offset) & (s_init | s_done);
#else
          const uint32_t v = (pState->x[(vb * 3 + l) * 3 + hb] >> offset);
#endif
          
          if (v & s_done)
          {
            DWORD index = 0;
            BitScanReverse(&index, v & ~s_done);

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

    ERROR_IF(read(&s, data, sizeRead), "Failed to parse file.");
  }

  print(&s);
}
