# 辅助函数：构建前操作
def _run_before_operations(keil_path, mdk_project_path, output_path):
    """执行构建前的所有操作"""
    print("执行构建前操作...")
    # 这里可以添加构建前需要执行的操作，比如清理临时文件、准备环境等
    # 示例：可以调用现有的before_rebuild.py脚本
    print("[OK] 构建前操作完成")
    return 0

