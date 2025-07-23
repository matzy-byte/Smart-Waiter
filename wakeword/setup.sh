wget http://download.tensorflow.org/data/speech_commands_v0.02.tar.gz
mkdir speech_commands
tar -xvzf speech_commands_v0.02.tar.gz -C speech_commands

python3 SortData.py

rm -rf speech_commands
rm speech_commands_v0.02.tar.gz
