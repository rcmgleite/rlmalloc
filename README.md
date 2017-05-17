# rlmalloc 
This a simple implementation of malloc using the first fit algorithm. It's NOT PRODUCTION READY and was created just for academic purposes.

## System requirements
Please use Linux.

## Compile
To compile the example on main.c just execute:
```
$ make
```

## main.c
This file uses rlmalloc and rlfree to manager memory with some simple examples.

## bibliography
- [Malloc tutorial](http://www.inf.udec.cl/~leo/Malloc_tutorial.pdf)
- [Simple malloc impl](https://danluu.com/malloc-tutorial/)
- [Redis zmalloc](https://github.com/antirez/redis/blob/unstable/src/zmalloc.c#L176)
- [Zero length array - C](http://gcc.gnu.org/onlinedocs/gcc/Zero-Length.html)
- [Memory Alignment](http://stackoverflow.com/questions/3994035/what-is-aligned-memory-allocation/3994235#3994235)

## TODO's
- [ ] Create better tests.
- [ ] Keep track of memory usage. (eg: [zmalloc](https://github.com/antirez/redis/blob/unstable/src/zmalloc.c))
- [ ] Thread safety.
- [ ] Create statistics getters (eg: number of blocks).
- [ ] Benchmark.
