import tkinter as tk
from tkinter import ttk, messagebox
from pathlib import Path

import file_ops


class FileManagerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("CS 3502 Project 3 - OwlTech File Manager")
        self.root.geometry("1000x650")

        self.current_path = Path.cwd()
        self.current_file = None
        self.entries = []

        self.build_gui()
        self.refresh_file_list()

    def build_gui(self):
        self.path_label = ttk.Label(
            self.root,
            text=f"Current Path: {self.current_path}",
            font=("Arial", 11),
        )
        self.path_label.pack(fill="x", padx=10, pady=5)

        main_frame = ttk.Frame(self.root)
        main_frame.pack(fill="both", expand=True, padx=10, pady=5)

        left_frame = ttk.Frame(main_frame)
        left_frame.pack(side="left", fill="both", expand=True)

        file_list_label = ttk.Label(left_frame, text="Files and Directories")
        file_list_label.pack(anchor="w")

        self.file_list = tk.Listbox(left_frame)
        self.file_list.pack(fill="both", expand=True, pady=5)
        self.file_list.bind("<Double-Button-1>", lambda event: self.open_selected_item())

        right_frame = ttk.Frame(main_frame)
        right_frame.pack(side="right", fill="both", expand=True, padx=(10, 0))

        editor_label = ttk.Label(right_frame, text="File Contents")
        editor_label.pack(anchor="w")

        self.text_editor = tk.Text(right_frame, wrap="word")
        self.text_editor.pack(fill="both", expand=True, pady=5)

        button_frame = ttk.Frame(self.root)
        button_frame.pack(fill="x", padx=10, pady=5)

        ttk.Button(button_frame, text="Refresh", command=self.refresh_file_list).pack(side="left", padx=4)
        ttk.Button(button_frame, text="Open", command=self.open_selected_item).pack(side="left", padx=4)
        ttk.Button(button_frame, text="Create File").pack(side="left", padx=4)
        ttk.Button(button_frame, text="Save").pack(side="left", padx=4)
        ttk.Button(button_frame, text="Delete").pack(side="left", padx=4)
        ttk.Button(button_frame, text="Rename").pack(side="left", padx=4)
        ttk.Button(button_frame, text="Create Folder").pack(side="left", padx=4)
        ttk.Button(button_frame, text="Go Up", command=self.go_up_directory).pack(side="left", padx=4)

        self.status_label = ttk.Label(self.root, text="Ready.", relief="sunken", anchor="w")
        self.status_label.pack(fill="x", side="bottom")

    def refresh_file_list(self):
        self.file_list.delete(0, tk.END)

        try:
            self.entries = file_ops.list_directory(self.current_path)

            for entry in self.entries:
                prefix = "[DIR] " if entry.is_dir() else "[FILE] "
                self.file_list.insert(tk.END, prefix + entry.name)

            self.path_label.config(text=f"Current Path: {self.current_path}")
            self.status_label.config(text=f"Loaded {len(self.entries)} item(s).")

        except PermissionError:
            self.status_label.config(text="Permission denied: cannot read this directory.")
        except FileNotFoundError:
            self.status_label.config(text="Directory not found.")
        except Exception as error:
            self.status_label.config(text=f"Error loading directory: {error}")

    def get_selected_entry(self):
        selected_indices = self.file_list.curselection()

        if not selected_indices:
            messagebox.showwarning("No Selection", "Please select a file or folder first.")
            return None

        selected_index = selected_indices[0]
        return self.entries[selected_index]

    def open_selected_item(self):
        selected_entry = self.get_selected_entry()

        if selected_entry is None:
            return

        if selected_entry.is_dir():
            self.current_path = selected_entry
            self.current_file = None
            self.text_editor.delete("1.0", tk.END)
            self.refresh_file_list()
            self.status_label.config(text=f"Opened directory: {selected_entry.name}")
        else:
            try:
                content = file_ops.read_file(selected_entry)

                self.current_file = selected_entry
                self.text_editor.delete("1.0", tk.END)
                self.text_editor.insert(tk.END, content)
                self.status_label.config(text=f"Opened file: {selected_entry.name}")

            except UnicodeDecodeError:
                messagebox.showerror(
                    "Cannot Read File",
                    "This file does not appear to be a readable text file.",
                )
                self.status_label.config(text="Error: attempted to open non-text file.")
            except PermissionError:
                messagebox.showerror(
                    "Permission Denied",
                    "You do not have permission to read this file.",
                )
                self.status_label.config(text="Permission denied: cannot read file.")
            except FileNotFoundError:
                messagebox.showerror(
                    "File Not Found",
                    "This file no longer exists.",
                )
                self.refresh_file_list()
            except Exception as error:
                messagebox.showerror("Error", f"Could not open file:\n{error}")
                self.status_label.config(text=f"Error opening file: {error}")

    def go_up_directory(self):
        parent_path = self.current_path.parent

        if parent_path == self.current_path:
            self.status_label.config(text="Already at the top-level directory.")
            return

        self.current_path = parent_path
        self.current_file = None
        self.text_editor.delete("1.0", tk.END)
        self.refresh_file_list()
        self.status_label.config(text=f"Moved up to: {self.current_path}")

def main():
    root = tk.Tk()
    app = FileManagerApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()