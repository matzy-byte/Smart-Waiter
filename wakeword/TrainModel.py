import tensorflow as tf
import numpy as np
import keras


LABELS = ["none", "juan"]
BATCH_SIZE = 64
EPOCHS = 30


def shuffle_datasets(datasets):
    shuffled = []
    for (x, y) in datasets:
        indices = np.arange(len(x))
        np.random.shuffle(indices)
        shuffled.append((x[indices], y[indices]))
    return shuffled


def main():
    training = np.load("data/training_spectrogram.npz")
    validation = np.load("data/validation_spectrogram.npz")
    test = np.load("data/test_spectrogram.npz")

    x_train, y_train = training["X"], training["Y"]
    x_val, y_val = validation["X"], validation["Y"]
    x_test, y_test = test["X"], test["Y"]
    x_train = np.expand_dims(x_train, axis=-1)
    x_val = np.expand_dims(x_val, axis=-1)
    x_test = np.expand_dims(x_test, axis=-1)
    img_width, img_height = x_train[0].shape[0], x_train[0].shape[1]

    (x_train, y_train), (x_val, y_val), (x_test, y_test) = shuffle_datasets([(x_train, y_train),
                                                                             (x_val, y_val),
                                                                             (x_test, y_test)])
    train_dataset = tf.data.Dataset.from_tensor_slices((x_train, y_train)).batch(BATCH_SIZE).prefetch(tf.data.AUTOTUNE)
    validation_dataset = tf.data.Dataset.from_tensor_slices((x_val, y_val)).batch(BATCH_SIZE) \
                                                                           .prefetch(tf.data.AUTOTUNE)
    test_dataset = tf.data.Dataset.from_tensor_slices((x_test, y_test)).batch(BATCH_SIZE).prefetch(tf.data.AUTOTUNE)

    model = keras.Sequential([
        keras.layers.Conv2D(
            4,
            3,
            padding="same",
            activation="relu",
            kernel_regularizer=keras.regularizers.l2(0.001),
            name="conv_layer1",
            input_shape=((img_width, img_height, 1))
        ),
        keras.layers.MaxPooling2D(
            name="max_pooling1",
            pool_size=(2, 2)
        ),
        keras.layers.Conv2D(
            4,
            3,
            padding="same",
            activation="relu",
            kernel_regularizer=keras.regularizers.l2(0.001),
            name="conv_layer2"
        ),
        keras.layers.MaxPooling2D(
            name="max_pooling2",
            pool_size=(2, 2)
        ),
        keras.layers.Flatten(),
        keras.layers.Dropout(0.2),
        keras.layers.Dense(
            40,
            activation="relu",
            kernel_regularizer=keras.regularizers.l2(0.001),
            name="hidden_layer1"
        ),
        keras.layers.Dense(
            1,
            activation="sigmoid",
            kernel_regularizer=keras.regularizers.l2(0.001),
            name="output"
        )
    ])
    model.summary()
    model.compile(
        optimizer="adam",
        loss=keras.losses.BinaryCrossentropy(),
        metrics=["accuracy"]
    )
    model_checkpoint_callback = keras.callbacks.ModelCheckpoint(
        filepath="model/checkpoint.keras",
        monitor="val_accuracy",
        mode="max",
        save_best_only=True
    )

    model.predict(tf.zeros((1, 124, 22, 1)))
    model.fit(
        train_dataset,
        steps_per_epoch=len(x_train) // BATCH_SIZE,
        epochs=EPOCHS,
        validation_data=validation_dataset,
        callbacks=[model_checkpoint_callback]
    )

    model.evaluate(test_dataset)
    model.save("model/trained_model.keras")

    complete_train_x = np.concatenate((x_train, x_val, x_test))
    del x_train, x_val, x_test
    complete_train_y = np.concatenate((y_train, y_val, y_test))
    del y_train, y_val, y_test

    (complete_train_x, complete_train_y) = shuffle_datasets([(complete_train_x, complete_train_y)])
    complete_train_dataset = tf.data.Dataset.from_tensor_slices((complete_train_x, complete_train_y)) \
                                            .batch(BATCH_SIZE).prefetch(tf.data.AUTOTUNE)

    model2 = keras.models.load_model("model/trained_model.keras")
    model2.fit(
        complete_train_dataset,
        steps_per_epoch=len(complete_train_x) // BATCH_SIZE,
        epochs=5
    )
    model2.save("model/fully_trained.keras")


if __name__ == "__main__":
    main()
