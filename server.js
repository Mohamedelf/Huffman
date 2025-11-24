const express = require('express');
const multer = require('multer');
const { execFile } = require('child_process');
const path = require('path');
const fs = require('fs');
const cors = require('cors');

const app = express();
// Important pour Render : utiliser le port donné par l'environnement
const PORT = process.env.PORT || 8080;

// --- Configuration ---

// Enable CORS for all routes (utile si dev local, moins critique en prod sur même origine)
app.use(cors());

// Configure Multer for file uploads
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
 * @desc Reçoit un fichier texte, le compresse via le binaire C, et renvoie le fichier .huff.
 */
app.post('/compress', upload.single('textFile'), (req, res) => {
    if (!req.file) {
        return res.status(400).send('No file uploaded.');
    }

    const inputPath = req.file.path;
    const outputPath = `${inputPath}.huff`;
    const outputFilename = `${req.file.originalname.split('.')[0]}.huff`;

    // Arguments: -c <input> <output>
    execFile(huffmanExecutable, ['-c', inputPath, outputPath], (error, stdout, stderr) => {
        if (error) {
            console.error(`execFile error: ${error.message}`);
            cleanupFiles([inputPath, outputPath]);
            return res.status(500).send(`Compression failed: ${stderr || error.message}`);
        }
        
        res.download(outputPath, outputFilename, (err) => {
            if (err) console.error('Download error:', err);
            cleanupFiles([inputPath, outputPath]);
        });
    });
});

/**
 * @route POST /decompress
 * @desc Reçoit un fichier .huff, le décompresse via le binaire C, et renvoie le texte.
 */
app.post('/decompress', upload.single('textFile'), (req, res) => {
    if (!req.file) {
        return res.status(400).send('No file uploaded.');
    }

    const inputPath = req.file.path;
    // On tente de retirer l'extension .huff pour le nom de sortie
    let outputFilename = req.file.originalname.replace('.huff', '');
    // Si le nom n'a pas changé (pas de .huff), on ajoute .txt par sécurité
    if (outputFilename === req.file.originalname) {
        outputFilename += '.txt';
    } else if (!outputFilename.includes('.')) {
        // Si après retrait de .huff il n'y a plus d'extension, on remet .txt
        outputFilename += '.txt';
    }
    
    const outputPath = path.join(path.dirname(inputPath), `decompressed-${Date.now()}.txt`);

    // Arguments: -d <input> <output>
    execFile(huffmanExecutable, ['-d', inputPath, outputPath], (error, stdout, stderr) => {
        if (error) {
            console.error(`execFile error: ${error.message}`);
            cleanupFiles([inputPath, outputPath]);
            return res.status(500).send(`Decompression failed: ${stderr || error.message}`);
        }

        res.download(outputPath, outputFilename, (err) => {
            if (err) console.error('Download error:', err);
            cleanupFiles([inputPath, outputPath]);
        });
    });
});

// --- SERVING FRONTEND (AJOUT CRITIQUE) ---

// 1. Servir les fichiers statiques (JS, CSS) générés par Vite dans 'dist'
app.use(express.static(path.join(__dirname, 'dist')));

// 2. Route "Catch-all" : renvoyer index.html pour n'importe quelle autre requête
// Cela permet à React Router de gérer l'affichage sans erreur 404
app.get('*', (req, res) => {
    res.sendFile(path.join(__dirname, 'dist', 'index.html'));
});

// --- Start Server ---
app.listen(PORT, () => {
    console.log(`✅ Server is running on port ${PORT}`);
    if (!fs.existsSync(huffmanExecutable.replace('./', ''))) {
         console.warn(`\n⚠️  WARNING: Huffman executable "${huffmanExecutable}" not found.`);
    }
});