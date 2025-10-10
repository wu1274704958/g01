#!/usr/bin/env python3
import os
import shutil
import sys
from pathlib import Path

def main():
    # 确定源目录
    if 'GSYNC_ROOT' in os.environ:
        gsync_root = Path(os.environ['GSYNC_ROOT'])
        print(f"使用环境变量 GSYNC_ROOT: {gsync_root}")
    elif len(sys.argv) > 1:
        gsync_root = Path(sys.argv[1])
        print(f"使用命令行参数 GSYNC_ROOT: {gsync_root}")
    else:
        print("错误：未设置 GSYNC_ROOT")
        print("用法:")
        print("  1. 设置环境变量: export GSYNC_ROOT=/path/to/gsync_root")
        print("  2. 或直接运行: ./copy_files.py /path/to/gsync_root")
        sys.exit(1)
    
    # 检查源目录是否存在
    if not gsync_root.is_dir():
        print(f"错误：源目录 {gsync_root} 不存在")
        sys.exit(1)
    
    # 目标目录是当前工作目录
    dest_dir = Path.cwd()
    print(f"从 {gsync_root} 复制文件到当前目录: {dest_dir}")
    
    # 复制 include 目录
    include_src = gsync_root / "include"
    if include_src.is_dir():
        print("复制 include/ 目录...")
        include_dest = dest_dir / "include"
        shutil.copytree(include_src, include_dest, dirs_exist_ok=True)
    else:
        print("警告：include/ 目录不存在")
    
    # 复制 output 目录
    output_src = gsync_root / "output"
    if output_src.is_dir():
        print("复制 output/ 目录...")
        output_dest = dest_dir / "output"
        shutil.copytree(output_src, output_dest, dirs_exist_ok=True)
    else:
        print("警告：output/ 目录不存在")
    
    # 复制所有 example* 目录中的 .h 文件（只查找 GSYNC_ROOT 下的直接子目录）
    print("复制 GSYNC_ROOT 下的 example* 目录中的 .h 文件...")
    
    # 查找所有以 "example" 开头的直接子目录
    example_dirs = [d for d in gsync_root.iterdir() 
                    if d.is_dir() and d.name.startswith("example")]
    
    if not example_dirs:
        print("警告：未找到以 'example' 开头的目录")
    
    # 复制每个 example 目录中的 .h 文件
    for example_dir in example_dirs:
        print(f"处理目录: {example_dir.name}")
        
        # 计算相对于源目录的相对路径
        rel_path = example_dir.relative_to(gsync_root)
        
        # 遍历所有 .h 文件（递归查找）
        for h_file in example_dir.rglob("*.h"):
            # 计算目标路径
            file_rel_path = h_file.relative_to(gsync_root)
            dest_file = dest_dir / file_rel_path
            
            # 创建目标目录并复制文件
            dest_file.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(h_file, dest_file)
            print(f"  已复制: {file_rel_path}")
    
    print("文件复制完成！")

if __name__ == "__main__":
    main()