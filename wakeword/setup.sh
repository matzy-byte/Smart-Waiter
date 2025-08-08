wget http://download.tensorflow.org/data/speech_commands_v0.02.tar.gz
# here would be the import of the personal_juan_dataset, but i dont want to (cannot) make it publically available
mkdir speech_commands
tar -xvzf speech_commands_v0.02.tar.gz -C speech_commands
tar -xvzf personal_juan_dataset.tar.gz

python3 wakeword/util/SortData.py
python3 wakeword/util/AugmentWakeword.py
python3 wakeword/util/AugmentBackground.py

rm -rf speech_commands
rm -rf personal_juan_dataset
rm speech_commands_v0.02.tar.gz
rm personal_juan_dataset.tar.gz
