#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys

def fib(n):
    if n <= 1:
        return n
    else:
        return fib(n - 2) + fib(n - 1)

if __name__ == '__main__' and len(sys.argv) > 1:
    print fib(int(sys.argv[1]))
