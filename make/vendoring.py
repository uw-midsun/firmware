#! /usr/bin/env python
"""Script for fetching vendored libraries."""

import subprocess


def fetch_and_unpack_release():
    ret_code = subprocess.run(
        [
            'curl -s https://api.github.com/repos/uw-midsun/codegen-tooling/releases/latest | grep "out*.zip" | cut -d : -f 2,3 | tr -d \" | wget -qi'
        ],
        shell=True)
    print(ret_code)


if __name__ == "__main__":
    fetch_and_unpack_release()
