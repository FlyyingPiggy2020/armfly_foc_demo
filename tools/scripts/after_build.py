from .keil2CompileCommands import generate_compile_commands

# 辅助函数：构建后操作
def _run_after_operations(keil_path, mdk_project_path, output_path):
    """执行构建后的所有操作"""
    # 这里可以添加其他构建后需要执行的操作
    print("执行构建后操作...")
    # 生成compile_commands.json
    print("\nGenerating compile_commands.json...")
    success = generate_compile_commands(mdk_project_path, output_path)
    if success:
        print("[OK] compile_commands.json generated successfully!")
    else:
        print("[ERROR] Failed to generate compile_commands.json")
    print("[OK] 构建后操作完成")

    return 0
