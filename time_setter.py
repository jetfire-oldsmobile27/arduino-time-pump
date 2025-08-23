# time_setter.py (post action)
Import("env")
import time
import os

try:
    import serial
    from serial import SerialException
except Exception as e:
    print("pyserial not available in PIO environment. Install pyserial into PIO penv.")
    raise

# настройки таймингов — при необходимости увеличьте
RETRY_OPEN_SECONDS = 10
OPEN_RETRY_INTERVAL = 0.4
POST_OPEN_DELAY = 2.5   # подождать после открытия порта, чтобы MCU успел запуститься
READ_TIMEOUT = 3        # таймаут чтения ответа от MCU

def find_port(env):
    p = env.get("UPLOAD_PORT") or env.get("upload_port")
    if p:
        return str(p)
    p = os.environ.get("PLATFORMIO_UPLOAD_PORT")
    if p:
        return p
    return None

def try_open_serial(port, baud, timeout=READ_TIMEOUT):
    deadline = time.time() + RETRY_OPEN_SECONDS
    last_exc = None
    while time.time() < deadline:
        try:
            ser = serial.Serial(port, baud, timeout=timeout)
            # ensure actually open
            if not ser.is_open:
                try:
                    ser.open()
                except Exception:
                    pass
            if ser.is_open:
                return ser
        except SerialException as se:
            last_exc = se
        except Exception as e:
            last_exc = e
        time.sleep(OPEN_RETRY_INTERVAL)
    raise SerialException("Could not open serial port {}: {}".format(port, last_exc))

def read_reply_once(ser):
    try:
        line = ser.readline().decode(errors="ignore").strip()
        return line
    except Exception:
        return None

def send_time_and_wait(ser):
    # очистим входящий буфер перед отправкой
    try:
        ser.reset_input_buffer()
    except Exception:
        pass
    time.sleep(0.05)
    timestamp = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())
    line = timestamp + "\n"
    ser.write(line.encode())
    ser.flush()
    # подождём небольшой момент и попробуем прочитать ответ
    t0 = time.time()
    while time.time() - t0 < READ_TIMEOUT:
        resp = read_reply_once(ser)
        if resp:
            return timestamp, resp
    return timestamp, None

def on_upload_finished(source, target, env):
    print("on_upload_finished({}, {})".format(source, target))
    port = find_port(env)
    if not port:
        print("No upload port found. Set upload_port in platformio.ini or PLATFORMIO_UPLOAD_PORT.")
        return

    # попробуем извлечь скорости (проверяем разные кейсы ключей)
    def get_env_speed(*keys):
        for k in keys:
            v = env.get(k)
            if v:
                try:
                    return int(v)
                except Exception:
                    try:
                        return int(str(v))
                    except Exception:
                        pass
        return None

    monitor_speed = get_env_speed("MONITOR_SPEED", "monitor_speed")
    upload_speed  = get_env_speed("UPLOAD_SPEED", "upload_speed")

    # дефолты
    if not monitor_speed:
        monitor_speed = 115200
    if not upload_speed:
        upload_speed = 57600

    print("Port:", port, "monitor_speed:", monitor_speed, "upload_speed:", upload_speed)

    # Сначала попробуем на скорости MONITOR (это скорость, в которой работает скетч)
    tried = []
    for baud in (monitor_speed, upload_speed):
        tried.append(baud)
        print("Attempting post-upload serial at {} baud...".format(baud))
        try:
            ser = try_open_serial(port, baud)
        except Exception as e:
            print("  Could not open port at {} baud: {}".format(baud, e))
            continue

        # даём MCU время прогреть Serial и вывести приветствие
        time.sleep(POST_OPEN_DELAY)

        try:
            sent_time, resp = send_time_and_wait(ser)
            if resp:
                print("  Sent time to MCU: {}".format(sent_time))
                print("  MCU replied: {}".format(resp))
            else:
                print("  Sent time to MCU: {} (no reply)".format(sent_time))

            ser.close()
        except Exception as e:
            print("  Error during send/read at {} baud: {}".format(baud, e))
            try:
                ser.close()
            except Exception:
                pass
            continue

        # если получили понятный ответ «OK» — успех
        if resp and ("OK" in resp or "OK" == resp):
            print("Time set OK at {} baud.".format(baud))
            return
        # если получили сырые данные, но содержат слова вроде "BAD FORMAT" или "ERR" — выводим и прекращаем
        if resp and ("BAD FORMAT" in resp or "ERR" in resp or "Received:" in resp):
            print("MCU responded (non-OK):", resp)
            return

        # иначе — попробуем следующую скорость
        print("No clear OK reply at {} baud, trying next if available...".format(baud))

    print("Tried baud rates: {} but did not get clear OK reply from MCU.".format(tried))
    print("If MCU still uses default time, check sketch Serial.begin(...) and that monitor_speed in platformio.ini matches it.")

env.AddPostAction("upload", on_upload_finished)
