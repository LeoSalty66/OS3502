import tkinter as tk
from tkinter import ttk


def main():
    root = tk.Tk()
    root.title("CS 3502 Project 3 - OwlTech File Manager")
    root.geometry("1000x650")

    # Top: current path
    path_label = ttk.Label(root, text="Current Path: ~/cs3502/project3", font=("Arial", 11))
    path_label.pack(fill="x", padx=10, pady=5)

    # Main horizontal layout
    main_frame = ttk.Frame(root)
    main_frame.pack(fill="both", expand=True, padx=10, pady=5)

    # Left side: file list
    left_frame = ttk.Frame(main_frame)
    left_frame.pack(side="left", fill="both", expand=True)

    file_list_label = ttk.Label(left_frame, text="Files and Directories")
    file_list_label.pack(anchor="w")

    file_list = tk.Listbox(left_frame)
    file_list.pack(fill="both", expand=True, pady=5)

    # Right side: file content editor
    right_frame = ttk.Frame(main_frame)
    right_frame.pack(side="right", fill="both", expand=True, padx=(10, 0))

    editor_label = ttk.Label(right_frame, text="File Contents")
    editor_label.pack(anchor="w")

    text_editor = tk.Text(right_frame, wrap="word")
    text_editor.pack(fill="both", expand=True, pady=5)

    # Buttons
    button_frame = ttk.Frame(root)
    button_frame.pack(fill="x", padx=10, pady=5)

    buttons = [
        "Refresh",
        "Open",
        "Create File",
        "Save",
        "Delete",
        "Rename",
        "Create Folder",
        "Go Up",
    ]

    for button_text in buttons:
        button = ttk.Button(button_frame, text=button_text)
        button.pack(side="left", padx=4)

    # Bottom status bar
    status_label = ttk.Label(root, text="Ready.", relief="sunken", anchor="w")
    status_label.pack(fill="x", side="bottom")

    root.mainloop()


if __name__ == "__main__":
    main()