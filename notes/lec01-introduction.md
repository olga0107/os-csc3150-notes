# Lecture 1 · Introduction: What Is an Operating System?

> CSC3150 · CUHK-Shenzhen · Fall 2026 · Slides adapted from Berkeley CS 162

**TL;DR**: The OS turns messy shared hardware into a **simple, private, seemingly-infinite virtual machine** for every application. It does this wearing three hats: **Referee, Illusionist, Glue**. We judge an OS by overhead, fairness, portability, reliability, security, and performance.

***

## 1. Why is OS design so hard?

核心问题：为什么写一个 OS 比写普通软件难得多？

### a) The OS is everywhere, and it is never finished (Bell's Law)

![Bell's law](../assets/lec01/page19.png)

Roughly **every 10 years a new device class appears**: mainframe → PC → laptop → cell → cloud/IoT. Each one forces the OS to be rethought. OS 设计永远赶不上硬件形态的演化。

### b) One OS spans ~8 orders of magnitude in time

![Jeff Dean's numbers](../assets/lec01/page20.png)

L1 cache reference costs **0.5 ns**; a CA↔Netherlands packet round trip costs **150,000,000 ns**. The OS must make correct decisions at *every* scale in between. 从纳秒级缓存到百毫秒级网络，调度策略没法"一刀切"。

### c) Complexity keeps exploding

![Lines of code growth](../assets/lec01/page21.png)

Original Unix: **4,501 LoC**. Linux 5.6: **27.8 M**. A modern car: **~100 M**.

背后的驱动力：smarter hardware、higher reliability/security/efficiency expectations、以及永远不会消失的 legacy interfaces。

***

## 2. So, what *is* an OS?

**Definition v1: the resource layer**

![OS between apps and hardware](../assets/lec01/page24.png)

> The layer of software interfacing **(many) applications** with **(diverse) hardware**.

**Definition v2: the virtual machine**（本课程真正的主线）

> The OS implements a **virtual machine** per application whose interface is *more convenient* than raw hardware. Convenient means **portable, reliable, secure**.

v1 说"OS 共享硬件"，v2 说"OS **改变硬件看起来的样子**"。从 v1 到 v2 的视角转换是整门课的钥匙。

***

## 3. Three hats of an OS

![Referee / Illusionist / Glue](../assets/lec01/page27.png)

### 3.1 Referee 🟥 protection, isolation, sharing

核心问题：多个**互不信任**的程序如何同时安全地跑？

![Referee's three concerns](../assets/lec01/page29.png)

| Concern | Question | Mechanism（后续章节展开） |
| ---------------- | ------------------- | -------------------------------- |
| Fault isolation | 程序之间、程序与 OS 之间如何隔离？ | Process, **dual-mode execution** |
| Resource sharing | 下一个跑谁？物理资源怎么分？ | Scheduling |
| Communication | 程序间如何安全地交换结果？ | Pipes / sockets |

> 💡 **Dual-mode execution**：CPU 分 user mode 和 kernel mode。用户态程序不能直接碰硬件和别人的内存，危险操作必须经 **system call** 进入内核。这是"敢跑不可信程序"的硬件地基。

#### Demo 1 · `cpu.c`: the many-CPUs illusion

**What the code does**（逐行拆解）:

```c
int main(int argc, char *argv[]) {
    char *str = argv[1];            /* 命令行第一个参数，如 "A" */
    while (1) { printf("%s\n", str); }   /* 无限循环打印它 */
}
```

这个程序一旦启动就**永远不会自己结束**，是观察 OS 行为的完美"探针"。

**The experiment**:

```bash
clang -o cpu cpu.c
./cpu A & ./cpu B & ./cpu C &    # shell 的 & = 后台运行，同时存在 3 个进程
```

Actual output captured on macOS（0.15 s 内的尾部切片）:

```
A B A A C A C B C B B A C B C B B A B C B C B C B C B A B A A C A C B ...
```

**How to read this output**:

1. 三个程序各自陷入死循环。理论上谁"先跑"谁就该永远霸占 CPU，输出应该全是 `A`。
2. 实际却是无规律的交替。这说明有"第三者"在**反复打断**当前程序，把 CPU 交给下一个。这个第三者就是 OS。
3. 交替无固定模式（不是整齐的 `ABCABC`），因为调度由 **timer interrupt** 触发，时机对程序不可见。

**The mechanism**: 每次 timer interrupt 到来，CPU 自动跳转到内核。OS 保存当前进程的寄存器现场（**context**），加载另一个进程的现场继续跑。这就是 **context switch**。切换足够快（毫秒级），每个进程就感觉自己在连续执行，这就是 **virtualized CPU**。

> 💡 **Cooperative vs preemptive**：如果 OS 只能*等程序主动*交出 CPU，即 **cooperative multitasking**（Mac OS 9 / Win 3.1 时代），这个 `while(1)` 程序会永远霸住处理器，**整机卡死**。现代 OS 用 **preemptive multitasking**：timer interrupt 是硬件行为，不需要程序配合，OS 随时能夺回控制权。一个死循环最多占满它自己的时间片，拖不垮系统。

### 3.2 Illusionist 🎩 hide hardware limits via virtualization

核心问题：如何让每个程序都觉得自己独占一台无限强的机器？

* **All alone**: exclusive use of the machine
* **All powerful**: resources feel infinite
* **All expressive**: capabilities that don't physically exist

#### Demo 2 · `memory.c`: the private-memory illusion

**What the code does**（逐行拆解）:

```c
int *p = malloc(sizeof(int));              /* 堆上申请 4 字节，p 存其地址 */
printf("(%d) p: %p\n", getpid(), p);       /* getpid() = 进程 ID；%p 打印 p 里的地址值 */
*p = 0;
while (1) { *p += 1; printf("(%d) p: %d\n", getpid(), *p); }   /* 反复给 *p 加 1 并打印 */
```

注意区分两个东西：`p` 是**地址**（这 4 字节"在哪里"），`*p` 是**值**（那 4 字节里"存了什么"）。

**The experiment**（本机真实运行，两个进程同时跑）:

```
(5536) p: 0x102b9da60      (5535) p: 0x104d45a60
(5536) p: 1                (5535) p: 1
(5536) p: 2                (5535) p: 2     ← 两个计数器完全独立
(5536) p: 3                (5535) p: 3
```

课件里 Linux 上的版本更戏剧化：两个进程打印出**完全相同的地址** `0x200000`，计数器却互不干扰。（macOS 上实测地址不同，因为 macOS 默认开 ASLR 随机化地址，但结论一样：各自的计数器互不可见。）

**Why this is astonishing**: 两个进程都在对"自己看到的那个地址"读写。如果程序直接操作**物理内存**，相同地址就是同一个内存单元，两个计数器必然互相覆盖（你加 1 我也加 1，数字会翻倍乱跳）。事实没有发生，所以程序看到的地址**不是**物理地址。

> 💡 **Virtual memory**：每个进程看到的地址是**虚拟地址**，OS 借助硬件 **MMU** 和 **page table** 把它翻译成各自不同的物理位置。效果：每个进程都以为自己独享一块从熟悉地址开始的连续内存。**隔离性**（谁也碰不到谁）和**便利性**（不用关心物理内存在哪）一次搞定。Address Space 章节会完整展开翻译机制。

**反向验证**：如果没有 virtual memory，任何程序的一个野指针 bug 就可能改写别的进程、甚至内核的内存，multiprogramming（多程序共存）根本不可能安全实现。这也是为什么 virtual memory 是 Referee（隔离）和 Illusionist（假象）两顶帽子的交汇点。

### 3.3 Glue 🩹 common services

核心问题：如何避免每个程序都重造轮子？

File system、UI、networking 等标准服务带来三个好处：sharing easier（大家用同一套 primitives）、reuse maximized、components evolve independently。

### Putting it together

![Referee + Illusionist + Glue = easy-to-use VM](../assets/lec01/page37.png)

左边是现实：一堆杂乱的 processor、memory、storage、networks。右边是每个程序*看到*的世界：infinite processors、infinite memory、标准服务。**这个从"杂乱共享"到"私有虚拟机"的变换，就是 OS 本身。**

***

## 4. How do we judge an OS?

核心问题："好 OS"的标准是什么？abstractions must be **efficient, low-overhead, equitable**。

| Criterion | Meaning | 关键词 |
| ----------- | -------------------- | ----------------------------------------- |
| Overhead | 提供 abstraction 的额外代价 | virtualization is not free |
| Fairness | 资源在应用间分配是否公平 | scheduling policy |
| Portability | 硬件变了，app/OS 要改多少 | **AMI / HAL**（见下图） |
| Reliability | 系统做它该做的；OS 挂掉是灾难性的 | availability = f(MTTF, MTTR) |
| Security | 攻击下维持正常功能 | integrity + privacy |
| Performance | 满足用户与管理员预期 | response time, throughput, predictability |

Portability 值得单独看，它解释了所有现代 OS 的分层设计：

![AMI and HAL](../assets/lec01/page39.png)

* **AMI (Abstract Machine Interface)**: 面向应用的 syscall 接口，硬件变了它也**保持稳定**。
* **HAL (Hardware Abstraction Layer)**: 面向设备的一层，吸收硬件差异。
* 不可能每出一款新硬件就重写所有 app，而且必须**为还不存在的硬件做设计**。

***

## 5. Why this course matters more in the AI age

Rich Sutton (Turing Award 2024), *The Bitter Lesson*: **handcrafted knowledge plateaus; learning + search scale with computation.**

本课的补充视角：**systems unlock that computation**。

![AI demand vs Moore's law](../assets/lec01/page47.png)

AI 训练需求每 18 个月涨 **10×**，Moore's Law 只给 **2×**，缺口越来越大。GPU/专用芯片、从 server 到 pod 的规模化，全都依赖更好的 scheduling、memory、parallelism、networking。全是 OS 的活。

> Takeaway: **More efficient computation creates more room for intelligence.** Scale changes, hardware changes, application changes, so the OS will keep changing.

***

## 6. Self-check

1. 用一句话向没学过 CS 的人解释 OS 是什么。
2. `memory.c` 里两个进程地址相同却互不干扰，靠的是什么机制？如果关掉它会发生什么？
3. Preemptive vs cooperative multitasking 的区别？哪一种能让一个死循环程序冻住整机？
4. Referee / Illusionist / Glue 各管什么？dual-mode execution 属于哪一顶 hat 的职责？
5. 为什么 portability 要求 AMI 稳定而 HAL 可变？

<br />

**A1.** OS 是一层软件，把杂乱共享的硬件变成每个程序眼中一台**简单、私有、看起来无限大的专属电脑（virtual machine）**。

**A2.** 靠 **virtual memory**：程序看到的地址是虚拟地址，OS 借助硬件 **MMU** 和 **page table**（虚拟地址 → 物理地址的字典）把每个进程的虚拟地址翻译到**不同的物理位置**，所以同地址、不同内存。**如果关掉它**（程序直接用物理地址）：两个进程写同一地址 = 写同一内存单元，计数器互相覆盖；更糟的是任何程序的野指针 bug 都能改写别的进程甚至内核，multiprogramming 根本没法安全实现。

**A3.** 区别在于**谁收回 CPU 控制权**：

| | Cooperative | Preemptive |
| ------ | ------------------ | ------------------------------- |
| 谁决定切换 | 程序**主动**交出 CPU | OS 靠硬件 **timer interrupt 强制**收回 |
| 死循环后果 | **整机卡死**（永不 yield） | 只占满自己的时间片，系统照常 |
| 代表 | Mac OS 9 / Win 3.1 | 所有现代 OS |

会冻住整机的是 **cooperative**。

**A4.** **Referee** = protection / isolation / sharing（process、dual-mode execution、scheduling、pipes）；**Illusionist** = virtualization 假象（virtual memory、virtualized CPU）；**Glue** = common services（file system、network、UI）。**Dual-mode execution 属于 Referee**：user mode 下程序碰不了硬件和别人内存，危险操作必须走 system call 进 kernel mode，裁判靠这条红线执法。

**A5.** **AMI** 是 OS 对**应用程序**的承诺（`fork()`、`read()` 等 syscall），全世界软件都照着它写。它跟着硬件变，生态就会崩溃，所以必须**几十年稳定**。**HAL** 是 OS 内部面向硬件的一层，新设备来了只改 HAL/驱动，应用无感。一句话：**对上用不变的接口稳住应用生态，对下用可换的一层吸收硬件变化**，这就是"为还不存在的硬件做设计"。

***

*Images extracted from the official Lecture 1 slides. Notes rewritten in my own words for review; errors are mine.*
