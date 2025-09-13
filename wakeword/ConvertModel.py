import tensorflow as tf
import numpy as np
import keras
import os
import random
from GenerateDatasets import get_spectrogram


KERAS_MODEL_PATH = "wakeword/model/fully_trained.keras"
TFLITE_MODEL_PATH = "wakeword/model/q_lite_model.tflite"


def load_data():
    base, classes = "wakeword/data", {"juan":0.4,"noise":0.5,"none":0.1}
    files = {c:[f for f in os.listdir(os.path.join(base,c)) if f.endswith(".wav")] for c in classes}
    total = 1000
    specs = []
    for c, r in classes.items():
        for f in random.sample(files[c], min(int(r*total), len(files[c]))):
            path = os.path.join(base, c, f)
            audio = tf.reshape(tf.audio.decode_wav(tf.io.read_file(path)).audio, [-1])
            spec = get_spectrogram(audio)
            if spec.shape[0] == 124:
                specs.append(spec)
                arr = spec.numpy().flatten()
                print(f"{c}/{f}: min:{np.min(arr)}, max:{np.max(arr)}")
    return specs


def main():
    data = load_data()

    def representative_data_gen():
        for i in range(min(1000, len(data))):
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