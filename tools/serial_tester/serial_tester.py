#!/usr/bin/env python3
"""
Shaker Serial Terminal
Kommunicerar med STM32 Shaker via binärt UART-protokoll.

Protokollformat:
  SYNC1(0xAA) SYNC2(0x55) ID_HI ID_LO CMD [PAYLOAD...] CRC_LO CRC_HI TERM(0x0A)

CRC: CRC-16/CCITT (poly 0x1021, init 0xFFFF) över alla bytes utom de 3 sista.
"""

import serial
import serial.tools.list_ports
import struct
import threading
import time
import sys

# --- Protokollkonstanter (matchar serial_handler.h) ---
SYNC1       = 0xAA
SYNC2       = 0x55
ADMIN_ID    = 0xB055
SHAKER_ID   = 0x555B
TERMINATION = 0x0A

CMD_IDENTIFY             = 0x00
CMD_CONFIGURE_PARAM      = 0x01
CMD_CONFIGURE_TEST_SUITE = 0x02
CMD_START_TEST           = 0x03
CMD_STOP_TEST            = 0x04
CMD_DATA                 = 0x05

CMD_NAMES = {
    CMD_IDENTIFY:             "IDENTIFY",
    CMD_CONFIGURE_PARAM:      "CONFIGURE_PARAM",
    CMD_CONFIGURE_TEST_SUITE: "CONFIGURE_TEST_SUITE",
    CMD_START_TEST:           "START_TEST",
    CMD_STOP_TEST:            "STOP_TEST",
    CMD_DATA:                 "DATA"
}


