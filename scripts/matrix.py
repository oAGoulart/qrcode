import math

def print_indexes(order, check):
  for row in range(0,order):
    for col in range(0,order):
      i = row * order + col
      if i in check:
        print("0000 ", end='')
      else:
        print(f"{i:04d} ", end='')
    print("")

def dups(v):
  for e in v:
    if v.count(e) > 1:
      print(f"{e}")

def maskinfo(order):
  for i in range(15):
    if i < 8:
      index = 8*order+i
      if i > 5:
        index += 1
      #print(index)
    else:
      index = (15-i)*order+8
      if i > 8:
        index -= order
      #print(index)
    if i < 7:
      index=order*(order-i-1)+8
      print(index)
    else:
      index=order*8+order-8+i-7
      print(index)


def xorp(v, order, pattern):
  for i, e in enumerate(v):
    row = math.floor(e / order)
    col = e % order
    print(f"{row} {col} |", end='')
    if (pattern == 0):
      print(f"{(row + col) % 2 == 0},", end='')
    elif (pattern == 1):
      print(f"{row % 2 == 0},", end='')
    elif (pattern == 2):
      print(f"{col % 3 == 0},", end='')
    elif (pattern == 3):
      print(f"{(row + col) % 3 == 0},", end='')
    elif (pattern == 4):
      print(f"{(math.floor(row / 2) + math.floor(col / 3)) % 2 == 0},", end='')
    elif (pattern == 5):
      print(f"{((row * col) % 2) + ((row * col) % 3) == 0},", end='')
    elif (pattern == 6):
      print(f"{(((row * col) % 2) + ((row * col) % 3) ) % 2 == 0},", end='')
    elif (pattern == 7):
      print(f"{(((row + col) % 2) + ((row * col) % 3) ) % 2 == 0},", end='')
  print("")

v = [
  440,439,419,418,398,397,377,376,356,355,335,334,
  314,313,293,292,272,271,251,250,230,229,209,208,
  207,206,228,227,249,248,270,269,291,290,312,311,
  333,332,354,353,375,374,396,395,417,416,438,437,
  436,435,415,414,394,393,373,372,352,351,331,330,
  310,309,289,288,268,267,247,246,226,225,205,204,
  203,202,224,223,245,244,266,265,287,286,308,307,
  329,328,350,349,371,370,392,391,413,412,434,433,
  432,431,411,410,390,389,369,368,348,347,327,326,
  306,305,285,284,264,263,243,242,222,221,201,200,
  180,179,159,158,117,116,96,95,75,74,54,53,33,32,12,11,
  10,9,31,30,52,51,73,72,94,93,115,114,157,156,178,
  177,199,198,220,219,241,240,262,261,283,282,304,
  303,325,324,346,345,367,366,388,387,409,408,430,429,
  260,259,239,238,218,217,197,196,
  194,193,215,214,236,235,257,256,
  255,254,234,233,213,212,192,191,
  190,189,211,210,232,231,253,252
]

#print_indexes(21, [])
#dups(v)
#xorp(v, 21, 0)
maskinfo(21)
