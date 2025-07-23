import tensorflow as tf
import tensorflow_io as tfio
import numpy as np
import os
import glob
from tqdm import tqdm


LABELS = ["none", "juan"]
SAMPLES = 16000

TRAIN_SIZE = 0.8
VALIDATION_SIZE = 0.1
TEST_SIZE = 0.1


def get_files(label):
    return tf.io.gfile.glob(f"data/{label}/*.wav")


def is_valid_file(file):
    global SAMPLES

    audio_binary = tf.io.read_file(file)
    audio_tensor = tf.audio.decode_wav(audio_binary)
    if not ((audio_tensor.audio.shape[0] == SAMPLES)):
        return False
    return True


def get_spectrogram(audio):
    stft = tf.signal.stft(
        audio,
        frame_length=256,
        frame_step=128
    )
    spectrogram = tf.abs(stft)
    spectrogram = tf.expand_dims(spectrogram, axis=0)
    spectrogram = tf.expand_dims(spectrogram, axis=-1)

    spectrogram = tf.nn.avg_pool2d(
        spectrogram,
        ksize=[1, 1, 6, 1],
        strides=[1, 1, 6, 1],
        padding="SAME"
    )

    spectrogram = tf.squeeze(spectrogram, axis=[0, -1])
    spectrogram = np.log10(spectrogram + 1e-6)
    return spectrogram


def process_file(file, repeat = 1):
    audio_binary = tf.io.read_file(file)
    audio_tensor = tf.audio.decode_wav(audio_binary)
    audio = audio_tensor.audio

    if repeat > 1:
        voice_start, voice_end = tfio.audio.trim(audio, axis=0, epsilon=0.1)
        end_gap=len(audio) - voice_end
        random_offset = np.random.uniform(0, voice_start + end_gap)
        audio = np.roll(audio,-random_offset + end_gap)

    audio = tf.reshape(audio, [-1])
    return get_spectrogram(audio)


def process_label(label, repeat = 1):
    global LABELS

    index = LABELS.index(label)
    files = [file_name for file_name in tqdm(get_files(label),
                                             desc="Checking files",
                                             leave=False) if is_valid_file(file_name)]
    np.random.shuffle(files)
    train_size = int(TRAIN_SIZE * len(files))
    validation_size = int(VALIDATION_SIZE * len(files))
    splits = {
        "training": files[:train_size],
        "validation": files[train_size:train_size + validation_size],
        "test": files[train_size + validation_size:]
    }

    for split_name, split_files in splits.items():
        specs, labels = [], []
        for f in tqdm(tf.repeat(split_files, repeat).numpy(), desc=f"{split_name} ({label})", leave=False):
            spec = process_file(f, repeat)
            specs.append(spec)
            labels.append(index)
        np.savez_compressed(f"data/{split_name}_{label}.npz", X=specs, Y=labels)
        print(f"Saved {split_name}_{label}.npz with {len(specs)} samples")


def process_background(file, index):
    global SAMPLES, TRAIN_SIZE, VALIDATION_SIZE, TEST_SIZE

    audio_binary = tf.io.read_file(file)
    audio_tensor = tf.audio.decode_wav(audio_binary)
    audio = audio_tensor.audio
    audio = tf.reshape(audio, [-1])
    samples, labels = [], []

    for section_start in tqdm(range(0, len(audio) - SAMPLES, 4000), desc=file, leave=False):
        section_end = section_start + SAMPLES
        section = audio[section_start:section_end]
        spectrogram = get_spectrogram(section)
        samples.append(spectrogram)
        labels.append(index)

    np.random.shuffle(samples)
    train_size=int(TRAIN_SIZE*len(samples))
    validation_size=int(VALIDATION_SIZE*len(samples))
    splits = {
        "training": (samples[:train_size], labels[:train_size]),
        "validation": (samples[train_size:train_size + validation_size], labels[train_size:train_size + validation_size]),
        "test": (samples[train_size + validation_size:], labels[train_size + validation_size:])
    }

    for split_name, (_samples, _labels) in splits.items():
        np.savez_compressed(f"data/{split_name}_background.npz", X=_samples, Y=_labels)
        print(f"Saved {split_name}_background.npz with {len(_samples)} samples")


def load_and_combine_datasets(pattern):
    files = glob.glob(pattern)
    X_all, Y_all = [], []
    for file in files:
        with np.load(file, allow_pickle=True) as data:
            X_all.extend(data["X"])
            Y_all.extend(data["Y"])

    X_all = np.stack(X_all)
    Y_all = np.array(Y_all)
    return X_all, Y_all, files


def main():
    global LABELS

    for label in tqdm(LABELS, desc="Processing labels"):
        repeat = 50 if label == "juan" else 1
        process_label(label, repeat)
    
    for file in tqdm(get_files("_background"), desc="Processing background noise"):
        process_background(file, LABELS.index("none"))
    
    all_files_to_delete = []
    for split in ["training", "validation", "test"]:
        X, Y, files_used = load_and_combine_datasets(f"data/{split}_*.npz")
        np.savez_compressed(f"data/{split}_spectrogram.npz", X=X, Y=Y)
        all_files_to_delete.extend(files_used)
    for file in all_files_to_delete:
        try:
            os.remove(file)
        except Exception as e:
            print(f"Failed to delete {file}: {e}")


if __name__ == "__main__":
    main()