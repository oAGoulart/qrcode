import math
import sys

# NOTE: starts at Version 2
align_patterns = [
  [18],[22],[26],[30],[34],
  [22,38],[24,42],[26,46],[28,50],[30,54],[32,58],[34,62],
  [26,46,66],[26,48,70],[26,50,74],[30,54,78],[30,56,82],[30,58,86],[34,62,90],
  [28,50,72,94],[26,50,74,98],[30,54,78,102],[28,54,80,106],[32,58,84,110],
  [30,58,86,114],[34,62,90,118],[26,50,74,98,122],[30,54,78,102,126],
  [26,52,78,104,130],[30,56,82,108,134],[34,60,86,112,138],[30,58,86,114,142],
  [34,62,90,118,146],[30,54,78,102,126,150],[24,50,76,102,128,154],
  [28,54,80,106,132,158],[32,58,84,110,136,162],[26,54,82,110,138,166],
  [30,58,86,114,142,170]
]
assert(len(align_patterns) == 39)

def is_pattern(version: int, row: int, col: int) -> bool:
  if not (2 <= version <= 40):
    return False
  version -= 2
  for pattern in align_patterns[version]:
    if (pattern - 2 <= row <= pattern + 2) and\
       (pattern - 2 <= col <= pattern + 2):
      return True
  return False

num_bytes = [
  26,44,70,100,134,172,196,242,292,346,404,466,532,581,655,733,815,901,991,
  1085,1156,1258,1364,1474,1588,1706,1828,1921,2051,2185,2323,2465,2611,
  2761,2876,3034,3196,3362,3532,3706
]
assert(len(num_bytes) == 40)

def generate_indexes(version: int) -> None:
  if not (1 <= version <= 40):
    raise ValueError("unsupported version.")
  order = 4 * version + 17
  num_bits = num_bytes[version - 1] * 8 + (7 if version > 1 else 0)

  def should_move(col_: int) -> bool:
    return col_ == 6

  def should_flip(row_: int, col_: int, dir_: int) -> bool:
    if dir_ == -1:
      return row_ == 0 or (row_ == 9 and (col_ < 9 or col_ > order - 9))
    else:
      return row_ == order - 1 or (row_ == order - 9 and col_ < 9)

  def is_vinfo(row_: int, col_: int) -> bool:
    if version < 7:
      return False
    return (row_ < 6 and col_ > order - 12) or\
           (col_ < 6 and row_ > order - 12)

  def should_skip(row_: int, col_: int) -> bool:
    return row_ == 6 or is_pattern(version, row_, col_) or\
           is_vinfo(row_, col_)

  direction = -1 # -1=up 1=down
  idx = order * order - 1
  flipped = False
  print("  .short ", end='')
  for i in range(num_bits):
    if flipped:
      flipped = False
      continue
    row = math.floor(idx / order)
    column = idx % order

    def next_index(index: int) -> int:
      k = 1 if column > 6 else 0
      if column % 2 == k:
        return index + direction * (order + direction)
      else:
        return index - 1

    if should_move(column):
      idx -= 1
      row = math.floor(idx / order)
      column = idx % order
    if should_flip(row, column, direction):
      if i + 2 == num_bits:
        print(f"{idx},{idx - 1}")
        break
      print(f"{idx},{idx - 1},", end='')
      flipped = True
      if row == order - 1 and column == 10:
        idx = (order - 9) * order + 8
      else:
        idx -= 2
      direction *= -1
      continue
    while should_skip(row, column):
      idx = next_index(idx)
      row = math.floor(idx / order)
      column = idx % order
    print(f"{idx},", end='')
    idx = next_index(idx)

if __name__ == "__main__":
  if len(sys.argv) < 2:
    raise ValueError("not enough arguments")
  generate_indexes(int(sys.argv[1]))
