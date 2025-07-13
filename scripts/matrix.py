import math

def print_indexes(order, check):
  for row in range(order):
    for col in range(order):
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

v = []

print_indexes(33, [])
#dups(v)
#xorp(v, 21, 0)
#maskinfo(21)
