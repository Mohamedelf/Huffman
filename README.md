
# Huffman Compression Project - Full Stack Guide

This project combines a C backend for Huffman compression with a Node.js server and a React frontend.

## Project Structure


```
/
|-- src/                  # C source files (huffman.c, heap.c, etc.)
|-- components/           # React components (FileUploader.tsx)
|-- uploads/              # Created automatically by the server for temp files
|
|-- App.tsx               # Main React component
|-- index.html            # Main HTML file
|-- index.tsx             # React entry point
|-- metadata.json
|
|-- Makefile              # makefile to compile the C code
|-- huffman               # The COMPILED C binary (on Linux/macOS)
|-- huffman.exe           # The COMPILED C binary (on Windows)
|
|-- package.json          # Node.js project file 
|-- server.js             # The Node.js server 
|-- README.md             # This file 
```
