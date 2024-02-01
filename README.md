# Filter - Transform Your BMP Images

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://https://github.com/Melkor-1/Filter/edit/main/LICENSE)

Filter is a C program that allows you to apply powerful filters to BMP images. 

## Table of Contents

- [Usage](#usage)
- [Building](#building)
- [Installation](#installation)
- [Contributing](#contributing)

## Usage

```shell
Usage: filter [OPTIONS] [FILE]
```
Transform your BMP images with powerful filters.

### Options:
*  -s, --sepia          Apply a sepia filter for a warm, vintage look.
*  -r, --reverse        Create a horizontal reflection for a mirror effect.
*  -g, --grayscale      Convert the image to classic greyscale.
*  -b, --blur           Add a soft blur to the image.
*  -o, --ouptput=FILE   Writes the output to the specified file.
*  -h, --help           Display this message and exit.

## Building 

1. Clone the repository:

```shell
git clone https://github.com/your-username/filter-project.git
```
2. Compile the program:
```shell
make
```
3. `cd` to the `bin` directory and run the program:
```shell
cd bin
./filter --help
```
## Installation:

```shell
sudo make install
```
By default, the program is installed to `/usr/local/bin`. The makefile can be
configured to change the path.

## Contributing

If you want to contribute to this project or report issues, please create a new issue or pull request on GitHub.
