# Convert JSON to a compatible string format for C++
# Use this until conversion can be done in MicroGUI

import sys

filename = sys.argv[1]

f = open(filename,'r+')
newfile = filename[0:filename.rfind(".")] + "_converted.json"
print("File found!\nOutput will be stored in a new file called " + newfile)
contents = f.read()
f.close()

converted = contents.replace('\n', '').replace('"', '\\"').replace('    ', '').replace('<div>', '\\n').replace('</div>', '')

f = open(newfile, "w")
f.write(converted)
f.close()