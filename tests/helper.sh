STR="test_super_small.x2017"
python3 main.py
cp test.out ${STR%.x2017}.out
cp test.x2017 ${STR%.x2017}.x2017
cp test.asm ${STR%.x2017}.asm