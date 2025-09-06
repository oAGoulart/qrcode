import math
import sys

def generate_indexes(version: int) -> None:
  if not (0 < version < 6):
    raise ValueError("unsupported version.")
  order = 4 * version + 17
  num_bytes = [26, 44, 70, 100, 134]
  num_bits = num_bytes[version - 1] * 8 + (7 if version > 1 else 0)

  def should_move(col_: int) -> bool:
    return col_ == 6

  def should_flip(row_: int, col_: int, dir_: int) -> bool:
    if dir_ == -1:
      return row_ == 0 or (row_ == 9 and (col_ < 9 or col_ > order - 9))
    else:
      return row_ == order - 1 or (row_ == order - 9 and col_ < 9)

  def should_skip(row_: int, col_: int) -> bool:
    if row_ == 6:
      return True
    if version > 1:
      if (order - 10 < row_ < order - 4) and\
         (order - 10 < col_ < order - 4):
        return True
    return False

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
