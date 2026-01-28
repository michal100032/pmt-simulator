import customtkinter as ctk
import serial
import serial.tools.list_ports

ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

class PMTSimulatorGUI(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("PMT Simulator Control Panel")
        self.geometry("500x450")
        self.ser = None
        self.grid_columnconfigure(0, weight=1)
        self.label_title = ctk.CTkLabel(self, text="PMT SIMULATOR BOARD", font=ctk.CTkFont(size=20, weight="bold"))
        self.label_title.pack(pady=20)
        self.main_frame = ctk.CTkFrame(self)
        self.main_frame.pack(padx=20, pady=10, fill="both", expand=True)
        self.top_frame = ctk.CTkFrame(self.main_frame, fg_color="transparent")
        self.top_frame.pack(pady=15, padx=20, fill="x")
        self.use_var = ctk.BooleanVar(value=True)
        self.switch_use = ctk.CTkSwitch(self.top_frame, text="USE", variable=self.use_var, command=self.update_params)
        self.switch_use.pack(side="left", padx=10)
        self.label_freq = ctk.CTkLabel(self.top_frame, text="FREQ:")
        self.label_freq.pack(side="left", padx=(20, 5))
        self.entry_freq = ctk.CTkEntry(self.top_frame, width=80)
        self.entry_freq.insert(0, "10")
        self.entry_freq.pack(side="left")
        self.label_hz = ctk.CTkLabel(self.top_frame, text="Hz")
        self.label_hz.pack(side="left", padx=5)
        self.hits_frame = ctk.CTkFrame(self.main_frame, fg_color="transparent")
        self.hits_frame.pack(pady=20, padx=20, fill="x")
        self.hit1_frame = ctk.CTkFrame(self.hits_frame, fg_color="gray20")
        self.hit1_frame.pack(side="left", padx=5, expand=True, fill="both")
        ctk.CTkLabel(self.hit1_frame, text="HIT 1", font=ctk.CTkFont(weight="bold")).pack(pady=5)
        self.entry_v1 = ctk.CTkEntry(self.hit1_frame, width=70)
        self.entry_v1.insert(0, "2.5")
        self.entry_v1.pack(pady=5)
        ctk.CTkLabel(self.hit1_frame, text="Volts (V)").pack(pady=2)
        self.time_frame = ctk.CTkFrame(self.hits_frame, fg_color="transparent")
        self.time_frame.pack(side="left", padx=10)
        ctk.CTkLabel(self.time_frame, text="TIME").pack()
        self.entry_time = ctk.CTkEntry(self.time_frame, width=70)
        self.entry_time.insert(0, "1")
        self.entry_time.pack(pady=5)
        ctk.CTkLabel(self.time_frame, text="ms").pack()
        self.hit2_frame = ctk.CTkFrame(self.hits_frame, fg_color="gray20")
        self.hit2_frame.pack(side="left", padx=5, expand=True, fill="both")
        ctk.CTkLabel(self.hit2_frame, text="HIT 2", font=ctk.CTkFont(weight="bold")).pack(pady=5)
        self.entry_v2 = ctk.CTkEntry(self.hit2_frame, width=70)
        self.entry_v2.insert(0, "3.2")
        self.entry_v2.pack(pady=5)
        ctk.CTkLabel(self.hit2_frame, text="Volts (V)").pack(pady=2)
        self.conn_frame = ctk.CTkFrame(self)
        self.conn_frame.pack(side="bottom", fill="x", padx=20, pady=10)
        self.port_menu = ctk.CTkOptionMenu(self.conn_frame, values=self.get_ports())
        self.port_menu.pack(side="left", padx=10, pady=10)
        self.btn_connect = ctk.CTkButton(self.conn_frame, text="Connect", command=self.toggle_connection)
        self.btn_connect.pack(side="left", padx=10)
        self.btn_update = ctk.CTkButton(self, text="SEND PARAMETERS", fg_color="green", hover_color="darkgreen", command=self.update_params)
        self.btn_update.pack(pady=10)

    def get_ports(self):
        ports = serial.tools.list_ports.comports()
        return [p.device for p in ports] if ports else ["No Ports Found"]

    def toggle_connection(self):
        if self.ser is None:
            port = self.port_menu.get()
            print(f"Connecting to port  {port}...")
            try:
                self.ser = serial.Serial(port, 115200, timeout=1)
                self.btn_connect.configure(text="Disconnect", fg_color="red")
                print("Connect")
            except Exception as e:
                print(f"Disconnect")
        else:
            self.ser.close()
            self.ser = None
            self.btn_connect.configure(text="Connect", fg_color=["#3B8ED0", "#1F6AA5"])
            print("Disconnect")

    def update_params(self):
        print("Send")
        if self.ser:
            try:
                v1 = float(self.entry_v1.get())
                v2 = float(self.entry_v2.get())
                t = float(self.entry_time.get())
                f = float(self.entry_freq.get())
                
                d_us = int(t * 1000)
                p_ms = int(1000 / f) if (self.use_var.get() and f > 0) else 0
                
                cmd = f"{v1},{v2},{d_us},{p_ms}\n"
                print(f"Command:  {cmd.strip()}")
                
                self.ser.write(cmd.encode())
                print("Send to Serial")
                
            except Exception as e:
                print(f"Error transferring data {e}")
        else:
            print("No active connection")

if __name__ == "__main__":
    app = PMTSimulatorGUI()
    app.mainloop()

