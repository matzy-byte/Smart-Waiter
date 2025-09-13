import tensorflow as tf
import numpy as np
import keras
from GenerateDatasets import get_spectrogram


FILE = "juan.wav"
MODEL_FILE = "wakeword/model/fully_trained.keras"


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