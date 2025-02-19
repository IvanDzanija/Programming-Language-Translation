import os
import subprocess


RESET = "\033[0m"
YELLOW = "\033[33m"
ORANGE = "\033[38;5;214m"
RED = "\033[31m"
BLUE = "\033[34m"
GREEN = "\033[32m"

gendir = "semantic_analyzer"

cwd = os.getcwd()
test = cwd + "/integration"
os.chdir(test)
items = os.listdir()
os.chdir(cwd)
ts, cr = 0, 0
for it in sorted(items):
    if ".DS_Store" in it:
        continue
    ts += 1
    print(BLUE + f"TESTING {it}" + RESET)
    subprocess.run(["cp", "integration/" + it + "/test.in", gendir + "/"])
    subprocess.run(["cp", "integration/" + it + "/test.out", gendir + "/"])
    nd = cwd + "/" + gendir
    os.chdir(nd)
    file = open("test.in", "r")
    file2 = open("frisc.out", "w")
    subprocess.run(["./analizator"], stdin=file, stderr=subprocess.DEVNULL)
    file.close()
    subprocess.run(
        ["node", "main.js", "a.frisc"], stdout=file2, stderr=subprocess.DEVNULL
    )
    out = subprocess.run(
        ["diff", "-w", "frisc.out", "test.out"], text=True, capture_output=True
    )
    file1 = open("frisc.out")
    file2 = open("test.out")
    if out.stdout == "":
        print(GREEN + f"TEST {it} PASSED" + RESET)
        cr += 1
    else:
        print(RED + f"TEST {it} FAILED" + RESET)
        # break
    subprocess.run(["rm", "test.out"])
    subprocess.run(["rm", "test.in"])
    os.chdir(cwd)

if cr / ts < 0.3:
    print(RED + f" {cr}/{ts} TESTS PASSED" + RESET)
elif cr / ts < 0.6:
    print(ORANGE + f" {cr}/{ts} TESTS PASSED" + RESET)
elif cr == ts:
    print(GREEN + f" {cr}/{ts} TESTS PASSED" + RESET)
else:
    print(YELLOW + f" {cr}/{ts} TESTS PASSED" + RESET)
