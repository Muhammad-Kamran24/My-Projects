import socket
import threading
import json
import tkinter as tk
from tkinter import scrolledtext, messagebox, filedialog, simpledialog, ttk, font
import base64
import os
import time 
import platform  
import subprocess
from PIL import Image, ImageTk
import io

# Sound Notification Support
try:
    import winsound
except ImportError:
    winsound = None # Non-Windows systems

# Optional: OpenCV for Video Call
try:
    import cv2 
    VIDEO_AVAILABLE = True 
except ImportError:
    VIDEO_AVAILABLE = False

# Optional: PyAudio for Voice
try:
    import pyaudio
    import wave 
    AUDIO_AVAILABLE = True 
except ImportError: 
    AUDIO_AVAILABLE = False 

# Configuration
PORT = 5555 
CHUNK = 1024 
FORMAT = pyaudio.paInt16 if AUDIO_AVAILABLE else None 
CHANNELS = 1  
RATE = 44100 

# --- THEME COLORS ---
COLOR_PRIMARY = "#008069"       
COLOR_BG_SIDEBAR = "#FFFFFF"    
COLOR_BG_CHAT = "#EFE7DD"       
COLOR_BUBBLE_SELF = "#D9FDD3"   
COLOR_BUBBLE_OTHER = "#FFFFFF"  
COLOR_ACCENT = "#00A884"        
COLOR_TEXT_HEADER = "#FFFFFF"
COLOR_TEXT_BODY = "#111B21"

class VoiceMessagePlayer:
    def __init__(self, filepath, button_widget, root):
        self.filepath = filepath
        self.btn = button_widget
        self.root = root
        self.is_playing = False
        self.is_paused = False
        self.p = None
        self.stream = None
        self.frames = []
        self.current_idx = 0
        self.wf = None

    def toggle(self):
        if not os.path.exists(self.filepath):
            messagebox.showerror("Error", "Audio file not found.")
            return
        if not AUDIO_AVAILABLE:
            self.open_external()
            return
        if self.is_playing:
            self.pause()
        else:
            self.play()

    def play(self):
        if self.is_paused:
            self.is_playing = True
            self.is_paused = False
            self.btn.config(text="⏸", bg="#FFD54F") 
            threading.Thread(target=self.playback_loop, daemon=True).start()
        else:
            try:
                self.wf = wave.open(self.filepath, 'rb')
                self.p = pyaudio.PyAudio()
                self.stream = self.p.open(format=self.p.get_format_from_width(self.wf.getsampwidth()),
                                          channels=self.wf.getnchannels(),
                                          rate=self.wf.getframerate(),
                                          output=True)
                self.frames = []
                data = self.wf.readframes(CHUNK)
                while data:
                    self.frames.append(data)
                    data = self.wf.readframes(CHUNK)
                self.current_idx = 0
                self.is_playing = True
                self.btn.config(text="⏸", bg="#FFD54F")
                threading.Thread(target=self.playback_loop, daemon=True).start()
            except Exception as e:
                print(f"Playback error: {e}")
                self.stop()

    def playback_loop(self):
        while self.is_playing and self.current_idx < len(self.frames):
            if self.stream:
                self.stream.write(self.frames[self.current_idx])
            self.current_idx += 1
        if self.current_idx >= len(self.frames):
            self.stop()

    def pause(self):
        self.is_playing = False
        self.is_paused = True
        self.btn.config(text="▶", bg="#E0F2F1")

    def stop(self):
        self.is_playing = False
        self.is_paused = False
        self.current_idx = 0
        if self.stream:
            self.stream.stop_stream()
            self.stream.close()
        if self.p:
            self.p.terminate()
        if self.wf:
            self.wf.close()
        try:
            self.root.after(0, lambda: self.btn.config(text="▶", bg="#E0F2F1"))
        except: pass

    def open_external(self):
        try:
            if platform.system() == 'Windows': os.startfile(self.filepath)
            elif platform.system() == 'Darwin': subprocess.call(('open', self.filepath))
            elif platform.system() == 'Linux': subprocess.call(('xdg-open', self.filepath))
        except: pass

