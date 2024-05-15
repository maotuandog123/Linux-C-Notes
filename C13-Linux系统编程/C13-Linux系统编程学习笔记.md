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

int fputc(int c, FILE *stream);
int fgetc(FILE *stream);

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
 * 三种缓冲模式: 
 *            _IONBF
 *            _IOLBF
 *            _IOFBF
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
 * !!! 但是根据chatgpt，好像直接 free(*lineptr) 就行了
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

可以使用`./open file &`来后台运行一个程序。
然后通过`ps`查看进程号
然后进入`/proc/进程号/fd`查看文件描述符
前三个是标准输入输出错误，后面的是打开的文件描述符

```c

/**
 * flag:
 *
 * r  -> O_RDONLY
 * r+ -> O_RDWR
 * w  -> O_WRONLY | O_CREAT | O_TRUNC
 * w+ -> O_RDWR   | O_TRUNC | O_CREAT
 *
 * O_RDONLY     只读
 * O_WRONLY     只写
 * O_RDWR       读写
 * O_CREAT      创建
 * O_TRUNC      截断
 * O_APPEND     追加
 * O_EXCL       排他(若要创建的文件已存在则报错)
 * O_NONBLOCK   非阻塞
 * O_SYNC       同步
 * O_DSYNC      数据同步
 * O_RSYNC      读同步
 * O_DIRECT     直接IO
 * O_LARGEFILE  大文件
 * O_DIRECTORY  目录
 * O_NOFOLLOW   不跟踪符号链接
 * O_CLOEXEC    close-on-exec
 * O_PATH       仅打开目录
 * O_TMPFILE    临时文件
 * O_NOCTTY     不分配控制终端
 * 
 * 如果有creat就必须用三参数的形式
 * C语言没有重载，这是变参函数
 * 
 * @prarm: pathname 文件路径
 * @prarm: flags    文件打开方式
 * @prarm: mode     文件权限
 *                  假如0666，就是rw-rw-rw-，110110110
 * 
*/
int open(const char *pathname, int flags);
int open(const char *pathname, int flags, mode_t mode);

int close(int fd);

/**
 * @return 读取的字节数，失败返回-1
*/
ssize_t read(int fd, void *buf, size_t count);

/**
 *  想要控制写入的位置，需要使用lseek
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

### 例题：通过文件IO处理csv表格
```csv
,语文,数学,英语,总分,评价
张三,90,91,92,,
李四,80,81,82,,
王五,70,71,72,,
```
思路：逐行处理
可以使用16进制查看工具


### 文件IO与标准IO的区别
区别：响应速度&吞吐量
文件IO需要频繁进入内核，标准IO通过缓冲区合并系统调用。
响应速度快就文件IO，吞吐量大就标准IO。
> [!warning]
> 二者不可混用

转换方法：`fileno`, `fdopen`

### IO的效率问题
习题：将`mycpy.c`程序进行更改，将`BUFSIZE`的值放大，观察进程消耗的时间，注意性能出现拐点的值以及程序何时段错误。

解答：
将`BUFSIZE`作为命令行参数传入，`int bufsize = atoi(argv[3]);`
通过脚本进行试验：
```bash
#!/bin/bash

# 生成一个 5GB 的文件
dd if=/dev/urandom of=/tmp/bigfile bs=1G count=5

# 输入和输出文件的路径
src="/tmp/bigfile"
dst="/tmp/outfile"

# 编译你的程序
gcc -o mycpy_bufsize mycpy_bufsize.c

# 初始化 BUFSIZE
bufsize=512

