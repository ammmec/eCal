from PIL import Image, ImageDraw, ImageFont

# === Constants ===
CANVAS_WIDTH = 480
CANVAS_HEIGHT = 800

MARGIN = 5
TOP_MARGIN = 0
BOTTOM_MARGIN = 0

RECT_WIDTH = CANVAS_WIDTH - (11*6+MARGIN*2 + MARGIN*2)
RECT_HEIGHT = 61
CLASSES_LENGTH = 0

LEFT_COLUMN_WIDTH = 11*6+MARGIN*2
RIGHT_COLUMN_START = LEFT_COLUMN_WIDTH+RECT_WIDTH+MARGIN*2
TOP_ROW = 0
BOTTOM_ROW = 0

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
    x_start = LEFT_COLUMN_WIDTH + MARGIN
    y_start = position * RECT_HEIGHT + TOP_MARGIN
    width = RECT_WIDTH
    height = RECT_HEIGHT * duration
    color = RED if position == 2 else BLACK

    # Draw dotted pattern background
    for x in range(x_start, x_start + width):
        for y in range(y_start + MARGIN, y_start + height - MARGIN):
            draw.point((x, y), fill=color if (x % 3 == 0 and y % 3 == 0) else WHITE)

    # Border around the class block
    draw.rectangle((x_start, y_start + MARGIN, x_start + width, y_start + height - MARGIN), outline=color, width=1)

    # Center the text
    bbox = draw.textbbox((0, 0), label, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]

    text_x = x_start + (width - text_width) // 2
    text_y = y_start + (height - text_height) // 2 - text_height
    draw.text((text_x, text_y), label, font=font, fill=BLACK, stroke_width=3, stroke_fill=WHITE)

def draw_hour_labels(start, end):
    """Draws hour ranges (8-21h) on the left."""
    global CLASSES_LENGTH
    x = MARGIN
    bbox = draw.textbbox((0, 0), "A", font=font)
    text_height = bbox[3] - bbox[1]
    y = TOP_MARGIN + (RECT_HEIGHT-text_height)//2 - text_height
    pos = 1
    for hour in range(start, end):
        draw.text((x, y), f"{hour}-{hour + 1}h", font=font, fill=BLACK)
        y += RECT_HEIGHT
        if hour != 20:
            draw.line((0, RECT_HEIGHT * pos + TOP_MARGIN, RIGHT_COLUMN_START, RECT_HEIGHT * pos + TOP_MARGIN), fill=BLACK)
        pos += 1
    CLASSES_LENGTH = RECT_HEIGHT * (pos-1) + TOP_MARGIN

def draw_layout_lines():
    """Draws vertical grid lines."""
    draw.line((RIGHT_COLUMN_START, TOP_MARGIN, RIGHT_COLUMN_START, CLASSES_LENGTH), fill=BLACK)
    draw.line((LEFT_COLUMN_WIDTH, TOP_MARGIN, LEFT_COLUMN_WIDTH, CLASSES_LENGTH), fill=BLACK)

def place_qr_code(x, y):
    """Places the QR code and a label below it."""
    qr = Image.open("icons\\qr.png").convert("RGB")
    qr_size = (125, 125)
    qr = qr.resize(qr_size, resample=Image.NEAREST)
    
    # Position QR at the bottom center between columns
    qr_x = (x - qr_size[0]) // 2
    qr_y = y - qr_size[1] - BOTTOM_MARGIN - MARGIN
    image.paste(qr, (qr_x, qr_y))
    
    # Draw label above QR
    label = "Horari sencer:"
    bbox = draw.textbbox((0, 0), label, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    text_x = (x - text_width) // 2
    text_y = qr_y - text_height - MARGIN*3
    draw.text((text_x, text_y), label, font=font, fill=BLACK)

    text_y -= text_height+MARGIN
    global BOTTOM_ROW
    BOTTOM_ROW = text_y
    draw.line((0, text_y, CANVAS_WIDTH, text_y), fill=BLACK)

def current_next_class():
    text = "Classe en curs:"
    bbox = draw.textbbox((0, 0), text, font=font)
    text_height = bbox[3] - bbox[1]
    text_x = RIGHT_COLUMN_START + 2*MARGIN
    text_y = MARGIN+TOP_MARGIN
    draw.text((text_x, text_y), text, font=font, fill=BLACK)
    text_y += text_height+MARGIN*2

    text = "SO2 20 T"
    draw.text((text_x, text_y), text, font=font, fill=BLACK)
    text_y += text_height+MARGIN*2
    draw.text((text_x, text_y), "9:00-10:00", font=font, fill=BLACK)
    text_y += text_height+MARGIN*4

    text = "Pròxima classe:"
    draw.text((text_x, text_y), text, font=font, fill=BLACK)
    text_y += text_height+MARGIN*2

    text = "CI 10 T"
    draw.text((text_x, text_y), text, font=font, fill=BLACK)
    text_y += text_height+MARGIN*2
    draw.text((text_x, text_y), "20:00-21:00", font=font, fill=BLACK)
    text_y += text_height+MARGIN*4
    
    global TOP_ROW 
    TOP_ROW = text_y
    draw.line((RIGHT_COLUMN_START, text_y, CANVAS_WIDTH, text_y), fill=BLACK)

def draw_announcements(msg, x, y):
    text_x = x
    text_y = y
    icon_size = 15
    bbox = draw.textbbox((0, 0), msg, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    draw.text((text_x+icon_size+MARGIN, text_y-text_height+icon_size), "Avisos:", font=font, fill=BLACK)
    img = Image.open("icons\\avisos.png").convert("RGB")
    img = img.resize((15, 15), resample=Image.NEAREST)
    image.paste(img, (text_x, text_y))

    text_y += icon_size+MARGIN*2
    if (msg == None):
        msg = "No hi ha cap avís"
        bbox = draw.textbbox((0, 0), msg, font=font)
        text_width = bbox[2] - bbox[0]
        text_height = bbox[3] - bbox[1]
        text_x = (CANVAS_WIDTH + RIGHT_COLUMN_START - text_width) // 2
        text_y = (BOTTOM_ROW-TOP_ROW)//2 + text_y
        draw.text((text_x, text_y), line, font=font, fill=BLACK)
    else:
        words = msg.split()
        lines = []  # This will hold the resulting lines
        current_line = ""  # This will build each line
        max_width = CANVAS_WIDTH-(x+MARGIN*6)

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
draw_hour_labels(8, 14)
draw_layout_lines()

# Draw some example class blocks
draw_class(0, "FM 10 T", 1)
draw_class(1, "DSBM 10 L", 1)
draw_class(2, "SO2 20 T", 2)
# draw_class(12, "CI 10 T", 1)

current_next_class()
place_qr_code(CANVAS_WIDTH, CANVAS_HEIGHT)
draw_announcements("Aquesta aula quedarà automàticament tancada quan no hi hagi classe.", MARGIN, CLASSES_LENGTH+2*MARGIN)

# === Final Output ===
image.save("simulated.png")
image.show()
