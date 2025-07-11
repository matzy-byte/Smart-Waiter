import tensorflow as tf
import tensorflow_io as tfio
import numpy as np
from tqdm import tqdm
import matplotlib.pyplot as plt


def get_files(command):
    return tf.io.gfile.glob(f"data/{command}/*.wav")


def get_voice_length(audio):
    global NOISE_FLOOR
    position = tfio.audio.trim(audio, axis=0, epsilon=NOISE_FLOOR)
    return (position[1] - position[0]).numpy()


def is_voice_present(audio):
    global MINIMUM_VOICE_LENGTH
    voice_length = get_voice_length(audio)
    return voice_length >= MINIMUM_VOICE_LENGTH


def is_valid_file(file):
    global EXPECTED_SAMPLES
    audio_binary = tf.io.read_file(file)
    audio_tensor = tf.audio.decode_wav(audio_binary)
    if not ((audio_tensor.audio.shape[0] == EXPECTED_SAMPLES)):
        return False
    if not is_voice_present(audio_tensor.audio):
        return False
    return True


def get_spectrogram(audio):
    audio = tf.reshape(audio, [-1])
    stft = tf.signal.stft(
        audio,
        frame_length=320,
        frame_step=160,
        fft_length=512,
        window_fn=tf.signal.hann_window,
        pad_end=False
    )
    spectrogram = tf.abs(stft) ** 2.0
    spectrogram = tf.expand_dims(spectrogram, axis=0)
    spectrogram = tf.expand_dims(spectrogram, axis=-1)
    spectrogram = tf.nn.pool(
        input=spectrogram,
        window_shape=[1, 6],
        strides=[1, 6],
        pooling_type='AVG',
        padding='SAME')

    spectrogram = tf.squeeze(spectrogram, [0, -1])
    spectrogram = tf.math.log(spectrogram + 1e-6) / tf.math.log(tf.constant(10.0))

    return spectrogram


def process_file(file):
    global NOISE_FLOOR
    audio_binary = tf.io.read_file(file)
    audio_tensor = tf.audio.decode_wav(audio_binary)

    audio = audio_tensor.audio
    voice_start, voice_end = tfio.audio.trim(audio, axis=0, epsilon=NOISE_FLOOR)
    end_gap=len(audio) - voice_end
    random_offset = np.random.uniform(0, voice_start + end_gap)
    audio = np.roll(audio,-random_offset + end_gap)

    return get_spectrogram(audio)


def process_files(files, command, label, repeat=1):
    files = tf.repeat(files, repeat).numpy()
    return [(process_file(file), label) for file in tqdm(files, desc=f"{command} ({label})", leave=False)]


def process_command(command, repeat=1):
    global COMMANDS, TRAIN_SIZE, VALIDATION_SIZE, TEST_SIZE, TRAIN, VALIDATE, TEST
    label = COMMANDS.index(command)
    files = [file_name for file_name in tqdm(get_files(command), desc="Checking", leave=False) if is_valid_file(file_name)]
    np.random.shuffle(files)

    train_size = int(TRAIN_SIZE * len(files))
    validation_size = int(VALIDATION_SIZE * len(files))
    test_size = int(TEST_SIZE * len(files))
    data = [
        (TRAIN, files[:train_size]),
        (VALIDATE, files[train_size:train_size+validation_size]),
        (TEST, files[-test_size:])
    ]

    for split, files in data:
        split.extend(
            process_files(files, command, label, repeat)
        )

    print(len(TRAIN), len(VALIDATE), len(TEST))


def plot_images2(images_arr, imageWidth, imageHeight):
    fig, axes = plt.subplots(5, 5, figsize=(10, 20))
    axes = axes.flatten()
    for img, ax in zip(images_arr, axes):
        ax.imshow(np.reshape(img, (imageWidth, imageHeight)))
        ax.axis("off")
    plt.tight_layout()
    plt.savefig("output.png")
    

COMMANDS = ['backward', 'bed', 'bird', 'cat']
NOISE_FLOOR = 0.1
EXPECTED_SAMPLES = 16000
MINIMUM_VOICE_LENGTH = EXPECTED_SAMPLES / 4

TRAIN_SIZE = 0.8
VALIDATION_SIZE = 0.1
TEST_SIZE = 0.1

TRAIN = []
VALIDATE = []
TEST = []

for command in tqdm(COMMANDS, desc="Processing commands"):
    process_command(command)

X_train, Y_train = zip(*TRAIN)
X_validate, Y_validate = zip(*VALIDATE)
X_test, Y_test = zip(*TEST)

IMG_WIDTH=X_train[0].shape[0]
IMG_HEIGHT=X_train[0].shape[1]

X_cat = np.array(X_train)[np.array(Y_train) == 3]
Y_cat = np.array(Y_train)[np.array(Y_train) == 3]
plot_images2(X_cat[:20], IMG_WIDTH, IMG_HEIGHT)
print(Y_cat[:20])