# 循环，每次 BUFSIZE * 2
while true; do
  # 用 time 命令运行你的程序，并将结果重定向到一个临时文件
  { time ./mycpy_bufsize $src $dst $bufsize; } 2> time.txt
  
  # 检查程序的退出状态
  if [ $? -ne 0 ]; then
    echo "Max BUFSIZE before segfault: $bufsize"
    break
  fi

  # 提取 time 的结果
  real_time=$(grep real time.txt | awk -F' ' '{print $2}')
  user_time=$(grep user time.txt | awk -F' ' '{print $2}')
  sys_time=$(grep sys time.txt | awk -F' ' '{print $2}')

  # 输出 BUFSIZE 和 time 的结果
  echo "BUFSIZE: $bufsize, Real Time: $real_time, User Time: $user_time, Sys Time: $sys_time"
  
  # BUFSIZE * 2
  bufsize=$((bufsize * 2))
done

# 删除临时文件
rm time.txt
rm $src
rm $dst

```

结果：
```bash
wan@SK-20240106UQUX:~/Linux-C-Notes/C13-Linux系统编程/io/sys$ ./time.sh
BUFSIZE: 512, Real Time: 0m7.672s, User Time: 0m0.650s, Sys Time: 0m7.007s
BUFSIZE: 1024, Real Time: 0m5.026s, User Time: 0m0.201s, Sys Time: 0m4.651s
BUFSIZE: 2048, Real Time: 0m3.535s, User Time: 0m0.158s, Sys Time: 0m3.183s
BUFSIZE: 4096, Real Time: 0m2.418s, User Time: 0m0.059s, Sys Time: 0m2.232s
BUFSIZE: 8192, Real Time: 0m2.363s, User Time: 0m0.040s, Sys Time: 0m2.150s
BUFSIZE: 16384, Real Time: 0m2.279s, User Time: 0m0.030s, Sys Time: 0m2.079s
BUFSIZE: 32768, Real Time: 0m2.238s, User Time: 0m0.020s, Sys Time: 0m2.026s
BUFSIZE: 65536, Real Time: 0m2.114s, User Time: 0m0.000s, Sys Time: 0m1.972s
BUFSIZE: 131072, Real Time: 0m2.302s, User Time: 0m0.019s, Sys Time: 0m1.982s
BUFSIZE: 262144, Real Time: 0m2.244s, User Time: 0m0.000s, Sys Time: 0m2.016s
BUFSIZE: 524288, Real Time: 0m2.254s, User Time: 0m0.000s, Sys Time: 0m2.039s
BUFSIZE: 1048576, Real Time: 0m2.249s, User Time: 0m0.010s, Sys Time: 0m2.037s
BUFSIZE: 2097152, Real Time: 0m2.304s, User Time: 0m0.000s, Sys Time: 0m2.108s
BUFSIZE: 4194304, Real Time: 0m2.234s, User Time: 0m0.010s, Sys Time: 0m2.082s
Max BUFSIZE before segfault: 8388608
```
在`ulimit -a`中，我的系统的`stack size`是`8192`，所以`BUFSIZE`不能超过`8192`，否则会段错误。与测试结果一致。


### 文件共享
多个任务共同操作一个文件或者协同完成任务

面试题：写程序删除一个文件的第10行
补充函数：
```c
// 截断文件到某长度
int truncate(const char *path, off_t length);
int ftruncate(int fd, off_t length);
```

```c
// 最简单思路，将11行开始的内容到第10行开始处覆盖写
while()
{
    lseek 11 + read +lseek 10 + write
}

// 优化思路，两个文件描述符，一个读一个写
1 -> open r  -> fd1 -> lseek 11
2 -> open r+ -> fd2 -> lseek 10

while()
{
    1->fd1-> read
    2->fd2-> write
}

// 两个进程, 设计进程间通信
process1 -> open -> r
process2 -> open -> r+

p1->read -> p2->write

```

### 原子操作
指不可分割的操作
作用：解决竞争和冲突
如`tmpnam`函数，产生文件名和创建文件是两步，会有并发问题。


### 程序中的重定向：`dup`, `dup2`
```c
/**
 *  dup 和 dup2 都是复制文件描述符
 *  dup2 可以指定新的文件描述符
 *  dup 会返回一个新的文件描述符
 */
