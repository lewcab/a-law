# :musical_note: A-Law Compression/Decompression

Implementation of A-Law compression and decompression in C on the Cora Z7-07S platform.

## :open_file_folder: Overview

- `440_w_board_files/`: Vivado files.
- `inputs/`: Input file examples. `speech.wav` used for profiling
- `outputs/`: Example output files.
- `profiles/`: gprof results.
- `aLaw.c`: A-Law encoding and decoding functions.
- `main.c`: Main program to parse file and write compressed output.
- `memory_generator.c`: Program to generate LUT values.

## :book: Resources

- A-Law
  - [Algorithm Details](https://en.wikipedia.org/wiki/A-law_algorithm)
  - [G.711](https://en.wikipedia.org/wiki/G.711)
- Cora Z7-07S (FPGA)
  - [Reference Manual](https://digilent.com/reference/programmable-logic/cora-z7/reference-manual)
  - [ARM Cortex-A9](https://developer.arm.com/Processors/Cortex-A9)
- ARM NEON (SIMD)
  - [Documentation](https://developer.arm.com/documentation/den0018/a/NEON-Microarchitecture/The-Cortex-A9-processor?lang=en)
  - [Intrinsics Search](https://developer.arm.com/architectures/instruction-sets/intrinsics)
