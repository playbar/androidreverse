#!/usr/bin/env python
# encoding=utf-8
__author__ = 'pfCao'

import subprocess
import os


def run_adb_cmd(cmd):
    try:
        args = ["adb"]
        args.extend(cmd)
        process = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        ret = process.communicate()
    except Exception, ex:
        print ex
        return None
    return ret


def main():
    module_name = "target"
    remote_path = "/data/local/tmp"

    run_adb_cmd(["push", os.path.join(os.getcwd(), "libtarget.so"), remote_path])

    os.chdir("..")
    project_path = os.getcwd()

    bin_path = os.path.join(project_path, "libs", "armeabi", module_name)
    run_adb_cmd(["push", bin_path, remote_path])
    run_adb_cmd(["shell", "chmod", "777", remote_path + "/" + module_name])



if __name__ == '__main__':
    main()
