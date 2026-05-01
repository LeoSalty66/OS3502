import tkinter as tk
from tkinter import ttk, messagebox, simpledialog
from pathlib import Path
import time

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
        ttk.Button(button_frame, text="Create File", command=self.create_file).pack(side="left", padx=4)
        ttk.Button(button_frame, text="Save", command=self.save_file).pack(side="left", padx=4)
        ttk.Button(button_frame, text="Delete", command=self.delete_selected_item).pack(side="left", padx=4)
        ttk.Button(button_frame, text="Rename", command=self.rename_selected_item).pack(side="left", padx=4)
        ttk.Button(button_frame, text="Create Folder", command=self.create_folder).pack(side="left", padx=4)
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
                metadata = file_ops.get_metadata(selected_entry)
                modified_time = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(metadata["modified"]))

                self.current_file = selected_entry
                self.text_editor.delete("1.0", tk.END)
                self.text_editor.insert(tk.END, content)
                self.status_label.config(
                    text=f"Opened file: {selected_entry.name} | Size: {metadata['size']} bytes | Modified: {modified_time}"
                )

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

    def create_file(self):
        file_name = simpledialog.askstring("Create File", "Enter new file name:")

        if not file_name:
            self.status_label.config(text="Create file canceled.")
            return

        new_file_path = self.current_path / file_name

        try:
            file_ops.create_file(new_file_path, "")
            self.refresh_file_list()
            self.status_label.config(text=f"Created file: {file_name}")

        except FileExistsError:
            messagebox.showerror("File Exists", "A file with that name already exists.")
            self.status_label.config(text="Error: file already exists.")
        except PermissionError:
            messagebox.showerror("Permission Denied", "You do not have permission to create a file here.")
            self.status_label.config(text="Permission denied: cannot create file.")
        except IsADirectoryError:
            messagebox.showerror("Invalid Operation", "That name refers to a directory.")
            self.status_label.config(text="Error: path is a directory.")
        except Exception as error:
            messagebox.showerror("Error", f"Could not create file:\n{error}")
            self.status_label.config(text=f"Error creating file: {error}")

    def save_file(self):
        if self.current_file is None:
            messagebox.showwarning("No File Open", "Open a file before saving.")
            self.status_label.config(text="Save failed: no file is currently open.")
            return

        content = self.text_editor.get("1.0", tk.END)

        try:
            file_ops.write_file(self.current_file, content)
            self.status_label.config(text=f"Saved file: {self.current_file.name}")
            self.refresh_file_list()

        except PermissionError:
            messagebox.showerror("Permission Denied", "You do not have permission to write to this file.")
            self.status_label.config(text="Permission denied: cannot save file.")
        except FileNotFoundError:
            messagebox.showerror("File Not Found", "This file no longer exists.")
            self.status_label.config(text="Save failed: file not found.")
            self.refresh_file_list()
        except Exception as error:
            messagebox.showerror("Error", f"Could not save file:\n{error}")
            self.status_label.config(text=f"Error saving file: {error}")

    def create_folder(self):
        folder_name = simpledialog.askstring("Create Folder", "Enter new folder name:")

        if not folder_name:
            self.status_label.config(text="Create folder canceled.")
            return

        new_folder_path = self.current_path / folder_name

        try:
            file_ops.create_directory(new_folder_path)
            self.refresh_file_list()
            self.status_label.config(text=f"Created folder: {folder_name}")

        except FileExistsError:
            messagebox.showerror("Folder Exists", "A file or folder with that name already exists.")
            self.status_label.config(text="Error: folder already exists.")
        except PermissionError:
            messagebox.showerror("Permission Denied", "You do not have permission to create a folder here.")
            self.status_label.config(text="Permission denied: cannot create folder.")
        except Exception as error:
            messagebox.showerror("Error", f"Could not create folder:\n{error}")
            self.status_label.config(text=f"Error creating folder: {error}")

    def delete_selected_item(self):
        selected_entry = self.get_selected_entry()

        if selected_entry is None:
            return

        confirm = messagebox.askyesno(
            "Confirm Delete",
            f"Are you sure you want to delete:\n{selected_entry.name}?"
        )

        if not confirm:
            self.status_label.config(text="Delete canceled.")
            return

        try:
            file_ops.delete_path(selected_entry)

            if self.current_file == selected_entry:
                self.current_file = None
                self.text_editor.delete("1.0", tk.END)

            self.refresh_file_list()
            self.status_label.config(text=f"Deleted: {selected_entry.name}")

        except OSError as error:
            messagebox.showerror(
                "Delete Failed",
                f"Could not delete item.\n\nThis may happen if a folder is not empty, "
                f"the file is in use, or permission is denied.\n\nDetails:\n{error}"
            )
            self.status_label.config(text=f"Delete failed: {error}")
        except Exception as error:
            messagebox.showerror("Error", f"Could not delete item:\n{error}")
            self.status_label.config(text=f"Error deleting item: {error}")

    def rename_selected_item(self):
        selected_entry = self.get_selected_entry()

        if selected_entry is None:
            return

        new_name = simpledialog.askstring(
            "Rename",
            "Enter new name:",
            initialvalue=selected_entry.name
        )

        if not new_name:
            self.status_label.config(text="Rename canceled.")
            return

        new_path = selected_entry.parent / new_name

        try:
            file_ops.rename_path(selected_entry, new_path)

            if self.current_file == selected_entry:
                self.current_file = new_path

            self.refresh_file_list()
            self.status_label.config(text=f"Renamed to: {new_name}")

        except FileExistsError:
            messagebox.showerror("Name Exists", "A file or folder with that name already exists.")
            self.status_label.config(text="Rename failed: name already exists.")
        except PermissionError:
            messagebox.showerror("Permission Denied", "You do not have permission to rename this item.")
            self.status_label.config(text="Permission denied: cannot rename item.")
        except FileNotFoundError:
            messagebox.showerror("Not Found", "This item no longer exists.")
            self.refresh_file_list()
        except Exception as error:
            messagebox.showerror("Error", f"Could not rename item:\n{error}")
            self.status_label.config(text=f"Error renaming item: {error}")

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