
import pygame
import serial
import time
import threading


# ============================================================
# SERIAL CONFIGURATION
# ============================================================

SERIAL_PORTS = [
    "COM13",
    "COM15"
]

BAUD_RATE = 115200

RECONNECT_DELAY = 2.0


# ============================================================
# PYGAME CONFIGURATION
# ============================================================

WINDOW_WIDTH = 1000
WINDOW_HEIGHT = 600


# ============================================================
# TRAFFIC STATE
# ============================================================

traffic = {
    "cars": [0, 0, 0],
    "green_lane": 0,
    "countdown": 0
}


# ============================================================
# CONNECTION STATE MACHINE
# ============================================================

CONNECTION_DISCONNECTED = 0
CONNECTION_TRYING = 1
CONNECTION_CONNECTED = 2

connection_state = CONNECTION_DISCONNECTED

connected_port = None


# ============================================================
# SERIAL CONNECTION
# ============================================================

def try_connect():

    global connection_state
    global connected_port

    connection_state = CONNECTION_TRYING

    for port in SERIAL_PORTS:

        print(f"Trying {port}...")

        try:

            ser = serial.Serial(
                port,
                BAUD_RATE,
                timeout=1
            )

            connected_port = port
            connection_state = CONNECTION_CONNECTED

            print(f"Connected to {port}")

            return ser

        except serial.SerialException:

            print(f"{port} unavailable.")

    return None


# ============================================================
# SERIAL TASK
# ============================================================

def serial_task():

    global connection_state
    global connected_port

    while True:

        # ----------------------------------------------------
        # TRY TO CONNECT
        # ----------------------------------------------------

        ser = try_connect()

        if ser is None:

            connected_port = None
            connection_state = CONNECTION_DISCONNECTED

            print(
                f"No STM32 found. "
                f"Retrying in {RECONNECT_DELAY} seconds..."
            )

            time.sleep(RECONNECT_DELAY)

            continue


        # ----------------------------------------------------
        # CONNECTED
        # ----------------------------------------------------

        while True:

            try:

                line = ser.readline().decode(
                    "utf-8",
                    errors="ignore"
                ).strip()

                if not line:
                    continue


                # Expected packet:
                #
                # L0:12,L1:7,L2:21,G:1,T:18

                parts = line.split(",")

                if len(parts) != 5:
                    continue


                traffic["cars"][0] = int(
                    parts[0].split(":")[1]
                )

                traffic["cars"][1] = int(
                    parts[1].split(":")[1]
                )

                traffic["cars"][2] = int(
                    parts[2].split(":")[1]
                )

                traffic["green_lane"] = int(
                    parts[3].split(":")[1]
                )

                traffic["countdown"] = int(
                    parts[4].split(":")[1]
                )


            except (
                serial.SerialException,
                OSError
            ):

                # --------------------------------------------
                # CONNECTION LOST
                # --------------------------------------------

                print(
                    f"Connection lost: {connected_port}"
                )

                try:
                    ser.close()
                except Exception:
                    pass

                connected_port = None
                connection_state = CONNECTION_DISCONNECTED

                break


            except (
                ValueError,
                IndexError
            ):

                # Bad packet.
                # Ignore it and keep reading.
                continue


# ============================================================
# PYGAME INITIALIZATION
# ============================================================

pygame.init()

screen = pygame.display.set_mode(
    (WINDOW_WIDTH, WINDOW_HEIGHT)
)

pygame.display.set_caption(
    "Traffic Control Monitor"
)

clock = pygame.time.Clock()


# ============================================================
# FONTS
# ============================================================

font_title = pygame.font.Font(None, 42)
font_lane = pygame.font.Font(None, 32)
font_count = pygame.font.Font(None, 64)
font_small = pygame.font.Font(None, 26)


# ============================================================
# DRAW LANE
# ============================================================

