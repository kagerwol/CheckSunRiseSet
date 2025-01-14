import customtkinter as ctk

def on_button_click():
    label.config(text="Button clicked!")

root = ctk.CTk()
root.title("CustomTkinter Example")

label = ctk.CTkLabel(root, text="Hello, CustomTkinter!")
label.pack(pady=10)

button = ctk.CTkButton(root, text="Click me", command=on_button_click)
button.pack(pady=10)

root.mainloop()
