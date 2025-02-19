import os
import subprocess

cwd = os.getcwd()
tests = cwd + "/../lexical_analyzer/integration"
os.chdir(tests)

items = os.listdir()
types = set()
for item in items:
    a = item.split(".")
    types.add(a[0])
os.chdir(cwd)
tn = 0
cr = 0
for it in sorted(types):
    if ".DS_Store" in it or len(it) == 0:
        continue
    lang = it + ".lan"
    iN = it + ".in"
    out = it + ".out"
    # print(lang, iN, out)
    print(f"\033[33mTESTING {it}\033[0m")
    tn += 1
    subprocess.run(["cp", "../lexical_analyzer/integration/" + lang, "./"])
    subprocess.run(
        [
            "cp",
            "../lexical_analyzer/integration/" + iN,
            "../lexical_analyzer/analizator/" + iN,
        ]
    )
    subprocess.run(
        [
            "cp",
            "../lexical_analyzer/integration/" + out,
            "../lexical_analyzer/analizator/correct.txt",
        ]
    )
    nd = cwd + "/../lexical_analyzer/"
    os.chdir(nd)
    file = open("../testers/" + lang, "r")
    subprocess.run(["./generator"], stdin=file, text=True)
    file.close()
    nd2 = nd + "/analizator"
    os.chdir(nd2)
    file = open(iN, "r")
    file2 = open("izlaz.txt", "w")
    subprocess.run(
        ["./analizator"], stdin=file, stdout=file2, stderr=subprocess.DEVNULL
    )
    file.close()
    file2.close()
    # file1 = open("correct.txt")
    # file2 = open("izlaz.txt")
    # print(file1.read())
    # print(file2.read())
    # file1.close()
    # file2.close()
    out = subprocess.run(
        ["diff", "izlaz.txt", "correct.txt"], text=True, capture_output=True
    )
    if out.stdout == "":
        print(f"\033[32mTEST {it} PASSED\033[0m")
        cr += 1
    else:
        print(f"\033[31mTEST {it} FAILED\033[0m")
    subprocess.run(["rm", "izlaz.txt"])
    subprocess.run(["rm", "correct.txt"])
    subprocess.run(["rm", iN])
    subprocess.run(["rm", "automat.txt"])
    os.chdir(cwd)
    subprocess.run(["rm", lang])
    os.chdir(cwd)

print(f"\033[34m{cr}/{tn} TESTS PASSED\033[0m")
