from PIL import Image, ImageDraw, ImageFont

# === Constants ===
CANVAS_WIDTH = 24
CANVAS_HEIGHT = 6

# Colors
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
RED = (255, 0, 0)

# === Setup ===
image = Image.new('RGB', (CANVAS_WIDTH, CANVAS_HEIGHT), WHITE)
draw = ImageDraw.Draw(image)

# Draw dotted pattern background
for i in range(0, CANVAS_WIDTH):
    for j in range(0, CANVAS_HEIGHT):
        if (i%3 == 0 and j%3 == 0):
            draw.point((i, j), fill=BLACK)

# === Final Output ===
image.save("rect.png")
image.show()
