# armfly_foc_demo 编码规范

## 1. 文件结构

### 1.1 头文件

```c
/*---------- includes ----------*/
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
```

### 1.2 源文件

```c
/*---------- includes ----------*/
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
/*---------- end of file ----------*/
```

### 1.3 文件头（必须）

```c
/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : filename.c
 * @Author       : lxf
 * @Date         : 2025-12-24 10:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-12-24 10:00:00
 * @Brief        : 模块功能说明
 */
```

### 1.4 驱动类文件扩展标签（可选）

| 标签             | 用途                          |
| ---------------- | ----------------------------- |
| `@hardware`      | 硬件接口说明                  |
| `@usage`         | 使用示例（`@code ... @endcode`） |
| `@features`      | 功能特性列表                  |
| `@warning`       | 重要警告                      |
| `@note`          | 使用注意事项                  |
| `@register_map`  | 寄存器映射表                  |
| `@performance`   | 性能对比表                    |
| `@transmit_flow` | 发送流程                      |
| `@receive_flow`  | 接收流程                      |

驱动类注释排版可参考现有驱动文件风格，保持整洁一致。

## 2. 命名规范

### 2.1 文件

- 小写下划线命名，例如 `app_motor.c`、`foc_profile.h`

### 2.2 函数

- 公开 API 使用模块前缀，例如 `device_open()`、`pmsm_foc_init()`
- 私有函数使用 `static`，命名保持模块内语义清晰，例如 `static void motor_update(void)`

### 2.3 变量

- 局部变量、全局变量使用小写下划线，例如 `timeout_ms`
- 宏常量使用全大写下划线，例如 `MAX_BUFFER_SIZE`
- 指针声明写法统一为 `uint8_t *ptr`

### 2.4 结构体

- 默认遵循 POSIX 风格，优先使用显式 `struct xxx`
- 只有明确作为句柄、抽象对象且需要简化用户接口时，才使用 `typedef`

```c
/* 推荐 */
struct torque_sensor {
    float value;
};

/* 不推荐 */
typedef struct {
    float value;
} torque_sensor_t;
```

## 3. 注释规范

### 3.1 基本要求

- 新增代码注释统一使用 UTF-8 中文
- 文件头中的 `@Brief` 使用中文，简要说明文件职责
- 保留项目现有分节风格，例如 `/*---------- includes ----------*/`
- 注释优先解释模块职责、边界、关键数据流和设计意图
- 不为显而易见的顺序语句逐行添加低信息注释

### 3.2 函数注释

对外接口、复杂内部函数、控制流程关键函数建议使用如下格式：

```c
/**
 * @brief  功能说明
 * @param  xxx: 参数说明
 * @return 0=成功, <0=失败
 */
```

### 3.3 结构体与代码块注释

- 结构体注释重点说明该对象负责什么，不逐字段重复显而易见的信息
- 对复杂控制流程、状态切换、坐标变换、保护逻辑，可在代码块前添加 1 到 2 行中文注释
- 注释保持简短直接，优先解释“为什么这样做”或“这一层负责什么”

## 4. 格式规范

- 缩进统一使用 4 个空格，不使用 Tab
- 函数的大括号另起一行
- `if`、`while`、`for` 的大括号遵循当前项目已有风格
- 函数与段落之间空 1 行，避免连续超过 1 个空行
- 新增文件默认使用 ASCII；注释和必要文档允许使用 UTF-8 中文

### 4.1 PowerShell 编码约定

- 使用 PowerShell 读取、搜索、打印包含中文的源码或文档时，默认不要依赖终端当前编码
- `Get-Content`、`Set-Content`、`Select-String` 等命令需要显式指定 `-Encoding utf8`
- 需要验证终端输出内容时，应先显式设置：

```powershell
[Console]::InputEncoding = [System.Text.UTF8Encoding]::new($false)
[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new($false)
$OutputEncoding = [Console]::OutputEncoding
```

- 向用户反馈“文件乱码”前，先用显式 UTF-8 方式重新读取验证，避免把终端编码问题误判为文件编码问题

## 5. FOC 模块分层规范

### 5.1 目录职责

- `components/fp-sdk/motion/foc/math/`：只放 FOC 数学相关代码，例如坐标变换、SVPWM、三角函数后端
- `components/fp-sdk/motion/foc/control/`：只放 FOC 专用控制器，例如电流环、速度环专用 PI、滤波、斜坡
- `components/fp-sdk/motion/foc/profile/`：放电机、功率板、传感器、控制参数配置结构
- `components/fp-sdk/motion/foc/`：放 FOC 主流程、状态机、运行时对象、硬件抽象接口

### 5.2 分层原则

- FOC 新增代码优先放在 `motion/foc` 域内，不为“潜在复用”提前塞进 `utilities`
- 只有当某段代码被多个非 FOC 模块稳定复用时，才考虑上提到通用层
- `board/` 负责具体硬件落地，不承载通用 FOC 算法实现
- `app/` 负责 demo 编排、业务流程与上层交互，不直接承载底层数学核

