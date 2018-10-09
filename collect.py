#!/usr/bin/env python3

# This file is essentially a stripped-down version of the precise-collect tool shipped with
# Mycroft-precise. Since it is provided under the APL, this is OK so long as I explain this
# and include their original license info

# To run this you will probably need to run something like
#    pip install pyaudio

# Copyright 2018 Mycroft AI Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
from select import select
from sys import stdin
from termios import tcsetattr, tcgetattr, TCSADRAIN

import pyaudio
import tty
import wave
from os.path import isfile

def key_pressed():
    return select([stdin], [], [], 0) == ([stdin], [], [])


def termios_wrapper(main):
    global orig_settings
    orig_settings = tcgetattr(stdin)
    try:
        hide_input()
        main()
    finally:
        tcsetattr(stdin, TCSADRAIN, orig_settings)


def show_input():
    tcsetattr(stdin, TCSADRAIN, orig_settings)


def hide_input():
    tty.setcbreak(stdin.fileno())


orig_settings = None

RECORD_KEY = ' '
EXIT_KEY_CODE = 27


def record_until(p, should_return):
    chunk_size = 1024
    stream = p.open(format=p.get_format_from_width(2), channels=1, rate=16000, input=True, frames_per_buffer=chunk_size)

    frames = []
    while not should_return():
        frames.append(stream.read(chunk_size))

    stream.stop_stream()
    stream.close()

    return b''.join(frames)


def save_audio(name, data):
    wf = wave.open(name, 'wb')
    wf.setnchannels(1)
    wf.setsampwidth(2)
    wf.setframerate(16000)
    wf.writeframes(data)
    wf.close()


def next_name(name):
    name += '.wav'
    pos, num_digits = None, None
    try:
        pos = name.index('#')
        num_digits = name.count('#')
    except ValueError:
        print("Name must contain at least one # to indicate where to put the number.")
        raise

    def get_name(i):
        nonlocal name, pos
        return name[:pos] + str(i).zfill(num_digits) + name[pos + num_digits:]

    i = 0
    while True:
        if not isfile(get_name(i)):
            break
        i += 1

    return get_name(i)


def wait_to_continue():
    while True:
        c = stdin.read(1)
        if c == RECORD_KEY:
            return True
        elif ord(c) == EXIT_KEY_CODE:
            return False


def record_until_key(p):
    def should_return():
        return key_pressed() and stdin.read(1) == RECORD_KEY

    return record_until(p, should_return)


def _main():
    show_input()
    file_label = input("File label (Ex. recording-##): ")
    hide_input()

    p = pyaudio.PyAudio()

    while True:
        print('Press space to record (esc to exit)...')

        if not wait_to_continue():
            break

        print('Recording...')
        d = record_until_key(p)
        name = next_name(file_label)
        save_audio(name, d)
        print('Saved as ' + name)

    p.terminate()


def main():
    termios_wrapper(_main)


if __name__ == '__main__':
    main()
