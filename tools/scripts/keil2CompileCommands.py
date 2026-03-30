import xml.etree.ElementTree as ET
import json
import os
import sys
from typing import List, Dict, Any, Optional


def parse_keil_project(keil_project_file_path: str, output_dir: str) -> List[Dict[str, Any]]:
    """
    解析Keil项目文件（.uvprojx），提取编译宏定义、包含路径和源文件，
    并生成符合compile_commands.json格式的编译命令列表。
    
    :param keil_project_file_path: Keil项目文件的路径。
    :param output_dir: 输出compile_commands.json的目录。
    :return: 包含编译命令字典的列表，每个字典代表一个源文件的编译命令。
    """
    try:
        tree = ET.parse(keil_project_file_path)
        root = tree.getroot()
        
        # 提取目标信息
        target = root.find('.//Target')
        if target is None:
            print("Error: Target element not found in Keil project file.")
            return []
        
        # 提取编译选项
        target_option = target.find('.//TargetOption')
        if target_option is None:
            print("Error: TargetOption element not found in Keil project file.")
            return []
        
        # 宏定义
        cads = target_option.find('.//Cads')
        define: str = ''
        include_path: str = ''
        if cads is not None:
            define_element = cads.find('VariousControls/Define')
            if define_element is not None and define_element.text is not None:
                define = define_element.text

            include_path_element = cads.find('VariousControls/IncludePath')
            if include_path_element is not None and include_path_element.text is not None:
                include_path = include_path_element.text

        macros: List[str] = [f"-D{macro}" for macro in define.split(',') if macro]
        

        # 包含路径
        uvprojx_dir: str = os.path.dirname(os.path.abspath(keil_project_file_path))  # Keil 项目文件所在目录

        raw_includes: List[str] = [inc for inc in include_path.split(';') if inc]
        includes: List[str] = []
        for inc in raw_includes:
            # 将相对于 uvprojx 目录的包含路径转换为相对于 output_dir 的路径
            abs_inc_path: str = os.path.abspath(os.path.join(uvprojx_dir, inc))
            rel_inc_path: str = os.path.relpath(abs_inc_path, output_dir)
            # 确保路径分隔符是 '/' 以符合JSON格式
            rel_inc_path = rel_inc_path.replace("\\", "/")
            includes.append(f"-I{rel_inc_path}")

        # 提取源文件路径
        groups = target.findall('.//Group')
        files: List[str] = []
        for group in groups:
            group_files = group.findall('.//File')
            for file in group_files:
                file_type_element = file.find('FileType')
                file_path_element = file.find('FilePath')
                
                file_type: Optional[int] = None
                if file_type_element is not None and file_type_element.text is not None:
                    try:
                        file_type = int(file_type_element.text)
                    except ValueError:
                        # 跳过无效的文件类型
                        print(f"Warning: Invalid FileType value: {file_type_element.text}")
                        continue

                file_path: Optional[str] = None
                if file_path_element is not None and file_path_element.text is not None:
                    file_path = file_path_element.text
                
                # 只处理C/C++源文件和汇编文件
                if file_type in [1, 2] and file_path is not None:
                    if file_path.endswith('.c') or file_path.endswith('.s'):
                        files.append(file_path)
        
        # 构建compile_commands.json内容
        # 注意：这里使用默认的编译器名称，实际使用时会被clangd的--query-driver覆盖
        compiler: str = "armcc"
        compile_commands: List[Dict[str, Any]] = []
        
        for source_file_rel_to_proj in files:
            # 计算源文件的绝对路径
            abs_source_file_path: str = os.path.abspath(os.path.join(uvprojx_dir, source_file_rel_to_proj))

            # 计算相对于输出目录的相对路径
            relative_file_path: str = os.path.relpath(abs_source_file_path, output_dir)
            # 确保路径分隔符是 '/' 以符合JSON格式
            relative_file_path = relative_file_path.replace("\\", "/")
            
            command: Dict[str, Any] = {
                "arguments": [compiler] + includes + macros + ["-c", relative_file_path],
                "directory": output_dir,
                "file": relative_file_path
            }
            compile_commands.append(command)

        return compile_commands
        
    except ET.ParseError as e:
        print(f"Failed to parse XML: {e}")
        return []
    except Exception as e:
        print(f"An error occurred: {e}")
        return []


def generate_compile_commands(keil_project_file_path: str, output_dir: str) -> bool:
    """
    生成compile_commands.json文件
    
    :param keil_project_file_path: Keil项目文件路径
    :param output_dir: 输出目录
    :return: 成功返回True，失败返回False
    """
    try:
        # 确保输出目录存在
        os.makedirs(output_dir, exist_ok=True)
        
        # 解析Keil项目文件
        compile_commands = parse_keil_project(keil_project_file_path, output_dir)
        
        if not compile_commands:
            print("Failed to generate compile commands.")
            return False
        
        # 生成compile_commands.json文件路径
        output_file = os.path.join(output_dir, "compile_commands.json")
        
        # 写入JSON文件
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(compile_commands, f, indent=4, ensure_ascii=False)
        
        print(f"Successfully generated compile_commands.json to {output_file}")
        return True
        
    except Exception as e:
        print(f"Error generating compile_commands.json: {e}")
        return False


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python keil2CompileCommands.py <keil_project_file> <output_dir>")
        sys.exit(1)
    
    project_file = sys.argv[1]
    output_dir = sys.argv[2]
    
    if not os.path.exists(project_file):
        print(f"Error: Project file not found: {project_file}")
        sys.exit(1)
    
    generate_compile_commands(project_file, output_dir)
