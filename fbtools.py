import sys
sys.dont_write_bytecode = True
import os
import click

from tools.scripts.build_mdk_project import build_mdk_project
from tools.scripts.after_build import _run_after_operations
from tools.scripts.before_build import _run_before_operations


# 你的 Keil 安装路径 (注意: 路径里如果反斜杠，建议前面加 r 或者用双反斜杠 \\)
my_keil = r"C:\Users\w1545\AppData\Local\Keil_v5\UV4\UV4.exe" 
my_project = os.path.dirname(os.path.abspath(__file__)) + r"\board\armfly-v7\.mdk\mdk.uvprojx"
log_path = os.path.dirname(os.path.abspath(__file__)) + r"\build"

def run_before_operations():
    return _run_before_operations(my_keil, my_project, log_path)

def run_after_operations():
    return _run_after_operations(my_keil, my_project, log_path)

@click.group()
def cli():
    """这是主入口"""
    pass

@cli.command()
def build():
    """这是编译命令"""

    # 1.编译MDK工程
    result = build_mdk_project(my_keil, my_project, log_path, "build")
    

@cli.command()
def rebuild():
    """这是重编译命令"""
    
    # 2. 清理并重新编译MDK工程
    result = build_mdk_project(my_keil, my_project, log_path, "rebuild")


@cli.command()
def before():
    """这是构建前执行的命令，在Keil MDK构建前调用"""
    return run_before_operations()

@cli.command()
def after():
    """这是构建后执行的命令，在Keil MDK构建后调用"""
    return run_after_operations()

if __name__ == '__main__':
    cli()