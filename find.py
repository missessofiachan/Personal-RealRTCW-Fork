import os

# Specific to ProjectWAR: Ignore C# compiler builds, IDE data, and binaries
IGNORE_DIRS = {
    ".git", "__pycache__", "node_modules", ".venv", "venv", "dist", "build",
    "bin", "obj", ".vs", ".idea", "logs"
}
IGNORE_EXTS = {".pyc", ".pyo", ".exe", ".dll", ".so", ".map", ".pdb", ".suo", ".user"}
OUTPUT_FILE = "codebase_map.md"

def generate_tree(dir_path, prefix=""):
    """
    Recursively builds a beautifully formatted tree map.
    Handles '├──' and '└──' correctly based on position.
    """
    lines = []
    
    try:
        # Filter out ignored directories and files
        items = sorted([
            item for item in os.listdir(dir_path)
            if item not in IGNORE_DIRS and os.path.splitext(item)[1] not in IGNORE_EXTS
        ])
    except PermissionError:
        return lines

    for i, item in enumerate(items):
        path = os.path.join(dir_path, item)
        is_last = (i == len(items) - 1)
        connector = "└── " if is_last else "├── "
        
        if os.path.isdir(path):
            lines.append(f"{prefix}{connector}{item}/")
            # Indent subsequent levels
            new_prefix = prefix + ("    " if is_last else "│   ")
            lines.extend(generate_tree(path, new_prefix))
        else:
            lines.append(f"{prefix}{connector}{item}")
            
    return lines

def main():
    root_dir = os.getcwd()
    print(f"Mapping files in: {root_dir}")
    
    # Get the visual structure
    tree = generate_tree(root_dir)
    
    # CRITICAL FIX: Changed from .append() to .write() to prevent runtime crashes
    with open(OUTPUT_FILE, "w", encoding="utf-8") as f:
        f.write("# Codebase File Map\n\n")
        f.write(f"**Root Directory:** `{os.path.basename(root_dir)}/`\n\n")
        f.write("## Directory Tree\n```text\n")
        f.write(f"{os.path.basename(root_dir)}/\n")
        f.write("\n".join(tree))
        f.write("\n```\n")
        
    print(f"Successfully mapped codebase to {OUTPUT_FILE}")

if __name__ == "__main__":
    main()
    