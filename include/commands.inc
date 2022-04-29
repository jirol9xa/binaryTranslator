DEF_CMD(0, IN, 
{
    
})
DEF_CMD(1, HLT, 
{
    ++i;
})
DEF_CMD(2, PUSH,
{
    i += makePushPop(src_arr + i, dst, 1);
})
DEF_CMD(3, OUT, 
{
    makeOut(dst);
    ++i;
})
DEF_CMD(4, ADD, 
{
    int oper = '+';
    makeArifm(dst, oper);
    ++i;
})
DEF_CMD(5, SUB, 
{
    int oper = '-';
    makeArifm(dst, oper);
    ++i;
})
DEF_CMD(6, MUL,
{
    int oper = '*';
    makeArifm(dst, oper);
    ++i;
})
DEF_CMD(7, DIV, 
{
    int oper = '/';
    makeArifm(dst, oper);
    ++i;
})
DEF_CMD(8, POP, 
{   
    i += makePushPop(src_arr + i, dst, 0);
})
DEF_CMD(9, JMP,
{

})
DEF_CMD(10, MRK,
{

})
DEF_CMD(11, JA,
{

})
DEF_CMD(12, JAE,
{

})
DEF_CMD(13, JB,
{

})
DEF_CMD(14, JBE,
{

})
DEF_CMD(15, JE,
{

})
DEF_CMD(16, JNE,
{

})
DEF_CMD(17, CALL,
{

})
DEF_CMD(18, RET,
{
    FILL1BYTE(0xC3);
    ++i;
})
DEF_CMD(19, SQRT, 
{

})
