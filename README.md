# 0811_stm32f4 — 结课作业:FreeRTOS + CAN 主从板通信系统

两人一组、合作完成的结课作业。**同一套工程代码**,通过 `BOARD_MASTER` 宏在主/从板之间切换,分别编译、分别烧录两块板。

> ✅ **当前状态:全部需求已实现。**
> 功能链路:主板收 VOFA 指令 → CAN 控制主/从板呼吸灯启停与快慢;从板 100Hz 反馈 float 到主板打波 + log;主从板接收 CANABLE 报文蜂鸣定次响应;双流水灯作工作状态指示。

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
| PA11 | CAN1_RX | CAN 控制器接收(需外接 CAN 收发器) |
| PA12 | CAN1_TX | CAN 控制器发送(需外接 CAN 收发器) |
| PC11 | INPUT_1 | 按键(EXTI,旧功能,未使用) |

### 外设参数

| 外设 | 参数 |
|---|---|
| CAN1 | **1 Mbps**(Prescaler=3, BS1=9TQ, BS2=4TQ),正常模式,自动离线恢复 |
| USART1 | 115200, 8N1,DMA + IDLE 变长接收 |
| TIM3 | 1kHz PWM(CH1/CH2 → 呼吸灯) |
| TIM1 | HAL 时基(1ms tick,供 `HAL_GetTick`) |
| TIM2 | 预留,当前未用 |


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
(见 `user/src/app_tasks.c`、`user/src/app_tasks.h`、`Core/Src/can.c` 过滤器)。


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
│       ├── freertos.c            # RTOS 初始化(调用 app_tasks_create 创建任务/队列)
│       ├── dma.c / tim.c / gpio.c / stm32f4xx_it.c / system_stm32f4xx.c
├── Middlewares/Third_Party/FreeRTOS/   # FreeRTOS 内核
├── Drivers/                      # HAL 库 + CMSIS
├── MDK-ARM/                      # Keil 工程
└── user/                         # ★ 应用层(主从板代码全部在这里)
    ├── inc/
    │   ├── protocol.h            # ★ 协议定义 + BOARD_MASTER 宏(唯一配置入口)
    │   ├── app_tasks.h / .c      # ★ FreeRTOS 任务 + 队列(主从分支)
    │   ├── can_irq.h / .c        # CAN 收发封装 + 接收中断回调
    │   ├── can_app.h / .c        # CAN 应用状态机(死代码,未被调用)
    │   ├── uart_irq.h / .c       # UART:VOFA A5..5A 指令解析 + justfloat 打波 + log
    │   ├── app.h / .c            # 旧按键状态机(死代码,未被调用)
    │   ├── breath_led.h / .c     # 呼吸灯 PWM 驱动
    │   ├── led_flow.h / .c       # 流水灯驱动
    │   ├── buzzer.h / .c         # 蜂鸣器(非阻塞)
    │   └── button.h / .c         # 按键(旧,未使用)
```

## 五、软件架构:FreeRTOS 任务与队列

`user/src/app_tasks.c` 定义全部任务,`app_tasks_create()` 在
`MX_FREERTOS_Init()` 中调用([freertos.c:98](Core/Src/freertos.c#L98)):

| 任务 | 职责 | 主从 |
|---|---|---|
| `heartbeat_task` | LED1/2 交替闪烁,板子工作状态指示(需求 1) | 双板 |
| `breath_task` | 按状态刷新呼吸灯占空比(状态机分发开/关) | 双板 |
| `buzzer_task` | 蜂鸣器定次数响应 | 双板 |
| `can_rx_task` | CAN 接收处理(蜂鸣命令/主板收 float/从板收控制) | 双板 |
| `can_tx_task` | CAN 周期发送:主板每 50ms 发 0x012;从板每 10ms 发 100Hz float | 双板 |
| `vofa_rx_task` | VOFA 指令解析(仅主板) | 主板 |

队列句柄(声明在 `app_tasks.h`,定义在 `app_tasks.c`):

| 队列 | 用途 | 主从 |
|---|---|---|
| `can_rx_queue` | CAN 接收消息投递给 can_rx_task | 双板 |
| `beep_queue` | 蜂鸣请求投递给 buzzer_task | 双板 |
| `vofa_cmd_queue` | VOFA 指令投递给 vofa_rx_task | 仅主板 |

启动流程(main.c):HAL 初始化 → 各 `MX_*_Init()` → `app_init` / `can_app_init` / `CAN_Start`
→ `osKernelInitialize()` → `MX_FREERTOS_Init()`(内部 `app_tasks_create()`)→ `osKernelStart()`。

## 六、通信协议

### 1. VOFA → 主板(串口 USART1,115200)

```
A5  XX  XX  XX  5A
└─  ┌┘  ┌┘   ┌┘  └─ 帧尾
    │   │    └────── 保留(示例中为周期第二字节,当前未使用)
    │   └─────────── 呼吸周期码(周期 = 周期码 × 100ms,限幅 100~2000ms)
    └─────────────── 开/关(0x01=开, 0x00=关)
