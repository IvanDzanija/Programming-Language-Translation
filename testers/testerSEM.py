import os
import subprocess

RESET = "\033[0m"
YELLOW = "\033[33m"
ORANGE = "\033[38;5;214m"
RED = "\033[31m"
BLUE = "\033[34m"
GREEN = "\033[32m"

semdir = "./"

cwd = os.getcwd()
test = cwd + "/../semantic_analyzer/integration"
os.chdir(test)
items = os.listdir()
os.chdir(cwd)
ts, cr = 0, 0
for it in sorted(items):
    if ".DS_Store" in it:
        continue
    ts += 1
    print(BLUE + f"TESTING {it}" + RESET)
    subprocess.run(
        ["cp", "../semantic_analyzer/integration/" + it + "/test.in", semdir + "/"]
    )
    subprocess.run(
        ["cp", "../semantic_analyzer/integration/" + it + "/test.out", semdir + "/"]
    )
    nd = cwd + "/" + semdir
    os.chdir(nd)
    file = open("test.in", "r")
    file2 = open("izlaz.txt", "w")
    subprocess.run(
        ["../semantic_analyzer/analizator"],
        stdin=file,
        stdout=file2,
        stderr=subprocess.DEVNULL,
    )
    file.close()
    file2.close()
    # file1 = open("test.out")
    # file2 = open("izlaz.txt")
    # print(file1.read())
    # print(file2.read())
    # file1.close()
    # file2.close()
    out = subprocess.run(
        ["diff", "izlaz.txt", "test.out"], text=True, capture_output=True
    )
    if out.stdout == "":
        print(GREEN + f"TEST {it} PASSED" + RESET)
        cr += 1
    else:
        print(RED + f"TEST {it} FAILED" + RESET)
        break
    subprocess.run(["rm", "izlaz.txt"])
    subprocess.run(["rm", "test.out"])
    subprocess.run(["rm", "test.in"])
    subprocess.run(["rm", "a.frisc"])
    os.chdir(cwd)

if cr / ts < 0.3:
    print(RED + f" {cr}/{ts} TESTS PASSED" + RESET)
elif cr / ts < 0.6:
    print(ORANGE + f" {cr}/{ts} TESTS PASSED" + RESET)
elif cr == ts:
    print(GREEN + f" {cr}/{ts} TESTS PASSED" + RESET)
else:
    print(YELLOW + f" {cr}/{ts} TESTS PASSED" + RESET)
