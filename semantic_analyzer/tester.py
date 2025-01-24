import os
import shutil
import subprocess


def run_tests():
    main_test_folder = "tests"
    main_folder = "."

    # Check if the main test folder exists
    if not os.path.exists(main_test_folder):
        print(f"Folder '{main_test_folder}' does not exist.")
        return

    # Iterate through all subdirectories in the main test folder
    for test_folder in sorted(os.listdir(main_test_folder)):
        test_path = os.path.join(main_test_folder, test_folder)
        if os.path.isdir(test_path):
            print(f"Running tests in folder: {test_folder}")

            # Define file paths
            test_in_path = os.path.join(test_path, "test.in")
            test_out_path = os.path.join(test_path, "test.out")
            a_frisc_path = os.path.join(main_folder, "a.frisc")
            gen_out_path = os.path.join(main_folder, "gen.out")

            # Ensure required files exist
            if not os.path.exists(test_in_path):
                print(f"Missing 'test.in' in {test_folder}. Skipping.")
                continue

            if not os.path.exists(test_out_path):
                print(f"Missing 'test.out' in {test_folder}. Skipping.")
                continue

            try:
                # Copy test.in and test.out to the main folder
                shutil.copy(test_in_path, os.path.join(main_folder, "test.in"))
                shutil.copy(test_out_path, os.path.join(main_folder, "test.out"))

                # Run ./analizator < test.in
                with open(os.path.join(main_folder, "test.in"), "r") as test_in:
                    subprocess.run(
                        ["./analizator"], stdin=test_in, cwd=main_folder, check=True
                    )

                # Run node main.js < a.frisc > gen.out
                with open(a_frisc_path, "r") as a_frisc, open(
                    gen_out_path, "w"
                ) as gen_out:
                    subprocess.run(
                        ["node", "../../main.js"],
                        stdin=a_frisc,
                        stdout=gen_out,
                        cwd=test_path,
                        check=True,
                    )

                # Compare gen.out with test.out
                with open(gen_out_path, "r") as gen_out, open(
                    os.path.join(main_folder, "test.out"), "r"
                ) as test_out:
                    gen_output = gen_out.read().strip()
                    expected_output = test_out.read().strip()

                    if gen_output == expected_output:
                        print(f"Test passed in folder: {test_folder}")
                    else:
                        print(f"Test failed in folder: {test_folder}")
                        print("Generated Output:")
                        print(gen_output)
                        print("Expected Output:")
                        print(expected_output)

            except subprocess.CalledProcessError as e:
                print(f"Error while running tests in folder {test_folder}: {e}")


if __name__ == "__main__":
    run_tests()
