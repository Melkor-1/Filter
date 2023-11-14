# Filter - Transform Your BMP Images

Filter is a C program that allows you to apply powerful filters to BMP images. This project is designed to demonstrate image processing and the usage of command-line arguments.

## Table of Contents

- [Usage](#usage)
- [Installation](#installation)
- [Contributing](#contributing)
- [License](#license)

## Usage

```shell
Usage: filter [OPTIONS] <infile> <outfile>
```
Transform your BMP images with powerful filters.

### Options:
*  -s, --sepia       Apply a sepia filter for a warm, vintage look.
*  -r, --reverse     Create a horizontal reflection for a mirror effect.
*  -g, --grayscale   Convert the image to classic greyscale.
*  -b, --blur        Add a soft blur to the image.
*  -h, --help        Display this message and exit.

## Installation

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

## Contributing

If you want to contribute to this project or report issues, please create a new issue or pull request on GitHub.

## License

This project is open-source and available under the MIT License. See the [LICENSE](LICENSE) file for details.
