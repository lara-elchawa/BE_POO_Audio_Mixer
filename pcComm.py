import customtkinter as ctk
import tkinter as tk
from tkinter import scrolledtext
import serial
import serial.tools.list_ports
import threading
import queue
import time
from pycaw.pycaw import AudioUtilities
import pyautogui
from PIL import Image
import win32ui
import win32gui
import win32con
import win32api

# --- CONFIGURATION ---
ctk.set_appearance_mode("Dark")
ctk.set_default_color_theme("blue")

BAUDRATE = 115200
HANDSHAKE_SEND_CMD = "BEGIN_AUDIO_MIXER"
HANDSHAKE_ACK_KEYWORD = "AUDIO_MIXER_STARTED"

# --- LOGIQUE AUDIO ---

class WindowsAudioController:
    def __init__(self):
        self.sessions = []
        self.app_names = []
        self.current_index = 0
        self.update_sessions()

    def update_sessions(self):
        """Met à jour la liste des applications qui produisent du son"""
        self.sessions = []
        self.app_names = []
        try:
            all_sessions = AudioUtilities.GetAllSessions()
            for session in all_sessions:
                if session.Process:
                    name = session.Process.name()
                    clean_name = name.replace(".exe", "").capitalize()
                    if clean_name in self.app_names: continue
                    self.sessions.append(session)
                    self.app_names.append(clean_name)
            
            self.sessions = self.sessions[:10]
            self.app_names = self.app_names[:10]
            if self.current_index >= len(self.sessions): 
                self.current_index = 0
        except Exception:
            pass 

    def get_current_app_name(self):
        """Retourne le nom de l'application actuellement sélectionnée"""
        if self.sessions and self.current_index < len(self.sessions):
            return self.app_names[self.current_index]
        return "Aucun"

    def get_dominant_color(self):
        """Extrait la couleur de l'icône de l'app active"""
        if not self.sessions or self.current_index >= len(self.sessions):
            return (255, 255, 255)
        try:
            exe_path = self.sessions[self.current_index].Process.exe()
            ico_x = win32gui.GetSystemMetrics(win32con.SM_CXICON)
            ico_y = win32gui.GetSystemMetrics(win32con.SM_CYICON)
            large, small = win32gui.ExtractIconEx(exe_path, 0)
            if not large: return (100, 100, 100)
            hdc = win32ui.CreateDCFromHandle(win32gui.GetDC(0))
            hbmp = win32ui.CreateBitmap()
            hbmp.CreateCompatibleBitmap(hdc, ico_x, ico_y)
            hdc_mem = hdc.CreateCompatibleDC()
            hdc_mem.SelectObject(hbmp)
            win32gui.DrawIconEx(hdc_mem.GetSafeHdc(), 0, 0, large[0], ico_x, ico_y, 0, None, win32con.DI_NORMAL)
            bmpinfo = hbmp.GetInfo()
            bmpstr = hbmp.GetBitmapBits(True)
            img = Image.frombuffer('RGBA', (bmpinfo['bmWidth'], bmpinfo['bmHeight']), bmpstr, 'raw', 'BGRA', 0, 1)
            win32gui.DestroyIcon(large[0])
            if small: win32gui.DestroyIcon(small[0])
            img = img.convert("RGB")
            img = img.resize((1, 1), resample=Image.Resampling.BILINEAR)
            return img.getpixel((0, 0))
        except:
            return (255, 255, 255)

    def get_software_list_string(self):
        self.update_sessions()
        return ",".join(self.app_names)
    
    def get_current_volume(self):
        if not self.sessions or self.current_index >= len(self.sessions): return 0
        try:
            vol = self.sessions[self.current_index].SimpleAudioVolume.GetMasterVolume()
            return int(vol * 100)
        except: return 0

    def change_volume_relative(self, delta):
        if not self.sessions: return 0
        try:
            session = self.sessions[self.current_index]
            vol = session.SimpleAudioVolume
            new_vol = max(0.0, min(1.0, vol.GetMasterVolume() + delta))
            vol.SetMasterVolume(new_vol, None)
            return int(new_vol * 100)
        except: return 0

    def set_volume_absolute(self, value):
        if not self.sessions: return 0
        try:
            vol_float = max(0.0, min(1.0, value / 100.0))
            self.sessions[self.current_index].SimpleAudioVolume.SetMasterVolume(vol_float, None)
            return value
        except: return 0

    def toggle_mute(self):
        if not self.sessions: return "ERR"
        try:
            vol = self.sessions[self.current_index].SimpleAudioVolume
            is_muted = vol.GetMute()
            vol.SetMute(not is_muted, None)
            return "MUTED" if not is_muted else "UNMUTED"
        except: return "ERR"

    def next_software(self):
        if not self.sessions: return None
        self.current_index = (self.current_index + 1) % len(self.sessions)
        return self.get_current_app_name()

