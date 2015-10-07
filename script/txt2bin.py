#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import re

def chunks(l, n):
    for i in range(0, len(l), n):
        yield l[i:i+n]

if __name__ == '__main__':
    src = open(sys.argv[1], "r")
    dst = open(sys.argv[2], "wb")
    re_comment = re.compile(r";.+$")
    re_space   = re.compile(r"\s")
    for line in open(sys.argv[1]):
        s = re_space.sub("", re_comment.sub("", line))
        a = ([int(b, 2) for b in chunks(s, 8)])
        ba = bytearray(a)
        dst.write(ba)
