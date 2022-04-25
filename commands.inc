DEF_CMD(0, IN, 
{
    printf("Enter the value:\n");
    int value = 0;
    while (scanf("%d", &value) != 1)
    {
        printf("Enter correct value (((((\n");
    }
    value *= 1000;
    stackPush(&(cpu->stk), value);
})
DEF_CMD(1, HLT, 
{
    return CMD_HLT;
})
DEF_CMD(2, PUSH,
{

    if (((code[cpu->ip - 1] & IS_REG) != 0) && ((code[cpu->ip - 1] & IS_RAM) != 0))
    {
        workWithParsedRAM(cpu, code, 0);
    }
    else if ((code[cpu->ip - 1] & IS_REG) != 0)
    {
        stackPush(&(cpu->stk), cpu->regs[*((int *) (code + cpu->ip))]);
        cpu->ip += sizeof(int);
        is_debug_lvl_0(cpu->real_ip ++);
    }
    else if (((code[cpu->ip - 1] & IS_RAM) != 0))
    {   
        sleep(0.5);
        stackPush(&(cpu->stk), cpu->RAM[*((int *) (code + cpu->ip))]);
        cpu->ip += sizeof(int);
        is_debug_lvl_0(cpu->real_ip ++);
    }
    else
    {
    int value = *((int*) (code + cpu->ip));
    value *= 1000;
    stackPush(&(cpu->stk), value);
    cpu->ip += sizeof(int);
    is_debug_lvl_0(cpu->real_ip ++);
    }
})
DEF_CMD(3, OUT, 
{
    int value = 0;
    stackPop(&(cpu->stk), &value);
    double value1 = value;
    printf("%lg\n", value1 / 1000);
    stackPush(&(cpu->stk), value);
})
DEF_CMD(4, ADD, 
{
    int first = 0;
    int second = 0;
    stackPop(&(cpu->stk), &first);
    stackPop(&(cpu->stk), &second);
    stackPush(&(cpu->stk), first + second);
})
DEF_CMD(5, SUB, 
{
    int first = 0;
    int second = 0;
    stackPop(&(cpu->stk), &first);
    stackPop(&(cpu->stk), &second);
    stackPush(&(cpu->stk), second - first);
})
DEF_CMD(6, MUL,
{
    int first = 0;
    int second = 0;
    stackPop(&(cpu->stk), &first);
    stackPop(&(cpu->stk), &second);
    stackPush(&(cpu->stk), first * second / 1000);
})
DEF_CMD(7, DIV, 
{
    int first = 0;
    int second = 0;
    stackPop(&(cpu->stk), &first);
    stackPop(&(cpu->stk), &second);
    stackPush(&(cpu->stk), second * 1000 / first);
})
DEF_CMD(8, POP, 
{   
    if ((code[cpu->ip - 1] & IS_REG) != 0 && (code[cpu->ip - 1] & IS_RAM) != 0)
    {
        workWithParsedRAM(cpu, code, 1);
    }
    else if ((code[cpu->ip - 1] & IS_REG) != 0)
    {
        stackPop(&(cpu->stk), &(cpu->regs[*((int *) (code + cpu->ip))]));
        cpu->ip += sizeof(int);
        is_debug_lvl_0(cpu->real_ip ++);
    }
    else if ((code[cpu->ip - 1] & IS_RAM) != 0)
    {
        sleep(0.5);
        stackPop(&(cpu->stk), &(cpu->RAM[*((int *) (code + cpu->ip))]));
        cpu->ip += sizeof(int);
        is_debug_lvl_0(cpu->real_ip ++);
    }
    else
    {
        int value = 0;
        stackPop(&(cpu->stk), &value);
    }
})
DEF_CMD(9, JMP,
{
    cpu->ip = *((int *) (code + cpu->ip));
    is_debug_lvl_0(cpu->real_ip ++);
})
DEF_CMD(10, MRK, 1,
{
    ;
})
DEF_CMD(11, JA,
{
    int first = 0;
    int second = 0;
    stackPop(&(cpu->stk), &first);
    stackPop(&(cpu->stk), &second);
    if (second > first)
    {
        cpu->ip = *((int *) (code + cpu->ip));
        is_debug_lvl_0(cpu->real_ip ++);
    }
    else
    {
        cpu->ip += sizeof(int);
        cpu->real_ip ++;
    }
})
DEF_CMD(12, JAE,
{
    int first = 0;
    int second = 0;
    stackPop(&(cpu->stk), &first);
    stackPop(&(cpu->stk), &second);
    if (second >= first)
    {
        cpu->ip = *((int *) (code + cpu->ip));
        is_debug_lvl_0(cpu->real_ip ++);
    }
    else
    {
        cpu->ip += sizeof(int);
        cpu->real_ip ++;
    }
})
DEF_CMD(13, JB,
{
    int first = 0;
    int second = 0;
    stackPop(&(cpu->stk), &first);
    stackPop(&(cpu->stk), &second);
    if (second < first)
    {
        cpu->ip = *((int *) (code + cpu->ip));
        is_debug_lvl_0(cpu->real_ip ++);
    }
    else
    {
        cpu->ip += sizeof(int);
        cpu->real_ip ++;
    }
})
DEF_CMD(14, JBE,
{
    int first = 0;
    int second = 0;
    stackPop(&(cpu->stk), &first);
    stackPop(&(cpu->stk), &second);
    if (second <= first)
    {
        cpu->ip = *((int *) (code + cpu->ip));
        is_debug_lvl_0(cpu->real_ip ++);
    }
    else
    {
        cpu->ip += sizeof(int);
        cpu->real_ip ++;
    }
})
DEF_CMD(15, JE,
{
    int first = 0;
    int second = 0;
    stackPop(&(cpu->stk), &first);
    stackPop(&(cpu->stk), &second);
    if (second == first)
    {
        cpu->ip = *((int *) (code + cpu->ip));
        is_debug_lvl_0(cpu->real_ip ++);
    }
    else
    {
        cpu->ip += sizeof(int);
        cpu->real_ip ++;
    }
})
DEF_CMD(16, JNE,
{
    int first = 0;
    int second = 0;
    stackPop(&(cpu->stk), &first);
    stackPop(&(cpu->stk), &second);
    if (second != first)
    {
        cpu->ip = *((int *) (code + cpu->ip));
        is_debug_lvl_0(cpu->real_ip ++);
    }
    else
    {
        cpu->ip += sizeof(int);
        cpu->real_ip ++;
    }
})
DEF_CMD(17, CALL,
{
    stackPush(&(cpu->ret), cpu->ip + sizeof(type_t));
    cpu->ip = *((int *) (code + cpu->ip));
})
DEF_CMD(18, RET,
{
    stackPop(&(cpu->ret), &(cpu->ip));
})
DEF_CMD(19, SQRT, 
{
    int value = 0;
    stackPop(&(cpu->stk), &value);
    value = (int) sqrt(value * 1000);
    stackPush(&(cpu->stk), value);
})