int dup(int oldfd);
int dup2(int oldfd, int newfd);
```

### 同步
同步内核层面的buffer和cache
```c
void sync(void);
int fsync(int fd);
int fdatasync(int fd); // 只刷新数据，不刷新亚数据

// 文件描述符所有的操作几乎都来源于该函数
int fcntl(int fd, int cmd, ... /* arg */);

// 设备相关的内容
int ioctl(int fd, unsigned long request, ... /* arg */);

```

### /dev/fd/目录
**虚目录**：显示当前进程的文件描述符信息



# 文件系统

类`ls`的实现，如`myls -l -a -i -n`

`cmd --长格式  -短格式  非选项的传参`

## 目录和文件

1. 获取文件属性
```c
/**
 *  将文件的属性存储到buf中
 *  stat : 通过文件路径获取属性，面对符号链接文件时，
 *         获取的是指向的目标文件的属性
 *  fstat: 通过文件描述符获取属性
 *  lstat: 通过文件路径获取属性，面对符号链接文件时，
*/
int stat(const char *path, struct stat *buf);
int fstat(int fd, struct stat *buf);
int lstat(const char *path, struct stat *buf);

struct stat {
    dev_t     st_dev;         /* ID of device containing file */
    ino_t     st_ino;         /* inode number */
    // 文件唯一标识，身份证号

    mode_t    st_mode;        /* protection */
    // st_mode: 文件权限+文件类型
    // 文件权限
    // 七种文件类型：dcb-lsp

    nlink_t   st_nlink;       /* number of hard links */
    uid_t     st_uid;         /* user ID of owner */
    gid_t     st_gid;         /* group ID of owner */
    dev_t     st_rdev;        /* device ID (if special file) */
    off_t     st_size;        /* total size, in bytes */
    // 在linux下，与windows不同，size值仅仅是属性
    // 不能实际体现占用磁盘大小，详见 big.c

    blksize_t st_blksize;    /* blocksize for file system I/O */
    blkcnt_t  st_blocks;     /* number of 512B blocks allocated */
    time_t    st_atime;       /* time of last access */
    time_t    st_mtime;       /* time of last modification */
    time_t    st_ctime;       /* time of last status change */
};
```

2. 文件访问权限
`st_mode`是一个16位的二进制数，文件类型，文件权限，特殊权限。

3. `umask`
作用：防止产生权限过松的文件。
`0666 &~umask`
`umask`也是一个终端命令，可以查看和设置。
`mode_t umask(mode_t mask);`

4. 文件权限的更改/管理
```c
/**
 * 更改文件权限
*/
int chmod(const char *path, mode_t mode);
int fchmod(int fd, mode_t mode);
```

5. 粘住位
t位，例如`/tmp`目录。

6. 文件系统：`FAT`, `UFS`
文件或数据的存储格式。
- `FAT`：静态存储的单链表
```c
struct node_st{
  int next[N];
  char data[N][SIZE];
};
```
- `UFS`：
缺点：不善于处理大量的小文件，因为每个文件都有一个`inode`，占用空间。

> 面试题：
> 不用比较，比较两个uint32_t的大小
> 使用位图

7. 硬链接，符号链接
- 硬链接
  `ln bigfile bigfile_link`
  与目录项是同义词
  相当于目录项又弄了一份，使用`ls -i`可以看到`inode`号相同。

  限制：不能给分区建立，不能给目录建立

- 符号链接
  `ln -s bigfile_link bigfile_s`

  优点：可以跨分区，可以给目录建立
  
```c
int link(const char *oldpath, const char *newpath);

/**
 *  只有没有引用的数据才会真正删除
 *  可以利用这一点创建匿名文件
*/
int unlink(const char *pathname);

int remove(const char *pathname);

