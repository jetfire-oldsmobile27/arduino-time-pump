"""
send current time (or provided time) to MCU via serial.
Format sent: "YYYY-MM-DD HH:MM:SS\\n"
Waits for a reply line from MCU (e.g. "OK", "ERR", "BAD FORMAT", or debug lines).
Usage examples:
  python independent_env_sync_time.py --port COM5
  python independent_env_sync_time.py --port /dev/ttyUSB0 --baud 115200
  python independent_env_sync_time.py --port COM5 --time "2025-08-23 20:12:00"
  python independent_env_sync_time.py --port auto --baud 115200 --wait-for-ok
"""

import argparse
import time
import sys
from datetime import datetime

try:
    import serial
    from serial.tools import list_ports
except Exception as e:
    print("pyserial required. Install with: pip install pyserial")
    raise

DEFAULT_BAUD = 115200
DEFAULT_WAIT_AFTER_OPEN = 1.5  # seconds
DEFAULT_READ_TIMEOUT = 3       # seconds

def list_serial_ports():
    return list(list_ports.comports())

def choose_port(interactive=False):
    ports = list_serial_ports()
    if not ports:
        return None
    if not interactive:
        return ports[0].device
    print("Available serial ports:")
    for i, p in enumerate(ports):
        print(f"{i}: {p.device} - {p.description}")
    idx = input("Choose port index (enter for 0): ").strip()
    if idx == "":
        idx = 0
    else:
        idx = int(idx)
    return ports[idx].device

def open_serial_with_retries(port, baud, timeout, retries=10, interval=0.5):
    deadline = time.time() + retries * interval
    last_exc = None
    while time.time() < deadline:
        try:
            ser = serial.Serial(port, baud, timeout=timeout)
            if not ser.is_open:
                try:
                    ser.open()
                except Exception:
                    pass
            if ser.is_open:
                return ser
        except Exception as e:
            last_exc = e
        time.sleep(interval)
    raise RuntimeError(f"Could not open serial port {port}: {last_exc}")

def send_time(ser, time_str):
    try:
        ser.reset_input_buffer()
    except Exception:
        pass
    time.sleep(0.05)
    payload = (time_str + "\n").encode()
    ser.write(payload)
    ser.flush()

def read_reply(ser, timeout=DEFAULT_READ_TIMEOUT):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            line = ser.readline()
            if not line:
                continue
            try:
                text = line.decode(errors="ignore").strip()
            except Exception:
                text = repr(line)
            return text
        except Exception:
            pass
    return None

def main():
    ap = argparse.ArgumentParser(description="Send time to MCU via serial (YYYY-MM-DD HH:MM:SS)")
    ap.add_argument("--port", "-p", default="auto",
                    help="Serial port (COMx or /dev/ttyUSBx). 'auto' picks first available.")
    ap.add_argument("--baud", "-b", default=DEFAULT_BAUD, type=int,
                    help=f"Baud rate (default {DEFAULT_BAUD})")
    ap.add_argument("--time", "-t", default=None,
                    help="Manual time string 'YYYY-MM-DD HH:MM:SS'. If omitted uses system local time.")
    ap.add_argument("--wait-after-open", default=DEFAULT_WAIT_AFTER_OPEN, type=float,
                    help="Seconds to wait after opening port before sending (default %(default)s)")
    ap.add_argument("--read-timeout", default=DEFAULT_READ_TIMEOUT, type=float,
                    help="Seconds to wait for MCU reply (default %(default)s)")
    ap.add_argument("--wait-for-ok", action="store_true",
                    help="Keep retrying send until MCU replies containing 'OK' (useful for flaky connections).")
    ap.add_argument("--interactive-port", action="store_true",
                    help="List ports and select interactively.")
    args = ap.parse_args()

    if args.port == "auto":
        if args.interactive_port:
            port = choose_port(interactive=True)
        else:
            port = choose_port(interactive=False)
        if not port:
            print("No serial ports detected. Connect your device and try again.")
            sys.exit(2)
    else:
        port = args.port

    if args.time:
        try:
            dt = datetime.fromisoformat(args.time.replace(" ", "T"))
        except Exception as e:
            print("Time parse error:", e)
            sys.exit(2)
    else:
        dt = datetime.now()

    time_str = dt.strftime("%Y-%m-%d %H:%M:%S")
    print(f"Will send time: {time_str} -> port {port} @ {args.baud} baud")

    attempt = 0
    while True:
        attempt += 1
        try:
            ser = open_serial_with_retries(port, args.baud, timeout=args.read_timeout, retries=10, interval=0.5)
        except Exception as e:
            print("ERROR opening port:", e)
            sys.exit(3)

        print(f"Opened {port}. Waiting {args.wait_after_open} s for MCU to initialize...")
        time.sleep(args.wait_after_open)

        try:
            send_time(ser, time_str)
            print("Sent:", time_str)
            reply = read_reply(ser, timeout=args.read_timeout)
            if reply:
                print("MCU replied:", reply)
            else:
                print("No reply from MCU (timeout).")
        except Exception as e:
            print("Communication error:", e)
            reply = None
        finally:
            try:
                ser.close()
            except Exception:
                pass

        if args.wait_for_ok:
            if reply and "OK" in reply:
                print("Got OK. Done.")
                break
            else:
                print("No OK received, retrying in 1s (attempt {})...".format(attempt))
                time.sleep(1.0)
                continue
        else:
            break

if __name__ == "__main__":
    main()
