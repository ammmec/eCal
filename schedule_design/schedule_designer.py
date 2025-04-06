from PIL import Image, ImageDraw, ImageFont

# === Constants ===
RECT_HEIGHT = 61
RECT_WIDTH = 200
CANVAS_WIDTH = 480
CANVAS_HEIGHT = 800
LEFT_COLUMN_WIDTH = 70
RIGHT_COLUMN_START = 280

# Colors
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
RED = (255, 0, 0)

# === Setup ===
image = Image.new('RGB', (CANVAS_WIDTH, CANVAS_HEIGHT), WHITE)
draw = ImageDraw.Draw(image)
font = ImageFont.truetype("FreeMonoBold.ttf", 16)

# === Functions ===
def draw_class(position, label, duration):
    """Draws a class block at a given position with a label and duration."""
    x_start = LEFT_COLUMN_WIDTH + 5
    y_start = position * RECT_HEIGHT + 4
    width = RECT_WIDTH
    height = RECT_HEIGHT * duration
    color = RED if position % 2 == 0 else BLACK

    # Draw dotted pattern background
    for x in range(x_start, x_start + width):
        for y in range(y_start + 5, y_start + height - 5):
            draw.point((x, y), fill=color if (x % 3 == 0 and y % 3 == 0) else WHITE)

    # Border around the class block
    draw.rectangle((x_start, y_start + 5, x_start + width, y_start + height - 5), outline=color, width=1)

    # Center the text
    bbox = draw.textbbox((0, 0), label, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]

    text_x = x_start + (width - text_width) // 2
    text_y = y_start + (height - text_height) // 2 - 10
    draw.text((text_x, text_y), label, font=font, fill=BLACK, stroke_width=3, stroke_fill=WHITE)

def draw_hour_labels():
    """Draws hour ranges (8-21h) on the left."""
    x = 5
    pos = 0
    for hour in range(8, 21):
        bbox = draw.textbbox((0, 0), "A", font=font)
        text_height = bbox[3] - bbox[1]
        y = 4 + (RECT_HEIGHT - text_height) // 2 - 10 + RECT_HEIGHT * pos
        draw.text((x, y), f"{hour}-{hour + 1}h", font=font, fill=BLACK)
        if hour > 8:
            draw.line((0, RECT_HEIGHT * pos + 4, RIGHT_COLUMN_START, RECT_HEIGHT * pos + 4), fill=BLACK)
        pos += 1

def draw_layout_lines():
    """Draws vertical grid lines."""
    draw.line((RIGHT_COLUMN_START, 4, RIGHT_COLUMN_START, CANVAS_HEIGHT - 3), fill=BLACK)
    draw.line((LEFT_COLUMN_WIDTH, 4, LEFT_COLUMN_WIDTH, CANVAS_HEIGHT - 3), fill=BLACK)

def place_qr_code():
    """Places the QR code and a label below it."""
    qr = Image.open("qr.png").convert("RGB")
    qr_size = (125, 125)
    qr = qr.resize(qr_size, resample=Image.NEAREST)
    
    # Position QR at the bottom center between columns
    qr_x = (CANVAS_WIDTH + RIGHT_COLUMN_START - qr_size[0]) // 2
    qr_y = CANVAS_HEIGHT - qr_size[1] - 10
    image.paste(qr, (qr_x, qr_y))
    
    # Draw label above QR
    label = "Horari sencer:"
    bbox = draw.textbbox((0, 0), label, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    text_x = (CANVAS_WIDTH + RIGHT_COLUMN_START - text_width) // 2
    text_y = qr_y - text_height - 20
    draw.text((text_x, text_y), label, font=font, fill=BLACK)

    text_y += text_height - 31
    draw.line((RIGHT_COLUMN_START, text_y, CANVAS_WIDTH, text_y), fill=BLACK)

def current_next_class():
    text = "Classe en curs:"
    bbox = draw.textbbox((0, 0), text, font=font)
    text_height = bbox[3] - bbox[1]
    text_x = RIGHT_COLUMN_START + 10
    text_y = 5
    draw.text((text_x, text_y), text, font=font, fill=BLACK)
    text_y += text_height + 12

    text = "SO2 20 T"
    draw.text((text_x, text_y), text, font=font, fill=BLACK)
    text_y += text_height + 10
    draw.text((text_x, text_y), "9:00-10:00", font=font, fill=BLACK)
    text_y += text_height + 20

    text = "Pròxima classe:"
    draw.text((text_x, text_y), text, font=font, fill=BLACK)
    text_y += text_height + 12

    text = "CI 10 T"
    draw.text((text_x, text_y), text, font=font, fill=BLACK)
    text_y += text_height + 10
    draw.text((text_x, text_y), "20:00-21:00", font=font, fill=BLACK)
    text_y += text_height + 20

    draw.line((RIGHT_COLUMN_START, text_y, CANVAS_WIDTH, text_y), fill=BLACK)

def draw_messages(msg):
    text_x = RIGHT_COLUMN_START + 15
    text_y = 160
    draw.text((text_x+25, text_y-5), "Avisos:", font=font, fill=BLACK)
    img = Image.open("avisos.png").convert("RGB")
    img = img.resize((15, 15), resample=Image.NEAREST)
    image.paste(img, (text_x, text_y))

    text_y += 20
    if (msg == None):
        msg = "No hi ha cap avís"
        bbox = draw.textbbox((0, 0), msg, font=font)
        text_width = bbox[2] - bbox[0]
        text_x = (CANVAS_WIDTH + RIGHT_COLUMN_START - text_width) // 2
        text_y = 740 // 2
        draw.text((text_x, text_y), line, font=font, fill=BLACK)
    else:
        words = msg.split()
        lines = []  # This will hold the resulting lines
        current_line = ""  # This will build each line
        max_width = CANVAS_WIDTH-(RIGHT_COLUMN_START+20)

        for word in words:
            # Check if adding the word to the current line exceeds the max width
            if (len(current_line) + len(word) + 3)*9 <= max_width:
                # If it fits, add the word to the current line (with a space if necessary)
                if current_line:
                    current_line += " "
                current_line += word
            else:
                # If it doesn't fit, push the current line to the lines list and start a new one
                lines.append(current_line)
                current_line = word  # Start the new line with the current word

        # Add the last line (if any)
        if current_line:
            lines.append(current_line)

        bbox = draw.textbbox((0, 0), msg, font=font)
        text_height = bbox[3] - bbox[1]

        for row, line in enumerate(lines):
            draw.text((text_x, text_y+row*text_height), line, font=font, fill=BLACK)

# === Drawing ===
draw_hour_labels()
draw_layout_lines()

# Draw some example class blocks
draw_class(0, "FM 10 T", 1)
draw_class(1, "DSBM 10 L", 1)
draw_class(2, "SO2 20 T", 2)
draw_class(12, "CI 10 T", 1)

current_next_class()
draw_messages("Crec que el que faré serà enviar els avisos ja separats, així és més fàcil fer el word wrap")
place_qr_code()

# === Final Output ===
image.save("simulated.png")
image.show()