/**
 *  改变文件的路径或者名字
*/
int rename(const char *oldpath, const char *newpath);
```

8. `utime`
```c
/**
 *  更改文件最后读/写的时间
*/
int utime(const char *filename, const struct utimbuf *times);

struct utimbuf {
    time_t actime;       /* access time */
    time_t modtime;      /* modification time */
};

struct time_t {
    long tv_sec;         /* seconds */
    long tv_usec;        /* microseconds */
};
```

9. 目录的创建和销毁
`mkdir, rmdir`
```c
int mkdir(const char *pathname, mode_t mode);

/**
 *  只有目录为空才能删除
*/
int rmdir(const char *pathname);
```

10. 更改当前工作路径
`cd, pwd`
```c
/**
 *  改变当前工作路径
 *  可以突破假根目录
 *  但是不能突破chroot
*/
int chdir(const char *path);
int fchdir(int fd);

/**
 *  获取当前工作路径
*/
long getcwd(char *buf, unsigned long size);
```

11. 分析目录/读取目录内容

```c

/**
 *  法一
 *  解析模式/通配符
 * 
 * @prarm: pattern 匹配模式
 * @prarm: flags   匹配标志
 * @prarm: errfunc 错误回调函数
 * @prarm: pglob   匹配结果
 * 
 * @return  匹配的文件数量
*/
int glob(const char *restrict pattern, int flags,
                int (*errfunc)(const char *epath, int eerrno),
                glob_t *restrict pglob);
/**
 *  释放glob_t结构体
*/
void globfree(glob_t *pglob);               

/**
 * 与argc, argv类似
*/
typedef struct {
    size_t   gl_pathc;    /* Count of paths matched so far */
    char   **gl_pathv;    /* List of matched pathnames */
    size_t   gl_offs;     /* Slots to reserve in gl_pathv */
} glob_t;

/**
 *  法二
*/

/**
 *  打开一个目录
 *  返回一个指向DIR结构体的指针
 *  是堆区，需要 closedir 释放
*/
DIR *opendir(const char *name);
DIR *fdopendir(int fd);

/**
 *  关闭一个目录
*/
int closedir(DIR *dirp);

/**
 *  读取一个目录
 * 
 *  返回指针指向静态区
*/
struct dirent *readdir(DIR *dirp);
int readdir_r(DIR *restrict dirp,
              struct dirent *restrict entry,
              struct dirent **restrict result);

struct dirent {
    ino_t          d_ino;       /* inode number */
    off_t          d_off;       /* offset to the next dirent */
    unsigned short d_reclen;    /* length of this record */
    unsigned char  d_type;      /* type of file; not supported
                                   by all file system types */
    char           d_name[256]; /* filename */
};


/**
 *  重置一个目录
*/
void rewinddir(DIR *dirp);

void seekdir(DIR *dirp, long offset);

long telldir(DIR *dirp);

/**
 *  du 命令
 *  以字节为单位，统计目录下所有文件的大小
 *
*/


```

作业：用另一套函数实现`mydu`


## 系统数据文件和信息

> 不同环境可能有区别，以具体查询为准，这里以Linux为例

1. `/etc/passwd`
```c
/**
 *  通过用户名获取用户信息
*/
struct passwd *getpwuid(uid_t uid);

/**
 *  通过用户ID获取用户信息
*/
struct passwd *getpwnam(const char *name);

struct passwd {
    char   *pw_name;       /* username */
    char   *pw_passwd;     /* user password */
    uid_t   pw_uid;        /* user ID */
    gid_t   pw_gid;        /* group ID */
    char   *pw_gecos;      /* user information */
    char   *pw_dir;        /* home directory */
    char   *pw_shell;      /* shell program */
};
```

2. `/etc/group`
```c
/**
 *  通过组ID获取组信息
*/
struct group *getgrgid(gid_t gid);

/**
 *  通过组名获取组信息
*/
struct group *getgrnam(const char *name);

