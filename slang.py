#!/usr/bin/python3

import argparse
import os

# Action Entry Points --------------------------------------------------------------------------------------------------
def assemble(file):
    if not os.path.exists(file):
        raise Exception("File does not exist.")
    
    if not file.endswith(".slap"):
        raise Exception("File must be a .slap file.")

    abspath = os.path.abspath(file)

    # This is hack because we don't have a proper slap cli yet
    os.chdir("slap/")
    os.system("./auto.sh -s " + abspath)
    os.system("mv out.bin ../out.slim")
    

def execute(file):
    if not os.path.exists(file):
        raise Exception("File does not exist.")
    
    if not file.endswith(".slim"):
        raise Exception("File must be a .slim file.")

    abspath = os.path.abspath(file)

    # This is hack because we don't have a proper slim cli yet
    os.chdir("slim/")
    os.system("./auto.sh -e " + abspath)

# Argument Parser ------------------------------------------------------------------------------------------------------
parser = argparse.ArgumentParser(
    prog="slang",
    description="SL Command Line Interface",
    epilog="This script is used as a command line interface for all of the SL related programs and tools.",
)

parser.add_argument(
    "file",
    help="The file to be assembled or run.",
)

parser.add_argument(
    "-a",
    help="Assembles the specified file.",
    action="store_true",
)

parser.add_argument(
    "-r",
    help="Runs the specified file.",
    action="store_true"
)

# Parse Arguments and Dispatch Actions ---------------------------------------------------------------------------------
args = parser.parse_args()

if args.a is True:
    print("Assembling...")
    assemble(args.file)
    
if args.r is True:
    print("Running...")
    execute(args.file)
