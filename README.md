# 0811_stm32f4 — 结课作业:FreeRTOS + CAN 主从板通信系统

两人一组、合作完成的结课作业。**同一套工程代码**,通过 `BOARD_MASTER` 宏在主/从板之间切换,两人分别在 `user/` 里各自的模块上开发。

## 一、结课作业需求(原题)

1. 使用 **FreeRTOS**,两颗流水灯作为板子**工作状态**的指示。
2. 商定通信协议,实现:
   - **主板**接收 **VOFA** 指令,发送 CAN 消息,控制**自己和从板**的呼吸灯**开始/停止**,
     并可通过 VOFA 指令参数调整**呼吸快慢**。格式:`A5 XX(开/关) XX(周期) 5A`。
   - **从板**以 **100Hz** 频率反馈 CAN 消息给主板,反馈 **1 个变化的 float 数据**,
     主板把它打到 VOFA 波形图里,并 log 输出呼吸灯参数。
3. 主/从板还需分别向总线发送 **ID 为 0x012** 和 **0x02010101** 的报文(需被对方过滤)。
4. 主/从板接收 **CANABLE** 发送的报文,实现**蜂鸣器定次数响应**。
5. 通信各只用一个 CAN 口;噪声 CAN 消息频率 500Hz 时不丢功能。

## 二、硬件平台

| 项目 | 内容 |
|---|---|
| 主控 | STM32F405RGT6(Cortex-M4F, 1MB Flash / 192KB RAM) |
| 系统时钟 | 外部 25MHz 晶振 → PLL → **84MHz** |
| 外设 | CAN1、USART1、TIM1/2/3、FreeRTOS(CMSIS-V2)、DMA |
| 工具链 | Keil MDK-ARM(µVision),工程文件 `MDK-ARM/0811_stm32f4.uvprojx` |

### 引脚分配

| 引脚 | 功能 | 说明 |
|---|---|---|
| PA4 | LED_1 | 流水灯/状态指示,GPIO 推挽输出 |
| PA5 | LED_2 | 流水灯/状态指示,GPIO 推挽输出 |
| PA6 | LED_3 | 呼吸灯,TIM3_CH1 PWM |
| PA7 | LED_4 | 呼吸灯,TIM3_CH2 PWM |
| PA8 | BEEP | 蜂鸣器,高电平响 |
| PA9 | USART1_TX | 串口(VOFA 上位机) |
| PA10 | USART1_RX | 串口(VOFA 上位机) |
| PA11 | CAN1_RX | CAN 总线接收 |
| PA12 | CAN1_TX | CAN 总线发送 |
| PC11 | INPUT_1 | 按键(EXTI,旧功能,可保留/移除) |

### 外设参数

| 外设 | 参数 |
|---|---|
| CAN1 | **1 Mbps**(Prescaler=3, BS1=9TQ, BS2=4TQ),正常模式,自动离线恢复 |
| USART1 | 115200, 8N1,DMA + IDLE 变长接收 |
| TIM3 | 1kHz PWM(CH1/CH2 → 呼吸灯) |
| TIM1 | HAL 时基(1ms tick,供 `HAL_GetTick`) |
| TIM2 | 预留(500Hz 更新中断,当前未用) |

## 三、主从板切换 —— 最重要的一处

两板共用同一份代码,只靠一个宏区分。改这里即可切换:

```
user/inc/protocol.h:
    #ifndef BOARD_MASTER
    #define BOARD_MASTER  1    /* 1=主板, 0=从板 */
    #endif
```

- `BOARD_MASTER == 1` → **主板**:收 VOFA 指令、下发控制、收从板 float 打波
- `BOARD_MASTER == 0` → **从板**:100Hz 反馈 float、收主板指令控呼吸灯

代码里所有主从分支都写在 `#if BOARD_MASTER / #else / #endif` 里
(见 `user/src/app_tasks.c`、`user/src/app_tasks.h`)。

> ⚠️ 两个开发者各自编译前,请确认自己板子上的 `BOARD_MASTER` 值。

## 四、工程结构