def crc16_ccitt(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


def build_frame(cmd: int, payload: bytes = b"") -> bytes:
    frame = bytes([SYNC1, SYNC2, (ADMIN_ID >> 8) & 0xFF, ADMIN_ID & 0xFF, cmd]) + payload
    crc = crc16_ccitt(frame[2:])  # exclude SYNC1, SYNC2
    frame += bytes([crc & 0xFF, (crc >> 8) & 0xFF, TERMINATION])
    return frame


def parse_frame(data: bytes):
    """Tolkar en inkommande frame från Shaker. Returnerar dict eller None."""
    if len(data) < 8:
        return None
    if data[0] != SYNC1 or data[1] != SYNC2:
        return None

    crc_lo   = data[-3]
    crc_hi   = data[-2]
    term     = data[-1]
    received_crc = (crc_hi << 8) | crc_lo
    computed_crc = crc16_ccitt(data[2:-3])  # exclude SYNC1, SYNC2

    if computed_crc != received_crc:
        return {"error": f"CRC-fel (fick 0x{received_crc:04X}, beräknat 0x{computed_crc:04X})"}

    if term != TERMINATION:
        return {"error": "Saknar terminering"}

    sender_id = (data[2] << 8) | data[3]
    cmd       = data[4]
    payload   = data[5:-3]

    return {
        "sender_id": sender_id,
        "cmd":       cmd,
        "payload":   payload,
    }


# --- Mottagartråd ---
class Receiver(threading.Thread):
    def __init__(self, ser: serial.Serial):
        super().__init__(daemon=True)
        self.ser = ser
        self._buf = bytearray()

    def run(self):
        while True:
            try:
                byte = self.ser.read(1)
            except serial.SerialException:
                break
            if not byte:
                continue
            self._buf.extend(byte)
            if byte[0] == TERMINATION:
                self._process_frame(bytes(self._buf))
                self._buf.clear()

    def _process_frame(self, data: bytes):
        raw_hex = " ".join(f"{b:02X}" for b in data)
        result = parse_frame(data)
        if result is None:
            print(f"\n[RX] Oläslig frame: {raw_hex}")
        elif "error" in result:
            print(f"\n[RX] {result['error']} | rå: {raw_hex}")
        else:
            sid  = result["sender_id"]
            cmd  = result["cmd"]
            pld  = result["payload"]
            name = CMD_NAMES.get(cmd, f"0x{cmd:02X}")
            print(f"\n[RX] ID=0x{sid:04X}  CMD={name}  payload=[{' '.join(f'{b:02X}' for b in pld)}]")

            if cmd == CMD_IDENTIFY and len(pld) >= 8:
                device_id = (pld[0] << 8) | pld[1]
                sw_str = bytes(pld).decode('ascii', errors='replace')
                print(f"     Device ID: 0x{sid:04X}  SW version: {sw_str}")

            if cmd == CMD_DATA:
                x_mm = pld[0] << 8 | pld[1]
                print(f" Data: {x_mm} mm")
        print("cmd> ", end="", flush=True)


# --- Hjälpfunktioner ---
def send(ser: serial.Serial, cmd: int, payload: bytes = b""):
    frame = build_frame(cmd, payload)
    hex_str = " ".join(f"{b:02X}" for b in frame)
    ser.write(frame)
    print(f"[TX] {hex_str}")


def list_ports():
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("Inga COM-portar hittades.")
    for p in ports:
        print(f"  {p.device:10s}  {p.description}")


def print_help():
    print("""
Kommandon:
  identify                        Skicka IDENTIFY
  configure_param                 Skicka CONFIGURE_PARAM (ingen payload)
  configure_test j_max a_max v_max d   Konfigurera testserie (4 x uint8)
  start                           Skicka START_TEST
  stop                            Skicka STOP_TEST
  raw <HEX bytes...>              Skicka rå hex-bytes, ex: raw AA 55 B0 55 00
  ports                           Lista tillgängliga COM-portar
  help                            Visa denna hjälp
  quit / exit                     Avsluta
""")


def main():
    import argparse
    parser = argparse.ArgumentParser(description="Shaker Serial Terminal")
    parser.add_argument("port", nargs="?", help="COM-port, t.ex. COM3")
    parser.add_argument("--baud", type=int, default=115200)
    args = parser.parse_args()

    if not args.port:
        print("Tillgängliga portar:")
        list_ports()
        args.port = input("Välj port: ").strip()

    try:
        ser = serial.Serial(args.port, args.baud, timeout=0.1)
    except serial.SerialException as e:
        print(f"Kunde inte öppna {args.port}: {e}")
        sys.exit(1)

    print(f"Ansluten till {args.port} @ {args.baud} baud")
    print_help()

    rx = Receiver(ser)
    rx.start()

    try:
        while True:
            try:
                line = input("cmd> ").strip()
            except EOFError:
                break

            if not line:
                continue

            parts = line.lower().split()
            cmd_word = parts[0]

            if cmd_word in ("quit", "exit"):
                break

            elif cmd_word == "help":
                print_help()

            elif cmd_word == "ports":
                list_ports()

            elif cmd_word == "identify":
                send(ser, CMD_IDENTIFY)

            elif cmd_word == "configure_param":
                send(ser, CMD_CONFIGURE_PARAM)

            elif cmd_word == "configure_test":
                if len(parts) != 5:
                    print("Användning: configure_test j_max a_max v_max d")
                    continue
                try:
                    vals = [int(p) for p in parts[1:]]
                    if any(v < 0 or v > 255 for v in vals):
                        raise ValueError
                    send(ser, CMD_CONFIGURE_TEST_SUITE, bytes(vals))
                except ValueError:
                    print("Värden måste vara heltal 0-255")

            elif cmd_word == "start":
                send(ser, CMD_START_TEST)

            elif cmd_word == "stop":
                send(ser, CMD_STOP_TEST)

            elif cmd_word == "raw":
                try:
                    raw_bytes = bytes(int(h, 16) for h in parts[1:])
                    hex_str = " ".join(f"{b:02X}" for b in raw_bytes)
                    ser.write(raw_bytes)
                    print(f"[TX RAW] {hex_str}")
                except ValueError:
                    print("Ogiltig hex. Exempel: raw AA 55 B0 55 00 XX XX 0A")

            else:
                print(f"Okänt kommando: '{cmd_word}'. Skriv 'help' för hjälp.")

    except KeyboardInterrupt:
        pass
    finally:
        ser.close()
        print("\nAnslutning stängd.")


if __name__ == "__main__":
    main()
