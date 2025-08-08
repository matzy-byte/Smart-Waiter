import tensorflow as tf
import numpy as np
import keras
import os
from GenerateDatasets import get_spectrogram


KERAS_MODEL_PATH = "wakeword/model/fully_trained.keras"
TFLITE_MODEL_PATH = "wakeword/model/q_lite_model.tflite"


def load_data():
    base_dir = "wakeword/data/representative_dataset"
    all_files = [f for f in os.listdir(base_dir) if f.endswith(".wav")]
    spectrograms = []
    for filename in all_files:
        filepath = os.path.join(base_dir, filename)
        audio_binary = tf.io.read_file(filepath)
        audio_tensor = tf.audio.decode_wav(audio_binary)
        audio = tf.reshape(audio_tensor.audio, [-1])
        spectrograms.append(get_spectrogram(audio))

    return spectrograms


def main():
    data = load_data()

    # for debug
    flat_values = []
    for spec in data:
        flat_values.extend(spec.flatten())

    flat_values = np.array(flat_values)
    print("Min value:", np.min(flat_values))
    print("Max value:", np.max(flat_values))

    def representative_data_gen():
        for i in range(min(100, len(data))):
            yield [np.expand_dims(data[i], axis=[0, -1])]

    model = keras.models.load_model(KERAS_MODEL_PATH)
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.representative_dataset = representative_data_gen
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.float32
    converter.inference_output_type = tf.float32
    tflite_model = converter.convert()

    with open(TFLITE_MODEL_PATH, "wb") as f:
        f.write(tflite_model)
    
    tf.lite.experimental.Analyzer.analyze(model_content=tflite_model)


if __name__ == "__main__":
    main()