#!/usr/bin/env python3

import os
import subprocess
import sys

# Directory containing this script
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

def main():
    shader_files = []

    # Recursively search for .hlsl files
    for root, directories, files in os.walk(SCRIPT_DIR):
        for file in files:
            if file.lower().endswith(".hlsl"):
                shader_files.append(os.path.join(root, file))

    if not shader_files:
        print("No .hlsl files found.")
        return

    for shader_file in shader_files:
        # Filename with extension
        filename = os.path.basename(shader_file)

        # Filename without extension
        filename_no_extension = os.path.splitext(filename)[0]

        # Directory containing the shader
        shader_directory = os.path.dirname(shader_file)
        vertexCompileCommand = [
            "dxc", "-spirv", "-T", "vs_6_0", "-E", "VSMain",
            shader_file, "-Fo", os.path.join(shader_directory, filename_no_extension + "Vertex.spv")
        ]
        pixelCompileCommand = [
            "dxc", "-spirv", "-T", "ps_6_0", "-E", "PSMain",
            shader_file, "-Fo", os.path.join(shader_directory, filename_no_extension + "Pixel.spv")
        ]

        subprocess.run(vertexCompileCommand, check=True)
        subprocess.run(pixelCompileCommand, check=True)


if __name__ == "__main__":
    main()
