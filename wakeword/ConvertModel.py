import tensorflow as tf
import keras


def main():
    model = keras.models.load_model("model/fully_trained.keras")
    converter = tf.lite.TFLiteConverter.from_keras_model(model)

    tflite_quant_model = converter.convert()

    with open("model/lite_model.tflite", "wb") as f:
        f.write(tflite_quant_model)


if __name__ == "__main__":
    main()