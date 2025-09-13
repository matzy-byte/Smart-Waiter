import tensorflow as tf
from tqdm import tqdm
import numpy as np

LABELS = ["none", "juan"]
SAMPLES = 16000

TRAIN = []
VALIDATE = []
TEST = []
TRAIN_SIZE = 0.8
VALIDATION_SIZE = 0.1
TEST_SIZE = 0.1


def get_files(label):
    return tf.io.gfile.glob(f"wakeword/data/{label}/*.wav")


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
    spectrogram = tf.math.log(spectrogram + 1e-6) / tf.math.log(tf.constant(10.0))
    spectrogram = (spectrogram - tf.reduce_min(spectrogram)) / (tf.reduce_max(spectrogram) - tf.reduce_min(spectrogram) + 1e-6)
    return spectrogram


def process_file(file):
    audio_binary = tf.io.read_file(file)
    audio_tensor = tf.audio.decode_wav(audio_binary)
    audio = audio_tensor.audio

    audio = tf.reshape(audio, [-1])
    return get_spectrogram(audio)


def process_label(label):
    global LABELS

    index = LABELS.index(label)
    files = [file_name for file_name in tqdm(get_files(label),
                                             desc="Checking files",
                                             leave=False) if is_valid_file(file_name)]
    np.random.shuffle(files)
    train_size = int(TRAIN_SIZE * len(files))
    validation_size = int(VALIDATION_SIZE * len(files))
    splits = [
        (TRAIN, files[:train_size]),
        (VALIDATE, files[train_size:train_size + validation_size]),
        (TEST, files[train_size + validation_size:])
    ]

    for (split, split_files) in splits:
        for f in tqdm(split_files, desc=f"{label}", leave=False):
            spec = process_file(f)
            split.append((spec, index))


def process_background():
    global LABELS
    
    bg_files = [file_name for file_name in tqdm(get_files("noise"),
                                             desc="Checking files",
                                             leave=False) if is_valid_file(file_name)]
    train_size = int(TRAIN_SIZE * len(bg_files))
    validation_size = int(VALIDATION_SIZE * len(bg_files))
    splits = [
        (TRAIN, bg_files[:train_size]),
        (VALIDATE, bg_files[train_size:train_size + validation_size]),
        (TEST, bg_files[train_size + validation_size:])
    ]

    for (split, split_files) in splits:
        for f in tqdm(split_files, desc=f"processing background", leave=False):
            spec = process_file(f)
            split.append((spec, LABELS.index("none")))


def main():
    for label in tqdm(LABELS, desc="Processing labels"):
        process_label(label)

    process_background()

    train_specs, train_labels = zip(*TRAIN)
    val_specs, val_labels = zip(*VALIDATE)
    test_specs, test_labels = zip(*TEST)

    np.savez_compressed("wakeword/data/training_spectrogram.npz", X=np.stack(train_specs), Y=np.array(train_labels))
    np.savez_compressed("wakeword/data/validation_spectrogram.npz", X=np.stack(val_specs), Y=np.array(val_labels))
    np.savez_compressed("wakeword/data/test_spectrogram.npz", X=np.stack(test_specs), Y=np.array(test_labels))


if __name__ == "__main__":
    main()