# --- INTERFACE GRAPHIQUE ---
class ModernMixerApp(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.title("Audio Mixer Controller - ESP32")
        self.geometry("950x700")
        
        self.audio = WindowsAudioController()
        self.serial_port = None
        self.running = True
        self.handshake_done = False 
        self.msg_queue = queue.Queue()

        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)

        self.setup_sidebar()
        self.setup_main_area()
        
        self.after(500, self.refresh_ports)
        self.after(100, self.process_queue)

    def setup_sidebar(self):
        self.sidebar_frame = ctk.CTkFrame(self, width=200, corner_radius=0)
        self.sidebar_frame.grid(row=0, column=0, rowspan=4, sticky="nsew")
        self.sidebar_frame.grid_rowconfigure(5, weight=1) # Ajusté pour le nouveau bouton

        self.logo_label = ctk.CTkLabel(self.sidebar_frame, text="CONNEXION", font=ctk.CTkFont(size=20, weight="bold"))
        self.logo_label.grid(row=0, column=0, padx=20, pady=(20, 10))

        self.lbl_port = ctk.CTkLabel(self.sidebar_frame, text="Port COM:", anchor="w")
        self.lbl_port.grid(row=1, column=0, padx=20, pady=(10, 0))
        
        self.option_port = ctk.CTkOptionMenu(self.sidebar_frame, values=["Scan..."])
        self.option_port.grid(row=2, column=0, padx=20, pady=(0, 10))

        self.btn_refresh = ctk.CTkButton(self.sidebar_frame, text="Actualiser", command=self.refresh_ports, fg_color="#555")
        self.btn_refresh.grid(row=3, column=0, padx=20, pady=10)

        self.btn_connect = ctk.CTkButton(self.sidebar_frame, text="CONNECTER", command=self.toggle_connection, fg_color="green")
        self.btn_connect.grid(row=4, column=0, padx=20, pady=10, sticky="n")

        # --- NOUVEAU BOUTON RESET ---
        self.btn_reset = ctk.CTkButton(self.sidebar_frame, text="RESET ESP", 
                                       command=self.reset_esp, 
                                       fg_color="#7B241C", hover_color="#922B21")
        self.btn_reset.grid(row=5, column=0, padx=20, pady=10, sticky="n")

        self.lbl_status = ctk.CTkLabel(self.sidebar_frame, text="🔴 Déconnecté", text_color="red")
        self.lbl_status.grid(row=6, column=0, padx=20, pady=20, sticky="s")
    
    def setup_main_area(self):
        self.main_frame = ctk.CTkFrame(self, fg_color="transparent")
        self.main_frame.grid(row=0, column=1, sticky="nsew", padx=20, pady=20)
        self.main_frame.grid_rowconfigure(2, weight=1)
        self.main_frame.grid_columnconfigure(0, weight=1)

        # DASHBOARD
        self.info_frame = ctk.CTkFrame(self.main_frame, corner_radius=10)
        self.info_frame.grid(row=0, column=0, sticky="ew", pady=(0, 20))
        self.info_frame.grid_columnconfigure(0, weight=1)
        self.info_frame.grid_columnconfigure(1, weight=0)

        self.lbl_current_app = ctk.CTkLabel(self.info_frame, text="...", font=ctk.CTkFont(size=28, weight="bold"), text_color="#3B8ED0", anchor="w")
        self.lbl_current_app.grid(row=0, column=0, padx=20, pady=(20, 5), sticky="w")

        self.lbl_vol_value = ctk.CTkLabel(self.info_frame, text="0%", font=ctk.CTkFont(size=28, weight="bold"), text_color="white", anchor="e")
        self.lbl_vol_value.grid(row=0, column=1, padx=20, pady=(20, 5), sticky="e")

        self.progress_bar = ctk.CTkProgressBar(self.info_frame, orientation="horizontal", height=15)
        self.progress_bar.set(0)
        self.progress_bar.grid(row=1, column=0, columnspan=2, padx=20, pady=(5, 15), sticky="ew")

        self.lbl_list_title = ctk.CTkLabel(self.info_frame, text="Logiciels détectés", font=ctk.CTkFont(size=12, weight="bold"))
        self.lbl_list_title.grid(row=2, column=0, columnspan=2, padx=20, pady=(10, 0), sticky="w")

        self.app_list_frame = ctk.CTkScrollableFrame(self.info_frame, height=120, fg_color="#2b2b2b")
        self.app_list_frame.grid(row=3, column=0, columnspan=2, padx=20, pady=(5, 20), sticky="ew")

        # LOG
        self.log_container = ctk.CTkFrame(self.main_frame)
        self.log_container.grid(row=2, column=0, sticky="nsew")
        
        self.txt_log = scrolledtext.ScrolledText(self.log_container, bg="#1a1a1a", fg="#dce4ee", 
                                                 font=("Consolas", 10), borderwidth=0, highlightthickness=0)
        self.txt_log.pack(fill="both", expand=True, padx=5, pady=5)
        
        self.txt_log.tag_config('rx', foreground='#5dade2')
        self.txt_log.tag_config('tx', foreground='#52be80')
        self.txt_log.tag_config('sys', foreground="#807F7F")
        self.txt_log.tag_config('err', foreground='#e74c3c')

        # INPUT & BUTTONS
        self.input_frame = ctk.CTkFrame(self.main_frame, height=50)
        self.input_frame.grid(row=3, column=0, sticky="ew", pady=(10, 0))
        
        self.entry_cmd = ctk.CTkEntry(self.input_frame, placeholder_text="Envoyer commande...")
        self.entry_cmd.pack(side="left", fill="x", expand=True, padx=10, pady=10)
        self.entry_cmd.bind("<Return>", self.send_manual_command)

        # --- NOUVEAU : BOUTON CLEAR ---
        self.btn_clear = ctk.CTkButton(self.input_frame, text="Effacer Logs", width=100, 
                                       fg_color="#444", hover_color="#333",
                                       command=self.clear_logs)
        self.btn_clear.pack(side="right", padx=10, pady=10)

        self.btn_send = ctk.CTkButton(self.input_frame, text="Envoyer", width=100, command=self.send_manual_command)
        self.btn_send.pack(side="right", padx=(0, 0), pady=10)

    # --- ACTION CLEAR LOGS ---
    def clear_logs(self):
        """Efface tout le contenu de la console de logs"""
        self.txt_log.delete('1.0', tk.END)
        self.log("Logs effacés.", 'sys')

    # --- UI UPDATES ---
    def update_app_list_ui(self):
        for widget in self.app_list_frame.winfo_children(): widget.destroy()
        current_app = self.audio.get_current_app_name()
        for i, app_name in enumerate(self.audio.app_names):
            is_active = (app_name == current_app)
            color = "#2fa572" if is_active else "transparent"
            text_col = "white" if is_active else "gray"
            weight = "bold" if is_active else "normal"
            lbl = ctk.CTkLabel(self.app_list_frame, text=f"{i+1}. {app_name}", 
                               text_color=text_col, fg_color=color, corner_radius=5,
                               font=ctk.CTkFont(weight=weight), anchor="w")
            lbl.pack(fill="x", pady=2, padx=5)

    def update_ui_state(self, full_refresh=False):
        app_name = self.audio.get_current_app_name()
        vol = self.audio.get_current_volume()
        self.lbl_current_app.configure(text=app_name)
        self.lbl_vol_value.configure(text=f"{vol}%")
        self.progress_bar.set(vol / 100.0)
        if full_refresh: self.update_app_list_ui()

    # --- CONNEXION ---
    def refresh_ports(self):
        ports = [p.device for p in serial.tools.list_ports.comports()]
        if not ports: ports = ["Aucun"]
        self.option_port.configure(values=ports)
        self.option_port.set(ports[0])

    def toggle_connection(self):
        if self.serial_port and self.serial_port.is_open:
            self.running = False
            self.handshake_done = False
            self.serial_port.close()
            self.btn_connect.configure(text="CONNECTER", fg_color="green")
            self.lbl_status.configure(text="Déconnecté", text_color="red")
            self.log("Déconnexion manuelle.", 'sys')
        else:
            port = self.option_port.get()
            if port == "Aucun" or port == "Scan...": return
            try:
                self.serial_port = serial.Serial(port, BAUDRATE, timeout=1)
                self.running = True
                self.handshake_done = False
                
                self.btn_connect.configure(text="DÉCONNECTER", fg_color="red")
                self.lbl_status.configure(text=f"Tentative sur {port}...", text_color="orange")
                self.log(f"Port ouvert. Attente synchronisation...", 'sys')
                
                threading.Thread(target=self.serial_listen_thread, daemon=True).start()
                self.handshake_loop()
            except Exception as e:
                self.log(f"Erreur: {e}", 'err')

    def handshake_loop(self):
        if self.running and self.serial_port and self.serial_port.is_open and not self.handshake_done:
            self.send_to_esp(HANDSHAKE_SEND_CMD, log=False)
            self.log("Envoi Handshake... (En attente de l'ESP)", 'tx')
            self.after(1000, self.handshake_loop)

    # --- PROCESSING ---
    def process_queue(self):
        while not self.msg_queue.empty():
            msg = self.msg_queue.get()
            
            if not self.handshake_done:
                if HANDSHAKE_ACK_KEYWORD in msg:
                    self.handshake_done = True
                    self.lbl_status.configure(text="Connecté & Synchronisé", text_color="#00ff00")
                    self.log(f"CONNEXION REUSSIE : {msg}", 'sys')
                    self.update_ui_state(full_refresh=True)
                else:
                    if msg.startswith("DEBUG:"): self.log(f"[ESP] {msg}", 'rx')
                continue 

            if msg.startswith("DEBUG:"):
                self.log(f"<< {msg}", 'rx')
            else:
                self.log(f"<< CMD: {msg}", 'rx')
                self.handle_command(msg)
        
        self.after(50, self.process_queue)

    def handle_command(self, cmd):
        need_full_refresh = False
        need_vol_refresh = False
        success = False
        ack_name = ""

        # On sauvegarde l'état actuel pour comparer après l'action (si nécessaire)
        old_vol = self.audio.get_current_volume()
        old_app = self.audio.get_current_app_name()

        # 1. Traitement des commandes
        if cmd == "GET_LIST" or cmd == "GET_SOFTWARE_LIST":
            resp = self.audio.get_software_list_string()
            self.send_to_esp(resp)
            self.log(f"Liste envoyée", 'sys')
            need_full_refresh = True
            success = True
            ack_name = "GET_SOFTWARE_LIST"

        elif cmd == "NEXT_SOFTWARE":
            self.audio.next_software()
            # On vérifie si l'app a bien changé (ou s'il n'y en a qu'une seule)
            if self.audio.get_current_app_name() != old_app or len(self.audio.sessions) <= 1:
                need_full_refresh = True
                success = True
                ack_name = "NEXT_SOFTWARE"

        elif cmd == "VOLUME_UP":
            self.audio.change_volume_relative(0.01)
            # Succès si le volume a augmenté OU s'il est déjà au max (100)
            if self.audio.get_current_volume() > old_vol or old_vol == 100:
                need_vol_refresh = True
                success = True
                ack_name = "VOLUME_UP"

        elif cmd == "VOLUME_DOWN":
            self.audio.change_volume_relative(-0.01)
            # Succès si le volume a baissé OU s'il est déjà au min (0)
            if self.audio.get_current_volume() < old_vol or old_vol == 0:
                need_vol_refresh = True
                success = True
                ack_name = "VOLUME_DOWN"

        elif cmd == "MUTE":
            res = self.audio.toggle_mute()
            if res != "ERR":
                need_vol_refresh = True
                success = True
                ack_name = "MUTE"

        elif cmd.startswith("SET_VOLUME"):
            try:
                val = int(cmd.replace("SET_VOLUME", "").replace(":", "").strip())
                self.audio.set_volume_absolute(val)
                # On vérifie avec une petite marge d'erreur de 1%
                if abs(self.audio.get_current_volume() - val) <= 1:
                    need_vol_refresh = True
                    success = True
                    ack_name = "SET_VOLUME:"+str(val)
            except: 
                pass

        elif cmd in ["NEXT_TRACK", "PREV_TRACK", "PAUSE_PLAY"]:
            # Pour les touches média, on simule l'appui et on valide par défaut
            key = {'NEXT_TRACK': 'nexttrack', 'PREV_TRACK': 'prevtrack', 'PAUSE_PLAY': 'playpause'}
            pyautogui.press(key[cmd])
            success = True
            ack_name = cmd

        # 2. Gestion des ACK et Mise à jour UI
        if success:
            # Envoi du ACK à l'ESP32 (Format : NOM_DE_LA_COMMANDE_ACK)
            self.send_to_esp(f"{ack_name}_ACK")
            
            if need_full_refresh: 
                self.update_ui_state(full_refresh=True)
                # On renvoie aussi la couleur si l'app change
                r, g, b = self.audio.get_dominant_color()
                self.send_to_esp(f"SET_COLOR:{r},{g},{b}")
            elif need_vol_refresh: 
                self.update_ui_state(full_refresh=False)
        else:
            self.log(f"Échec de la commande: {cmd}", 'err')

    def send_to_esp(self, cmd, log=True):
        if self.serial_port and self.serial_port.is_open:
            try:
                self.serial_port.write(f"{cmd}\n".encode('utf-8'))
                if log: self.log(f">> {cmd}", 'tx')
            except Exception as e:
                self.log(f"Erreur TX: {e}", 'err')
    
    def send_manual_command(self, event=None):
        cmd = self.entry_cmd.get()
        if cmd:
            self.send_to_esp(cmd)
            self.entry_cmd.delete(0, 'end')

    def reset_esp(self):
        """Envoie la commande de redémarrage système à l'ESP"""
        if self.serial_port and self.serial_port.is_open:
            self.send_to_esp("NVIC_SYSTEM_RESET")
            self.log("Commande de REDÉMARRAGE envoyée.", 'sys')
        else:
            self.log("Erreur : Impossible de reset (non connecté).", 'err')
            
    def serial_listen_thread(self):
        while self.running and self.serial_port and self.serial_port.is_open:
            try:
                if self.serial_port.in_waiting:
                    line = self.serial_port.readline().decode('utf-8', errors='ignore').strip()
                    if line: self.msg_queue.put(line)
            except: break
            time.sleep(0.01)

    def log(self, message, tag):
        timestamp = time.strftime("[%H:%M:%S]")
        self.txt_log.insert(tk.END, f"{timestamp} {message}\n", tag)
        self.txt_log.see(tk.END)

if __name__ == "__main__":
    app = ModernMixerApp()
    app.mainloop()