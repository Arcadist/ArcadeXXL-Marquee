from PIL import Image
import os

width = 112
height = 8
size = (width, height)

im = Image.new('RGB', size)

def ran():
    return os.urandom(width*height)

for n in range(100):
    pixels = zip(ran(), ran(), ran())
    im.putdata(list(pixels))
    outfile = os.path.join('images', str(n) + '.png')
    im.save(outfile)