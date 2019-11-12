---
title: ACM中一些常见的问题及原因
tags:
  - ACM
connments: true
date: 2019-01-30 17:23:34
categories: 
	- Notes
desc:
summary:
---


最近在 ACM 群灌水，顺便解决了一点问题，感觉可能会成为日经系列，寻思可以写个博文专门写写，正好也是对自己学习知识的检验。

我觉得这篇文章阅读方式可以按照《C语言常见问题集》，遇到问题了来看看也行，电脑阅读的时候右边应该会有个目录可以跳转，顺着读一遍也行。

文章只能慢慢更新，因为我并不想随意地对待这种难得的问题，然而工作在身并不能全身心投入。

以下内容如无特殊声明，采用C99和C++11标准。

<!--more-->

# 为什么在读入的时候不加&经常会 RE，但是数组名不加&就没问题？

我们直接来看一段代码：

```c
#include <stdio.h>

int main()
{
	int a;
	scanf("%d", a);
	return 0;
}
```

编译运行的结果基本是`Segmentation fault (core dumped)`，如果是在windows上用codeblocks、devcpp之类的写的程序，多半会看到![1573525460495](ACM中一些常见的问题及原因/1573525460495.png)或者![1573526342103](ACM中一些常见的问题及原因/1573526342103.png)如果是devcpp，还可能是在一段时间的无响应后退出，同时返回值是一个大得惊人的数字![1573525655820](ACM中一些常见的问题及原因/1573525655820.png)

这里我们需要对比一下`a`和`&a`的区别，我们先直接把它们打印出来看一下。

```c
#include <stdio.h>

int main()
{
	int a;
	printf("%x\n%x\n", a, &a); 
	return 0;
}
```

`%x`的意思是按十六进制打印，运行我们应该会看到类似如下的结果：

```bash
➜  ./format
0  # 这里值不一定，也可能不是0
f35ab98c # 这里的值也不一定，但是通常会很大，比如这个十六进制转成十进制就是‭3759417480‬
```

这里我们再进一步，做一个测试，首先我们改写程序，其次用系统工具查看一下系统调用。

先介绍一下我们用到的工具：

`echo`是能将输入原样输出的一个程序，如果下面是一个运行的例子：

```bash
➜  echo 12341234
12341234
```

`ltrace`是一个能跟踪库函数调用的工具，也就是说它能追踪到你程序调用的库里的类似`scanf`、`printf`这样的函数并且把调用函数的参数打印出来。

然后我们对下面这段程序使用上面写的工具，把`1234 1234`作为输入喂给程序。

```c
#include <stdio.h>

int main()
{
	int a;
	printf("%d", a);
	printf("%d", &a);
	scanf("%d", &a);
	scanf("%d", a);
	return 0;
}
```

运行结果如下：

```bash
➜  echo "1234 1234" | ltrace ./format
printf("%d", 0)                                             = 1
printf("%d", -787805524)                                    = 10
__isoc99_scanf(0x7fa859609004, 0x7fffd10b0aac, 0x7fa85959d8c0, 0) = 1
__isoc99_scanf(0x7fa859609004, 1234, 0x7fa85959d8d0, 16 <no return ...>
--- SIGSEGV (Segmentation fault) ---
+++ killed by SIGSEGV +++
```

我们可以看到，`printf`打印出了`a`的值，也就是`0`，同时第一个赋值没有出错，将`a`改为了1234，但是接下来的一行代码又出现了1234这个数字`__isoc99_scanf(0x7fa859609004, 1234, 0x7fa85959d8d0, 16 <no return ...>`。重复实验并且改变输入的数字，会发现这一行调用函数的第二个参数总是我们输入的第一个数字，也就是第一次给a赋值并成功的结果。

学过操作系统的同学或者对计算机组成有所了解的同学会知道，程序运行时的各种所需都是存储在内存里的，变量也不例外，而它们在内存里都是有个地址的。基于这些知识和刚才实验的结果，我们可以推测，`scanf("%d", a);`是将a的值认为是一个指针的地址并尝试向里面写入数据，但是a的默认值并不一定是一块给写数据的内存空间，这时候就引发了一个错误。

