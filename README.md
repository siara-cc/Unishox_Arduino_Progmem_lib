# Compression and decompression Library for use with Arduino Flash Memory (PROGMEM)

[Unishox](https://github.com/siara-cc/Unishox) is a technique used for compressing and decompressing short Unicode strings. This library allows storing of compressed text strings in Arduino Program Memory and provides API to retrieve each string individually using index.

Unishox can achieve upto 60% compression depending on the composition of text.  This can be especially useful in constrained environments like Arduino Uno where only 32kb of Flash memory is available.

To find out how Unishox works, read [this article](https://github.com/siara-cc/Unishox/blob/master/Unishox_Article_1.pdf?raw=true).

## Applications

- Displaying descriptive error or warning messages
- Storing UTF-8 file content (such as html)
- Transferring compressed content over low speed networks

## Usage

First, the strings that need to compressed should to be listed in a file (usually with .txt extension).

Download [Unishox](https://github.com/siara-cc/Unishox) and compile it using instructions provided in the repository. Then create compressed PROGMEM header using following command:

```
./unishox1 -g <text_file> <output_header_name>
```

or

```
./unishox -G <text_file> <output_header_name>
```

The command with capital G provides more compression taking into account repeating texts across the file.

For example, if we use the file `Proverbs1.txt` given in the repository and the command `./unishox1 -g Proverbs.txt Proverbs_Uni`, a file by name `proverbs.h` will be generated.

This file can be copied alongside any `.ino` file and included.  Then the `Unishox1_pgm_decompress()` API from `unishox_progmem.h` in this library can be called to uncompress the strings.

Please try the examples provided along with this library to see how it works.

## Compatibility

This library can be used with Arduino Duemilanove, Uno upwards.  It is useful only if saving by compressing text content is over 3000 bytes since the decompressor takes as much space.

## Limitations

- Unicode (UTF-8) text is compressed well, but binary data (ASCII 0-31, ASCII 128-255) is not compressed well.

## Examples and screenshots

### Proverbs

This example demonstrates de-compression of compressed proverbs from different languages. Source: Wikipedia

![](ss_proverbs.png?raw=true)

## Issues

If you face any problems, please create issue on GitHub.
