# 0811_stm32f4 — 按键控制的流水灯 / 呼吸灯

//aigc

基于 **STM32F405RGT6** 的嵌入式课程设计:一颗按键控制四颗 LED 的流水灯、呼吸灯切换,并带蜂鸣器反馈。工程由 **STM32CubeMX + HAL 库** 生成框架,应用逻辑写在 `user/` 目录,采用**主循环 + 定时器中断**的非阻塞设计。

## 硬件平台

| 项目 | 内容 |
|---|---|
| 主控 | STM32F405RGT6(Cortex-M4F,LQFP64,1MB Flash / 192KB RAM) |
| 时钟 | 外部 25MHz 晶振(HSE)→ PLL 输出 **84MHz** 系统时钟 |
| 外设 | 4 × LED、1 × 按键(PC11)、1 × 蜂鸣器(PA8)、TIM1 / TIM2 / TIM3 |
| 工具链 | Keil MDK-ARM(µVision) |

### 引脚分配

| 引脚 | 功能 | 说明 |
|---|---|---|
| PA4 | LED_1 | 流水灯,GPIO 推挽输出(低有效下拉) |
| PA5 | LED_2 | 流水灯,GPIO 推挽输出 |
| PA6 | LED_3 | 呼吸灯,TIM3_CH1 PWM |
| PA7 | LED_4 | 呼吸灯,TIM3_CH2 PWM |
| PA8 | BEEP | 蜂鸣器,高电平响 |
| PC11 | INPUT_1 | 按键,EXTI 上升沿中断(按下为高电平) |

## 功能说明

一颗按键实现开关机与模式切换,每次状态切换蜂鸣器响一声(时长不同以区分状态):

| 操作 | 当前状态 | 结果 |
|---|---|---|
| 长按(≥ 800ms) | 关机 OFF | 开机 → 流水灯(ST_FLOW) |
| 长按(≥ 800ms) | 任意运行状态 | 关机 → 全灭(ST_OFF) |
| 短按 | 流水灯 | 切换到呼吸灯 |
| 短按 | 呼吸灯 | 切回流水灯 |
| 短按 | 关机 | 无动作 |

### 状态机(`user/src/app.c`)

```
                    ┌──── 长按 ────┐
                    ▼              │
  ST_OFF ──长按──> ST_FLOW ──短按──> ST_BREATH
   (全灭)            (LED1/2 流水)    (LED3/4 呼吸)
      ▲                               │
      └─────────── 长按 ──────────────┘
```

- **ST_OFF**:四灯全灭,初始状态,上电不响蜂鸣器
- **ST_FLOW**:LED1 / LED2 交替点亮模拟流水(`led_flow.c`)
- **ST_BREATH**:TIM3 PWM 占空比按三角波变化实现呼吸(`breath_led.c`)

### 模块划分(`user/`)

| 模块 | 职责 |
|---|---|
| `app.c` | 状态机与状态转移,主循环调度 |
| `button.c` | 按键消抖 + 短按/长按识别(EXTI 上升沿记录按下,主循环轮询检测松开) |
| `breath_led.c` | TIM3 PWM 呼吸灯驱动(三角波占空比) |
| `led_flow.c` | 流水灯驱动(TIM 时基) |
| `buzzer.c` | 蜂鸣器非阻塞驱动(定时关闭) |

## 软件架构

```
┌─────────────────────────────────────────────┐
│  user/  应用层:状态机 + 各外设驱动模块        │
├─────────────────────────────────────────────┤
│  Core/  CubeMX 生成:main.c、gpio.c、tim.c、 │
│         HAL 时基、中断向量、时钟配置          │
├─────────────────────────────────────────────┤
│  Drivers/  STM32F4xx HAL 库 + CMSIS 内核头   │
└─────────────────────────────────────────────┘
```

所有非阻塞延时基于 `HAL_GetTick()`(由 **TIM1** 提供 1ms 时基);`TIM2` 已配置 500Hz 更新中断但应用层暂未使用(可作扩展);`TIM3` 输出 1kHz PWM 驱动呼吸灯。

## 目录结构

```
0811_stm32f4/
├── 0811_stm32f4.ioc         # CubeMX 工程配置(重新生成用)
├── .mxproject               # CubeMX 元数据(勿删)
├── Core/                    # CubeMX 生成代码
│   ├── Inc/                 # main.h、gpio.h、tim.h、stm32f4xx_hal_conf.h 等
│   └── Src/                 # main.c、gpio.c、tim.c、stm32f4xx_it.c、
│                            #   system_stm32f4xx.c、HAL MSP/时基配置
├── Drivers/
│   ├── CMSIS/
│   │   ├── Core/Include     # Cortex-M4 内核头文件(core_cm4.h 等)
│   │   ├── Include          # CMSIS 编译器适配头文件
│   │   └── Device/ST/STM32F4xx/Include   # 仅保留 F405 芯片头文件
│   └── STM32F4xx_HAL_Driver/
│       ├── Inc/             # HAL 头文件(含 Legacy 兼容层)
│       └── Src/             # HAL 源文件(工程实际仅编译 15 个)
├── MDK-ARM/                 # Keil 工程
│   ├── 0811_stm32f4.uvprojx # 工程文件(双击打开)
│   ├── startup_stm32f405xx.s
│   └── ...
└── user/                    # 应用层代码(手写)
    ├── inc/                 # app.h、button.h、breath_led.h、buzzer.h、led_flow.h
    └── src/                 # 对应 .c 实现
```


