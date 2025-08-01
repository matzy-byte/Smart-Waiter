#include <Global.h>
#include <Arduino.h>
#include <Microphone.h>
#include <Speaker.h>
#include <ModelInference.h>

void setup() {
    Serial.begin(115200);
    delay(1000);

    setupMicrophone();
    setupSpeaker();
    setupModel();

    Serial.println("System ready. Listening for \"juan\"...");
    delay(1000);
    
    readMicrophoneSamples(rawMicrophoneSamples, processedMicrophoneSamples);
    
    if (runInference(processedMicrophoneSamples)) {
        playMelody();
        delay(5000);
    }
}

void loop() {
    readMicrophoneSamples(rawMicrophoneSamples, processedMicrophoneSamples);
    
    if (runInference(processedMicrophoneSamples)) {
        playMelody();
        delay(5000);
    }
}