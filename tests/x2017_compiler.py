import textwrap

filename = "test.asm"


def write_addresss(str, num):
    '''
        Write an address by its type and value
    '''

    res = ""
    mcode = 0
    if str == "VAL":
        mcode = 0
        res += format(int(num), '#010b')[2:]
    elif str == "REG":
        mcode = 1
        res += format(int(num), '#05b')[2:]
    elif str == "STK":
        mcode = 2
        if ord(num) - ord('a') >= 0:
            res += format(ord(num) - ord('a') + 26, '#07b')[2:]
        else:
            res += format(ord(num) - ord('A'), '#07b')[2:]
    elif str == "PTR":
        mcode = 3
        if ord(num) - ord('a') >= 0:
            res += format(ord(num) - ord('a') + 26, '#07b')[2:]
        else:
            res += format(ord(num) - ord('A'), '#07b')[2:]
    res += format(int(mcode), '#04b')[2:]
    return res


'''
    Open assembly code file
'''
file = open(filename, 'r')
str = ""
cnt = 0

'''
    Read and compile
'''
for line in file.readlines():
    line = line.strip().split()
    r = ""
    print(cnt)
    print(line)

    if line[0] == "FUNC":
        if cnt != 0:
            str += format(int(cnt), '#07b')[2:]
        str += format(int(line[-1]), '#05b')[2:]
        cnt = 0
    elif line[0] == "MOV":
        str += write_addresss(line[-2], line[-1]) + write_addresss(
            line[-4], line[-3]) + format(int(0), '#05b')[2:]
        cnt += 1
    elif line[0] == "CAL":
        str += write_addresss(line[-2], line[-1]) + format(int(1), '#05b')[2:]
        cnt += 1
    elif line[0] == "RET":
        str += format(int(2), '#05b')[2:]
        cnt += 1
    elif line[0] == "REF":
        str += write_addresss(line[-2], line[-1]) + write_addresss(
            line[-4], line[-3]) + format(int(3), '#05b')[2:]
        cnt += 1
    elif line[0] == "ADD":
        str += write_addresss(line[-2], line[-1]) + write_addresss(
            line[-4], line[-3]) + format(int(4), '#05b')[2:]
        cnt += 1
    elif line[0] == "PRINT":
        str += write_addresss(line[-2], line[-1]) + format(int(5), '#05b')[2:]
        cnt += 1
    elif line[0] == "NOT":
        str += write_addresss(line[-2], line[-1]) + format(int(6), '#05b')[2:]
        cnt += 1
    elif line[0] == "EQU":
        str += write_addresss(line[-2], line[-1]) + format(int(7), '#05b')[2:]
        cnt += 1

'''
    Write the instruction count for final function at last
'''
str += format(int(cnt), '#07b')[2:]

# Debug
print("padding")
print(8 - (len(str) % 8))

'''
    Add padding at the front
'''
if len(str) % 8 != 0:
    str = "0" * (8 - len(str) % 8) + str
bs = []

# Debug
print(textwrap.wrap(str, 8))

'''
    Convert to bytearray
'''
for i in textwrap.wrap(str, 8):
    bs.append(int(i, 2))

# Debug
print(bs)
print(bytearray(bs))

'''
    Write to file
'''
file = open(filename[:-3] + "x2017", "wb")
file.write(bytearray(bs))
file.close()