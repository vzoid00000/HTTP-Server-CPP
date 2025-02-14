# HTTP-Server-CPP

A very simple and poorly written HTTP server in C++.

## Overview

HTTP-Server-CPP is a basic showcase demonstrating an understanding of how HTTP works. It is incomplete and currently only supports GET requests. PUT, POST, and DELETE methods are not yet implemented. This server is exclusively for Windows as it does not use any third-party libraries, relying only on the WinAPI Winsock for networking.

## Features

- Supports HTTP 1.1 (Only GET requests for now)
- Simple file serving
- No third-party dependencies, only uses WinAPI Winsock
- Generates a `settings.txt` file for configuration

## Requirements

To build HTTP-Server-CPP, you need:

- Window 10/11
- Visual Studio

## Building and Running the Server

### 1. Clone the Repository

```sh
$ git clone https://github.com/yourusername/HTTP-Server-CPP.git
$ cd HTTP-Server-CPP
```

### 2. Open in Visual Studio

- Open the project in Visual Studio.
- Build the solution.

### 3. Run the Server

- Place a folder called `files` next to the built `.exe`.
- Inside `files`, put all the necessary assets such as HTML, CSS, JS, images, and JSON files.
- The first time you start the `.exe`, it will generate a `settings.txt` file with default settings:

  ```
  port: 80
  documentsPath: files
  ```

- Modify `settings.txt` to adjust server settings.

## Usage

- The server listens on `http://localhost:80` by default.
- The `files` directory serves as the document root.

## License

This project is licensed under the MIT License.