接下来让我们到`scanf`的[manual page](http://man7.org/linux/man-pages/man3/scanf.3.html)看看描述。

这里我把关键的两段截出来，其他部分对这个问题帮助不大所以就自己看了。重点看加粗部分

>        The scanf() family of functions scans input according to *format* as
>        described below.  This format may contain *conversion specifications*;
>        the results from such conversions, **if any, are stored in the
>        locations pointed to by the *pointer* arguments that follow format**.
>        Each pointer argument must be of a type that is appropriate for the
>        value returned by the corresponding conversion specification.
> 
>        **If the number of conversion specifications in format exceeds the
>        number of pointer arguments, the results are undefined.  If the
>        number of pointer arguments exceeds the number of conversion
>        specifications, then the excess pointer arguments are evaluated, but
>        are otherwise ignored.**

这里format指的是[格式化字符串](https://en.wikipedia.org/wiki/Printf_format_string)，简单来说就是printf/printf的第一个参数，fscanf/fprintf的第二个参数，而这个字符串里的conversion specifications（中文维基直译为转换说明）就是指指定类型的那个e/p/d/x，比如%d中的d。同时从这里可能也就能理解为什么要叫printf/scanf而不叫print/scan，因为这是print format/scan format的简写。顺便一提，格式化字符串还是一类漏洞的名称，有兴趣的话不妨搜索了解一下。

然后回到话题，看到第一处加粗的文字，也就是说，若里面有类似%d这样的参数，而且也有对应的参数（比如`scanf("%d", a)；`里的a就是%d对应的参数）scanf会按照格式读入数据（并将其转化为二进制表示），然后存入对应**指针**参数指向的地址。也就是说它把后面的参数认为是指针了。

这里C语言熟练的朋友就知道，局部自动(auto)变量定义时候不会隐式初始化，所以a的值是不确定的，把它直接给scanf用的结果大抵相当于向野指针指的地方写数据。所以段错误什么的很正常吧，

那么为什么下面这段代码不会出错呢？

```c
#include <stdio.h>

int main()
{
	int a[1];
	printf("%d", a);
	printf("%d", &a);
	scanf("%d", &a);
	scanf("%d", a);
	return 0;
}
```

> 因为数组名是常量指针。

肯定有认真听课的这样回答，但是很遗憾，不是的。数组名不是指针，但是它的确被转换成了指针。具体的内容可以搜索《说出来你们可能不信，但是数组名确实不是指针常量》。但是这个东西并不太重要，如果不当语言律师也没打算深度钻研C，也没有必要理解。

# 在函数里开不了大数组吗？

这里我们继续用代码做实验

```c
#include <stdio.h>

int main()
{
	int a[524288];
	printf("OK\n");
	return 0;
}
```

编译运行，程序多半并不能让我们看到`OK`，而是会出现和前面一样的情况，也就是段错误。（如果没出现就把这个数字翻几倍就能看到效果了）。

简单来说的话，函数中声明定义的非全局、非static，且没有使用`*alloc`、`new`产生的变量都会被放在栈上，栈的大小由编译选项决定，在我的电脑上大致就是能开500,000个int的大小。也可以在OJ上测试一下栈的大小，但是不建议在栈上开这么大，在ACM中还是全局的数组比较好用。

# 我开了一个很大的全局数组，但是编译无法通过，这是为什么？

编译器会阻止你开一个大小超过size_t表达范围的数组，也就是`sizeof`不会超过表达范围导致溢出的范围。而size_t是一个和平台相关的数据结构，在32位Linux机下，有以下结果：

```bash
root@vultr:~# cat test.c
#include<stdio.h>
int a[10000000000];
int main()
{
        printf("%d\n%d",sizeof(size_t),sizeof(long long));
        return 0;
}

root@vultr:~# gcc test.c  -o test
test.c:2:5: error: size of array ‘a’ is too large
 int a[10000000000];
     ^
```

而在64位Linux机下：

```bash
root@vultr:~# cat test.c
#include<stdio.h>
int a[10000000000];
int main()
{
                printf("%d\n%d",sizeof(size_t),sizeof(long long));
                        return 0;
}

root@vultr:~# gcc test.c -o test
root@vultr:~# cat test2.c
#include<stdio.h>
int a[10000000000000000000];
int main()
{
                printf("%d\n%d",sizeof(size_t),sizeof(long long));
                        return 0;
}

root@vultr:~# gcc test2.c -o test2
test2.c:2:5: error: size of array ‘a’ is too large
 int a[10000000000000000000];
     ^
```

# 我开了一个稍小一些的全局数组，为什么编译还是失败了而且错误的原因非常复杂



# 为什么越界会导致 RE？为什么有时候越界不会 RE 但是会 TLE/WA？



# 我只定义过一个 left，为什么编译器告诉我`[Error] reference to 'left' is ambiguous`？

如果你仔细查看报错信息，应该会看到类似

```
916	3	{PATH}	[Note] std::ios_base& std::left(std::ios_base&)
```

出现这种问题是因为引入的头文件中的变量，比如left这个是在`bits/ios_base.h`里，而`algorithm`里有count这样的函数。

这些变量通常都在有两种方法可以解决，一是不要使用`using namespace std;`这个语句，另一个是不要用类似`left`, `count`这样的名称。从工程角度来说，第一个做法是比较合理的。

# 我这样写有问题吗？为什么读入会失败？（关于整数的读入）



# 总是说的堆栈，是指什么？

首先需要了解一些前置知识，这些前置知识能在《计算机组成原理》《操作系统》这样的书籍里找到，这里仅作简单地补充。