struct group {
    char   *gr_name;       /* group name */
    char   *gr_passwd;     /* group password */
    gid_t   gr_gid;        /* group ID */
    char  **gr_mem;        /* group members */
};
```

3. `/etc/shadow`
ll显示root用户也不可读写，但是只有root用户才可读写
这样是提醒你，即便是root用户，也不要随便读写这个文件

> 密码
> hash - 混淆，不可逆
> 如果原串一样，hash值也一样
> 防备管理员监守自盗
> 
> 加密 - 解密
> 
> 加密为了安全，攻击成本大于收益
> 安全？穷举：口令随机校验（第一遍明明对了给你报错，让你连续两遍成功输入正确）
>
> 推荐书籍：《应用密码学》

```c
/**
  * 获得用户的密码信息
*/
struct *spwd getspnam(const char *name);

/**
 * 加密密码
 *
 * @prarm: key  密码
 * @prarm: salt 盐 杂字串
 *
 * 默认 md5 加密方式
*/
char *crypt(const char *key, const char *salt);

struct spwd {
    char *sp_namp;      /* login name */
    char *sp_pwdp;      /* encrypted password */
    long  sp_lstchg;    /* last change */
    long  sp_min;       /* min days between changes */
    long  sp_max;       /* max days between changes */
    long  sp_warn;      /* warning days before password
                           expires */
    long  sp_inact;     /* days before account inactive */
    long  sp_expire;    /* days since 1970-01-01 until account
                           expires */
    unsigned long sp_flag; /* reserved */
};

/**
 * 输入提示符
*/
char *getpass(const char *prompt);
```

4. 时间戳
机器喜欢大整数 `time_t`
人类喜欢字符串 `char *`
程序员喜欢结构体 `struct tm`

```c

/**
 *  从内核获取以秒为单位的一个时戳
 *  从 UTC 1970年1月1日0时0分0秒 到现在的秒数
*/
time_t time(time_t *t);

// eg: 两种用法
time_t stamp;
time(&stamp);
stamp=time(NULL);

/**
 *  将时间戳转换为结构体
*/
struct tm *gmtime(const time_t *timep);
struct tm *localtime(const time_t *timep);

sturct tm {
    int tm_sec;    /* seconds */
    int tm_min;    /* minutes */
    int tm_hour;   /* hours */
    int tm_mday;   /* day of the month */
    int tm_mon;    /* month */
    int tm_year;   /* year */
    int tm_wday;   /* day of the week */
    int tm_yday;   /* day in the year */
    int tm_isdst;  /* daylight saving time */
                   /* daylight 夏令时调整 */
};

/**
 *  将结构体转换为时间戳
 *  ! 没有 const，可能更改 tm
*/
time_t mktime(struct tm *tm);

/**
 * 格式化输出时间
*/
size_t strftime(char *s, size_t max, const char *format,
                const struct tm *tm);

// eg
strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
```


## 进程环境

### `main`函数
```c
int main(int argc, char *argv[]);
```

### 进程的终止
1. 正常终止:
    - 从`main`函数返回
    - 调用`exit`
      `void exit(int status);`
      status & 0377 有符号的char -128~127

    - 调用`_exit`或者`_Exit`（系统调用）
    > `exit`与`_exit _Exit`的区别
    > `_exit`不执行`atexit`注册的函数，不刷新`stdio`缓冲区
    > 这样可以防止错误扩散
    - 最后一个线程从其启动例程返回
    - 最后一个线程调用了`pthread_exit`

2. 异常终止
    - 调用`abort`
    - 接到一个信号并终止
    - 最后一个线程对其取消请求作出响应


```c
/**
 *  注册一个函数，当进程终止时调用
 *  
 *  钩子函数：逆序执行
 *  可以进行资源释放
*/
int atexit(void (*function)(void));// 钩子函数
```

### 命令行参数的分析
```c
#include <unistd.h>

extern char *optarg; // 选项参数
// optind: 下一个要处理的参数的索引
extern int optind, opterr, optopt;

