#!/bin/bash
#make vm_x2017
#make objdump_x2017
#
#cd tests
echo 
echo ============================================================
echo "                 testing on objdump_x2017                 "
echo ============================================================
echo 

pass=0
tests=0
for file in tests/*.x2017;
do
    DIFF=$(./objdump_x2017 ${file} 2>&1 | diff - ${file%.x2017}.asm)
    if [ "$DIFF" != "" ]
    then
        echo "[FAILED!]         Testing on ${file%.*}"
        echo $DIFF
        tests=$((tests + 1))
    else
        pass=$((pass + 1))
        tests=$((tests + 1))
        echo "[PASSED]          Testing on ${file%.*}"
    fi

done

echo 
echo $tests test total, $pass test passed, $((tests - pass)) test failed.
echo
echo ============================================================
echo "                 testing on vm_x2017                      "
echo ============================================================
echo 

pass=0
tests=0
for file in tests/*.x2017;
do
    DIFF=$(./vm_x2017 ${file} 2>&1 | diff - ${file%.x2017}.out)
    if [ "$DIFF" != "" ]
    then
        echo "[FAILED!]         Testing on ${file%.*}"
        echo $DIFF
        tests=$((tests + 1))
    else
        pass=$((pass + 1))
        tests=$((tests + 1))
        echo "[PASSED]          Testing on ${file%.*}"
    fi

done

echo 
echo $tests test total, $pass test passed, $((tests - pass)) test failed.
echo