### 5.3 数值后端兼容性

- 第一版允许使用 `float32`
- 设计时必须保留后续切换定点实现的兼容边界
- 角度、标量、坐标系类型应统一封装，避免在业务层直接散落 `float` 细节
- 不允许在上层控制流程中直接写死 `sinf`、`cosf`、SVPWM 常数等实现细节

## 5.4 BSP 中间层规范

- `board/<board-name>/bsp/` 下只放中间层移植文件，不再额外拆 `motor/`、`key/`、`port/` 等子目录
- BSP 中间层文件命名统一使用 `bsp_xxx.c` 风格，例如 `bsp_foc.c`、`bsp_adc.c`、`bsp_pwm.c`、`bsp_key.c`
- 只有确实需要提供给 `app/` 或其他模块调用的接口，才保留对应头文件，例如 `bsp_foc.h`、`bsp_key.h`
- 纯内部移植实现默认不单独创建 `.h` 文件，避免为单个 `.c` 文件再做一层无意义暴露
- `app/` 层只能包含必要的 BSP 对外头文件，不直接依赖具体子目录路径约定
- BSP 中间层负责板级 GPIO、PWM、ADC、按键等硬件落地，不承载通用算法和业务编排

## 5.5 BSP 按键约定

- 按键驱动的通用逻辑放在 `components/fp-sdk/drivers/device/input/button.*`
- 板级按键移植放在 `board/<board-name>/bsp/bsp_key.c`
- `app_key.c` 负责按键初始化、周期扫描调度、事件读取和业务分发
- `bsp_key.c` 负责具体引脚初始化、按键有效电平判断、组合键判定和设备注册
- 在未明确需要前，不额外抽象 `pin` 驱动，按键移植层可以直接使用 HAL GPIO 读取

## 5.6 MDK 工程目录映射

- BSP 中间层源码在 MDK 工程中统一放到 `bsp` 分组
- `board/<board-name>/bsp/` 下的源文件在 `.uvprojx` 中直接使用 `..\bsp\xxx.c` 路径
- 不再为 BSP 中间层单独创建 `bsp/motor`、`bsp/key`、`port/key` 之类的 MDK 分组
- include path 优先指向 `..\bsp` 根目录，由 BSP 对外头文件控制可见边界

## 6. 驱动目录分层补充

### 6.1 drivers 目录职责

- `components/fp-sdk/drivers/core/`：设备框架和驱动框架基础设施
- `components/fp-sdk/drivers/device/bus/`：总线类基础驱动，例如 `i2c_bus`
- `components/fp-sdk/drivers/device/input/`：输入类设备驱动，例如 `button`
- `components/fp-sdk/drivers/device/storage/`：存储类设备驱动，例如 `at24cxx`
- `components/fp-sdk/drivers/device/sensor/`：传感器类设备驱动，例如 `paj7620`
- 其他仍属于设备层的驱动，继续放在 `components/fp-sdk/drivers/device/` 根目录

### 6.2 drivers 分层原则

- 总线基础能力和上层器件驱动要分层表达，不再混放为同一语义层
- 器件驱动可以依赖总线驱动，但应用层不直接承担总线细节
- 目录结构要能直接表达依赖方向：`core -> device/bus -> device/<category> -> app/board`

## 6. 协议模块独立化规范

### 6.1 适用场景

当应用层模块中包含特定硬件通信协议时，应将协议层独立到 `components/fp-sdk/packages/` 目录。

### 6.2 判断标准

满足以下任一条件时应独立协议层：

- 协议具有通用性，可用于其他项目
- 协议代码超过 50 行
- 包含复杂的数据解析或状态机逻辑

### 6.3 文件组织

```text
components/fp-sdk/packages/<模块名>/
├── <模块名>_sensor.h
├── <模块名>_sensor.c
└── readme.md
```

### 6.4 命名规范

- 模块名使用小写下划线，例如 `mgt_abs`、`sentong_torque`
- 结构体命名为 `struct <模块名>_sensor`
- 函数命名为 `<模块名>_sensor_<功能>`

### 6.5 API 设计原则

- 协议层只负责通信与数据解析，不包含业务逻辑
- 协议层不直接调用 `device_open`，由应用层传入设备句柄
- 协议层不使用静态全局变量，默认支持多实例
- 保持简单，不主动加入超时重试等复杂机制，除非明确需要

### 6.6 应用层封装

- 应用层负责设备管理、轮询调度、业务封装
- 应用层结构体可包含协议层对象和接收缓冲区
- 应用层调用协议层 API 完成通信

### 6.7 实施步骤

1. 在 `packages/` 下创建协议模块目录
2. 实现协议层头文件和源文件
3. 修改应用层，保留管理封装功能
4. 按需要补充编译器 include 路径
5. 更新 `AGENTS.md` 文档
