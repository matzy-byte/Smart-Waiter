import os
import librosa
import soundfile as sf
from audiomentations import Compose, AddGaussianNoise, TimeStretch, PitchShift, Shift, Gain


SOURCE_DIR = "data/_background"
DEST_DIR = "data/noise"
SAMPLE_RATE = 16000
CHUNK_DURATION = 1.0
AUGMENT_TIMES = 25
AUGMENT = Compose([
    AddGaussianNoise(min_amplitude=0.001, max_amplitude=0.015, p=0.5),
    TimeStretch(min_rate=0.8, max_rate=1.25, p=0.3),
    PitchShift(min_semitones=-2, max_semitones=2, p=0.4),
    Shift(min_shift=-0.2, max_shift=0.2, rollover=True, p=0.5),
    Gain(min_gain_db=-6, max_gain_db=6, p=0.5),
])


def slice_and_augment(filepath, filename):
    audio, sr = librosa.load(filepath, sr=SAMPLE_RATE)
    chunk_samples = int(CHUNK_DURATION * SAMPLE_RATE)
    total_chunks = len(audio) // chunk_samples

    for i in range(total_chunks):
        chunk = audio[i*chunk_samples:(i+1)*chunk_samples]

        original_filename = f"{os.path.splitext(filename)[0]}_{i:03d}.wav"
        original_path = os.path.join(DEST_DIR, original_filename)
        sf.write(original_path, chunk, SAMPLE_RATE)

        for n in range(AUGMENT_TIMES):
            augmented_chunk = AUGMENT(samples=chunk, sample_rate=SAMPLE_RATE)
            aug_filename = f"{os.path.splitext(filename)[0]}_{i:03d}_aug{n+1}.wav"
            aug_path = os.path.join(DEST_DIR, aug_filename)
            sf.write(aug_path, augmented_chunk, SAMPLE_RATE)

    print(f"Saved {total_chunks} original + {total_chunks * AUGMENT_TIMES} augmented chunks for {filename}")


def main():
    os.makedirs(DEST_DIR, exist_ok=True)
    for filename in os.listdir(SOURCE_DIR):
        if not filename.endswith(".wav"):
            continue

        filepath = os.path.join(SOURCE_DIR, filename)
        print(f"Processing {filepath}...")
        slice_and_augment(filepath, filename)


if __name__ == "__main__":
    main()
