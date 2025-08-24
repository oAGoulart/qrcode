import math
import sys

def generate_indexes(version: int) -> None:
  if not (version > 0 and version < 6):
    raise ValueError("unsupported version.")
  order = 4 * version + 17
  num_bytes = [26, 44, 70, 100, 134]
  num_bits = num_bytes[version - 1] * 8 + (7 if version > 1 else 0)

  def should_move(column: int) -> bool:
    return column == 6

  def should_flip(row: int, column: int, direction: int) -> bool:
    if direction == -1:
      return row == 0 or (row == 9 and (column < 9 or column > order - 9))
    else:
      return row == order - 1 or (row == order - 9 and column < 9)

  def should_skip(row: int, column: int) -> bool:
    if row == 6:
      return True
    if version > 1:
      if (row > order - 10 and row < order - 4) and\
         (column > order - 10 and column < order - 4):
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
