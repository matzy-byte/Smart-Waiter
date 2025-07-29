import tensorflow as tf
import numpy as np
import keras


FILE = "juan.wav"
MODEL_FILE = "model/trained_model.keras"


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


def load_and_process_file(filename):
    audio_bin = tf.io.read_file(filename)
    audio_ten = tf.audio.decode_wav(audio_bin)
    audio = audio_ten.audio
    audio = tf.reshape(audio, [-1])
    spec = get_spectrogram(audio)
    return np.expand_dims(spec, axis=[0, -1])


def run_inference(file_model, data):
    model = keras.models.load_model(file_model)
    pred = model.predict(data)
    return pred


def main():
    data = load_and_process_file(FILE)
    pred = run_inference(MODEL_FILE, data)
    print(pred)


if __name__ == "__main__":
    main()