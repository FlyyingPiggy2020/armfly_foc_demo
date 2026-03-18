import subprocess
import os
import sys
import time
import argparse

# ================= 配置区域 =================
# 默认的 Keil 路径 (如果没有通过参数传入，就用这个)
DEFAULT_KEIL_PATH = r"C:\Users\w1545\AppData\Local\Keil_v5\UV4\UV4.exe"
# ===========================================

def stream_log_file(process, log_file_path, encoding='gbk'):
    """
    实时读取日志文件并打印到屏幕，直到进程结束
    """
    # 1. 等待日志文件被创建 (Keil 启动可能需要几毫秒)
    wait_count = 0
    while not os.path.exists(log_file_path):
        time.sleep(0.1)
        wait_count += 1
        if process.poll() is not None: # 如果进程意外挂了
            return
        if wait_count > 50: # 等待超过5秒没日志，可能出错了
            print("[警告] 没检测到日志文件生成...")
            return

    # 2. 循环读取
    with open(log_file_path, 'r', encoding=encoding, errors='replace') as f:
        while True:
            # 尝试读取一行
            line = f.readline()
            
            if line:
                # print 自带换行，line 也带换行，所以 end=''
                print(line, end='')
                # 刷新缓冲区，确保能在终端马上看到
                sys.stdout.flush() 
            else:
                # 如果没读到内容，检查进程是否结束
                if process.poll() is not None:
                    break
                # 进程还在跑，但暂时没输出，睡一小会儿
                time.sleep(0.05)

def build_mdk_project(keil_path, project_path, log_dir, mode="build"):
    """
    核心编译函数
    :param mode: 'build' (增量), 'rebuild' (全编译), 'clean' (清理hex文件)
    """
    
    # --- 1. 路径检查 ---
    if not os.path.exists(keil_path):
        print(f"[Fatal] 找不到 Keil: {keil_path}")
        return -1
    if not os.path.exists(project_path):
        print(f"[Fatal] 找不到工程: {project_path}")
        return -1

    # --- 2. 准备日志 ---
    if not os.path.exists(log_dir):
        os.makedirs(log_dir, exist_ok=True)
    
    log_file = os.path.join(log_dir, "build_log.txt")
    if os.path.exists(log_file):
        try:
            os.remove(log_file)
        except OSError:
            print("[警告] 旧日志文件被占用，无法删除，尝试直接覆盖...")

    # --- 3. 构造命令 ---
    # Keil UV4 参数: -b (Build), -r (Rebuild), -c (Clean), -j0 (Hide GUI), -o (Output Log)
    flag_map = {
        "build": "-b",
        "rebuild": "-r",
        "clean": "-c"
    }
    
    cmd_flag = flag_map.get(mode, "-b")

    cmd = [keil_path, cmd_flag, project_path, "-j0", "-o", log_file]
    
    print("=" * 60)
    print(f"执行操作: {mode.upper()}")
    print(f"工程文件: {os.path.basename(project_path)}")
    print("=" * 60)

    # --- 4. 启动进程 (非阻塞) ---
    try:
        process = subprocess.Popen(cmd, shell=False)
        
        # --- 5. 实时显示进度 (Log Tailing) ---
        # 如果是 Clean 模式，Keil 跑得极快且可能不写日志，就不 tail 了
        if mode != "clean":
            try:
                stream_log_file(process, log_file)
            except KeyboardInterrupt:
                print("\n\n[用户终止] 检测到 Ctrl+C，正在杀掉 Keil 进程...")
                process.kill()
                return -1

        # 等待进程彻底结束拿到返回值
        process.wait()
        exit_code = process.returncode

    except Exception as e:
        print(f"[Error] 执行异常: {e}")
        return -1

    # --- 6. 结果判定 ---
    print("-" * 60)
    if exit_code == 0:
        print(f"[OK] {mode.capitalize()} Success (0 Errors)")
    elif exit_code == 1:
        print(f"[WARN] {mode.capitalize()} Completed with WARNINGS (Exit Code: 1)")
    else:
        print(f"[FAIL] {mode.capitalize()} FAILED (Exit Code: {exit_code})")
        # 如果失败了，通常建议用户去看看完整日志
        # print(f"Log saved to: {log_file}")
    
    return exit_code

# ================= CLI 入口 =================
if __name__ == "__main__":
    # 定义默认工程路径 (方便直接运行)
    DEFAULT_PROJECT = r"c:\Users\w1545\Desktop\FbProject\FBR2101_XJ\board\custom-stm32f103cbt6\.mdk\mdk.uvprojx"
    DEFAULT_LOG_DIR = r"c:\Users\w1545\Desktop\FbProject\FBR2101_XJ\build"

    # 使用 argparse 处理命令行参数
    parser = argparse.ArgumentParser(description="Python MDK Builder Tool")
    
    # 添加参数
    parser.add_argument('command', nargs='?', default='build', choices=['build', 'rebuild', 'clean'], 
                        help="执行命令: build, rebuild 或 clean (默认: build)")
    
    parser.add_argument('--project', '-p', default=DEFAULT_PROJECT, 
                        help="指定 .uvprojx 工程文件路径")
    
    parser.add_argument('--keil', '-k', default=DEFAULT_KEIL_PATH, 
                        help="指定 UV4.exe 路径")
    
    parser.add_argument('--log', '-l', default=DEFAULT_LOG_DIR, 
                        help="指定日志输出目录")

    args = parser.parse_args()

    # 执行主逻辑
    sys.exit(build_mdk_project(
        keil_path=args.keil,
        project_path=args.project,
        log_dir=args.log,
        mode=args.command
    ))
