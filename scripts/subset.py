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

v = [0x30, 0x31, 0x32, 0x33, 0x34, 0x41, 0x42, 0x65, 0x66]
initial = "byte"
if xsubset(v[0]) != "byte":
  seg = xsegment(v, "alpha")
  if seg >= 6 and xsubset(v[seg]) == "byte":
    initial = "alpha"
  else:
    seg = xsegment(v, "num")
    if seg < 4 and xsubset(v[seg]) == "byte":
      pass
    elif seg < 7 and xsubset(v[seg]) == "alpha":
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
    if seg - i >= 13 and xsubset(v[seg]) == "alpha":
      mode = "num"
  else:
    #encode
    print(mode)
    #then ->
    seg = xsegment(v[i:], "num")
    if seg - i >= 6:
      subset = xsubset(v[seg])
      if subset == "byte" or subset == "alpha":
        mode = "num"
      continue
    seg = xsegment(v[i:], "alpha")
    if seg - i >= 11 and xsubset(v[seg]) == "byte":
      mode = "alpha"
