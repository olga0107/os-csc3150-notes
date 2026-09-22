---
prev: false
next: false
---
# Foundations · C Pointers & API Parameters

**TL;DR**

- `x` 取值，`&x` 取地址，`*p` 沿着 p 保存的地址访问对象。
- C 按值传参；传地址时，复制的是地址，函数仍能借此找到原对象。
- 读 API 时分别找出：输入是什么、输出写在哪里、对象要存活多久。

## 1. 先区分：对象、值、地址

**核心问题：`p` 和 `*p` 为什么不是同一个东西？**

```c
int x = 7;
int *p = &x;
*p = 9;
```

逐行执行：

1. `int x = 7`：创建一个 int 对象 x，其中存 7。
2. `int *p = &x`：创建一个指针对象 p，其中存 x 的地址。
3. `*p = 9`：按 p 中的地址找到 x，把 x 改成 9；p 中的地址没有变化。

```text
p [x 的地址] ──────► x [9]
```

| 表达式 | 含义 | 此时得到什么？ |
|---|---|---|
| `x` | 读 x 的值 | 9 |
| `&x` | 取 x 的地址 | 指向 x 的地址 |
| `p` | 读 p 保存的地址 | 与 `&x` 相同 |
| `*p` | 沿 p 访问对象 | 访问 x，读它得到 9 |
| `&p` | 取指针对象 p 自己的地址 | 指向 p，不是指向 x |

**声明中的 `*` 与表达式中的 `*`：**

- `int *p` 是声明，说明“p 是指向 int 的指针”。
- 后面的 `*p` 是 dereference（解引用），说明“访问 p 指向的对象”。
- 声明一个指针，不会自动创建它将来指向的 int。必须先让它指向合法对象，才能通过它读写。

## 2. 传值、传地址：到底复制了什么？

**核心问题：为什么一个函数改不了 x，另一个却可以？**

```c
void by_value(int n) { n = 42; }
void by_pointer(int *p) { *p = 42; }

int x = 7;
by_value(x);     // x 仍为 7
by_pointer(&x);  // x 变为 42
```

**调用 `by_value(x)`：**

1. 把 x 的值 7 复制给参数 n。
2. 函数把自己的 n 改成 42。
3. `x` 和 `n` 是不同对象，所以 x 仍然是 7。

**调用 `by_pointer(&x)`：**

1. 把 x 的地址复制给参数 p。
2. `p` 虽然也是一份参数副本，但其中的地址仍指向原来的 x。
3. `*p = 42` 沿地址修改 x，所以 x 变为 42。

> 💡 两次都按值传递。第二次传的是“地址这个值”，不是把 C 改成了按引用传递。

### 两层指针：修改 p 本身

**核心问题：如果要换掉 p 指向的位置，该传什么？**

```c
void redirect(int **out, int *target) {
    *out = target;
}

int x = 7;
int y = 99;
int *p = &x;
redirect(&p, &y);
```

| 对象 / 表达式 | 在 redirect 中表示什么？ |
|---|---|
| `out` | p 这个指针对象的地址 |
| `*out` | p 这个对象本身 |
| `target` | y 的地址 |
| `*out = target` | 把 y 的地址写入 p |

调用后，p 指向 y，`*p` 是 99；x 仍然是 7。

```text
调用前：p → x [7]       调用后：p → y [99]
函数中：out → p
```

**对比两个赋值：** `p = &y` 改的是指针的去向；`*p = 99` 改的是它当前指向的对象。

## 3. 函数指针与 void *

**核心问题：为什么 pthread_create 既要传函数，也要传数据地址？**

新线程需要知道两件事：“运行哪个函数”和“把什么数据交给它”。

```c
void *worker(void *arg);                 // 一个函数的声明
void *(*start_routine)(void *) = worker; // 指向这种函数的指针
```

从中间往外读第二行的类型：

1. `(*start_routine)`：start_routine 是一个指针。
2. 后面的 `(void *)`：它指向的函数接收一个 void 指针参数。
3. 最前面的 `void *`：这个函数返回一个 void 指针。

**传 `worker` 与写 `worker(arg)` 不同：** 前者把函数地址交出去，后者就在当前执行流中调用函数。

### void * 不等于“什么都知道”

`void *` 可以携带不同对象的地址，但不会附带“这是 int”“有多少个元素”等信息。发送方与接收方必须约定真实类型。

```c
struct job { int input; int output; };

void *worker(void *arg) {
    struct job *job = arg;
    job->output = job->input * job->input;
    return job;
}
```

