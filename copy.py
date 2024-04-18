import os
import sys
import shutil


def copy_file(source, destination):
    if os.path.exists(destination):
        os.remove(destination)

    shutil.copyfile(source, destination)


if __name__ == "__main__":
    src = sys.argv[1]
    dst = sys.argv[2]

    copy_file(src, dst)