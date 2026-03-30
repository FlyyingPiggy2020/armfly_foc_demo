import os
import sys

def generate_folder_tree(start_path, max_depth=3):
    """
    生成文件夹树结构
    :param start_path: 起始路径
    :param max_depth: 最大深度
    :return: 文件夹树字符串
    """
    tree = []
    start_path = os.path.abspath(start_path)
    # 获取起始路径的深度
    start_depth = start_path.count(os.sep)
    
    # 要排除的目录列表
    exclude_dirs = ['.git', '.github', '.vscode', '.mdk', '.cache', 'bin', 'build', 'Unity', 'Example', 'unitTest']
    
    for root, dirs, files in os.walk(start_path):
        # 计算当前深度
        current_depth = root.count(os.sep) - start_depth
        
        # 超过最大深度则跳过
        if current_depth > max_depth:
            continue
            
        # 跳过指定的目录
        dir_name = os.path.basename(root)
        if dir_name in exclude_dirs:
            # 不递归处理被排除的目录
            dirs[:] = []
            continue
            
        # 格式化当前文件夹名称
        indent = ' ' * 2 * current_depth
        folder_name = os.path.basename(root)
        
        # 根目录特殊处理
        if current_depth == 0:
            tree.append(folder_name + '/')
        else:
            tree.append(f'{indent}- {folder_name}/')
        
        # 从dirs中移除要排除的目录，避免递归处理
        dirs[:] = [d for d in dirs if d not in exclude_dirs]
    
    return '\n'.join(tree)

if __name__ == '__main__':
    # 使用当前目录作为起始路径
    folder_tree = generate_folder_tree('.', max_depth=3)
    print(folder_tree)
