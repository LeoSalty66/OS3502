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

def create_file(path, content=""):
    """
    Create a new text file with optional starting content.
    """
    file_path = Path(path)

    with open(file_path, "x", encoding="utf-8") as file:
        file.write(content)


def write_file(path, content):
    """
    Overwrite an existing text file with new content.
    """
    file_path = Path(path)

    with open(file_path, "w", encoding="utf-8") as file:
        file.write(content)

def create_directory(path):
    """
    Create a new directory.
    """
    directory_path = Path(path)
    directory_path.mkdir()


def delete_path(path):
    """
    Delete a file or empty directory.
    """
    target_path = Path(path)

    if target_path.is_dir():
        target_path.rmdir()
    else:
        target_path.unlink()


def rename_path(old_path, new_path):
    """
    Rename a file or directory.
    """
    old_path = Path(old_path)
    new_path = Path(new_path)

    old_path.rename(new_path)

def get_metadata(path):
    """
    Return metadata about a file or directory.
    """
    file_path = Path(path)
    stats = file_path.stat()

    return {
        "size": stats.st_size,
        "modified": stats.st_mtime,
        "is_dir": file_path.is_dir(),
    }