Реализовал вариант с ThreadPool.
Результат прогона тестов:

```
[==========] Running 4 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 4 tests from ThreadPool
[ RUN      ] ThreadPool.SubmitReturnsResult
[       OK ] ThreadPool.SubmitReturnsResult (0 ms)
[ RUN      ] ThreadPool.VoidCorrectness
[       OK ] ThreadPool.VoidCorrectness (0 ms)
[ RUN      ] ThreadPool.WaitForCorrectness
[       OK ] ThreadPool.WaitForCorrectness (101 ms)
[ RUN      ] ThreadPool.SubmitPropagatesException
[       OK ] ThreadPool.SubmitPropagatesException (1 ms)
[----------] 4 tests from ThreadPool (104 ms total)

[----------] Global test environment tear-down
[==========] 4 tests from 1 test suite ran. (104 ms total)
[  PASSED  ] 4 tests.
```

Прогнал их скриптом 100 раз, падений/зависаний не обнаружил.
