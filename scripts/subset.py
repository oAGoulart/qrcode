def xsubset(c: int) -> str:
  alpha = [0x20, 0x24, 0x25, 0x2A, 0x2B, 0x2D, 0x2E, 0x2F, 0x3F]
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
  next = xsegment(v, "alpha")
  if next >= 6 and xsubset(v[next]) == "byte":
    initial = "alpha"
  else:
    next = xsegment(v, "num")
    if next < 4 and xsubset(v[next]) == "byte":
      pass
    elif next < 7 and xsubset(v[next]) == "alpha":
      initial = "alpha"
    else:
      initial = "num"

print(initial)
