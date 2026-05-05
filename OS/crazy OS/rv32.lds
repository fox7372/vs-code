SECTIONS
{
    . = 0x80000000;
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    /DISCARD/ : { *(*) }
}
//提取有效的代码和数据删除没用的