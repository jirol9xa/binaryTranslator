DEF_CMD(0, IN, 
{
    
})
DEF_CMD(1, HLT, 
{
    FILL1BYTE(0xC3);
})
DEF_CMD(2, PUSH,
{
    i += makePushPop(src_arr + i, dst, 1);
})
DEF_CMD(3, OUT, 
{
    
})
DEF_CMD(4, ADD, 
{

})
DEF_CMD(5, SUB, 
{

})
DEF_CMD(6, MUL,
{

})
DEF_CMD(7, DIV, 
{

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

})
DEF_CMD(19, SQRT, 
{

})
