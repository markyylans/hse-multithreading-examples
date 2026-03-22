В apply_function.hpp находится реализация функции по заданию.

В test.cpp написано 10 тестов, проверяющих разные ТК, проверяющие работу с пустым вектором, в однопоточном режиме, многопоточном с разными размерами векторов, количеством потоков и типами элементов.

Результат запуска тестов:

```
[==========] Running 10 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 10 tests from ApplyFunction
[ RUN      ] ApplyFunction.EmptyVector
[       OK ] ApplyFunction.EmptyVector (0 ms)
[ RUN      ] ApplyFunction.SingleElement
[       OK ] ApplyFunction.SingleElement (0 ms)
[ RUN      ] ApplyFunction.SingleThread
[       OK ] ApplyFunction.SingleThread (0 ms)
[ RUN      ] ApplyFunction.MultipleThreads
[       OK ] ApplyFunction.MultipleThreads (1 ms)
[ RUN      ] ApplyFunction.ManyThreadsSmallVector
[       OK ] ApplyFunction.ManyThreadsSmallVector (0 ms)
[ RUN      ] ApplyFunction.DoubleType
[       OK ] ApplyFunction.DoubleType (0 ms)
[ RUN      ] ApplyFunction.StringType
[       OK ] ApplyFunction.StringType (0 ms)
[ RUN      ] ApplyFunction.LargeVectorCorrectness
[       OK ] ApplyFunction.LargeVectorCorrectness (3 ms)
[ RUN      ] ApplyFunction.AllElementsProcessed
[       OK ] ApplyFunction.AllElementsProcessed (0 ms)
[ RUN      ] ApplyFunction.IdentityTransform
[       OK ] ApplyFunction.IdentityTransform (0 ms)
[----------] 10 tests from ApplyFunction (6 ms total)

[----------] Global test environment tear-down
[==========] 10 tests from 1 test suite ran. (6 ms total)
[  PASSED  ] 10 tests.
```

Также написан бенчмарк с двумя тестовыми вариантами: BM_LightWork и BM_HeavyWork.

В BM_LightWork используется функтор, который просто прибавляет единицу к элементам числового вектора. Он выполняется быстро.  
В BM_HeavyWork в функторе преобразования вызывается std::format для чисел с плавающей точкой. Эта операция более долгая, внутри есть сисколл для аллокации памяти строки, само форматирование числа.
Результат выполнения бенчмарка на моей машине:

```
Running ./apply_function_benchmark
Run on (16 X 3187.08 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 1280 KiB (x8)
  L3 Unified 18432 KiB (x1)
Load Average: 0.24, 0.29, 0.40
***WARNING*** Library was built as DEBUG. Timings may be affected.
--------------------------------------------------------------------------
Benchmark                                Time             CPU   Iterations
--------------------------------------------------------------------------
BM_LightWork/100/1/real_time          2.36 us         2.17 us       302552
BM_LightWork/100/4/real_time          97.4 us         70.2 us         5968
BM_LightWork/1000/1/real_time         21.5 us         19.8 us        32648
BM_LightWork/1000/4/real_time          106 us         70.1 us         6482
BM_LightWork/100000/1/real_time       2261 us         2087 us          310
BM_LightWork/100000/4/real_time        700 us          102 us          935
BM_HeavyWork/100/1/real_time         0.068 ms        0.063 ms        10044
BM_HeavyWork/100/4/real_time         0.110 ms        0.071 ms         5753
BM_HeavyWork/1000/1/real_time        0.671 ms        0.620 ms          990
BM_HeavyWork/1000/4/real_time        0.315 ms        0.088 ms         2278
BM_HeavyWork/5000/1/real_time         3.34 ms         3.08 ms          210
BM_HeavyWork/5000/4/real_time         1.21 ms        0.115 ms          547
```

По его результатам видно, что для первого функтора однопоточный вариант выполняется меньшее количество времени, нежели многопоточный при маленьких размерах вектора. Это объясняется тем, что временные затраты на создание потоков превышают время выполнения полезной нагрузки. При увеличении размера вектора ситуация меняется - параллельное выполнение даёт выигрыш, несмотря на затраты при создании потоков.

В случае с тяжёлой нагрузкой ситуация обратная - однопоточная реализация выиграла только на маленьком размере вектора. На больших размерах векторов многопоточная реализация стабильно тратит меньше времени на выполнение.
