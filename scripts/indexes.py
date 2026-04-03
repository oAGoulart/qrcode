import math
import sys

def visualize_module_path(version_: int, indexes_: list) -> None:
  # NOTE: keep imports here to use only in debug mode
  import matplotlib.pyplot as plt
  from collections import Counter

  order = 4 * version_ + 17
  x_coords = [idx % order for idx in indexes_]
  y_coords = [idx // order for idx in indexes_]

  # NOTE: checks for duplicates
  counts = Counter(indexes_)
  dup_indexes = [idx for idx, amount in counts.items() if amount > 1]
  dup_x = [idx % order for idx in dup_indexes]
  dup_y = [idx // order for idx in dup_indexes]

  plt.figure(figsize=(10, 10))
  sequence_numbers = list(range(len(indexes_)))
  sc = plt.scatter(x_coords, y_coords, c=sequence_numbers, cmap='viridis', 
                   s=40, marker='s', label='Modules')
  cbar = plt.colorbar(sc, label='Placement sequence', shrink=0.7)
  cbar.ax.text(0.5, 1.01, 'end', transform=cbar.ax.transAxes,
               ha='center', va='bottom', fontstyle='italic')
  cbar.ax.text(0.5, -0.02, 'start', transform=cbar.ax.transAxes,
               ha='center', va='top', fontstyle='italic')
  plt.plot(x_coords, y_coords, color='gray', linewidth=1, alpha=0.5)

  if dup_indexes:
    plt.scatter(dup_x, dup_y, c='red', s=120, marker='X',
                label=f'Duplicates({len(dup_indexes)})')
    print(f"DEBUG: Found {len(dup_indexes)} duplicated modules!")
  else:
    print("DEBUG: No duplicate indixes found.")

  plt.title(f"QR Code Version {version_} module path\n" +
            f"Total modules placed: {len(indexes_)}")
  plt.xlim(-1, order)
  plt.ylim(-1, order)
  plt.gca().invert_yaxis()
  plt.gca().set_aspect('equal', adjustable='box')
  plt.grid(True, linestyle='-', alpha=0.3, color='lightgray')
  plt.xticks(range(0, order))
  plt.yticks(range(0, order))
  plt.tick_params(axis='both', which='both', bottom=False, left=False,
                  labelbottom=False, labelleft=False)
  plt.legend(loc='upper left', bbox_to_anchor=(1, 1))
  plt.tight_layout(rect=[0, 0, 1, 0.92])
  plt.show()

def generate_xor_masks(version_: int, indexes_: list) -> None:
  order = 4 * version_ + 17

  def should_xor(row_: int, col_: int, pattern_: int) -> bool:
    if pattern_ == 0:
      return (row_ + col_) % 2 == 0
    elif pattern_ == 1:
      return row_ % 2 == 0
    elif pattern_ == 2:
      return col_ % 3 == 0
    elif pattern_ == 3:
      return (row_ + col_) % 3 == 0
    elif pattern_ == 4:
      return (math.floor(row_ / 2) + math.floor(col_ / 3)) % 2 == 0
    elif pattern_ == 5:
      return ((row_ * col_) % 2) + ((row_ * col_) % 3) == 0
    elif pattern_ == 6:
      return (((row_ * col_) % 2) + ((row_ * col_) % 3) ) % 2 == 0
    elif pattern_ == 7:
      return (((row_ + col_) % 2) + ((row_ * col_) % 3) ) % 2 == 0
    return False

  for pattern in range(8):
    cnt = 0
    ubyte = 0
    ubytes = []
    for i in indexes_:
      col = i % order
      row = (i - col) / order
      is_xor = should_xor(row, col, pattern)
      ubyte = ubyte | (is_xor << (7 - cnt))
      cnt = cnt + 1
      if cnt == 8:
        ubytes.append(ubyte)
        cnt = 0
        ubyte = 0
    print("  .byte " + ",".join(map(str, ubytes)))

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

align_patterns_coords = [
  [
    (col, row)\
    for col in ([6] + p)\
    for row in ([6] + p)\
    if (col, row) not in ((6, 6), (6, p[-1]), (p[-1], 6))
  ]
  for p in align_patterns
]

orders = [4 * i + 17 for i in range(2, 41)]

def is_pattern(version_: int, row: int, col: int) -> bool:
  if not (2 <= version_ <= 40):
    return False
  version_ -= 2
  for coords in align_patterns_coords[version_]:
    if (coords[0] - 2 <= col <= coords[0] + 2) and\
       (coords[1] - 2 <= row <= coords[1] + 2):
      return True
  return False

num_bytes = [
  26,44,70,100,134,172,196,242,292,346,404,466,532,581,655,733,815,901,991,
  1085,1156,1258,1364,1474,1588,1706,1828,1921,2051,2185,2323,2465,2611,
  2761,2876,3034,3196,3362,3532,3706
]
assert(len(num_bytes) == 40)

def remainder_bits(version_: int) -> int:
  if 2 <= version_ <= 6:
    return 7
  if 14 <= version_ <= 20 or 28 <= version_ <= 34:
    return 3
  if 21 <= version_ <= 27:
    return 4
  return 0

def generate_indexes(version_: int, mode_: str) -> None:
  order = 4 * version_ + 17
  num_bits = num_bytes[version_ - 1] * 8 + remainder_bits(version_)

  def is_vinfo(row_: int, col_: int) -> bool:
    if version_ < 7:
      return False
    return (row_ < 6 and col_ > order - 12) or\
           (col_ < 6 and row_ > order - 12)

  def should_move(col_: int) -> bool:
    return col_ == 6

  def should_flip(row_: int, col_: int, dir_: int) -> bool:
    if dir_ == -1:
      return row_ == 0 or (row_ == 9 and (col_ < 9 or col_ > order - 9))
    else:
      return row_ == order - 1 or (row_ == order - 9 and col_ < 9)

  def should_skip(row_: int, col_: int) -> bool:
    return row_ == 6 or is_pattern(version_, row_, col_) or is_vinfo(row_, col_)

  direction = -1 # -1=up 1=down
  idx = (order * order) - 1
  flipped = False
  modules_index = []
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
      modules_index.extend([idx, idx - 1])
      flipped = True
      if row == order - 1 and column == 10:
        idx = (order - 9) * order + 8
      else:
        idx -= 2
      direction *= -1
      continue
    # NOTE: makes sure 'i' is not incremented
    while should_skip(row, column):
      if should_flip(row, column, direction):
        idx -= 2
        direction *= -1
      else:
        idx = next_index(idx)
      row = math.floor(idx / order)
      column = idx % order
    modules_index.append(idx)
    idx = next_index(idx)
  modules_index = modules_index[:num_bits]
  if mode_ == "debug":
    visualize_module_path(version_, modules_index)
  elif mode_ == "masks":
    generate_xor_masks(version_, modules_index)
  else:
    print("  .short " + ",".join(map(str, modules_index)))

if __name__ == "__main__":
  count = len(sys.argv)
  mode = "none"
  if count < 2:
    raise ValueError("not enough arguments")
  version = int(sys.argv[1])
  if not (1 <= version <= 40):
    raise ValueError("unsupported version.")
  if count > 2:
    mode = sys.argv[2]
  if mode == "align" and version > 1:
    indexes = [
      ((row - 2) * orders[version - 2]) + (col - 2)\
      for col, row in align_patterns_coords[version - 2]
    ]
    indexes.append(0)
    print("  .short " + ",".join(map(str, indexes)))
  else:
    generate_indexes(version, mode)
