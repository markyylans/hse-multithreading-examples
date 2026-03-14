Реализовал буферизованный канал.

Результаты запуска бенчмарка и тестов:

```
Running ./buffered_channel_benchmark
Run on (1 X 2700 MHz CPU )
CPU Caches:
  L1 Data 32 KiB (x1)
  L1 Instruction 32 KiB (x1)
  L2 Unified 4096 KiB (x1)
  L3 Unified 16384 KiB (x1)
Load Average: 0.27, 0.07, 0.02
***WARNING*** Library was built as DEBUG. Timings may be affected.
-----------------------------------------------------------------------------------------------
Benchmark                                                     Time             CPU   Iterations
-----------------------------------------------------------------------------------------------
Run/2/2/1/min_time:0.100/process_time/real_time            1927 ms         1865 ms            1
Run/10/1/1/min_time:0.100/process_time/real_time            352 ms          351 ms            1
Run/100000/1/1/min_time:0.100/process_time/real_time        113 ms          113 ms            1
```
```
Running main() from /root/bench/build/_deps/googletest-src/googletest/src/gtest_main.cc
[==========] Running 6 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 6 tests from Correctness
[ RUN      ] Correctness.Simple
[       OK ] Correctness.Simple (317 ms)
[ RUN      ] Correctness.Senders
[       OK ] Correctness.Senders (333 ms)
[ RUN      ] Correctness.Receivers
[       OK ] Correctness.Receivers (330 ms)
[ RUN      ] Correctness.SmallBuf
[       OK ] Correctness.SmallBuf (342 ms)
[ RUN      ] Correctness.BigBuf
[       OK ] Correctness.BigBuf (757 ms)
[ RUN      ] Correctness.Random
[       OK ] Correctness.Random (365 ms)
[----------] 6 tests from Correctness (2447 ms total)

[----------] Global test environment tear-down
[==========] 6 tests from 1 test suite ran. (2447 ms total)
[  PASSED  ] 6 tests.
```