int getopt(int argc, char *const argv[], const char *optstring);

int getopt_long(int argc, char *const argv[], const char *optstring,
                const struct option *longopts, int *longindex);
```

### 环境变量
**KEY = VALVE**
可以通过`export`命令查看

```c
char *getenv(const char *name);

/*
 * change or add
 * 
 * @prarm: overwrite  是否覆盖
 *
 * 覆盖时是释放原来的空间，重新分配
 */
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);

/*
 * 和getenv一样的作用，change or add
 * 用法不一样，且没有const修饰
 */
int putenv(char *string);
```

### C程序的存储空间布局
`pmap`命令，查看进程空间布局

### 库
- 动态库
- 静态库
- 手工装载库
    ```c
    void *dlopen(const char *filename, int flag);
    char *dlerror(void);
    int dlclose(void *handle);
    void *dlsym(void *handle, const char *symbol);
    // Link with -ldl
    ```

### 函数之间正常的跳转
`goto`无法跨函数跳转。

```c
/*
 *  设置跳转点
 *  
 * @return  0  说明是在设置跳转点
 * @return 非0 说明是通过 longjmp 返回
 */
int setjmp(jmp_buf env);

/*
 * 跳转到跳转点
 *
 * @prarm: env  跳转点 
 * @prarm: val  传递给 setjmp 的值
 */
void longjmp(jmp_buf env, int val);
```

### 资源的获取及控制
`ulimit -a`

```c
// get/set resource limits
int getrlimit(int resource, struct rlimit *rlim);
int setrlimit(int resource, const struct rlimit *rlim);

// 普通用户不能设置超过硬限制
// root 用户可以 升高/降低 硬限制
struct rlimit {
    rlim_t rlim_cur;  /* soft limit */
    rlim_t rlim_max;  /* hard limit */
};
```


# 进程基本知识

已经进入**多进程**阶段

## 进程标识符`pid`
类型`pid_t`，传统意义上是一个16位有符号整型数。
命令`ps`
常用命令：`ps axf`，`ps aux`，`ps axm`，`ps ax -L`
进程号是顺次向下使用
```c
// 返回当前进程号
pid_t getpid(void);

// 返回父进程的进程号
pid_t getppid(void);
```

## 进程的产生

`pid_t fork();`
- 以**复制（duplicating）**当前进程的方式创建一个新进程
- 和`setjmp`一样，执行一次，返回两次
- 在`fork`处复制，不会从头运行

`fork`后父子进程的不同之处：
1. `fork`的返回值不一样
2. `pid`不同
3. `ppid`也不同
4. 未决信号和文件锁不继承
5. 资源利用量清0

`init`进程：**1号**，是所有进程的祖先进程

调度器的调度策略来决定哪个进程先执行

`fflush()`的重要性

```c
/*
 *  vfork创建的子进程只能做exec或者exit
 *  ! 基本废弃
 */
pid_t vfork(void);
```

## 进程的消亡及释放资源
```c
// 等待进程状态发生变化
pid_t wait(int *status); // 阻塞

pid_t waitpid(pid_t pid, int *status, int options);

int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);


wait3();
wait4();
```

分配法和交叉分配法，90%优先选择交叉分配法。

池类算法：
上游往池子里放任务，下游三个线程从池子里取任务。

## `exec`函数族
eg. `bash`进程创建`primer`进程

```c
// exec函数族：替换当前进程的映像

extern char **environ;

int execl(const char *path, const char *arg, ...);

int execlp(const char *file, const char *arg, ...);

int execle(const char *path, const char *arg, ..., char * const envp[]);

int execv(const char *path, char *const argv[]);

int execvp(const char *file, char *const argv[]);

