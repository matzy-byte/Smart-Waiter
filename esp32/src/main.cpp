#include <Global.h>
#include <Arduino.h>
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
    }
}

void setup() {
    Serial.begin(115200);
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed!");
        while (1);
    }
    delay(1000);

    setupRingbuffer();
    setupMicrophone();
    setupSpeaker();
    setupModel();

    Serial.println("System ready. Listening for \"juan\"...");
    delay(1000);

    //Fill buffer
    fillBuffer(AUDIO_SLICES - 1);
}

void loop() {
    readMicrophoneSamples(rawMicrophoneSamples, processedMicrophoneSamples);
    writeBuffer(processedMicrophoneSamples);
    
    if (runInference(readBuffer())) {
        correct++;
    } else {
        correct = 0;
    }

    if (correct >= 3) {
        playMelody();
        correct = 0;
        fillBuffer(AUDIO_SLICES);
    }
}