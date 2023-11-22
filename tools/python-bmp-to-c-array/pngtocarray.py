import os, sys
from PIL import Image

path = "C://Users//parek//OneDrive//Desktop//lasertag assets"

f = open(path + "//header.h", "w")

for File in os.listdir(path):
    try:
        im = Image.open(path + "//" + File, 'r')
        pix = im.load()
        f.write("//" + File + " " + str(im.height) + "x" + str(im.width) + "px\n")
        f.write("const bool " + File[0:-4] + " [" + str(im.height) + "][" + str(im.width) + "] {\n")
        for hidx in range(im.height):
            for widx in range(im.width):
                if hidx == 0 and widx == 0:
                    f.write(str(pix[widx, hidx]))
                else:
                    f.write(", " + str(pix[widx, hidx]))

        f.write("\n};\n\n")
    except OSError:
        pass