```

## 用户权限及组权限

`u+s`：如果文件是可执行的，则执行文件时，是以文件的拥有者的权限执行的。

```bash
-rwsr-xr-x 1 root root 68248 Mar 23  2023 /usr/bin/passwd
```
所以普通用户执行`passwd`时，是以`root`的权限执行的。

`g+s`：如果文件是可执行的，则执行文件时，是以文件的所在组的权限执行的。

`uid`和`gid`都有三种类型：
1. `real uid`：进程的实际所有者
2. `effective uid`：进程的有效所有者
3. `saved uid`：进程的保存的有效所有者

```
shell获取身份的流程
      fork         exec         fork
init -->--> getty -->--> login -->--> shell
      exec                      exec
root        root         root         user
```

```c

// 获取当前用户的real uid
uid_t getuid(void);

// 获取当前用户的effective uid
uid_t geteuid(void);

// 获取当前进程的real gid
pid_t getegid(void);

// 获取当前进程的effective gid
pid_t getgid(void);

// 设置当前进程的real uid
int setuid(uid_t uid);

// 设置当前进程的effective uid
int seteuid(uid_t uid);

// 设置当前进程的real gid
int setgid(gid_t gid);

// 设置当前进程的effective gid  
int setegid(gid_t gid);

// 交换uid和gid （原子操作）
int setreuid(uid_t ruid, uid_t euid);

// 交换gid和egid （原子操作）
int setregid(gid_t rgid, gid_t egid);

```

## 观摩课：解释器文件

> unix讲究机制而非策略

脚本，后缀名是什么都可以，一般用`sh`, `exec`
```bash
#!/bin/cat

# some shell
```
`#!`是一种约定俗成的标记，告诉系统这个脚本应该用什么解释器来执行。


## `system()`函数
```c
/*
 *  运行一个shell命令
 *  调用/bin/sh
 */
int system(const char *command);
```
相当于`fork+exec+wait`的封装

## 进程会计
```c
//! freeBSD系统的方言
int acct(const char *filename);
```

## 进程时间
```c
clock_t times(struct tms *buf);

// clock_t 滴答数

struct tms{
    clock_t tms_utime;  /* user time */
    clock_t tms_stime;  /* system time */
    clock_t tms_cutime; /* user time of children */
    clock_t tms_cstime; /* system time of children */
}

```

## 守护进程

1. 守护进程`PPID`为1
2. 守护进程没有控制终端，`TTY`为?
3. `PID, PGID, SID`相同

```c
pid_t setpgid(pid_t pid, pid_t pgid);
pid_t getpgid(pid_t pid);

pid_t getpgrp(void); //! 方言
pid_t getpgrp(psid_t pid); //! 方言
```

- 会话（session）：一个或多个进程组的集合，以`sid`为标识
  `pid_t setsid(void);`
  `setsid`必须由非`leader`进程调用，从而创建一个新的会话。
    - 前台进程组：正在与终端交互的进程组
    - 后台进程组：正在运行，但不与终端交互的进程组

- 终端：
  我们接触的都是虚拟终端


**单实例守护进程**：锁文件`/var/run/name.pid`

启动脚本文件：`/etc/rc*...`


## 系统日志
`syslogd`服务
```c
#include <syslog.h>

/**
 *  打开系统日志
 *
 * @prarm: ident  标识符
 * @prarm: option 选项   LOG_CONS, LOG_NDELAY, LOG_NOWAIT, LOG_PERROR ...
 * @prarm: facility  来源  LOG_USER, LOG_DAEMON, LOG_KERN, LOG_LOCAL0~7 ...
 */
void openlog(const char *ident, int option, int facility);

/**
 *  记录系统日志
 *
 * @prarm: priority  优先级 以 ERR 与 WARNING 为分界点
 * @prarm: format 格式化字符串
 * @prarm: ... 格式化参数
 */
void syslog(int priority, const char *format, ...);

/**
 *  关闭系统日志
 */ 
void closelog(void);
```

```bash
sudo tail /var/log/messages  # 老师
journalctl -r  # 我的debian
```

























