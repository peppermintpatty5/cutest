# cutest

A minimal C unit testing framework

## Background

I was inspired by Python's [`unittest`](https://docs.python.org/3/library/unittest.html) framework. If you have used `unittest` before, then this library should feel familiar.

I am aware that there exists another C unit testing framework by the same name. Although our code is similar in many ways, one of the biggest differences is that my project uses `return` instead of `setjmp()` / `longjmp()`. My code is also way ~~cuter~~ better :wink:

Lastly, this project adheres to the ANSI C (C89) specification.

## Usage

You only need the files [`cutest.c`](cutest.c) and [`cutest.h`](cutest.h). Please refer to [`main.c`](main.c) for an example of how to use this library. To compile and run this example:

```sh
make
./a.out
```

### Time Elapsed

Measuring time in milliseconds is not portable in C. Therefore, measuring time elapsed had to be externalized. There is an example function named `timer` in [`main.c`](main.c). Also see the following comment on `timer_fn_t` in [`cutest.h`](cutest.h) if you wish to write your own timer logic:

```c
/**
 * Stateful timer function pointer that measures time elapsed in seconds.
 *
 * If passed a non-zero value, then the timer shall be reset and 0.0 returned.
 * Otherwise, the time elapsed since the most recent reset shall be returned.
 */
typedef float (*timer_fn_t)(int);
```