```

主板收到后:更新本地呼吸灯状态(立即生效),经 CAN 0x012 转发给从板,并在串口 log 输出
`breath=xx period=xxxms`。

### 2. CAN 帧(当前代码中的 ID)

| 方向 | 帧类型 | ID | 含义 | 负载 |
|---|---|---|---|---|
| 主板 → 从板 | 标准帧 | `0x012` | 呼吸控制,每 50ms | data[0]=开关, data[1]=周期码 |
| 从板 → 主板 | 扩展帧 | `0x02010101` | 100Hz float 反馈,每 10ms | data[0..3]=float 占空比 |
| CANABLE → 双板 | 扩展帧 | `0x01020101` | 蜂鸣器定次数响应 | data[0]=次数 |

**CAN 过滤器**(can.c, FilterBank=0, IDLIST 模式)按板别放行:

| 板别 | 放行 ID |
|---|---|
| 主板(`BOARD_MASTER=1`) | `0x02010101`(从板反馈) + `0x01020101`(蜂鸣) |
| 从板(`BOARD_MASTER=0`) | `0x012`(主板控制) + `0x01020101`(蜂鸣) |

> 过滤器在硬件层丢弃未放行的 ID,因此 500Hz 噪声帧不会进入软件、不丢功能(需求 5)。
> 从板打波:100Hz float 经 CAN 到主板后,主板用 justfloat 协议
> (`00 00 80 7F` 帧尾)打到 VOFA 波形图。

## 七、模块职责与完成状态

| 模块 | 文件 | 职责 | 状态 |
|---|---|---|---|
| CAN 硬件 + 过滤器 | `Core/Src/can.c` | CAN1 1Mbps、主从过滤器 | ✅ 完成 |
| CAN 收发 + 中断 | `user/src/can_irq.c` | `CAN_Send`/`CAN_Start`、RX 回调 → 队列 | ✅ 完成 |
| FreeRTOS 任务 + 队列 | `user/src/app_tasks.c` | 6 个任务 + 3 队列,主从分支 | ✅ 完成 |
| VOFA 指令 A5..5A | `user/src/uart_irq.c` | 指令解析、justfloat 打波、log | ✅ 完成 |
| 主/从板周期报文 | `app_tasks.c` `can_tx_task` | 0x012 / 0x02010101 周期发送 | ✅ 完成 |
| 蜂鸣定次响应 | `app_tasks.c` `buzzer_task` + `can_rx_task` | CANABLE 命令 → 响 N 次 | ✅ 完成 |
| 流水灯状态指示 | `user/src/led_flow.c` | LED1/2 交替 | ✅ 完成 |
| 呼吸灯 PWM | `user/src/breath_led.c` | TIM3 占空比三角波,周期可调 | ✅ 完成 |
| 蜂鸣器 | `user/src/buzzer.c` | 非阻塞定次 | ✅ 完成 |
| CAN 应用状态机 | `user/src/can_app.c` | 旧状态机(阻塞式) | ⚠️ 死代码,未被调用 |
| 旧按键状态机 | `user/src/app.c` / `button.c` | 按键控流水/呼吸 | ⚠️ 死代码,未被调用 |


