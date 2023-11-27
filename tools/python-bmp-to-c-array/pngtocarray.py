import os, sys
from PIL import Image

path = "C://Users//parek//OneDrive//Desktop//LTA"

f = open(path + "//header.h", "w")

for File in os.listdir(path):
    try:
        im = Image.open(path + "//" + File, 'r')
        pix = im.load()
        arrayDim = int((im.height * im.width) / 8)
        f.write("//" + File + " " + str(im.height) + "x" + str(im.width) + "px\n")
        f.write("const uint8_t " + File[0:-4] + "[" + str(arrayDim) + "] = {\n")
        idx = 0
        byte = ""
        for hidx in range(im.height):
            for widx in range(im.width):
                byte += str(pix[widx, hidx])
                idx += 1
                if idx % 8 == 0:
                    byteValue = hex(int(byte, 2))

                    if idx == 8:
                        f.write(str(byteValue))
                    else:
                        f.write(", " + str(byteValue))

                    byte = ""

        f.write("\n};\n\n")
    except OSError:
        pass
