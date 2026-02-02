import socket
import threading
import tkinter as tk

# Configuration
UDP_IP = "127.0.0.1"
UDP_PORT = 4444
WINDOW_SIZE = "300x100"
# Default Color = Green
TEXT_COLOR = "#0DFF00"
BG_COLOR = "black"

class BombTimerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("CS2 Bomb Timer")
        self.root.geometry(WINDOW_SIZE)
        
        # Window Attributes
        self.root.attributes("-topmost", True)
        # Remove title bar and borders
        self.root.overrideredirect(True)
        # Transparent background
        self.root.attributes("-transparentcolor", BG_COLOR)
        
        # Timer Display Label
        self.label = tk.Label(root, text="", font=("Impact", 48), fg=TEXT_COLOR, bg=BG_COLOR)
        self.label.pack(expand=True)

        self.running = True

        # Start UDP Listener in separate thread
        self.udp_thread = threading.Thread(target=self.udp_listener, daemon=True)
        self.udp_thread.start()

        # Add a way to close it (Right Click to quit)
        self.root.bind("<Button-3>", lambda e: self.root.quit())

    def update_display(self, text, color=None):
        self.label.config(text=text)
        if color:
            self.label.config(fg=color)
        self.root.update()

    def udp_listener(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind((UDP_IP, UDP_PORT))
        print(f"GUI Listening on {UDP_PORT}...")

        while self.running:
            try:
                data, _ = sock.recvfrom(1024)
                message = data.decode('utf-8')
                
                if message.startswith("TIME:"):
                    parts = message.split(":")
                    seconds = int(parts[1])
                    status = parts[2] if len(parts) > 2 else "SAFE"
                    
                    display_text = f"BOMB: {seconds}"
                    
                    # Change Color based on (Round) Status
                    if status == "SAVE":
                        # Red (No Time)
                        self.update_display("NO TIME!", "#FF0000")
                    elif seconds <= 10 and status == "SAFE":
                        # Orange (Running out of Time Warning)
                        self.update_display(display_text, "#FFA500")
                    else:
                        # Green (Standard)
                        self.update_display(display_text, "#0DFF00")
                
                # Round End Events
                elif message == "EVENT:EXPLODED":
                    # Terrorist Round Win (RED)
                    self.update_display("Ts WIN", "#FF4444")
                
                elif message == "EVENT:DEFUSED":
                    # Counter-Terrorist Round Win (Blue)
                    self.update_display("CTs WIN", "#00C3FF")
                
                elif message == "EVENT:CANCEL":
                    self.update_display("")
                    
            except Exception as e:
                print(f"Error: {e}")

if __name__ == "__main__":
    root = tk.Tk()
    root.configure(bg=BG_COLOR)
    
    # Window position is top left by default
    # Can change "+50+50" to move (X+Y)
    root.geometry(f"{WINDOW_SIZE}+50+50")
    
    app = BombTimerApp(root)
    root.mainloop()