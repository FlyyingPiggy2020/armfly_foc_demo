# 辅助嵌入式软件开发的脚本
用于命令行编译mdk工程，并且生成compile_commands.json文件。


# 工具使用

##  fbtools.py

项目构建和管理工具，提供以下命令：

### build 命令

编译MDK工程并自动生成compile_commands.json文件：

```bash
python fbtools.py build
```

### rebuild 命令

重新编译MDK工程并自动生成compile_commands.json文件：

```bash
python fbtools.py rebuild
```

#### before 命令

构建前执行的命令，在Keil MDK构建前调用：

```bash
python fbtools.py before
```

#### after 命令

构建后执行的命令，在Keil MDK构建后调用：

```bash
python fbtools.py after
```

**功能说明**：

- 执行编译后，若编译成功，会自动在`build/`目录下生成`compile_commands.json`文件
- `compile_commands.json`文件包含项目所有源文件的编译信息，可用于代码补全和静态分析工具

**Keil MDK中配置方法**：

1. 打开Keil MDK项目，进入Options for Target...

2. 切换到"User"选项卡

3. 在"Before Build/Rebuild"文本框中添加：

   ```
   python "c:\Users\w1545\Desktop\FbProject\FBR2101_XJ\fbtools.py" before
   ```

4. 在"After Build/Rebuild"文本框中添加：

   ```
   python "c:\Users\w1545\Desktop\FbProject\FBR2101_XJ\fbtools.py" after
   ```

5. 点击"OK"保存配置

这样，当Keil MDK执行构建操作时，会自动调用fbtools.py的before和after命令，实现构建前后的自定义操作。
