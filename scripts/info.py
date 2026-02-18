import csv
import sys

def generate_info(filename: str) -> None:
  with open(filename, mode="r") as file:
    lines = csv.DictReader(file)
    for line in lines:
      print(f"  .short {line['length']}")
      print(f"  .byte {line['ecc_per_block']}, ", end="")
      print(f"{line['g1_blocks']}, ", end="")
      print(f"{line['g2_blocks'] or '0'}, ", end="")
      print(f"{line['data_count_g1_block']}, ", end="")
      print(f"{line['data_count_g2_block'] or '0'}")

if __name__ == "__main__":
  if len(sys.argv) < 2:
    raise ValueError("not enough arguments")
  generate_info(sys.argv[1])
