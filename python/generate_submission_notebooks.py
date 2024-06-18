import nbformat as nbf
import os
from dotenv import load_dotenv
from pathlib import Path

load_dotenv()
PROJECT_DIRECTORY = Path(os.environ["PROJECT_DIRECTORY"])
OUTPUT_DIRECTORY = Path(os.environ["OUTPUT_DIRECTORY"])

def remove_lines_with_prefix(lines: list[str], prefixes: list[str]) -> list[str]:
    return [line for line in lines if all(not line.startswith(prefix) for prefix in prefixes)]

def add_code_cell(nb: nbf.NotebookNode, code: str) -> None:
    nb["cells"].append(nbf.v4.new_code_cell(code))

def save_notebook(nb: nbf.NotebookNode, name: str) -> Path:
    file = OUTPUT_DIRECTORY / f"{name}.ipynb"
    print(f"Writing {name} notebook to {file}")

    with file.open("w+", encoding="utf-8") as f:
        nbf.write(nb, f)

def generate_online_notebook() -> None:
    nb = nbf.v4.new_notebook()

    conanfile = (PROJECT_DIRECTORY / "conanfile.txt").read_text(encoding="utf-8").strip().splitlines()
    conanfile = remove_lines_with_prefix(conanfile, [
        "duckdb/",
        "gtest/",
        "nlohmann_json/",
    ])

    add_code_cell(nb, "\n".join(["%%writefile conanfile.txt"] + conanfile))
    add_code_cell(nb, "!rm -f /opt/conda/lib/libcurl*")
    add_code_cell(nb, "!pip install conan")
    add_code_cell(nb, "!mkdir -p /root/.conan2/profiles")
    add_code_cell(nb, """
%%writefile /root/.conan2/profiles/default
[settings]
arch=x86_64
build_type=Release
compiler=gcc
compiler.cppstd=gnu17
compiler.libcxx=libstdc++11
compiler.version=9
os=Linux
                      """.strip())
    add_code_cell(nb, "!conan install . --build=missing")
    add_code_cell(nb, "!cp -r /root/.conan2 .conan2")
    add_code_cell(nb, "!rm -f conanfile.txt")

    save_notebook(nb, "online")

def generate_offline_notebook() -> None:
    nb = nbf.v4.new_notebook()

    cmakelists = (PROJECT_DIRECTORY / "CMakeLists.txt").read_text(encoding="utf-8").strip().splitlines()
    cmakelists = cmakelists[:cmakelists.index("endforeach ()") + 1]
    cmakelists = remove_lines_with_prefix(cmakelists, [
        "find_package(duckdb ",
        "find_package(GTest ",
        "find_package(nlohmann_json ",
    ])

    source_files = [f for f in (PROJECT_DIRECTORY / "src" / "uspto").iterdir() if f.is_file()]
    source_files += [f for f in (PROJECT_DIRECTORY / "src" / "uspto" / "whoosh").iterdir() if f.is_file()]
    source_files += [PROJECT_DIRECTORY / "src" / "uspto" / "tools" / "run-submission.cpp"]
    source_files = sorted(source_files)

    add_code_cell(nb, "\n".join(["%%writefile CMakeLists.txt"] + cmakelists))
    add_code_cell(nb, "!mkdir -p src/uspto/tools")
    add_code_cell(nb, "!mkdir -p src/uspto/whoosh")

    for file in source_files:
        add_code_cell(nb, f"%%writefile {file.relative_to(PROJECT_DIRECTORY)}\n" + file.read_text(encoding="utf-8").strip())

    add_code_cell(nb, "!rm -f /opt/conda/lib/libcurl*")
    add_code_cell(nb, "!cp -r /kaggle/input/uspto-explainable-ai-ensemble-dependencies/.conan2 /root/.conan2")
    add_code_cell(nb, "!cmake . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/kaggle/input/uspto-explainable-ai-ensemble-dependencies/build/Release/generators/conan_toolchain.cmake")
    add_code_cell(nb, "!make -j")
    add_code_cell(nb, "!./run-submission")

    save_notebook(nb, "offline")

generate_online_notebook()
generate_offline_notebook()