```
0811_stm32f4/
├── 0811_stm32f4.ioc              # CubeMX 配置(重新生成用)
├── Core/                         # CubeMX 生成代码
│   ├── Inc/                      #   main.h、can.h、usart.h、tim.h、dma.h、gpio.h
│   └── Src/
│       ├── main.c                # 入口:外设初始化 + 启动 FreeRTOS
│       ├── can.c                 # CAN1 初始化 + 过滤器(★ 主从过滤在此改)
│       ├── usart.c               # USART1 初始化 + DMA 收发通道
│       ├── freertos.c            # RTOS 初始化(目前只有 defaultTask)
│       ├── dma.c / tim.c / gpio.c / stm32f4xx_it.c / system_stm32f4xx.c
├── Middlewares/Third_Party/FreeRTOS/   # FreeRTOS 内核
├── Drivers/                      # HAL 库 + CMSIS
├── MDK-ARM/                      # Keil 工程
└── user/                         # ★ 应用层(主从板代码全部在这里)
    ├── inc/
    │   ├── protocol.h            # ★ 协议定义 + BOARD_MASTER 宏(唯一配置入口)
    │   ├── app_tasks.h / .c      # ★ FreeRTOS 任务 + 队列(主从分支)
    │   ├── can_app.h / .c        # CAN 应用状态机(蜂鸣/流水)
    │   ├── can_irq.h / .c        # CAN 收发封装 + 接收中断回调
    │   ├── uart_irq.h / .c       # UART 接收(待改成 A5..5A VOFA 协议)
    │   ├── app.h / .c            # 旧按键状态机(按键控流水/呼吸,可选保留)
    │   ├── breath_led.h / .c     # 呼吸灯 PWM 驱动
    │   ├── led_flow.h / .c       # 流水灯驱动
    │   ├── buzzer.h / .c         # 蜂鸣器(非阻塞)
    │   └── button.h / .c         # 按键(旧)
```

## 五、软件架构:FreeRTOS 任务与队列

`user/src/app_tasks.c` 已定义好任务骨架与队列句柄,任务逻辑待填充:

| 任务 | 职责 | 主从 |
|---|---|---|
| `heartbeat_task` | LED1/2 交替闪烁,板子工作状态指示(需求 1) | 双板 |
| `breath_task` | 呼吸灯占空比周期刷新(周期可调) | 双板 |
| `buzzer_task` | 蜂鸣器定次数响应 | 双板 |
| `can_rx_task` | CAN 接收处理(主从不同) | 双板 |
| `can_tx_task` | CAN 周期发送:主板每 50ms 发 0x012;从板每 10ms 发 100Hz float | 双板 |
| `vofa_rx_task` | VOFA 指令解析(仅主板) | 主板 |

队列句柄(声明在 `app_tasks.h`,定义在 `app_tasks.c`):

| 队列 | 用途 | 主从 |
|---|---|---|
| `can_rx_queue` | CAN 接收消息投递给 can_rx_task | 双板 |
| `beep_queue` | 蜂鸣请求投递给 buzzer_task | 双板 |
| `vofa_cmd_queue` | VOFA 指令投递给 vofa_rx_task | 仅主板 |

启动流程(main.c):HAL 初始化 → 各 `MX_*_Init()` → `app_init` / `can_app_init` / `CAN_Start`
→ `osKernelInitialize()` → `MX_FREERTOS_Init()` → `osKernelStart()`。

> ⚠️ 当前 `MX_FREERTOS_Init()` 里只创建了 `defaultTask`,`app_tasks_create()`
> **还没有被调用**——任务骨架写好了但还没接线。把 `app_tasks_create()` 加进
> `MX_FREERTOS_Init()` 的 `RTOS_QUEUES/RTOS_THREADS` 区即可。

## 六、通信协议

### 1. VOFA → 主板(串口 USART1,待实现)

```
A5  XX  XX  5A
└─  ┌┘  ┌┘   └─ 帧尾
    │   └────── 呼吸周期(调快慢)
    └────────── 开/关(如 0x01=开, 0x00=关)
```
主板收到后解析,决定本地呼吸灯状态,并经 CAN 下发从板。

### 2. CAN 帧(当前代码中的 ID)

`can_irq.c` 收发封装与接收回调,`can.c` 过滤器:

| 方向 | ID(扩展帧) | 含义 | 备注 |
|---|---|---|---|
| 收 | `0x01020101` | 蜂鸣器响次数 | data[0]=次数 |
| 收 | `0x01020201` | 流水灯开关 | data[0]=0/1 |
| 发 | `0x02010101` | 蜂鸣完成 OK | can_app.c |
| 发 | `0x02010201` | 流水开关 OK | can_app.c |

CAN 过滤器(can.c,FilterBank=0, IDLIST 模式)当前只放行 `0x01020101`、`0x01020201` 两个扩展帧。

