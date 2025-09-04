def xsubset(c: int) -> str:
  alpha = [0x20, 0x24, 0x25, 0x2A, 0x2B, 0x2D, 0x2E, 0x2F, 0x3A]
  if c >= 0x30 and c <= 0x39:
    return "num"
  if c in alpha or (c >= 0x41 and c <= 0x5A):
    return "alpha"
  return "byte"

def xsegment(v: list, subset: str) -> int:
  count = 0
  for e in v:
    if xsubset(e) != subset:
      break
    count += 1
  return count

def xseglen(version: int, it: int) -> int:
  ver = 0
  if version > 0: pass
  elif version > 9: ver = 1
  elif version > 26: ver = 2
  seg = [
    (6, 7, 8), (4, 4, 5), (7, 8, 9),
    (13, 15, 17), (6, 8, 9), (6, 7, 8), (11, 15, 16)
  ]
  return seg[it][ver]

v = [0x6F,0x61,0x67,0x6F,0x75,0x6C,0x61,0x72,0x74,0x2E,0x67,0x69,0x74,0x68,0x75,0x62,0x2E,0x69,0x6F,0x2F,0x72,0x61,0x6D,0x62,0x6C,0x65,0x73,0x2F,0x6B,0x65,0x65,0x70,0x2D,0x74,0x68,0x79,0x73,0x65,0x6C,0x66,0x2D,0x63,0x72,0x65,0x64,0x69,0x62,0x6C,0x65]
initial = "byte"
if xsubset(v[0]) != "byte":
  seg = xsegment(v, "alpha")
  if xsubset(v[seg]) == "byte" and seg >= xseglen(1, 0):
    initial = "alpha"
  else:
    seg = xsegment(v, "num")
    if xsubset(v[seg]) == "byte" and seg < xseglen(1, 1):
      pass
    elif xsubset(v[seg]) == "alpha" and seg < xseglen(1, 2):
      initial = "alpha"
    else:
      initial = "num"

print(initial)
mode = initial
for i in range(len(v)):
  #switch case
  if mode == "num":
    #encode
    print(mode)
    #then ->
    subset = xsubset(v[i + 1])
    if subset != "num":
      mode = subset
  elif mode == "alpha":
    #encode
    print(mode)
    #then ->
    subset = xsubset(v[i + 1])
    if subset == "byte":
      mode = subset
      continue
    seg = xsegment(v[i:], "num")
    if xsubset(v[seg]) == "alpha" and seg - i >= xseglen(1, 3):
      mode = "num"
  else:
    #encode
    print(mode)
    #then ->
    seg = xsegment(v[i:], "num")
    subset = xsubset(v[seg])
    if subset == "byte" and seg - i >= xseglen(1, 4):
      mode = "num"
      continue
    if subset == "alpha" and seg - i >= xseglen(1, 5):
      mode = "num"
      continue
    seg = xsegment(v[i:], "alpha")
    if subset == "byte" and seg - i >= xseglen(1, 6):
      mode = "alpha"
