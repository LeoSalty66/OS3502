from pathlib import Path


def list_directory(path):
    """
    Return a sorted list of files/directories in the given path.
    Directories appear first, then files.
    """
    current_path = Path(path)

    entries = list(current_path.iterdir())

    entries.sort(key=lambda entry: (not entry.is_dir(), entry.name.lower()))

    return entries

def read_file(path):
    """
    Read and return the text contents of a file.
    """
    file_path = Path(path)

    with open(file_path, "r", encoding="utf-8") as file:
        return file.read()