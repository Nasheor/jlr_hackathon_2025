import os

def change_cpp_to_c(directory):
    # List all files in the directory
    for filename in os.listdir(directory):
        # Check for common C++ extensions
        if filename.endswith('.cpp') or filename.endswith('.cxx') or filename.endswith('.cc'):
            base = os.path.splitext(filename)[0]  # filename without extension
            new_name = base + '.c'
            old_path = os.path.join(directory, filename)
            new_path = os.path.join(directory, new_name)
            print(f'Renaming {old_path} -> {new_path}')
            os.rename(old_path, new_path)

if __name__ == "__main__":
    target_dir = '../data/test'  # Current directory; change this if needed
    change_cpp_to_c(target_dir)
