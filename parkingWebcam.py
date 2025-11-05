# n8n Matrix Display
# Script to take webcam pictures of the parking in order for the AI to identify empty spaces
# Roni Bandini
# November 2025
# MIT License

import cv2
import time
from ftplib import FTP
import os

# --- Settings
WEBCAM_INDEX = 0  # Usually 0
INTERVALO_SEGUNDOS = 5 * 60  # 5 min
RUTA_FOTOS = "fotos_webcam/" # folder
NOMBRE_IMAGEN = "parking.jpg"

# FTP
FTP_HOST = "your_ftp_host.com"
FTP_USER = "your_ftp_username"
FTP_PASSWORD = "your_ftp_password"
FTP_REMOTE_PATH = "/" # folder

def tomar_foto(cam_index, ruta_guardar, nombre_archivo):
    cap = cv2.VideoCapture(cam_index)
    if not cap.isOpened():
        print(f"Error: cannot open webca, {cam_index}")
        return None

    ret, frame = cap.read()
    if ret:
        ruta_completa = os.path.join(ruta_guardar, nombre_archivo)
        cv2.imwrite(ruta_completa, frame)
        print(f"Picture saved as: {ruta_completa}")
        cap.release()
        return ruta_completa
    else:
        print("Error: cannot get picture.")
        cap.release()
        return None

def subir_a_ftp(archivo_local, host, user, password, ruta_remota):
    try:
        with FTP(host) as ftp:
            ftp.login(user=user, passwd=password)
            print(f"Connected to FTP: {host}")

            with open(archivo_local, 'rb') as f:
                nombre_archivo = os.path.basename(archivo_local)
                ftp.storbinary(f'STOR {os.path.join(ruta_remota, nombre_archivo).replace("\\", "/")}', f)
            print(f"Picture uploaded: {nombre_archivo}")
    except Exception as e:
        print(f"Error al subir a FTP: {e}")


if __name__ == "__main__":
    if not os.path.exists(RUTA_FOTOS):
        os.makedirs(RUTA_FOTOS)

    print("Starting...")
    print(f"Pictures every {INTERVALO_SEGUNDOS / 60} minutes.")

    while True:
        ruta_foto_tomada = tomar_foto(WEBCAM_INDEX, RUTA_FOTOS, NOMBRE_IMAGEN)
        if ruta_foto_tomada:
            subir_a_ftp(ruta_foto_tomada, FTP_HOST, FTP_USER, FTP_PASSWORD, FTP_REMOTE_PATH)
        
        print(f"Waiting {INTERVALO_SEGUNDOS} seconds...")
        time.sleep(INTERVALO_SEGUNDOS)