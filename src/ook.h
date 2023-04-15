#pragma once

#include <stdint.h>
#include <stdlib.h>

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
