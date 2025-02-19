import os
import subprocess


sadir = "./"

cwd = os.getcwd()
tests = cwd + "/../syntax_analyzer/integration"
os.chdir(tests)
items = os.listdir()
os.chdir(cwd)
ts, cr = 0, 0
for it in sorted(items):
    if ".DS_Store" in it:
        continue
    ts += 1
    print(f"\033[33mTESTING {it}\033[0m")
    subprocess.run(["cp", "../syntax_analyzer/integration/" + it + "/test.san", sadir])
    subprocess.run(
        [
            "cp",
            "../syntax_analyzer/integration/" + it + "/test.in",
            sadir + "../syntax_analyzer/analizator",
        ]
    )
    subprocess.run(
        [
            "cp",
            "../syntax_analyzer/integration/" + it + "/test.out",
            sadir + "../syntax_analyzer/analizator",
        ]
    )
    nd = cwd + "/" + "../syntax_analyzer/"
    os.chdir(nd)
    file = open("../testers/test.san", "r")
    subprocess.run(["./generator"], stdin=file, text=True)
    file.close()
    nd2 = nd + "/analizator"
    os.chdir(nd2)
    file = open("test.in", "r")
    file2 = open("izlaz.txt", "w")
    subprocess.run(
        ["./analizator"],
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
        print(f"\033[32mTEST {it} PASSED\033[0m")
        cr += 1
    else:
        print(f"\033[31mTEST {it} FAILED\033[0m")
    subprocess.run(["rm", "izlaz.txt"])
    subprocess.run(["rm", "table.txt"])
    subprocess.run(["rm", "test.out"])
    subprocess.run(["rm", "test.in"])
    os.chdir(cwd)
    subprocess.run(["rm", "test.san"])


print(f"\033[34m{cr}/{ts} TESTS PASSED\033[0m")
