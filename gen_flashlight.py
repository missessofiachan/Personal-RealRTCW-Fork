from PIL import Image, ImageDraw
import sys

def create_flashlight_icon(filename):
    # Create a 64x64 image with a transparent background
    img = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # White color
    color = (255, 255, 255, 255)
    
    # Draw body (handle)
    # coordinates: x1, y1, x2, y2
    draw.rectangle([8, 26, 36, 38], fill=color)
    
    # Draw head (cone)
    draw.polygon([(36, 26), (50, 18), (50, 46), (36, 38)], fill=color)
    
    # Draw button
    draw.rectangle([20, 22, 26, 26], fill=color)
    
    # Draw light beam (optional, but a little indicator is nice)
    draw.polygon([(52, 22), (62, 14), (62, 22)], fill=color)
    draw.polygon([(52, 42), (62, 50), (62, 42)], fill=color)
    draw.rectangle([54, 30, 62, 34], fill=color)

    # Flip horizontally
    img = img.transpose(Image.FLIP_LEFT_RIGHT)

    # Save as TGA
    img.save(filename)

if __name__ == '__main__':
    create_flashlight_icon(sys.argv[1])
