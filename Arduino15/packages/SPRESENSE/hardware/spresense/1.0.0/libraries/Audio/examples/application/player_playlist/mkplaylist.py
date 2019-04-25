#!/usr/bin/env python3
# -*- coding: utf-8 -*-
############################################################################
#
#   Copyright 2019 Sony Semiconductor Solutions Corporation
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name of Sony Semiconductor Solutions Corporation nor
#    the names of its contributors may be used to endorse or promote
#    products derived from this software without specific prior written
#    permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
############################################################################

# mkplaylist.py
#
# This is a simple tool to generate a playlist file for audio player.
# Because this tool uses ffmpeg-python, please install it in advance.
#
# $ sudo apt-get install ffmpeg
# $ pip install ffmpeg-python
#
# Example of usage:
#
# 1. Generate TRACK_DB.CSV from audio files in SD card
# $ python mkplaylist.py /mnt/your-sdcard-rootdir/AUDIO
#
# 2. Copy the generated TRACK_DB.CSV into SD card
# $ cp TRACK_DB.CSV /mnt/your-sdcard-rootdir/PLAYLIST
#
# Note:
#
# It does not support filename contains 2-byte character.
# If filename has 2byte character, it will be excluded the file from playlist.
#

from __future__ import unicode_literals, print_function
import ffmpeg
import sys
import os
import re

def is_ascii(str):
    if str:
        return max([ord(ch) for ch in str]) < 128
    return True

def audio_info(filename):
    try:
        probe = ffmpeg.probe(filename)
    except ffmpeg.Error as e:
        print(e.stderr, file=sys.stderr)
        sys.exit(1)

    streams = next((stream for stream in probe['streams'] if stream['codec_type'] == 'audio'), None)
    if streams is None:
        print('No audio stream found', file=sys.stderr)
        return
    #print(streams)

    format = probe['format']
    #print(format)

    if 'tags' in format:
        # audio metadata must be permitted only ascii character.
        # these metadata don't include a comma character ','.
        tags = format['tags']
        title  = tags.get('title','-').replace(',', '-')
        artist = tags.get('artist','-').replace(',', '-')
        album  = tags.get('album','-').replace(',', '-')
        if not is_ascii(title):  title  = '-'
        if not is_ascii(artist): artist = '-'
        if not is_ascii(album):  album  = '-'
    else:
        # if audio metadata are none, record '-'.
        title  = '-'
        artist = '-'
        album  = '-'

    format_name = format['format_name']
    if format_name == 'mp3':
        # mp3' bit_length set to a fixed value 16
        bit_length = 16
    elif format_name == 'wav':
        bit_length = int(streams['bits_per_sample'])
    else:
        print('Not supported format', file=sys.stderr)
        sys.exit(1)

    channels = int(streams['channels'])
    sample_rate = int(streams['sample_rate'])

    #str = '%s,%s,%s,%d,%d,%d,%s' % (title, artist, album, channels, bit_length, sample_rate, format_name)
    str = '%s,%s,%s,%d,%d,%d,%s' % (os.path.basename(filename), artist, album, channels, bit_length, sample_rate, format_name)
    print(str)

    # output file with append mode
    with open('TRACK_DB.CSV', mode='a') as f:
        print(str, file=f)

def search_audio_file(path):
    for root, dirs, files in os.walk(path):
        for file in files:
            base, ext = os.path.splitext(file)
            if ',' in file:
                # exclude file which name includes comma character
                print('---> skip with comma character [%s]' % file)
                continue
            if not is_ascii(file):
                # exclude file which name is non-ascii character
                print('---> skip with non-ascii character [%s]' % file)
                continue
            if '.mp3' in ext.lower() or '.wav' in ext.lower():
                # file extension must be .mp3 or .wav.
                audio_info(os.path.join(root, file))

if __name__ == '__main__':
    args = sys.argv

    if (len(args) < 2):
        print('usage: python %s dirname [dirname2] [dirname3] ...' % args[0])
        print('usage: python %s -f filename' % args[0])
        print('')
        print('Generate an audio playlist file named as TRACK_DB.CSV')
        print('')
        sys.exit(1)

    if (args[1] == '-f'):
        # examine only a file
        audio_info(args[2])
        sys.exit(0)

    for dir in args[1:]:
        # examine multiple directories recursively
        search_audio_file(dir)
