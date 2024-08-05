# create cosinus lookup tables
import math

STEP = 10

print("int cosLookup[] = {\\")
for i in range(0, 360, STEP):
    lookup_element = math.cos(math.radians(i))
    lookup_element = round(lookup_element, 4)

    if i + STEP >= 360:
        print("   FLOATTOFIX({})\\".format(lookup_element))
    else:
        print("   FLOATTOFIX({}),\\".format(lookup_element))
print("};")