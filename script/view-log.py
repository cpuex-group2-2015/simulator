#!/usr/bin/env python
# -*- coding: utf-8 -*-

from sys import argv

if __name__ == '__main__':
    if len(argv) < 3:
        print "usage: ./view-log.py LOGFILE MNEMONIC\nex: ./view-log.py test.bin-20151229_120000.log fadd"
        exit(-1)

    filename = argv[1]
    mne = argv[2]

    for s in open(filename):
        if s.find(mne) != -1:
            print s[0:-1]
