import os
import sys
import subprocess

# Get the script directory
script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.join(os.path.dirname(script_dir), "")

# Get the configuration parameter
configuration = "Debug"  # Default configuration
if len(sys.argv) > 1:
    configuration = sys.argv[1]

# Build the path to the executable
editor_exe_path = os.path.join(root_dir, "Build", configuration, "OvEditor.exe")

# Print launch message
print(f"Launching Overload Editor in {configuration} mode...")

try:
    # Launch the editor as a subprocess and don't wait for it to complete
    subprocess.Popen([editor_exe_path])
    print("Editor launched successfully.")
except Exception as e:
    print(f"Launching the editor failed: {e}")
    sys.exit(1)
    