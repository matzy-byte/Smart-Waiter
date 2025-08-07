#include <Global.h>
#include <Arduino.h>
#include <ESPConfigurator.h>
#include <SPIFFS.h>
#include <Ringbuffer.h>
#include <Microphone.h>
#include <Speaker.h>
#include <ModelInference.h>

int correct = 0;

void fillBuffer(int count) {
    for (int i = 0; i < count; i++) {
        readMicrophoneSamples(rawMicrophoneSamples, processedMicrophoneSamples);
        writeBuffer(processedMicrophoneSamples);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void inferenceTask(void *parameter) {
    while (true) {
        readMicrophoneSamples(rawMicrophoneSamples, processedMicrophoneSamples);
        writeBuffer(processedMicrophoneSamples);

        if (runInference(readBuffer())) {
            correct++;
        } else {
            correct = 0;
        }

        if (correct >= 2) {
            playMelody();
            correct = 0;
            fillBuffer(AUDIO_SLICES);
        }

        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

void setup() {
    Serial.begin(115200);
    configure_esp();
    delay(1000);

    setupRingbuffer();
    setupMicrophone();
    setupSpeaker();
    setupModel();

    Serial.println("System ready. Listening for \"juan\"...");
    delay(1000);

    //Fill buffer
    fillBuffer(AUDIO_SLICES - 1);
    xTaskCreatePinnedToCore(inferenceTask, "InferenceTask", 8192, NULL, 1, NULL, 1);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}