#!/bin/bash
make vm_x2017.c
./vm_x2017 tests/sp2.x2017
make objdump_x2017.c
./objdump_x2017 tests/sp2.x2017


# Trigger all your test cases with this script
