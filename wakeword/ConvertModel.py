import tensorflow as tf
import numpy as np
import keras


def main():
    training = np.load("data/training_spectrogram.npz")
    validation = np.load("data/validation_spectrogram.npz")
    test = np.load("data/test_spectrogram.npz")

    x_train = training["X"]
    x_val = validation["X"]
    x_test = test["X"]
    x_train = np.expand_dims(x_train, axis=-1)
    x_val = np.expand_dims(x_val, axis=-1)
    x_test = np.expand_dims(x_test, axis=-1)

    x_complete_train = np.concatenate((x_train, x_val, x_test))
    del x_train, x_val, x_test
    np.random.shuffle(x_complete_train)

    def representative_dataset_gen():
        for i in range(0, len(x_complete_train), 100):
            yield [x_complete_train[i:i + 100]]

    model = keras.models.load_model("model/fully_trained.keras")
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.representative_dataset = representative_dataset_gen
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.int8
    converter.inference_output_type = tf.int8

    tflite_quant_model = converter.convert()

    with open("model/lite_model.tflite", "wb") as f:
        f.write(tflite_quant_model)


if __name__ == "__main__":
    main()