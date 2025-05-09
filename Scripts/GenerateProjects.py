import os
import sys
import subprocess
import platform

# Get command line argument (default to "vs2022" if not provided)
version = sys.argv[1] if len(sys.argv) > 1 else "vs2022"

# Detect platform and set path separator
is_windows = platform.system() == "Windows"

if not is_windows:
    print("Only Windows is supported at the moment.")
    sys.exit(0)

# Build paths relative to script directory
root_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
premake_path = os.path.join(root_dir, "Dependencies", "premake5", "bin", "premake5")
premake_root_script = os.path.join(root_dir, "Sources", "Overload", "premake5.lua")

# Add .exe extension on Windows
if is_windows:
    premake_path += ".exe"

print(f"-- Generating projects with {version} --")
print(f"-- Premake path: {premake_path} --")
print(f"-- Premake root script: {premake_root_script} --")

# Build and execute the command
command = [premake_path, version, f"--file={premake_root_script}"]

try:
    result = subprocess.run(command, check=True)
    print("Projects generated successfully")
except subprocess.CalledProcessError as e:
    print(f"Failed to generate projects. Error code: {e.returncode}")
    sys.exit(1)