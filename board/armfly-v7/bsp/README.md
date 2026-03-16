# armfly-v7 BSP

该目录用于放置 `armfly-v7` 的板级外设适配模块。

- 推荐使用 `new module --type bsp --board armfly-v7 --name <module>` 继续生成 `bsp_<name>.c/.h`
- 仅保留强依赖当前板卡引脚、DMA、时钟或外设资源的适配代码