> ⚠️ **作业要求 vs 当前代码的 ID 差异,需两人最终对齐:**
> 作业要求主/从板分别发 **0x012** 与 **0x02010101**(并被对方过滤),
> 而当前代码用的是 0x01020101 / 0x01020201 / 0x02010201 等。定稿协议时
> 请把 `can_irq.c`、`can_app.c`、`can.c` 过滤器三处一起改,保持一致。

### 3. 从板 → 主板 100Hz float(待实现)

从板 `can_tx_task` 每 10ms 发一帧 CAN,负载放 1 个 float(小端);
主板 `can_rx_task` 收到后提取 float,经 USART 打到 VOFA 波形图并 log。

## 七、模块职责与完成状态

| 模块 | 文件 | 职责 | 状态 |
|---|---|---|---|
| CAN 硬件 + 过滤器 | `Core/Src/can.c` | CAN1 1Mbps、过滤器配置 | ✅ 完成 |
| CAN 收发 + 中断 | `user/src/can_irq.c` | `CAN_Send`/`CAN_Start`、RX 回调解析 | ✅ 完成 |
| CAN 应用状态机 | `user/src/can_app.c` | 蜂鸣定次数、流水开关,回复 OK 帧 | 🟡 部分(阻塞式蜂鸣) |
| FreeRTOS 任务骨架 | `user/src/app_tasks.c` | 任务/队列定义与分支 | 🟡 骨架,未创建 |
| VOFA 指令 A5..5A | `user/src/uart_irq.c` | 主板解析指令 | ❌ 未做(还是旧协议) |
| 从板 100Hz float | 待开发 | 从板反馈 float | ❌ 未做 |
| 0x012 / 0x02010101 周期发送 | 待开发 | 主从周期报文 | ❌ 未做 |
| 流水灯状态指示 | `user/src/led_flow.c` | LED1/2 交替 | ✅ 完成 |
| 呼吸灯 PWM | `user/src/breath_led.c` | TIM3 占空比三角波 | ✅ 完成 |
| 蜂鸣器 | `user/src/buzzer.c` | 非阻塞定次 | ✅ 完成 |
| 旧按键状态机 | `user/src/app.c` | 按键控流水/呼吸(旧功能) | 🟡 与 RTOS 呼吸任务可能冲突,决定去留 |

## 八、开发分工建议

| 开发者 | 负责模块 | 主要工作 |
|---|---|---|
| **主板开发者** | `vofa_rx_task`、`can_tx_task`、`can_rx_task` | 解析 A5..5A 指令 → 下发 CAN;每 50ms 发 0x012;收从板 float 打波到 VOFA + log |
| **从板开发者** | `can_tx_task`、`can_rx_task`、`breath_task` | 每 10ms 发 100Hz float;收主板指令控呼吸灯;呼吸周期可调 |
| **共同** | `can_irq.c`、`can_app.c`、`can.c` 过滤器 | 定稿 CAN ID、蜂鸣定次数响应、确认过滤配置一致 |

**改 `protocol.h` 里的 `BOARD_MASTER` 即可在自己板子上开发对应一侧。**

## 九、当前注意事项 / 待办

1. **旧超级循环是死代码**:`osKernelStart()` 之后 `main.c` 里 `while(1)` 中的
   `app_run()` / `can_app_run()` / `UART_Send_Sine()` 在 RTOS 跑起来后**不会执行**。
   这些逻辑要迁移进 FreeRTOS 任务(app_tasks.c),而不是留在 while 里。
2. **`uart_irq.c` 接收未重启**:`HAL_UARTEx_RxEventCallback` 末尾重装接收的那行被注释掉了,
   需恢复 `HAL_UARTEx_ReceiveToIdle_DMA(...)`,否则只收到一帧就停了。
3. **VOFA 协议待重写**:`uart_irq.c` 目前还是旧的 `0xFF/0x01` 协议,要改成 `A5..5A`。
4. **CAN 过滤器待更新**:定稿 ID 后同步改 `can.c` 的 FilterBank 0 配置。
5. **`app_tasks_create()` 未接线**:在 `freertos.c` 的 `MX_FREERTOS_Init()` 中调用它来创建任务与队列。
6. **`freertos.c` include 不一致**:当前 `#include "cmsis_os.h"`(V1),但任务代码用 V2 API
   (`cmsis_os2.h`),建议统一为 V2 头文件。
