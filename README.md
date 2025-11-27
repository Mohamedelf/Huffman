# Huffman Compression Project

## Description

This project is a full-stack web application that implements **Huffman coding**, a popular algorithm for lossless data compression. The solution allows users to upload text files to be compressed into a custom binary format (`.huff`) and allows those files to be decompressed back to their original state.

The architecture is designed to leverage the performance of **C** for the core algorithmic logic while providing a modern, user-friendly interface using **React** and **Node.js**.

## Key Features

* **High-Performance Core**: The compression and decompression algorithms (Min-Heap construction, Tree building, Bitwise I/O) are written in **C**.

* **Web Interface**: A clean, responsive UI built with **React**, **TypeScript**, and **Tailwind CSS**.

* **REST API**: A **Node.js/Express** backend that handles file uploads and orchestrates the execution of the C binary.

* **Dockerized**: Fully containerized application ready for deployment (e.g., on Render).

* **CLI Support**: The C program can also be used continuously as a standalone Command Line Interface tool.

## Project Structure

The project organizes both the system-level C code and the web-frontend TypeScript code within a unified directory structure.

```text
/
├── src/                        # Source code for both C and React
│   ├── components/             # React UI components
│   │   └── FileUploader.tsx    # Drag-and-drop file upload component
│   ├── App.tsx                 # Main React application logic
│   ├── index.tsx               # React entry point
│   ├── utils.ts                # Frontend utility functions (e.g., file size formatting)
│   │
│   ├── main.c                  # Entry point for the C CLI tool
│   ├── huffman.c / .h          # Huffman tree construction and code generation logic
│   ├── heap.c / .h             # Min-Heap implementation (priority queue)
│   └── io.c / .h               # Bitwise I/O and custom file header handling
│
├── dist/                       # Production build of the React frontend (generated)
├── uploads/                    # Temporary storage for file processing
├── huffman                     # Compiled C executable (Linux/macOS)
│
├── server.js                   # Node.js Express server
├── Makefile                    # Build script for the C program
├── Dockerfile                  # Configuration for containerization
├── package.json                # Node.js dependencies and scripts
├── tsconfig.json               # TypeScript configuration
├── vite.config.ts              # Vite build configuration
└── README.md                   # Project documentation
```