- Arg 实际来自一个 job 对象的地址，所以先用 `struct job *` 接收。
- `job->input` 等价于 `(*job).input`：先找到 struct，再访问其中的 input 成员。
- Return 交出的是这个对象的地址，不会自动复制整个 struct。
- 如果实际传入 int 的地址，却按 job 对象访问，就用错了类型。

## 4. 把指针对应到 pthread API

**核心问题：为什么 create 用 `&tid`，join 却用 `tid` 和 `&result`？**

下面只展示调用关系，完整可运行代码与错误处理见[第三讲的示例](../notes/lec03-thread-process.md#完整示例-参数与结果由谁持有)。

```c
pthread_t tid;
void *result = NULL;
pthread_create(&tid, NULL, worker, &input);
pthread_join(tid, &result);
```

| 参数 | 方向 | 原因 |
|---|---|---|
| `&tid` | 输出 | Create 要把线程标识写进 tid，需要知道 tid 的位置 |
| `NULL` 属性 | 输入 | 使用默认线程属性 |
| `worker` | 输入 | 指定新线程执行的函数 |
| `&input` | 输入 | 把一个对象的地址交给 worker |
| Join 中的 `tid` | 输入 | 标明要等哪条线程，不是要求修改 tid |
| `&result` | 输出 | Join 要把 worker 返回的地址写入 result |

`result` 的类型是 `void *`，所以 `&result` 的类型是 `void **`。与上一节的 out 一样：**要写进某个变量，就传这个变量的地址。**

### 为什么不能反复传同一个循环变量？

假设创建 worker 时总传 `&i`：

1. Main 在 i=0 时创建第一个 worker，但 worker 不一定立刻读取 i。
2. Main 继续循环，把同一个 i 改成 1、2。
3. Worker 后来读取的仍是同一个对象，不是创建时自动保存的 0。
4. 若修改与读取之间没有同步，还可能构成 data race。

正确思路是准备独立参数，例如 `jobs[0]`、`jobs[1]`，分别传它们的地址，并让数组一直存活到相关线程结束。**传指针不会替调用者保存对象的历史值。**

## 5. 地址能保存，对象却可能已经失效

**核心问题：返回一个地址，为什么不保证以后还能读取？**

```c
// 错误示例：只用于说明生命周期问题。
void *bad_worker(void *unused) {
    int result = 42;
    return &result;
}
```

- `result` 是这次调用的局部对象，函数返回后生命周期结束。
- 返回的地址数值即使还在，也不代表原对象仍然存在。
- 把它转换成 void 指针、让 main 接收它、或等待 worker 结束，都不能延长 result 的生命周期。

可行做法：

- 由 main 准备对象，保证 worker 结束后仍可使用；第三讲的 jobs 数组就是这种做法。
- 使用 `malloc` 分配，并约定由谁在最后 `free`，同时处理分配失败。
- 不需要结果时返回 NULL，join 也可以用 NULL 表示不接收结果。

## Demo · 分别观察三种修改

在项目根目录编译运行：

```sh
cc -std=c11 -Wall -Wextra demos/lec03/pointer_review.c -o /tmp/os-pointer-review
/tmp/os-pointer-review
```

实际输出：

```text
after value: 7
after pointer: 42
redirected: 99
```

三行分别验证：修改参数副本不改原值；通过地址可修改原对象；两层指针可修改指针本身。

## Self-check

1. `p = &y` 与 `*p = y` 各修改什么？
2. 传 `&x` 时，函数参数还是副本吗？为什么能修改 x？
3. `worker` 与 `worker(arg)` 有什么区别？
4. `pthread_join(tid, &result)` 为什么需要两层指针？传 NULL 又表示什么？
5. 为什么返回局部变量地址，即使暂时能读出正确数值，也仍然错误？

**A1.** 前者改变 p 保存的地址；后者改变 p 当前指向的对象的值。

**A2.** 仍是副本，只是复制了地址。副本中的地址和原来的地址指向同一个 x，所以通过它能修改 x。

**A3.** 前者提供函数地址；后者调用函数。Create 要获得入口地址，交给新线程执行。

**A4.** `result` 本身是 `void *`，join 要修改它，就接收其地址 `void **`。NULL 表示不接收结果，但仍等待目标线程结束。

**A5.** 对象的生命周期已经结束，旧字节尚未被覆盖不是继续访问的许可。需要返回仍存活对象的地址。
