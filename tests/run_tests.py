import subprocess

RED_COLOR = '\033[31m'
GREEN_COLOR = '\033[32m'
END_COLOR = '\033[0m'

SYSTEM_COMPILER = "clang"
ANCL_COMPILER = "../build/src/ancl-cli"

ANCL_ASMFILE = "ancl.s"
ANCL_EXEFILE = "./ancl.out"
SYSTEM_EXEFILE = "./clang.out"


def print_ok(test_file):
    print(f"{GREEN_COLOR}[ OK ]{END_COLOR} {test_file}")

def print_ret_failed(test_file, ancl_retcode, system_retcode):
    print(f"{RED_COLOR}[ FAILED ]{END_COLOR} {test_file} ANCL={ancl_retcode}, SYSTEM={system_retcode}")

def print_output_failed(test_file, ancl_output, system_output):
    print(f"{RED_COLOR}[ FAILED ]{END_COLOR} {test_file}")
    print("===========================")
    print("ANCL OUTPUT:")
    print(ancl_output)
    print("SYSTEM OUTPUT:")
    print(system_output)
    print("===========================")


test_files = [
    "basic/answer.c",
    "call/variadic_hello.c", "call/long_answer.c",
    "loop/count.c", "loop/fib.c", "loop/nested.c",
]

use_optimizations = False

for test_file in test_files:
    subprocess.call([SYSTEM_COMPILER, test_file, f"-o{SYSTEM_EXEFILE}", "-I."])

    ancl_flags = [f"-f{test_file}", f"-n{ANCL_ASMFILE}"]
    if use_optimizations:
        ancl_flags.append("-O")

    subprocess.call([ANCL_COMPILER, *ancl_flags], stdout=subprocess.DEVNULL)
    subprocess.call([SYSTEM_COMPILER, ANCL_ASMFILE, f"-o{ANCL_EXEFILE}"])

    system_proc = subprocess.run(SYSTEM_EXEFILE, shell=True, stdout=subprocess.PIPE)
    ancl_proc = subprocess.run(ANCL_EXEFILE, shell=True, stdout=subprocess.PIPE)

    system_retcode = system_proc.returncode
    ancl_retcode = ancl_proc.returncode

    system_output = system_proc.stdout.decode()
    ancl_output = ancl_proc.stdout.decode()

    if ancl_retcode != system_retcode:
        print_ret_failed(test_file, ancl_retcode, system_retcode)
    elif ancl_output != system_output:
        print_output_failed(test_file, ancl_output, system_output)
    else:
        print_ok(test_file)
