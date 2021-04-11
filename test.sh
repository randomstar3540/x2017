#!/bin/bash
make vm_x2017
make objdump_x2017

#cd tests
pass=0
for file in tests/*.x2017;
do
    DIFF=$(./objdump_x2017 ${file} 2>&1 | diff - ${file%.x2017}.asm)
    ./objdump_x2017 ${file}
    if [ "$DIFF" != "" ]
    then
        echo "[FAILED!]         Testing on ${file%.*}"
        echo $DIFF
    else
        pass=$((pass + 1))
        echo "[PASSED]          Testing on ${file%.*}"
    fi

done

#DIFF=$(cat /dev/null | xargs -a no_data_input_2.args .././timetable 2>&1 | diff - no_data_input_2.out)
#if [ "$DIFF" != "" ]
#then
#    echo "[FAILED!]         Testing on no_data_input_2"
#    echo $DIFF
#else
#    pass=$((pass + 1))
#    echo "[PASSED]          Testing on no_data_input_2"
#fi

echo 29 test total, $pass test passed, $((29 - pass)) test failed.

# Trigger all your test cases with this script
