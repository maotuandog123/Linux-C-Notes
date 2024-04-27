# I/O操作

输入输出是一切实现的基础。

标准IO：`stdio`

系统调用IO（文件IO）：`sysio`

优先使用标准IO，兼容性更好，还有合并系统调用的优势。

## 标准IO

```c
/* stdio */
/* FILE类型贯穿始终 */

FILE *fopen(const char *path, const char *mode);
/**
 * fopen 返回指针的储存位置？ 1.栈  2.静态区  3.堆
 * 正确答案：3.堆。
 * 因为如果是栈，就是函数内部局部变量，无法返回地址。
 * 如果是静态区，无法确定需要多少个这个变量。
 * 
 * 只有 r 和 r+ 一定要求文件存在
 * 另外几种不存在会创建
 * 
 * 创建文件的权限
 * 0666 & ~umask
 * 
 * 对于普通用户
 * umask 得到 022
 * 
*/
int fclose(FILE *fp);

int fputc(FILE *stream);
int fgetc(int c, FILE *stream);

char *fgets(char *s, int size, FILE *stream);
/**
 * 两种正常返回的情况：
 * 1. 读了 size-1 个字节，最后一个字节留给 '\0'
 * 2. 读到了 '\n'
 * 
 * eg. 加入用fgets(buf, 5, stream) 来读 abcd
 * 是会读两次的
 * 第一次：abcd'\0'
 * 第二次：'\n''\0'
*/
int fputs(const char *restrict s, FILE *restrict stream);

/**
 * 这一对函数常用但是无法验证边界
 * 尽量一次只读单字节，更安全
 * 
 * 返回值：成功读/写的对象的数量
*/
size_t fread(void *ptr, size_t size, size_t nemmb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);


int printf(const char *restrict format, ...);
/**
 * 常用于 fprintf(stderr,...)
*/
int fprintf(FILE *restrict stream, const char *restrict format, ...);

// TODO:
int dprintf(int fd, const char *restrict format, ...);

/**
 * 将格式化内容输出到一个字符串
 * 
 * 和 atoi() 正好相反
 * 
*/
int sprintf(char *restrict str, const char *restrict format, ...);

/**
 * 比sprintf多了size参数，更安全
*/
int snprintf(char   str[restrict.size],
             size_t size,
             const char *restrict format,
             ...)

// !!! 慎用%s
int scanf(const char *restrict format, ...);
int fscanf(FILE *restrict stream,
                  const char *restrict format, ...);


/**
 * 移动文件当前位置指针
 * 
 * 可用于生成空洞文件，下载器原理
 * 
 * @prarm: offset 移动多远
 * @prarm: whence 移动方向
 *         SEEK_SET, SEEK_CUR, SEEK_END
 * 
 * @return  成功0，失败-1
*/
int fseek(FILE *stream, long offset, int whence);

/**
 * 反映当前文件指针所在位置
 * 
 * 这个long的负值部分无法使用。
 * 所以文件无法超过2G。
 * 
*/
long ftell(FILE *stream);

/**
 * 解决上面long的问题。
 * 
 * 最好编译时加上
 * #define _FILE_OFFSET_BITS 64
 * 可以写入makefile
 * 
 * 但是这俩函数是方言，前面那个long的一对支持C89，C99
 * 
*/
int fseeko(FILE *stream, off_t offset, int whence);
off_t ftello(FILE *stream);

/**
 * 将文件指针置于文件首 
 * equivalent to:
 * fseek(stream, 0L, SEEK_SET);
*/
void rewind(FILE *stream);

/**
 *  缓冲区的作用：
 *     大多数情况下是好事，合并系统调用
 * 
 * 行缓冲： 换行时候刷新，满了的时候刷新，强制刷新（标准输出是这样的，因为是终端设备）
 * 
 * 全缓冲： 满了的时候刷新，强制刷新（默认，只要不是终端设备）
 * 
 * 无缓冲： 如stderr，需要立即输出的内容
*/
fflush();

/**
 * @prarm: mode
 *         _IONBF
 *         _IOLBF
 *         _IOFBF
*/
int setvbuf(FILE *stream, char *buf, int mode, size_t size);

/**
 * 为了读取一行
 * 
 * 使用办法：
 *   #define _GNU_SOURCE  这个不想写到代码里面的话可以写到makefile
 *   eg. CFLAGS+=-D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE
 *   #include <stdio.h>
 * 
 * !!! 里面有 malloc 动作，未释放
 * !!! 是方言，可以自己封装一个mygetline和mygetline_free
 * 
*/
ssize_t getline(char **lineptr, size_t *n, FILE *stream);

/**
 * 临时文件
 *       1. 如何不冲突的创建
 *       2. 及时销毁
 * 
 * tmpnam: 创建临时文件名字
 *         有并发危险，因为产生名字和创建文件是两步
 * 
 * tmpfile: 创建临时文件
 *          是匿名文件，ls -a 都看不到
 *          避免冲突
*/
char *tmpnam(char *s);
FILE *tmpfile(void);



```


## 文件IO/系统调用IO

文件描述符（`fd`）是在文件IO中贯穿始终的类型。

### 文件描述符的概念
是一个整型数，是一个指针数组的下标。
优先使用当前可用范围内最小的。

### 文件IO操作相关函数：
- open
- close
- read
- write
- lsee

```c

/**
 * r  -> O_RDONLY
 * r+ -> O_RDWR
 * w  -> O_WRONLY | O_CREAT | O_TRUNC
 * w+ -> O_RDWR   | O_TRUNC | O_CREAT
 * 
 * 如果有creat就必须用三参数的形式
 * C语言没有重载，这是变参函数
 * 
 * @prarm: pathname 文件路径
 * @prarm: flags    文件打开方式
 * @prarm: mode     文件权限
 * 
*/
int open(const char *pathname, int flags);
int open(const char *pathname, int flags, mode_t mode);

int close(int fd);

/**
 *  read from a file descriptor
 * 
 * @return 读取的字节数，失败返回-1
*/
ssize_t read(int fd, void *buf, size_t count);

/**
 *  write to a file descriptor
 * 
 * @return 写入的字节数，失败返回-1
*/
ssize_t write(int fd, const void *buf, size_t count);

/**
 *  移动文件指针
 * 
 * @prarm: offset 移动多远
 * @prarm: whence 移动方向
 *         SEEK_SET, SEEK_CUR, SEEK_END
 * 
 * @return  成功0，失败-1
*/
off_t lseek(int fd, offt offset, int whence);

```


### 文件IO与标准IO的区别

### IO的效率问题

### 文件共享

### 原子操作

### 程序中的重定向：`dup`, `dup2`

### 同步：`sync`, `fsync`, `fdatasync`, `fcntl`, `ioctl`

### /dev/fd/目录