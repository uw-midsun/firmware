"""Download and install script for github.com/uw-midsun/codegen-tooling

Fetches the latest out codegen-tooling-out.zip release and unpacks it for
the build system to pick up.
"""
#! /usr/bin/env python

import subprocess
import os
import re
import zipfile
import shutil
import socket

# Fetch the latest release from the official GitHub API
TAG = 'latest'
URL = 'https://api.github.com/repos/uw-midsun/codegen-tooling/releases/'
# This file name may need to be updated
FILE = 'codegen-tooling-out.zip'

# Hash the file to prevent reinstalling the same file.
HASH_FILE = 'hash.txt'

# For validating there is an internet connection
REMOTE_SERVER = "www.google.com"


def connected():
    """Connected checks for a internet connection

    Tries to connect to Google over an HTTP socket and will fail if there is no
    connection available.
    """
    try:
        host = socket.gethostbyname(REMOTE_SERVER)
        _ = socket.create_connection((host, 80), 2)
        return True
    except Exception:
        pass
    return False


def fetch_release(url, tag, output_file):
    """Fetches the release from the specified GitHub API URL

    Args:
        url: string that is the GitHub API URL
        tag: string of release tag
        output_file: string containing the release output file to fetch

    Returns:
        The return code object of the subprocess used to fetch the file.
    """
    ret_code = subprocess.run(
        [
            'curl -s {}{} | grep "{}" | cut -d : -f 2,3 |tr -d \\" | wget -qi -'.
            format(url, tag, output_file)
        ],
        shell=True,
        stdout=subprocess.PIPE)
    return ret_code


def check_hash(filename, hash_file):
    """Checks that the hash of the downloaded file against the previous hash.

    Args:
        filename: string containing the name of the downloaded file
        hash_file: string contrianing the name of the hash recording file

    Returns:
        A boolean value True if the hashes match of False if they don't
    """
    ret_code = subprocess.run(
        ['sha256sum {}'.format(filename)], shell=True, stdout=subprocess.PIPE)
    if hash_file in os.listdir(os.getcwd()):
        with open(hash_file, 'r+') as hashfp:
            if hashfp.readline() == ret_code.stdout.decode('utf-8'):
                return True
            else:
                hashfp.truncate(0)
                hashfp.write(ret_code.stdout.decode('utf-8'))
                hashfp.flush()
    else:
        with open(hash_file, 'w+') as hashfp:
            hashfp.write(ret_code.stdout.decode('utf-8'))
            hashfp.flush()
    return False


def clean_up(filename):
    """Cleans up by deleting the filename specified

    Also deletes files that may have been downloaded extra and not cleaned up
    properly.

    Args:
        filename: a string of the file to delete

    Returns:
        None
    """
    pattern = re.compile(filename + r'\.*[0-9]*')
    for fname in os.listdir(os.getcwd()):
        if re.search(pattern, fname):
            os.remove(os.path.join(os.getcwd(), fname))


def unpack(filename):
    """Unpacks the specified zip file

    Places the *.h files into inc/ and *.c into src/ and will create any missing
    directories. It will also delete any previous files in these folers.

    Args:
        filename: string containing the name of the zip file to unpack

    Returns:
        Boolean value indicating success or failure
    """
    _, zip_ext = os.path.splitext(os.path.join(os.getcwd(), filename))
    if zip_ext != '.zip':
        return False

    lsdir = os.listdir(os.getcwd())
    with zipfile.ZipFile(os.path.join(os.getcwd(), filename), 'r') as zip_ref:
        zip_ref.extractall(os.getcwd())
    lsdir_post = os.listdir(os.getcwd())
    newdir = list(set(lsdir_post) - set(lsdir))
    if len(newdir) != 1:
        return False

    if 'inc' in lsdir:
        clean_folder(os.path.join(os.getcwd(), 'inc'))
    else:
        os.mkdir(os.path.join(os.getcwd(), 'inc'))
    if 'src' in lsdir:
        clean_folder(os.path.join(os.getcwd(), 'src'))
    else:
        os.mkdir(os.path.join(os.getcwd(), 'src'))

    for file_path in os.listdir(os.path.join(os.getcwd(), newdir[0])):
        _, ext = os.path.splitext(file_path)
        if ext == '.h':
            os.rename(
                os.path.join(os.getcwd(), newdir[0], file_path),
                os.path.join(os.getcwd(), 'inc', os.path.basename(file_path)))
        if ext == '.c':
            os.rename(
                os.path.join(os.getcwd(), newdir[0], file_path),
                os.path.join(os.getcwd(), 'src', os.path.basename(file_path)))

    # Delete the unzipped dir
    shutil.rmtree(newdir[0])
    return True


def clean_folder(directory):
    """Cleans up a folder by removing all files in it.

    Arg:
        directory: file_path of the directory to delete

    Returns:
        Boolean value indicating success or failure
    """
    if not os.path.isdir(directory):
        return False

    for file_path in os.listdir(directory):
        if os.path.isfile(file_path):
            os.unlink(file_path)
        elif os.path.isdir(file_path):
            shutil.rmtree(file_path)

    return True


def main():
    """Main function"""
    if not connected():
        print('No internet connection, skipping hook, will use local copy.')
        return
    os.chdir(os.path.dirname(__file__))
    try:
        fetch_release(URL, TAG, FILE)
        if check_hash(FILE, HASH_FILE):
            clean_up(FILE)
            return
        if not unpack(FILE):
            raise IOError('Failed to unpack file.')
        clean_up(FILE)
    except Exception as err:
        print('Failed to fetch release {}{}: {}'.format(URL, TAG, err))


if __name__ == '__main__':
    main()