def draw_lane(x, lane):

    title = font_lane.render(
        f"LANE {lane}",
        True,
        (255, 255, 255)
    )

    screen.blit(
        title,
        (x, 130)
    )


    # --------------------------------------------------------
    # TRAFFIC LIGHT POSITION
    # --------------------------------------------------------

    light_x = x + 55
    light_y = 190


    # --------------------------------------------------------
    # RED LIGHT
    # --------------------------------------------------------

    red_on = (
        traffic["green_lane"] != lane
    )

    pygame.draw.circle(
        screen,
        (220, 40, 40)
        if red_on
        else (60, 30, 30),
        (light_x, light_y),
        35
    )


    # --------------------------------------------------------
    # GREEN LIGHT
    # --------------------------------------------------------

    green_on = (
        traffic["green_lane"] == lane
    )

    pygame.draw.circle(
        screen,
        (40, 220, 80)
        if green_on
        else (30, 70, 40),
        (light_x, light_y + 90),
        35
    )


    # --------------------------------------------------------
    # CAR COUNT
    # --------------------------------------------------------

    count = font_count.render(
        str(traffic["cars"][lane]),
        True,
        (255, 255, 255)
    )

    screen.blit(
        count,
        (x + 40, 330)
    )


    cars_text = font_small.render(
        "CARS",
        True,
        (180, 180, 180)
    )

    screen.blit(
        cars_text,
        (x + 48, 400)
    )


# ============================================================
# START SERIAL THREAD
# ============================================================

thread = threading.Thread(
    target=serial_task,
    daemon=True
)

thread.start()


# ============================================================
# MAIN PYGAME LOOP
# ============================================================

running = True

while running:

    # --------------------------------------------------------
    # EVENTS
    # --------------------------------------------------------

    for event in pygame.event.get():

        if event.type == pygame.QUIT:

            running = False


    # --------------------------------------------------------
    # BACKGROUND
    # --------------------------------------------------------

    screen.fill(
        (20, 22, 28)
    )


    # --------------------------------------------------------
    # TITLE
    # --------------------------------------------------------

    title = font_title.render(
        "TRAFFIC CONTROL MONITOR",
        True,
        (255, 255, 255)
    )

    screen.blit(
        title,
        (270, 30)
    )


    # --------------------------------------------------------
    # CONNECTION STATUS
    # --------------------------------------------------------

    if connection_state == CONNECTION_CONNECTED:

        status = (
            f"STM32 CONNECTED [{connected_port}]"
        )

        status_color = (
            50, 220, 100
        )

    elif connection_state == CONNECTION_TRYING:

        status = (
            "SEARCHING FOR STM32..."
        )

        status_color = (
            240, 200, 50
        )

    else:

        status = (
            "STM32 DISCONNECTED - RETRYING"
        )

        status_color = (
            220, 60, 60
        )


    status_text = font_small.render(
        status,
        True,
        status_color
    )

    screen.blit(
        status_text,
        (20, 25)
    )


    # --------------------------------------------------------
    # LANES
    # --------------------------------------------------------

    draw_lane(120, 0)
    draw_lane(450, 1)
    draw_lane(780, 2)


    # --------------------------------------------------------
    # GREEN COUNTDOWN
    # --------------------------------------------------------

    countdown_text = font_small.render(
        f"GREEN COUNTDOWN: "
        f"{traffic['countdown']} s",
        True,
        (255, 255, 255)
    )

    screen.blit(
        countdown_text,
        (350, 500)
    )


    # --------------------------------------------------------
    # NEXT GREEN LANE
    # --------------------------------------------------------

    next_lane = (
        traffic["green_lane"] + 1
    ) % 3

    next_lane_text = font_small.render(
        f"NEXT GREEN: LANE {next_lane}",
        True,
        (255, 220, 80)
    )

    screen.blit(
        next_lane_text,
        (350, 540)
    )


    # --------------------------------------------------------
    # UPDATE DISPLAY
    # --------------------------------------------------------

    pygame.display.flip()

    clock.tick(30)


# ============================================================
# SHUTDOWN
# ============================================================

pygame.quit()
