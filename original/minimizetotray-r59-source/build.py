#!/usr/bin/python

import zipfile
import glob
import os
import shutil

NAME = "minimizetotray"
PLATFORM = "WINNT-32"

MOZILLA_SOURCE = os.path.join(os.getcwd(), "../..")
LIBRARY = os.getcwd()

def main():
    xpi = zipfile.ZipFile(NAME + ".xpi", "w", zipfile.ZIP_DEFLATED)
    path = os.path.join(os.path.join(os.path.join(MOZILLA_SOURCE, "dist"), "xpi-stage"), NAME)
    os.chdir(path)
    files = glob.glob("*")
    for file_name in files:
        if(os.path.isfile(file_name)):
            xpi.write(file_name, file_name)
    file_names = (
        glob.glob(os.path.join("components", "*.js"))
        + glob.glob(os.path.join("components", "*.xpt"))
        + glob.glob(os.path.join("components", "*.manifest"))
        + glob.glob(os.path.join("chrome", "*.jar"))
        + glob.glob(os.path.join(os.path.join("defaults", "preferences"), "*.js")))

    file_names.extend(glob.glob(os.path.join("components", "*.dll"))
        + glob.glob(os.path.join("components", "*.so")) 
        + glob.glob(os.path.join("components", "*.mainifest")))
    for file_name in file_names:
        xpi.write(file_name, file_name)
    xpi.close()

if __name__ == "__main__":
    main()
