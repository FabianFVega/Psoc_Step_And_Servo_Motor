import serial
import time
import sys
import tkinter as tk
from tkinter import ttk, scrolledtext
from tkinter import messagebox

# --- Configuración del Puerto Serial ---
# IMPORTANTE: Reemplaza 'COMx' o '/dev/ttyUSBx' con el puerto serial de tu PSoC.
# Puedes encontrarlo en el Administrador de Dispositivos (Windows) o
# ejecutando `ls /dev/tty*` en Linux/macOS (usualmente /dev/ttyUSB0 o /dev/ttyACM0 para placas Arduino/PSoC).
SERIAL_PORT = 'COM3'  # Ejemplo para Windows
# SERIAL_PORT = '/dev/ttyUSB0' # Ejemplo para Linux/macOS
BAUD_RATE = 115200    # Debe coincidir con el Baud Rate configurado en PSoC Creator

class SerialGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Control PSoC")
        self.root.geometry("600x500")
        
        # Inicializar la conexión serial
        self.ser = None
        self.connect_serial()
        
        # Crear la interfaz
        self.create_widgets()
        
        # Iniciar la lectura de datos en segundo plano
        self.root.after(100, self.read_serial)
        
    def connect_serial(self):
        try:
            self.ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
            time.sleep(2)
            self.ser.flushInput()
        except serial.SerialException as e:
            messagebox.showerror("Error", f"Error al abrir el puerto serial: {e}\nAsegúrate de que el PSoC esté conectado y el puerto sea el correcto.")
            self.root.destroy()
            sys.exit(1)

    def create_widgets(self):
        # Frame para el control del motor
        motor_frame = ttk.LabelFrame(self.root, text="Control del Motor", padding="10")
        motor_frame.pack(fill="x", padx=10, pady=5)
        
        # Dirección del motor
        ttk.Label(motor_frame, text="Dirección:").grid(row=0, column=0, padx=5, pady=5)
        self.direction_var = tk.StringVar(value="left")
        ttk.Radiobutton(motor_frame, text="Izquierda", variable=self.direction_var, value="left").grid(row=0, column=1)
        ttk.Radiobutton(motor_frame, text="Derecha", variable=self.direction_var, value="right").grid(row=0, column=2)
        
        # RPM del motor
        ttk.Label(motor_frame, text="RPM (1-320):").grid(row=1, column=0, padx=5, pady=5)
        self.rpm_var = tk.StringVar(value="150")
        ttk.Entry(motor_frame, textvariable=self.rpm_var, width=10).grid(row=1, column=1, columnspan=2)
        
        # Distancia
        ttk.Label(motor_frame, text="Distancia (1-300):").grid(row=2, column=0, padx=5, pady=5)
        self.distance_var = tk.StringVar(value="100")
        ttk.Entry(motor_frame, textvariable=self.distance_var, width=10).grid(row=2, column=1, columnspan=2)
        
        # Botón para enviar comando del motor
        ttk.Button(motor_frame, text="Enviar Comando Motor", command=self.send_motor).grid(row=3, column=0, columnspan=3, pady=10)
        
        # Frame para el control de servos
        servo_frame = ttk.LabelFrame(self.root, text="Control de Servos", padding="10")
        servo_frame.pack(fill="x", padx=10, pady=5)
        
        # Ángulos de los servos
        ttk.Label(servo_frame, text="Servo 1 (0-180):").grid(row=0, column=0, padx=5, pady=5)
        self.servo1_var = tk.StringVar(value="90")
        ttk.Entry(servo_frame, textvariable=self.servo1_var, width=10).grid(row=0, column=1)
        
        ttk.Label(servo_frame, text="Servo 2 (0-180):").grid(row=1, column=0, padx=5, pady=5)
        self.servo2_var = tk.StringVar(value="90")
        ttk.Entry(servo_frame, textvariable=self.servo2_var, width=10).grid(row=1, column=1)
        
        # Botón para enviar comando de servos
        ttk.Button(servo_frame, text="Enviar Comando Servos", command=self.send_servo).grid(row=2, column=0, columnspan=2, pady=10)
        
        # Área de mensajes
        msg_frame = ttk.LabelFrame(self.root, text="Mensajes", padding="10")
        msg_frame.pack(fill="both", expand=True, padx=10, pady=5)
        
        self.msg_area = scrolledtext.ScrolledText(msg_frame, height=10)
        self.msg_area.pack(fill="both", expand=True)
        
    def send_motor(self):
        try:
            rpm = int(self.rpm_var.get())
            distance = int(self.distance_var.get())
            
            if not (1 <= rpm <= 50):
                messagebox.showerror("Error", "RPM debe estar entre 1 y 50")
                return
                
            if not (1 <= distance <= 300):
                messagebox.showerror("Error", "La distancia debe estar entre 1 y 300")
                return
                
            direction = self.direction_var.get()
            command = f"M,{direction[0].upper()},{rpm},{distance}\n"
            self.ser.write(command.encode('utf-8'))
            self.msg_area.insert(tk.END, f"Enviado: {command.strip()}\n")
            self.msg_area.see(tk.END)
        except ValueError:
            messagebox.showerror("Error", "RPM y distancia deben ser números válidos")
            
    def send_servo(self):
        try:
            servo1 = int(self.servo1_var.get())
            servo2 = int(self.servo2_var.get())
            
            if not (0 <= servo1 <= 180 and 0 <= servo2 <= 180):
                messagebox.showerror("Error", "Los ángulos deben estar entre 0 y 180")
                return
                
            command = f"S,{servo1},{servo2}\n"
            self.ser.write(command.encode('utf-8'))
            self.msg_area.insert(tk.END, f"Enviado: {command.strip()}\n")
            self.msg_area.see(tk.END)
        except ValueError:
            messagebox.showerror("Error", "Los ángulos deben ser números válidos")
            
    def read_serial(self):
        if self.ser and self.ser.in_waiting > 0:
            try:
                response = self.ser.readline().decode('utf-8').strip()
                if response:
                    self.msg_area.insert(tk.END, f"Recibido: {response}\n")
                    self.msg_area.see(tk.END)
            except:
                pass
        self.root.after(100, self.read_serial)
        
    def on_closing(self):
        if self.ser and self.ser.is_open:
            self.ser.close()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = SerialGUI(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()