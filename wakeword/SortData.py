import os
import shutil


ROOT_DIR = "speech_commands"
DESTINATION_DIR = "data/none"
SUBFOLDERS = ["backward", "bed", "bird", "cat", "dog", "down", "eight", "five", "follow", "forward", "four", "go",
            "happy", "house", "learn", "left", "marvin", "nine", "no", "off", "on", "one", "right", "seven",
            "sheila", 'six', "stop", "three", "tree", "two", "up", "visual", "wow", "yes", "zero"]


def main():
    os.makedirs(DESTINATION_DIR, exist_ok=True)

    for subfolder in SUBFOLDERS:
        subfolder_path = os.path.join(ROOT_DIR, subfolder)
        if not os.path.isdir(subfolder_path):
            print(f"Warning: {subfolder_path} does not exist or is not a directory.")
            continue

        for filename in os.listdir(subfolder_path):
            file_path = os.path.join(subfolder_path, filename)
            if not os.path.isfile(file_path):
                continue

            new_filename = f"{os.path.splitext(filename)[0]}_{subfolder}{os.path.splitext(filename)[1]}"
            destination_path = os.path.join(DESTINATION_DIR, new_filename)

            shutil.copy2(file_path, destination_path)

    background_source = os.path.join("speech_commands", "_background_noise_")
    background_destination = os.path.join("data", "_background")
    shutil.copytree(background_source, background_destination, dirs_exist_ok=True)


if __name__ == "__main__":
    main()