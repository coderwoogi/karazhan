import difflib
import sys

def compare_files(file1, file2):
    try:
        with open(file1, 'r', encoding='utf-8', errors='ignore') as f1, \
             open(file2, 'r', encoding='utf-8', errors='ignore') as f2:
            lines1 = f1.readlines()
            lines2 = f2.readlines()
            
        diff = difflib.unified_diff(lines1, lines2, fromfile=file1, tofile=file2, n=0)
        
        diff_str = "".join(diff)
        if diff_str:
            print(diff_str)
        else:
            print("No differences found.")
            
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    compare_files("temp_player_head.cpp", "src/server/game/Entities/Player/Player.cpp")
