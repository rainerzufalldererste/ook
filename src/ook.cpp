#include "ook.h"
#include "io.h"
#include "solve.h"

#include <stdio.h>
#include <inttypes.h>
#include <math.h>

#include <Windows.h>

//////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////

void init(state *pState)
{
  pState->blocks = 0;
  pState->lineH = 0;

  for (size_t i = 0; i < OOK_ARRAYSIZE(pState->x); i++)
    pState->x[i] = s_initRow;
}

static const char _Arg_Benchmark[] = "--bench";
static const char _Arg_PreferPreSolve[] = "--prefer-pre-solve";
static const char _Arg_PreferGuessSolve[] = "--prefer-guess-solve";

int32_t main(const int32_t argc, char **ppArgv)
{
  bool benchmark = false;
  bool preferPreSolve = false;
  bool preferGuessSolve = false;

  if (argc <= 1)
  {
    puts("Usage: ook <filename>");
    printf("\t[ % 25s ]: \tRun Benchmark\n", _Arg_Benchmark);
    printf("\t[ % 25s ]: \tPrefer Advanced Solver before guessing Values\n", _Arg_PreferPreSolve);
    printf("\t[ % 25s ]: \tPrefer Advanced Solver when guessing Values\n", _Arg_PreferGuessSolve);

    return EXIT_FAILURE;
  }

  int32_t argIndex = 2;

  while (argIndex < argc)
  {
    const int32_t argsLeft = argc - argIndex;
    (void)argsLeft; // currently none of the arguments have additional parameters.

    if (strncmp(ppArgv[argIndex], _Arg_Benchmark, sizeof(_Arg_Benchmark)) == 0)
    {
      benchmark = true;
    }
    else if (strncmp(ppArgv[argIndex], _Arg_PreferPreSolve, sizeof(_Arg_PreferPreSolve)) == 0)
    {
      preferPreSolve = true;
    }
    else if (strncmp(ppArgv[argIndex], _Arg_PreferGuessSolve, sizeof(_Arg_PreferGuessSolve)) == 0)
    {
      preferGuessSolve = true;
    }
    else
    {
      printf("Invalid Argument'%s'. Aborting.\n", ppArgv[argIndex]);
      return EXIT_FAILURE;
    }

    argIndex++;
  }

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

  if (benchmark)
  {
    // Dry Run.
    {
      puts("Dry Run...");

      state s1 = s;
      (preferPreSolve ? simple_solve_advanced : simple_solve)(&s1);

      if (s1.blocks != 0b111111111)
      {
        size_t _0, _1;
        (preferGuessSolve ? recursive_guess_advanced : recursive_guess)(&s1, &_0, &_1);
      }
    }

    puts("Benchmarking...");

    size_t sampleCount = 0;
    size_t *pNsSamples = nullptr;
    size_t *pOpSamples = nullptr;
    size_t totalTicks = 0;
    size_t totalOps = 0;
    double meanNs = 0;
    double meanOps = 0;

    const uint64_t ticksStart = _get_ticks();

    while (_ticks_to_ns(_get_ticks() - ticksStart) < 5000000000ULL)
    {
      const uint64_t before = _get_ticks();
      const uint64_t beforeEvents = __rdtsc();

      for (size_t i = 0; i < 1000; i++)
      {
        state s1 = s;
        (preferPreSolve ? simple_solve_advanced : simple_solve)(&s1);

        if (s1.blocks != 0b111111111)
        {
          size_t _0, _1;
          (preferGuessSolve ? recursive_guess_advanced : recursive_guess)(&s1, &_0, &_1);
        }
      }

      const uint64_t afterEvents = __rdtsc();
      const uint64_t after = _get_ticks();

      const size_t sampleIndex = sampleCount;
      sampleCount++;

      pNsSamples = reinterpret_cast<size_t *>(realloc(pNsSamples, sizeof(size_t) * sampleCount));
      ERROR_IF(pNsSamples == nullptr, "Failed to allocate memory.");

      pOpSamples = reinterpret_cast<size_t *>(realloc(pOpSamples, sizeof(size_t) * sampleCount));
      ERROR_IF(pOpSamples == nullptr, "Failed to allocate memory.");

      pNsSamples[sampleIndex] = _ticks_to_ns(after - before);
      pOpSamples[sampleIndex] = afterEvents - beforeEvents;
      
      totalTicks += after - before;
      totalOps += afterEvents - beforeEvents;

      meanNs = _ticks_to_ns(totalTicks) / (double)sampleCount;
      meanOps = totalOps / (double)sampleCount;
      
      printf("\r%7.5f ms (%5.3fM Ops) / 1k solves", meanNs * 1e-6, meanOps * 1e-6);
    }

    double sigmaNs = 0;
    double sigmaOps = 0;

    for (size_t i = 0; i < sampleCount; i++)
    {
      const double diffNs = pNsSamples[i] - meanNs;
      const double diffOps = pOpSamples[i] - meanOps;
      sigmaNs += diffNs * diffNs;
      sigmaOps += diffOps * diffOps;
    }

    sigmaNs = sqrt(sigmaNs / sampleCount);
    sigmaOps = sqrt(sigmaOps / sampleCount);
    
    printf(" (std dev: %7.5f ms | %5.3fM Ops | %" PRIu64 " samples)\n", sigmaNs * 1e-6, sigmaOps * 1e-6, sampleCount);
  }
  else
  {
    // Simple Solve.
    {
      fputs("Simple Solve...", stdout);

      const uint64_t before = _get_ticks();
      (preferPreSolve ? simple_solve_advanced : simple_solve)(&s);
      const uint64_t after = _get_ticks();

      printf(" (Completed in %9.6f ms)\n", _ticks_to_ns(after - before) * 1e-6f);

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
      ERROR_IF(!(preferGuessSolve ? recursive_guess_advanced : recursive_guess)(&s, &guesses, &total), "Failed to solve by guessing.");

      const uint64_t after = _get_ticks();

      printf(" (Completed in %9.6f ms | %" PRIu64 " consecutive guesses needed | %" PRIu64 " total)\n", _ticks_to_ns(after - before) * 1e-6f, guesses, total);

      print(&s);
    }
  }

  return EXIT_SUCCESS;
}