class ChatClient:
    def __init__(self, root):
        self.root = root
        self.root.title("SpeakMe")
        self.root.geometry("1100x750")
        self.root.configure(bg=COLOR_BG_SIDEBAR) 
        
        self.custom_font = font.Font(family="Segoe UI", size=10)
        self.header_font = font.Font(family="Segoe UI", size=11, weight="bold")
        self.bubble_font = font.Font(family="Segoe UI", size=10)
        
        style = ttk.Style()
        style.theme_use('clam')
        style.layout("TNotebook", [("Notebook.client", {"sticky": "nswe"})])
        style.configure("TNotebook", background=COLOR_BG_SIDEBAR, borderwidth=0)
        style.configure("TFrame", background=COLOR_BG_SIDEBAR)
        
        self.client_socket = None
        self.username = ""
        self.is_connected = False
        
        # UI State
        self.tabs = {} 
        self.chat_widgets = {}
        self.unread_tabs = set()
        
        # Logic State
        self.is_recording = False
        self.audio_frames = []
        self.record_stream = None
        self.audio_p = None
        self.active_players = {} 
        
        # Call/Video State
        self.active_call_window = None
        self.call_wait_window = None
        self.incoming_call_window = None
        self.in_call = False
        self.local_video_label = None
        self.remote_video_label = None

        self.setup_login_screen()

    def open_file(self, filepath):
        try:
            if platform.system() == 'Windows': os.startfile(filepath)
            elif platform.system() == 'Darwin': subprocess.call(('open', filepath))
            elif platform.system() == 'Linux': subprocess.call(('xdg-open', filepath))
        except Exception as e: messagebox.showerror("Error", f"Failed to open file:\n{e}")

    def setup_login_screen(self):
        self.login_frame = tk.Frame(self.root, padx=20, pady=20, bg='#f0f2f5')
        self.login_frame.place(relx=0.5, rely=0.5, anchor="center")

        tk.Label(self.login_frame, text="SpeakMe", font=("Segoe UI", 28, "bold"), fg=COLOR_PRIMARY, bg='#f0f2f5').pack(pady=(0, 10))
        tk.Label(self.login_frame, text="Connect to start chatting", font=("Segoe UI", 12), fg="#54656f", bg='#f0f2f5').pack(pady=(0, 20))
        
        tk.Label(self.login_frame, text="Server IP", font=("Segoe UI", 10, "bold"), bg='#f0f2f5', anchor="w").pack(fill="x")
        self.ip_entry = tk.Entry(self.login_frame, font=("Segoe UI", 12), width=30, bd=1, relief="solid")
        self.ip_entry.insert(0, "127.0.0.1") 
        self.ip_entry.pack(pady=(5, 15), ipady=5)

        tk.Label(self.login_frame, text="Username", font=("Segoe UI", 10, "bold"), bg='#f0f2f5', anchor="w").pack(fill="x")
        self.username_entry = tk.Entry(self.login_frame, font=("Segoe UI", 12), width=30, bd=1, relief="solid")
        self.username_entry.pack(pady=(5, 20), ipady=5)

        tk.Button(self.login_frame, text="Start Chatting", command=self.connect_to_server, 
                  bg=COLOR_PRIMARY, fg="white", font=("Segoe UI", 12, "bold"), width=25, relief=tk.FLAT, cursor="hand2").pack(pady=10)

    def connect_to_server(self):
        ip = self.ip_entry.get()
        self.username = self.username_entry.get().strip()
        if not self.username: return
        try:
            self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.client_socket.settimeout(5)
            self.client_socket.connect((ip, PORT))
            self.client_socket.settimeout(None)
            self.is_connected = True
            self.send_packet({"type": "LOGIN", "name": self.username})
            self.setup_main_interface()
            threading.Thread(target=self.receive_messages, daemon=True).start()
        except Exception as e:
            messagebox.showerror("Error", f"Connection failed: {e}")

    def send_packet(self, data):
        if self.is_connected and self.client_socket:
            try:
                json_str = json.dumps(data) + "\n"
                self.client_socket.sendall(json_str.encode('utf-8'))
            except Exception as e:
                print(f"Send error: {e}")
                self.is_connected = False
                self.root.after(0, self.reset_to_login)

    def setup_main_interface(self):
        if hasattr(self, 'login_frame'): self.login_frame.destroy()
        self.main_paned = tk.PanedWindow(self.root, orient=tk.HORIZONTAL, sashwidth=2, bg="#d1d7db", bd=0)
        self.main_paned.pack(fill=tk.BOTH, expand=True)

        self.sidebar_frame = tk.Frame(self.main_paned, bg=COLOR_BG_SIDEBAR, width=300)
        self.sidebar_frame.pack_propagate(False)
        self.main_paned.add(self.sidebar_frame, minsize=250)

        header_side = tk.Frame(self.sidebar_frame, bg="#f0f2f5", height=60, padx=10)
        header_side.pack(fill=tk.X)
        header_side.pack_propagate(False)
        lbl_avatar = tk.Label(header_side, text=self.username[0].upper(), font=("Segoe UI", 14, "bold"), 
                              bg="#dfe3e5", fg="#54656f", width=3, height=1, relief="flat")
        lbl_avatar.pack(side=tk.LEFT, pady=10)
        tk.Label(header_side, text=self.username, font=("Segoe UI", 12, "bold"), bg="#f0f2f5", fg="#111b21").pack(side=tk.LEFT, padx=10)

        search_frame = tk.Frame(self.sidebar_frame, bg=COLOR_BG_SIDEBAR, pady=5, padx=10)
        search_frame.pack(fill=tk.X)
        tk.Entry(search_frame, bg="#f0f2f5", fg="#54656f", font=("Segoe UI", 10), relief="flat", highlightthickness=0).pack(fill=tk.X, ipady=5, padx=5)

        self.list_container = tk.Frame(self.sidebar_frame, bg=COLOR_BG_SIDEBAR)
        self.list_container.pack(fill=tk.BOTH, expand=True)

        tk.Label(self.list_container, text="ONLINE USERS", font=("Segoe UI", 9, "bold"), fg=COLOR_PRIMARY, bg=COLOR_BG_SIDEBAR, anchor="w").pack(fill=tk.X, padx=15, pady=(10,5))
        self.user_listbox = tk.Listbox(self.list_container, font=("Segoe UI", 11), bg=COLOR_BG_SIDEBAR, fg="#111b21", 
                                      selectbackground="#f0f2f5", selectforeground="#111b21", bd=0, highlightthickness=0, activestyle="none", height=8)
        self.user_listbox.pack(fill=tk.X, padx=5)
        self.user_listbox.bind("<<ListboxSelect>>", self.on_user_click_wrapper)

        tk.Label(self.list_container, text="GROUPS", font=("Segoe UI", 9, "bold"), fg=COLOR_PRIMARY, bg=COLOR_BG_SIDEBAR, anchor="w").pack(fill=tk.X, padx=15, pady=(15,5))
        self.group_listbox = tk.Listbox(self.list_container, font=("Segoe UI", 11), bg=COLOR_BG_SIDEBAR, fg="#111b21", 
                                       selectbackground="#f0f2f5", selectforeground="#111b21", bd=0, highlightthickness=0, activestyle="none")
        self.group_listbox.pack(fill=tk.BOTH, expand=True, padx=5)
        self.group_listbox.bind("<<ListboxSelect>>", self.on_group_click_wrapper)

        bottom_bar = tk.Frame(self.sidebar_frame, bg="#f0f2f5", height=50)
        bottom_bar.pack(fill=tk.X, side=tk.BOTTOM)
        tk.Button(bottom_bar, text="➕ New Group", command=self.create_group_dialog, 
                  bg="#00a884", fg="white", font=("Segoe UI", 10, "bold"), relief="flat", cursor="hand2").pack(side=tk.RIGHT, padx=10, pady=8)

        self.right_frame = tk.Frame(self.main_paned, bg=COLOR_BG_CHAT)
        self.main_paned.add(self.right_frame, minsize=400)
        self.notebook = ttk.Notebook(self.right_frame)
        self.notebook.pack(fill=tk.BOTH, expand=True)
        self.create_welcome_screen()
        self.create_chat_tab("Public", "Public Chat") 

    def create_welcome_screen(self):
        welcome_frame = tk.Frame(self.notebook, bg="#f0f2f5")
        self.notebook.add(welcome_frame, text="Home")
        center_frame = tk.Frame(welcome_frame, bg="#f0f2f5")
        center_frame.place(relx=0.5, rely=0.5, anchor="center")
        tk.Label(center_frame, text="SpeakMe", font=("Segoe UI", 30, "bold"), fg="#41525d", bg="#f0f2f5").pack()
        tk.Label(center_frame, text="Send and receive messages safely.", font=("Segoe UI", 12), fg="#8696a0", bg="#f0f2f5").pack(pady=10)
        self.notebook.select(welcome_frame)

    def create_chat_tab(self, chat_id, label):
        if chat_id in self.tabs: return
        frame = tk.Frame(self.notebook, bg=COLOR_BG_CHAT)
        self.notebook.add(frame, text=label)
        self.tabs[chat_id] = frame
        is_group = chat_id in self.get_all_groups()
        
        header = tk.Frame(frame, bg="#f0f2f5", height=60, bd=1, relief="solid")
        header.pack(fill=tk.X)
        header.pack_propagate(False)
        tk.Label(header, text="👤", font=("Segoe UI", 18), bg="#f0f2f5", fg="#54656f").pack(side=tk.LEFT, padx=(15, 10))
        
        info_frame = tk.Frame(header, bg="#f0f2f5")
        info_frame.pack(side=tk.LEFT, fill=tk.Y, pady=10)
        tk.Label(info_frame, text=label, font=("Segoe UI", 12, "bold"), bg="#f0f2f5", fg="#111b21").pack(anchor="w")
        
        btn_frame = tk.Frame(header, bg="#f0f2f5")
        btn_frame.pack(side=tk.RIGHT, padx=10)
        if chat_id != "Public":
            if is_group:
                tk.Button(btn_frame, text="👤+", command=lambda: self.add_member_to_group(chat_id), bg="#f0f2f5", fg=COLOR_PRIMARY, font=("Segoe UI", 12, "bold"), bd=0, cursor="hand2").pack(side=tk.LEFT, padx=5)
                tk.Button(btn_frame, text="🚪", command=lambda: self.leave_group_by_name(chat_id), bg="#f0f2f5", fg="#D32F2F", font=("Segoe UI", 12), bd=0, cursor="hand2").pack(side=tk.LEFT, padx=5)
            else:
                tk.Button(btn_frame, text="📹", command=lambda: self.initiate_call(chat_id, "Video"), bg="#f0f2f5", fg="#54656f", font=("Segoe UI", 14), bd=0, cursor="hand2").pack(side=tk.LEFT, padx=5)
                tk.Button(btn_frame, text="📞", command=lambda: self.initiate_call(chat_id, "Audio"), bg="#f0f2f5", fg="#54656f", font=("Segoe UI", 14), bd=0, cursor="hand2").pack(side=tk.LEFT, padx=5)

        chat_area = scrolledtext.ScrolledText(frame, state='disabled', wrap=tk.WORD, bg=COLOR_BG_CHAT, font=self.bubble_font, bd=0, highlightthickness=0)
        chat_area.pack(fill=tk.BOTH, expand=True, padx=0, pady=0)
        
        chat_area.tag_config('self_msg', foreground="#111b21", background=COLOR_BUBBLE_SELF, lmargin1=100, lmargin2=100, rmargin=10, spacing1=5, spacing3=5)
        chat_area.tag_config('other_msg', foreground="#111b21", background=COLOR_BUBBLE_OTHER, lmargin1=10, lmargin2=10, rmargin=100, spacing1=5, spacing3=5)
        chat_area.tag_config('system', foreground="#54656f", justify="center", font=("Segoe UI", 9, "italic"), spacing1=10, spacing3=10)
        chat_area.tag_config('right_align', justify='right')
        chat_area.tag_config('left_align', justify='left')
        self.chat_widgets[chat_id] = chat_area

        footer = tk.Frame(frame, bg="#f0f2f5", height=60, padx=10, pady=10)
        footer.pack(fill=tk.X)
        tk.Button(footer, text="📎", command=lambda: self.send_file_from_tab(chat_id), bg="#f0f2f5", fg="#54656f", font=("Segoe UI", 16), bd=0, cursor="hand2").pack(side=tk.LEFT, padx=(0, 10))
        entry_bg = tk.Frame(footer, bg="white", bd=1, relief="solid")
        entry_bg.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=5, ipady=2)
        entry = tk.Entry(entry_bg, font=("Segoe UI", 12), bd=0, bg="white")
        entry.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)
        entry.bind("<Return>", lambda e: self.send_msg_from_tab(chat_id, entry))
        voice_btn = tk.Button(footer, text="🎤", bg="#f0f2f5", fg="#54656f", font=("Segoe UI", 14), bd=0, cursor="hand2")
        voice_btn.config(command=lambda: self.toggle_voice_recording(chat_id, voice_btn))
        voice_btn.pack(side=tk.LEFT, padx=(10, 5))
        tk.Button(footer, text="➤", command=lambda: self.send_msg_from_tab(chat_id, entry), bg="#f0f2f5", fg=COLOR_PRIMARY, font=("Segoe UI", 16, "bold"), bd=0, cursor="hand2").pack(side=tk.LEFT)

    def on_user_click_wrapper(self, event):
        self.group_listbox.selection_clear(0, tk.END)
        self.on_user_double_click(event)
        sel = self.user_listbox.curselection()
        if sel:
            user = self.user_listbox.get(sel[0]).strip().replace("🟢 ", "")
            if user in self.tabs: self.notebook.select(self.tabs[user])

    def on_group_click_wrapper(self, event):
        self.user_listbox.selection_clear(0, tk.END)
        self.on_group_double_click(event)
        sel = self.group_listbox.curselection()
        if sel:
            group = self.group_listbox.get(sel[0]).strip().replace("🟢 ", "")
            if group in self.tabs: self.notebook.select(self.tabs[group])

    def send_msg_from_tab(self, chat_id, entry_widget):
        msg = entry_widget.get().strip()
        if not msg: return
        if chat_id == "Public": self.send_packet({"type": "PUBLIC_MSG", "msg": msg})
        elif chat_id in self.get_all_groups(): self.send_packet({"type": "GROUP_MSG", "target": chat_id, "msg": msg})
        else: self.send_packet({"type": "PRIVATE_MSG", "target": chat_id, "msg": msg})
        self.append_to_tab(chat_id, f"{msg}   ", ['right_align', 'self_msg'])
        entry_widget.delete(0, tk.END)

    def send_file_from_tab(self, chat_id):
        filepath = filedialog.askopenfilename()
        if not filepath: return
        filename = os.path.basename(filepath)
        try:
            with open(filepath, "rb") as f: encoded = base64.b64encode(f.read()).decode('utf-8')
            is_group = chat_id in self.get_all_groups()
            target = "All" if chat_id == "Public" else chat_id
            self.send_packet({"type": "FILE", "filename": filename, "data": encoded, "target": target, "is_group": is_group})
            self.append_file_button(chat_id, f"📄 {filename}", filepath, is_echo=True)
        except Exception as e: messagebox.showerror("Error", str(e))

    def toggle_voice_recording(self, chat_id, btn_widget):
        if not AUDIO_AVAILABLE: return messagebox.showinfo("Error", "PyAudio not found.")
        if not self.is_recording:
            self.is_recording = True
            btn_widget.config(text="⏹", fg="#D32F2F")
            self.audio_frames = []
            self.audio_p = pyaudio.PyAudio()
            self.record_stream = self.audio_p.open(format=FORMAT, channels=CHANNELS, rate=RATE, input=True, frames_per_buffer=CHUNK)
            threading.Thread(target=self.record_audio_loop, daemon=True).start()
        else:
            self.is_recording = False
            btn_widget.config(text="🎤", fg="#54656f")
            time.sleep(0.1)
            if self.record_stream:
                self.record_stream.stop_stream()
                self.record_stream.close()
            if self.audio_p: self.audio_p.terminate()
            if self.audio_frames:
                temp_filename = f"Sent_Voice_{int(time.time())}.wav"
                wf = wave.open(temp_filename, 'wb')
                wf.setnchannels(CHANNELS)
                wf.setsampwidth(self.audio_p.get_sample_size(FORMAT))
                wf.setframerate(RATE)
                wf.writeframes(b''.join(self.audio_frames))
                wf.close()
                with open(temp_filename, "rb") as f: encoded = base64.b64encode(f.read()).decode('utf-8')
                is_group = chat_id in self.get_all_groups()
                target = "All" if chat_id == "Public" else chat_id
                self.send_packet({"type": "VOICE_MSG", "data": encoded, "target": target, "is_group": is_group})
                self.append_voice_button(chat_id, "🎤 Voice Message", temp_filename, is_echo=True)

    def record_audio_loop(self):
        while self.is_recording:
            try: self.audio_frames.append(self.record_stream.read(CHUNK))
            except: break

    def handle_voice_button_click(self, filepath, btn):
        if filepath not in self.active_players: self.active_players[filepath] = VoiceMessagePlayer(filepath, btn, self.root)
        self.active_players[filepath].toggle()

    def on_tab_selected(self, event):
        selected_tab_id = self.notebook.select()
        if not selected_tab_id: return
        for chat_id, frame in self.tabs.items():
            if str(frame) == selected_tab_id:
                if chat_id in self.unread_tabs: 
                    self.unread_tabs.remove(chat_id)
                    self.mark_chat_as_read(chat_id)
                break

    def trigger_notification(self, chat_id, sender, message_preview):
        current_tab_id = self.notebook.select()
        if current_tab_id and str(self.tabs.get(chat_id)) == current_tab_id:
            self.play_notification_sound()
            return
        if chat_id not in self.unread_tabs: 
            self.unread_tabs.add(chat_id)
            self.mark_chat_as_unread(chat_id)
        self.show_toaster(f"New Message: {sender}", message_preview)
        self.play_notification_sound()

    # --- GREEN DOT LOGIC ---
    def mark_chat_as_unread(self, chat_id):
        # Check User List
        users = self.user_listbox.get(0, tk.END)
        for idx, user_text in enumerate(users):
            clean_name = user_text.strip().replace("🟢 ", "")
            if clean_name == chat_id:
                if "🟢" not in user_text:
                    self.user_listbox.delete(idx)
                    self.user_listbox.insert(idx, f"🟢 {clean_name}")
                    self.user_listbox.itemconfig(idx, {'fg': '#00C853'}) 
                return

        # Check Group List
        groups = self.group_listbox.get(0, tk.END)
        for idx, group_text in enumerate(groups):
            clean_name = group_text.strip().replace("🟢 ", "")
            if clean_name == chat_id:
                if "🟢" not in group_text:
                    self.group_listbox.delete(idx)
                    self.group_listbox.insert(idx, f"🟢 {clean_name}")
                    self.group_listbox.itemconfig(idx, {'fg': '#00C853'})
                return

    def mark_chat_as_read(self, chat_id):
        # Check User List
        users = self.user_listbox.get(0, tk.END)
        for idx, user_text in enumerate(users):
            if "🟢" in user_text and chat_id in user_text:
                clean_name = user_text.replace("🟢 ", "").strip()
                self.user_listbox.delete(idx)
                self.user_listbox.insert(idx, f"  {clean_name}") # Restore padding
                self.user_listbox.itemconfig(idx, {'fg': '#111b21'})
                return

        # Check Group List
        groups = self.group_listbox.get(0, tk.END)
        for idx, group_text in enumerate(groups):
            if "🟢" in group_text and chat_id in group_text:
                clean_name = group_text.replace("🟢 ", "").strip()
                self.group_listbox.delete(idx)
                self.group_listbox.insert(idx, f"  {clean_name}")
                self.group_listbox.itemconfig(idx, {'fg': '#111b21'})
                return

    def show_toaster(self, title, message):
        try:
            toaster = tk.Toplevel(self.root)
            toaster.overrideredirect(True)
            toaster.attributes('-topmost', True)
            toaster.configure(bg="#37474F")
            w, h = 300, 80
            x = self.root.winfo_screenwidth() - w - 20
            y = self.root.winfo_screenheight() - h - 60 
            toaster.geometry(f"{w}x{h}+{x}+{y}")
            tk.Label(toaster, text=title, font=("Segoe UI", 10, "bold"), fg="white", bg="#37474F").pack(pady=(10,2), padx=10, anchor="w")
            display_msg = (message[:35] + '...') if len(message) > 35 else message
            tk.Label(toaster, text=display_msg, font=("Segoe UI", 10), fg="#B0BEC5", bg="#37474F").pack(pady=0, padx=10, anchor="w")
            toaster.after(4000, toaster.destroy)
            toaster.bind("<Button-1>", lambda e: toaster.destroy())
        except: pass

    def play_notification_sound(self):
        if winsound:
            try: winsound.MessageBeep(winsound.MB_ICONASTERISK)
            except: pass
        else: print('\a')

    def append_to_tab(self, chat_id, text, tags):
        if chat_id not in self.tabs: self.create_chat_tab(chat_id, chat_id)
        widget = self.chat_widgets[chat_id]
        widget.config(state='normal')
        widget.insert(tk.END, text + "\n\n", tags)
        widget.see(tk.END)
        widget.config(state='disabled')

    def append_file_button(self, chat_id, text, filename, is_echo=False):
        if chat_id not in self.tabs: self.create_chat_tab(chat_id, chat_id)
        widget = self.chat_widgets[chat_id]
        align = 'right_align' if is_echo else 'left_align'
        bubble_tag = 'self_msg' if is_echo else 'other_msg'
        widget.config(state='normal')
        widget.insert(tk.END, text + "  ", [align, bubble_tag])
        btn = tk.Button(widget, text="Open", command=lambda: self.open_file(filename), bg="#ffffff", fg="#008069", font=("Segoe UI", 9, "bold"), relief="flat", padx=10)
        widget.window_create(tk.END, window=btn)
        widget.tag_add(bubble_tag, "end-2c", "end") 
        widget.tag_add(align, "end-2c", "end")
        widget.insert(tk.END, "\n\n", [align, bubble_tag])
        widget.see(tk.END)
        widget.config(state='disabled')

    def append_voice_button(self, chat_id, text, filename, is_echo=False):
        if chat_id not in self.tabs: self.create_chat_tab(chat_id, chat_id)
        widget = self.chat_widgets[chat_id]
        align = 'right_align' if is_echo else 'left_align'
        bubble_tag = 'self_msg' if is_echo else 'other_msg'
        widget.config(state='normal')
        widget.insert(tk.END, text + "  ", [align, bubble_tag])
        btn = tk.Button(widget, text="▶", command=lambda: self.handle_voice_button_click(filename, btn), bg="#ffffff", fg="#008069", font=("Segoe UI", 12, "bold"), relief="flat", width=3)
        widget.window_create(tk.END, window=btn)
        widget.insert(tk.END, "\n\n", [align, bubble_tag])
        widget.see(tk.END)
        widget.config(state='disabled')

    def initiate_call(self, target, call_type):
        self.send_packet({"type": f"{call_type.upper()}_CALL_REQUEST", "target": target})
        self.call_wait_window = tk.Toplevel(self.root)
        self.call_wait_window.title("Calling...")
        self.call_wait_window.geometry("300x150")
        tk.Label(self.call_wait_window, text=f"Calling {target}...", font=("Segoe UI", 12)).pack(pady=30)
        tk.Button(self.call_wait_window, text="Cancel", command=lambda: self.end_call_locally(target), bg="red", fg="white").pack(pady=10)

    def incoming_call_handler(self, caller, call_type):
        if self.incoming_call_window:
            try: self.incoming_call_window.destroy()
            except: pass
        self.incoming_call_window = tk.Toplevel(self.root)
        self.incoming_call_window.title("Incoming Call")
        self.incoming_call_window.geometry("350x200")
        self.incoming_call_window.attributes('-topmost', True)
        self.incoming_call_window.configure(bg="#37474F")
        tk.Label(self.incoming_call_window, text=f"📞 Incoming {call_type} Call", font=("Segoe UI", 14, "bold"), bg="#37474F", fg="white").pack(pady=20)
        tk.Label(self.incoming_call_window, text=f"From: {caller}", font=("Segoe UI", 12), bg="#37474F", fg="#CFD8DC").pack(pady=5)
        btn_frame = tk.Frame(self.incoming_call_window, bg="#37474F")
        btn_frame.pack(pady=20)
        tk.Button(btn_frame, text="ACCEPT", bg="#00a884", fg="white", command=lambda: self.accept_call(caller, call_type)).pack(side=tk.LEFT, padx=20)
        tk.Button(btn_frame, text="DECLINE", bg="#F44336", fg="white", command=lambda: self.decline_call(caller)).pack(side=tk.LEFT, padx=20)

    def accept_call(self, caller, call_type):
        if self.incoming_call_window:
            self.incoming_call_window.destroy()
            self.incoming_call_window = None
        self.send_packet({"type": "CALL_ACCEPTED", "target": caller, "call_type": call_type})
        self.open_call_window(caller, call_type)

    def decline_call(self, caller):
        if self.incoming_call_window:
            self.incoming_call_window.destroy()
            self.incoming_call_window = None
        self.send_packet({"type": "CALL_DECLINED", "target": caller})

    def end_call_locally(self, target):
        self.send_packet({"type": "CALL_ENDED", "target": target})
        self.close_call_window()

    def open_call_window(self, partner, call_type):
        if self.call_wait_window: self.call_wait_window.destroy()
        if self.active_call_window: self.active_call_window.destroy()
        self.active_call_window = tk.Toplevel(self.root)
        self.active_call_window.title(f"{call_type} Call - {partner}")
        self.active_call_window.geometry("700x500") 
        self.active_call_window.configure(bg='#111b21')
        self.active_call_window.protocol("WM_DELETE_WINDOW", lambda: self.end_call_locally(partner)) 
        self.in_call = True
        self.current_call_partner = partner
        
        if AUDIO_AVAILABLE:
            try:
                self.audio_p = pyaudio.PyAudio()
                self.audio_output_stream = self.audio_p.open(format=FORMAT, channels=CHANNELS, rate=RATE, output=True)
                threading.Thread(target=self.stream_audio, args=(partner,), daemon=True).start()
            except Exception as e: print(f"Audio Error: {e}")

        if call_type == "Video":
            video_frame = tk.Frame(self.active_call_window, bg="black")
            video_frame.pack(fill=tk.BOTH, expand=True)
            self.remote_video_label = tk.Label(video_frame, text="Waiting for partner...", bg="black", fg="white")
            self.remote_video_label.place(relx=0, rely=0, relwidth=1, relheight=1)
            self.local_video_label = tk.Label(video_frame, bg="black", borderwidth=2, relief="solid")
            self.local_video_label.place(relx=0.7, rely=0.7, relwidth=0.28, relheight=0.28)
            if VIDEO_AVAILABLE:
                threading.Thread(target=self.stream_video, args=(partner,), daemon=True).start()
            else:
                tk.Label(video_frame, text="OpenCV missing", bg="black", fg="white").pack()
        else:
            frame = tk.Frame(self.active_call_window, bg="#111b21")
            frame.pack(fill=tk.BOTH, expand=True)
            tk.Label(frame, text="📞", font=("Segoe UI", 60), bg="#111b21", fg="white").pack(pady=50)
            tk.Label(frame, text="Audio Call Active", font=("Segoe UI", 16, "bold"), bg="#111b21", fg="#00a884").pack()
        tk.Button(self.active_call_window, text="End Call", command=lambda: self.end_call_locally(partner), bg="#F44336", fg="white", font=("Segoe UI", 12, "bold")).pack(side=tk.BOTTOM, fill=tk.X, pady=10, padx=10)

    def stream_video(self, target):
        cap = cv2.VideoCapture(0)
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, 320)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 240)
        while self.in_call and cap.isOpened():
            ret, frame = cap.read()
            if ret:
                local_frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                local_small = cv2.resize(local_frame_rgb, (160, 120))
                img_local = ImageTk.PhotoImage(image=Image.fromarray(local_small))
                if self.active_call_window and self.local_video_label:
                    try: self.root.after(0, lambda i=img_local: self.update_local_video(i))
                    except: break
                
                encoded, buffer = cv2.imencode('.jpg', frame, [int(cv2.IMWRITE_JPEG_QUALITY), 40])
                b64_data = base64.b64encode(buffer).decode('utf-8')
                self.send_packet({"type": "VIDEO_STREAM", "target": target, "data": b64_data})
                time.sleep(0.03) 
            if not self.in_call: break
        cap.release()

    def update_local_video(self, imgtk):
        if self.local_video_label:
            self.local_video_label.imgtk = imgtk 
            self.local_video_label.configure(image=imgtk)

    def stream_audio(self, target):
        try:
            input_stream = self.audio_p.open(format=FORMAT, channels=CHANNELS, rate=RATE, input=True, frames_per_buffer=CHUNK)
            while self.in_call:
                try:
                    data = input_stream.read(CHUNK, exception_on_overflow=False)
                    b64_data = base64.b64encode(data).decode('utf-8')
                    self.send_packet({"type": "AUDIO_STREAM", "target": target, "data": b64_data})
                except Exception: break
            input_stream.stop_stream()
            input_stream.close()
        except: pass

    def close_call_window(self):
        self.in_call = False
        if hasattr(self, 'audio_output_stream'):
            try:
                self.audio_output_stream.stop_stream()
                self.audio_output_stream.close()
                self.audio_p.terminate()
            except: pass
        if self.active_call_window:
            self.active_call_window.destroy()
            self.active_call_window = None
        if self.call_wait_window:
            self.call_wait_window.destroy()
            self.call_wait_window = None

    def receive_messages(self):
        buffer = ""
        while self.is_connected:
            try:
                data = self.client_socket.recv(2 * 1024 * 1024) 
                if not data: break
                buffer += data.decode('utf-8')
                while "\n" in buffer:
                    msg_str, buffer = buffer.split("\n", 1)
                    if msg_str.strip():
                        try: self.process_message(json.loads(msg_str))
                        except: pass
            except: break
        self.root.after(0, self.reset_to_login)

    def process_message(self, data):
        msg_type = data.get("type")
        if msg_type == "CHAT":
            sender = data["from"]
            msg = data["msg"]
            chat_id = data.get("chat_id", "Public")
            self.root.after(0, lambda: self.trigger_notification(chat_id, sender, msg))
            display = msg if chat_id != "Public" else f"{sender}: {msg}"
            self.root.after(0, lambda: self.append_to_tab(chat_id, display, ['left_align', 'other_msg']))

        elif msg_type == "USER_LIST": self.root.after(0, lambda: self.update_list(self.user_listbox, data["users"]))
        elif msg_type == "GROUP_LIST": self.root.after(0, lambda: self.update_list(self.group_listbox, data["groups"]))
        elif msg_type == "SERVER": self.root.after(0, lambda: self.append_to_tab("Public", f"[System]: {data['msg']}", 'system'))

        elif msg_type in ["FILE_RX", "VOICE_RX"]:
            sender = data["from"]
            filename = data["filename"]
            chat_id = data.get("chat_id", "Public")
            content = base64.b64decode(data["data"])
            save_name = f"Rx_{sender}_{int(time.time())}_{filename}"
            with open(save_name, "wb") as f: f.write(content)
            media_type = "Voice Note" if msg_type == "VOICE_RX" else "File"
            self.root.after(0, lambda: self.trigger_notification(chat_id, sender, f"Sent a {media_type}"))
            if msg_type == "VOICE_RX":
                self.root.after(0, lambda: self.append_voice_button(chat_id, f"{sender} Voice:", save_name, is_echo=False))
            else:
                self.root.after(0, lambda: self.append_file_button(chat_id, f"{sender} sent '{filename}':", save_name, is_echo=False))

        elif msg_type == "VIDEO_STREAM":
            if self.active_call_window:
                try:
                    img_data = base64.b64decode(data["data"])
                    img_arr = Image.open(io.BytesIO(img_data))
                    imgtk = ImageTk.PhotoImage(image=img_arr)
                    self.root.after(0, lambda: self.update_remote_video_ui(imgtk))
                except: pass
        elif msg_type == "AUDIO_STREAM":
            if self.in_call and hasattr(self, 'audio_output_stream'):
                try: self.audio_output_stream.write(base64.b64decode(data["data"]))
                except: pass
        elif msg_type == "VIDEO_CALL_REQUEST": self.root.after(0, lambda: self.incoming_call_handler(data["from"], "Video"))
        elif msg_type == "AUDIO_CALL_REQUEST": self.root.after(0, lambda: self.incoming_call_handler(data["from"], "Audio"))
        elif msg_type == "CALL_ACCEPTED": self.root.after(0, lambda: self.open_call_window(data["from"], data["call_type"]))
        elif msg_type == "CALL_DECLINED":
            if self.call_wait_window: self.root.after(0, self.call_wait_window.destroy)
            self.root.after(0, lambda: messagebox.showinfo("Declined", f"{data['from']} declined."))
        elif msg_type == "CALL_ENDED":
            self.root.after(0, self.close_call_window)
            self.root.after(0, lambda: messagebox.showinfo("Call Ended", "The call ended."))
        elif msg_type == "CALL_FAILED":
            if self.call_wait_window: self.root.after(0, self.call_wait_window.destroy)
            self.root.after(0, lambda: messagebox.showerror("Error", data["msg"]))

    def update_remote_video_ui(self, imgtk):
        if self.active_call_window and hasattr(self, 'remote_video_label'):
            self.remote_video_label.imgtk = imgtk
            self.remote_video_label.configure(image=imgtk)

    def update_list(self, listbox, items):
        listbox.delete(0, tk.END)
        for item in items:
            if item != self.username: listbox.insert(tk.END, f"  {item}") # Add padding

    def on_user_double_click(self, event):
        sel = self.user_listbox.curselection()
        if sel: 
            username = self.user_listbox.get(sel[0]).strip().replace("🟢 ", "")
            self.create_chat_tab(username, username)
            self.notebook.select(self.tabs[username])

    def on_group_double_click(self, event):
        sel = self.group_listbox.curselection()
        if sel: 
            groupname = self.group_listbox.get(sel[0]).strip().replace("🟢 ", "")
            self.create_chat_tab(groupname, groupname)
            self.notebook.select(self.tabs[groupname])

    def create_group_dialog(self):
        name = simpledialog.askstring("Create Group", "Group Name:")
        if name: self.send_packet({"type": "CREATE_GROUP", "group_name": name})

    def add_member_to_group(self, group_name):
        member = simpledialog.askstring("Add Member", "Enter Username to Add:")
        if member:
            self.send_packet({"type": "ADD_MEMBER", "group_name": group_name, "member_name": member})
            
    def leave_group_by_name(self, group_name):
        if messagebox.askyesno("Confirm Leave", f"Are you sure you want to leave {group_name}?"):
            self.send_packet({"type": "LEAVE_GROUP", "group_name": group_name})
            if group_name in self.tabs:
                self.notebook.forget(self.tabs[group_name])
                del self.tabs[group_name]
                del self.chat_widgets[group_name]
                # Switch to welcome screen or public
                self.notebook.select(0)

    def get_all_groups(self): return [self.group_listbox.get(i).strip() for i in range(self.group_listbox.size())]

    def reset_to_login(self):
        self.is_connected = False
        self.in_call = False
        if self.client_socket:
            try: self.client_socket.close()
            except: pass
            self.client_socket = None
        if hasattr(self, 'main_container'): self.main_container.destroy()
        self.username = ""
        self.tabs = {}
        self.chat_widgets = {}
        self.setup_login_screen()

if __name__== "__main__":
    root = tk.Tk()
    client = ChatClient(root)
    root.mainloop()