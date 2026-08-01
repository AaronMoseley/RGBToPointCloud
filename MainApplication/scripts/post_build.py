#!/usr/bin/env python3

import os
import shutil
import subprocess
import sys

# Project root (parent of the scripts directory)
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)

# ----------------------------------------------------------------------
# Configuration
# ----------------------------------------------------------------------

SHADER_COMPILATION_SCRIPT = os.path.join(PROJECT_ROOT, "shaders", "compile.py")

# Directories to copy after the shell script succeeds
DIRECTORIES_TO_COPY = [
    os.path.join(PROJECT_ROOT, "fonts"),
    os.path.join(PROJECT_ROOT, "models"),
    os.path.join(PROJECT_ROOT, "shaders"),
    os.path.join(PROJECT_ROOT, "textures"),
]

# Destination build directory
# Option 1: passed in from CLion (recommended)
if len(sys.argv) > 1:
    BUILD_DIR = os.path.abspath(sys.argv[1])
else:
    BUILD_DIR = os.path.join(PROJECT_ROOT, "build")

# ----------------------------------------------------------------------


def run_shell_script():
    print(f"Running {SHADER_COMPILATION_SCRIPT}")

    subprocess.run(
        ["python3", str(SHADER_COMPILATION_SCRIPT)],
        check=True,
    )


def copy_directory(source, destination_root):
    if not os.path.exists(source):
        print(f"Skipping missing directory: {source}")
        return

    destination = os.path.join(
        destination_root,
        os.path.basename(source)
    )

    if os.path.exists(destination):
        shutil.rmtree(destination)

    print(f"Copying {source} -> {destination}")

    shutil.copytree(
        source,
        destination
    )


def main():
    os.makedirs(BUILD_DIR, exist_ok=True)

    run_shell_script()

    for directory in DIRECTORIES_TO_COPY:
        copy_directory(directory, BUILD_DIR)

    print("Post-build step complete.")


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as e:
        print(f"Shell script failed with exit code {e.returncode}")
        sys.exit(e.returncode)
    except Exception as e:
        print(f"Post-build failed: {e}")
        sys.exit(1)