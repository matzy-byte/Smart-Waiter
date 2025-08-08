import os
import shutil
import random


NONE_ROOT_DIR = "speech_commands"
NONE_DESTINATION_DIR = "wakeword/data/none"
NONE_SUBFOLDERS = ["backward", "bed", "bird", "cat", "dog", "down", "eight", "five", "follow", "forward", "four", "go",
              "happy", "house", "learn", "left", "marvin", "nine", "no", "off", "on", "one", "right", "seven",
              "sheila", "six", "stop", "three", "tree", "two", "up", "visual", "wow", "yes", "zero"]
NONE_TOTAL_TARGET_FILES = 50000
NONE_FILES_PER_FOLDER = NONE_TOTAL_TARGET_FILES // len(NONE_SUBFOLDERS)

JUAN_ROOT_DIR = "personal_juan_dataset"
JUAN_DESTINATION_DIR = "wakeword/data/juan"


def sort_speech_commands():
    os.makedirs(NONE_DESTINATION_DIR, exist_ok=True)

    for subfolder in NONE_SUBFOLDERS:
        src_dir = os.path.join(NONE_ROOT_DIR, subfolder)
        if not os.path.isdir(src_dir):
            print(f"Warning: {src_dir} does not exist.")
            continue

        all_files = [f for f in os.listdir(src_dir)
                     if os.path.isfile(os.path.join(src_dir, f)) and f.endswith(".wav")]

        if len(all_files) < NONE_FILES_PER_FOLDER:
            print(f"Warning: Not enough files in {subfolder} (found {len(all_files)}, need {NONE_FILES_PER_FOLDER})")
            selected_files = all_files
        else:
            selected_files = random.sample(all_files, NONE_FILES_PER_FOLDER)

        for filename in selected_files:
            src_path = os.path.join(src_dir, filename)
            name, ext = os.path.splitext(filename)
            new_filename = f"{name}_{subfolder}{ext}"
            dest_path = os.path.join(NONE_DESTINATION_DIR, new_filename)
            shutil.copy2(src_path, dest_path)

    background_src = os.path.join(NONE_ROOT_DIR, "_background_noise_")
    background_dest = os.path.join("wakeword/data", "_background")
    shutil.copytree(background_src, background_dest, dirs_exist_ok=True)


def sort_personal_juan():
    os.makedirs(JUAN_DESTINATION_DIR, exist_ok=True)

    src_dir = os.path.join(JUAN_ROOT_DIR, "juan")
    if not os.path.isdir(src_dir):
        print(f"Warning: {src_dir} does not exist. Exiting.")
        return

    all_files = [f for f in os.listdir(src_dir)
                    if os.path.isfile(os.path.join(src_dir, f)) and f.endswith(".wav")]

    for filename in all_files:
            src_path = os.path.join(src_dir, filename)
            dest_path = os.path.join(JUAN_DESTINATION_DIR, filename)
            shutil.copy2(src_path, dest_path)
    
    background_src = os.path.join(JUAN_ROOT_DIR, "_background")
    background_dest = os.path.join("wakeword/data", "_background")
    shutil.copytree(background_src, background_dest, dirs_exist_ok=True)


def main():
    sort_speech_commands()
    sort_personal_juan()


if __name__ == "__main__":
    main()
