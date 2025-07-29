import os
from audiomentations import Compose, PitchShift, TimeStretch, AddBackgroundNoise, Gain, Shift
import soundfile as sf


WAKEWORD_DIR = "data/juan"
BACKGROUND_NOISE_DIR = "data/_background"
AUGMENTATIONS = 41
AUGMENT = Compose([
    AddBackgroundNoise(sounds_path=BACKGROUND_NOISE_DIR, min_snr_db=5, max_snr_db=20, noise_rms="relative", p=0.4),
    PitchShift(min_semitones=-2, max_semitones=2, p=0.5),
    TimeStretch(min_rate=0.8, max_rate=1.2, leave_length_unchanged=True, p=0.5),
    Gain(min_gain_db=-6, max_gain_db=6, p=0.5)
])
SHIFT_AUGMENTATIONS = 9
SHIFT_AUGMENT = Compose([
    Shift(min_shift=-0.5, max_shift=0.5, shift_unit="fraction", rollover=True, p=1.0)
])


def augment(augment, augmentations, ext):
    for filename in os.listdir(WAKEWORD_DIR):
        file_path = os.path.join(WAKEWORD_DIR, filename)
        samples, sr = sf.read(file_path)
        for i in range(augmentations):
            aug = augment(samples=samples, sample_rate=sr)
            out_path = os.path.join(WAKEWORD_DIR, f"{filename[:-4]}_{ext}{i}.wav")
            sf.write(out_path, aug, sr)


def main():
    augment(AUGMENT, AUGMENTATIONS, "aug")
    augment(SHIFT_AUGMENT, SHIFT_AUGMENTATIONS, "shift")


if __name__ == "__main__":
    main()