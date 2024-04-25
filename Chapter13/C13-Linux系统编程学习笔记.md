# I/O操作

输入输出是一切实现的基础。

标准IO：`stdio`

系统调用IO（文件IO）：`sysio`

优先使用标准IO，兼容性更好，还有合并系统调用的优势。



```c
/* stdio */
/* FILE类型贯穿始终 */

FILE *fopen(const c);
fclose();

fgetc();
fputc();
fgets();
fputs();
fread();
fwrite();

printf();
scanf();

fseek();
ftell();
rewind();

fflush();
```

