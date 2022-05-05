DEF_CMD(0, IN, 
{
    makeWrapCall(dst, 0, (u_int64_t) &wrapScanf, 1);
    ++i;
},
{
    src_ip++;
    dst_ip += 30;
})
DEF_CMD(1, HLT, 
{
    ++i;
},
{
    src_ip++;
    dst_ip++;
})
DEF_CMD(2, PUSH,
{
    i += makePushPop(src_arr + i, dst, 1);
},
{
    if ((src_arr[src_ip] & IS_REG) && (src_arr[src_ip] & IS_RAM))
    {
        src_ip++;
        switch (src_arr[src_ip++])
        {
            case (2 | IS_REG):
                src_ip += 2 * sizeof(int) + sizeof(char);
                dst_ip += 18;
                break;
            case (2 | IS_RAM | IS_REG):
                src_ip += 2 * sizeof(int) + sizeof(char);
                dst_ip += 18;
                break;
            case (1 | IS_REG):
                src_ip += sizeof(int);
                dst_ip += 9;
                break;
            case (2 | IS_RAM):
                src_ip += 2 * sizeof(int) + sizeof(char);
                dst_ip += 18;
                break;
            case (2):
                src_ip += 2 * sizeof(int) + sizeof(char);
                dst_ip += 12;
                break;
        }
    }
    else if (src_arr[src_ip] & IS_REG) // for PUSH reg
    {
        src_ip += sizeof(unsigned) + sizeof(char);
        dst_ip++;
    }
    else if (src_arr[src_ip] & IS_RAM)
    {
        src_ip += sizeof(unsigned) + sizeof(char);
        dst_ip += 12;
    }
    else    // for PUSH num
    {
        src_ip += sizeof(unsigned) + sizeof(char);
        dst_ip += 5;
    }
})
DEF_CMD(3, OUT, 
{
    makeWrapCall(dst, 1, (u_int64_t) &wrapPrintf, 0);
    ++i;
},
{
    src_ip++;
    dst_ip += 30;
})
DEF_CMD(4, ADD, 
{
    int oper = '+';
    makeArifm(dst, oper);
    ++i;
},
{
    src_ip += sizeof(char);
    dst_ip += 9;
})
DEF_CMD(5, SUB, 
{
    int oper = '-';
    makeArifm(dst, oper);
    ++i;
},
{
    src_ip += sizeof(char);
    dst_ip += 9;
})
DEF_CMD(6, MUL,
{
    int oper = '*';
    makeArifm(dst, oper);
    ++i;
},
{
    src_ip += sizeof(char);
    dst_ip += 19;
})
DEF_CMD(7, DIV, 
{
    int oper = '/';
    makeArifm(dst, oper);
    ++i;
},
{
    src_ip += sizeof(char);
    dst_ip += 22;
})
DEF_CMD(8, POP, 
{   
    i += makePushPop(src_arr + i, dst, 0);
},
{
    if ((src_arr[src_ip] & IS_REG) && (src_arr[src_ip] & IS_RAM))
    {
        src_ip++;
        switch (src_arr[src_ip++])
        {
            case (2 | IS_REG):
                src_ip += 2 * sizeof(int) + sizeof(char);
                dst_ip += 18;
                break;
            case (2 | IS_RAM | IS_REG):
                src_ip += 2 * sizeof(int) + sizeof(char);
                dst_ip += 18;
                break;
            case (1 | IS_REG):
                src_ip += sizeof(int);
                dst_ip += 9;
                break;
            case (2 | IS_RAM):
                src_ip += 2 * sizeof(int) + sizeof(char);
                dst_ip += 18;
                break;
            case (2):
                src_ip += 2 * sizeof(int) + sizeof(char);
                dst_ip += 12;
                break;
        }
    }
    else if (src_arr[src_ip] & IS_REG) // for PUSH reg
    {
        src_ip += sizeof(unsigned) + sizeof(char);
        dst_ip++;
    }
    else if (src_arr[src_ip] & IS_RAM)
    {
        src_ip += sizeof(unsigned) + sizeof(char);
        dst_ip += 12;
    }
    else    // for PUSH num
    {
        src_ip += sizeof(char);
        dst_ip += 2;
    }
})
DEF_CMD(9, JMP,
{
    makeJmp(src_arr + i, dst, 9, i);
    i += sizeof(char) + sizeof(int);
},
{
    src_ip += sizeof(char) + sizeof(unsigned);
    dst_ip += sizeof(char) + sizeof(int);
})
DEF_CMD(10, MRK,
{
    ++i;
},
{
    src_ip++;
})
DEF_CMD(11, JA,
{
    makeJmp(src_arr + i, dst, 11, i);
    i += sizeof(char) + sizeof(int);
},
{
    src_ip += sizeof(char) + sizeof(unsigned);
    dst_ip += sizeof(char) * 2 + sizeof(int) + sizeof(char) * 7;
})
DEF_CMD(12, JAE,
{
    makeJmp(src_arr + i, dst, 12, i);
    i += sizeof(char) + sizeof(int);
},
{
    src_ip += sizeof(char) + sizeof(unsigned);
    dst_ip += sizeof(char) * 2 + sizeof(int) + sizeof(char) * 7;
})
DEF_CMD(13, JB,
{
    makeJmp(src_arr + i, dst, 13, i);
    i += sizeof(char) + sizeof(int);
},
{
    src_ip += sizeof(char) + sizeof(unsigned);
    dst_ip += sizeof(char) * 2 + sizeof(int) + sizeof(char) * 7;
})
DEF_CMD(14, JBE,
{
    makeJmp(src_arr + i, dst, 14, i);
    i += sizeof(char) + sizeof(int);
},
{
    src_ip += sizeof(char) + sizeof(unsigned);
    dst_ip += sizeof(char) * 2 + sizeof(int) + sizeof(char) * 7;
})
DEF_CMD(15, JE,
{
    makeJmp(src_arr + i, dst, 15, i);
    i += sizeof(char) + sizeof(int);
},
{
    src_ip += sizeof(char) + sizeof(unsigned);
    dst_ip += sizeof(char) * 2 + sizeof(int) + sizeof(char) * 7;
})
DEF_CMD(16, JNE,
{
    makeJmp(src_arr + i, dst, 16, i);
    i += sizeof(char) + sizeof(int);
},
{
    src_ip += sizeof(char) + sizeof(unsigned);
    dst_ip += sizeof(char) * 2 + sizeof(int) + sizeof(char) * 7;
})
DEF_CMD(17, CALL,
{
    makeCall(src_arr + i, dst);
    i += sizeof(char) + sizeof(int);
},
{
    src_ip += sizeof(char) + sizeof(int);
    dst_ip += sizeof(char) + sizeof(int);
})
DEF_CMD(18, RET,
{
    FILL1BYTE(0xC3);
    ++i;
},
{
    src_ip++;
    dst_ip++;
})
DEF_CMD(19, SQRT, 
{
    makeWrapCall(dst, 1, (u_int64_t) &wrapSqrt, 1);
    ++i;
},
{
    src_ip += sizeof(char);
    dst_ip += 35;
})
