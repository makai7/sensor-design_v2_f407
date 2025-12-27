#!/usr/bin/env python3
"""
STM32F407 Smart Gimbal - Serial Image Receiver
Author: Auto-generated for sensor-design_v2_f407-1
Description: Receives JPEG images and telemetry data from STM32 via UART

Usage:
    python3 serial_receiver.py /dev/ttyUSB0 115200
    python3 serial_receiver.py COM3 115200  # Windows
"""

import serial
import sys
import os
import time
from datetime import datetime


class STM32ImageReceiver:
    def __init__(self, port, baudrate=115200):
        """Initialize serial connection"""
        self.port = port
        self.baudrate = baudrate
        self.ser = None
        self.image_count = 0
        self.output_dir = "captured_images"

        # Create output directory
        if not os.path.exists(self.output_dir):
            os.makedirs(self.output_dir)
            print(f"[INFO] Created output directory: {self.output_dir}/")

    def connect(self):
        """Open serial connection"""
        try:
            self.ser = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=1
            )
            print(f"[OK] Connected to {self.port} at {self.baudrate} baud")
            time.sleep(2)  # Wait for serial to stabilize
            return True
        except serial.SerialException as e:
            print(f"[ERROR] Failed to open {self.port}: {e}")
            return False

    def run(self):
        """Main receive loop"""
        if not self.connect():
            return

        print("[INFO] Listening for data... (Press Ctrl+C to stop)\n")

        try:
            image_data = bytearray()
            receiving_image = False

            while True:
                if self.ser.in_waiting > 0:
                    line = self.ser.readline().decode('utf-8', errors='ignore').strip()

                    if line:
                        # Check for image start marker
                        if "IMG_START" in line:
                            print(f"\n[IMAGE] Receiving image #{self.image_count + 1}...")
                            image_data = bytearray()
                            receiving_image = True

                        # Check for image end marker
                        elif "IMG_END" in line:
                            if receiving_image and len(image_data) > 0:
                                self.save_image(image_data)
                                print(f"[IMAGE] {line}")
                                image_data = bytearray()
                            receiving_image = False

                        # Regular telemetry data
                        elif not receiving_image:
                            # Print telemetry with timestamp
                            timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                            print(f"[{timestamp}] {line}")

                    # If receiving image, read raw binary data
                    if receiving_image:
                        # Read all available binary data
                        while self.ser.in_waiting > 0:
                            chunk = self.ser.read(self.ser.in_waiting)
                            # Filter out text markers if they appear
                            for byte in chunk:
                                # Only add non-ASCII or valid JPEG bytes
                                if byte < 32 or byte > 126 or byte in [0xFF, 0xD8, 0xD9]:
                                    image_data.append(byte)

        except KeyboardInterrupt:
            print("\n\n[INFO] Stopped by user")
        except Exception as e:
            print(f"\n[ERROR] {e}")
        finally:
            if self.ser and self.ser.is_open:
                self.ser.close()
                print("[INFO] Serial port closed")

    def save_image(self, data):
        """Save received image data to file"""
        # Verify JPEG header (0xFF 0xD8)
        if len(data) < 2:
            print("[WARN] Image data too short, skipping")
            return

        if data[0] != 0xFF or data[1] != 0xD8:
            print(f"[WARN] Invalid JPEG header: {data[0]:02X} {data[1]:02X}, attempting to save anyway")

        # Generate filename with timestamp
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"{self.output_dir}/img_{timestamp}_{self.image_count:04d}.jpg"

        try:
            with open(filename, 'wb') as f:
                f.write(data)
            self.image_count += 1
            print(f"[OK] Saved: {filename} ({len(data)} bytes)")
        except IOError as e:
            print(f"[ERROR] Failed to save image: {e}")


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 serial_receiver.py <PORT> [BAUDRATE]")
        print("\nExamples:")
        print("  Linux:   python3 serial_receiver.py /dev/ttyUSB0 115200")
        print("  Windows: python3 serial_receiver.py COM3 115200")
        print("\nCommon Linux serial ports:")
        print("  /dev/ttyUSB0  - USB to Serial adapter")
        print("  /dev/ttyACM0  - STM32 Virtual COM Port")
        sys.exit(1)

    port = sys.argv[1]
    baudrate = int(sys.argv[2]) if len(sys.argv) > 2 else 115200

    print("="*50)
    print("STM32F407 Smart Gimbal - Serial Receiver")
    print("="*50)

    receiver = STM32ImageReceiver(port, baudrate)
    receiver.run()


if __name__ == "__main__":
    main()
