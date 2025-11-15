
const express = require('express');
const multer = require('multer');
const { execFile } = require('child_process');
const path = require('path');
const fs = require('fs');
const cors = require('cors');

const app = express();
const PORT = 8080;

// --- Configuration ---

// Enable CORS for all routes to allow requests from the frontend
app.use(cors());

// Configure Multer for file uploads
// This will save uploaded files to a directory named 'uploads'
const uploadDir = path.join(__dirname, 'uploads');
if (!fs.existsSync(uploadDir)) {
    fs.mkdirSync(uploadDir);
}

const storage = multer.diskStorage({
    destination: (req, file, cb) => {
        cb(null, uploadDir);
    },
    filename: (req, file, cb) => {
        // Use a unique name to avoid conflicts
        cb(null, `${Date.now()}-${file.originalname}`);
    }
});

const upload = multer({ storage: storage });

// Determine the C binary executable name based on the OS
// Make sure you compile your C code and place the executable in the root of this Node.js project
const huffmanExecutable = process.platform === 'win32' ? 'huffman.exe' : './huffman';


// --- Helper function for cleanup ---
const cleanupFiles = (files) => {
    files.forEach(filePath => {
        if (filePath && fs.existsSync(filePath)) {
            fs.unlink(filePath, (err) => {
                if (err) console.error(`Failed to delete temporary file: ${filePath}`, err);
            });
        }
    });
};

// --- API Routes ---

/**
 * @route POST /compress
 * @desc Receives a text file, compresses it using the C binary, and returns the compressed file.
 */
app.post('/compress', upload.single('textFile'), (req, res) => {
    if (!req.file) {
        return res.status(400).send('No file uploaded.');
    }

    const inputPath = req.file.path;
    // Create a unique output path in the same directory
    const outputPath = `${inputPath}.huff`;
    const outputFilename = `${req.file.originalname.split('.')[0]}.huff`;

    // Execute the C compression program
    // Arguments: -c <input_path> <output_path>
    execFile(huffmanExecutable, ['-c', inputPath, outputPath], (error, stdout, stderr) => {
        if (error) {
            console.error(`execFile error: ${error.message}`);
            cleanupFiles([inputPath, outputPath]); // Cleanup on error
            return res.status(500).send(`Compression failed: ${stderr || error.message}`);
        }
        
        if (stderr) {
            console.warn(`Compression stderr: ${stderr}`);
        }

        // Send the compressed file back to the client for download
        res.download(outputPath, outputFilename, (err) => {
            if (err) {
                console.error('Download error:', err);
            }
            // Cleanup temporary files after download (or if download fails)
            cleanupFiles([inputPath, outputPath]);
        });
    });
});

// --- Start Server ---
app.listen(PORT, () => {
    console.log(`✅ Server is running on http://localhost:${PORT}`);
    console.log(`   Waiting for file uploads...`);
    // Check if the executable exists
    if (!fs.existsSync(huffmanExecutable.replace('./', ''))) {
         console.warn(`\n⚠️  WARNING: Huffman executable "${huffmanExecutable}" not found.`);
         console.warn(`   Make sure you have compiled your C code and placed the executable`);
         console.warn(`   in the same directory as this server file.\n`);
    }
});
