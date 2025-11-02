
import React, { useState, useCallback } from 'react';
import FileUploader from './components/FileUploader';
import CompressionResult from './components/CompressionResult';

// --- TYPE DEFINITIONS ---
export interface CompressionResultData {
  originalSize: number;
  compressedSize: number;
  ratio: number;
  compressedData: string;
  fileName: string;
}

// --- HELPER FUNCTIONS ---
export const formatBytes = (bytes: number, decimals = 2): string => {
    if (bytes === 0) return '0 Bytes';
    const k = 1024;
    const dm = decimals < 0 ? 0 : decimals;
    const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + ' ' + sizes[i];
};

// --- SVG LOADER ICON ---
const LoaderIcon: React.FC<React.SVGProps<SVGSVGElement>> = (props) => (
    <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round" {...props}>
        <path d="M21 12a9 9 0 1 1-6.219-8.56"/>
    </svg>
);


const App: React.FC = () => {
    const [file, setFile] = useState<File | null>(null);
    const [result, setResult] = useState<CompressionResultData | null>(null);
    const [isLoading, setIsLoading] = useState<boolean>(false);

    const handleFileSelect = useCallback((selectedFile: File) => {
        setFile(selectedFile);
        setResult(null); // Reset result when a new file is selected
    }, []);

    const handleFileRemove = useCallback(() => {
        setFile(null);
        setResult(null);
    }, []);

    const handleCompress = useCallback(async () => {
        if (!file) return;

        setIsLoading(true);
        setResult(null);

        // Simulate network delay and compression time
        await new Promise(resolve => setTimeout(resolve, 1500));

        // --- START SIMULATION LOGIC ---
        // TODO: Replace this block with a call to the C backend API
        const originalSize = file.size;
        
        // Randomize compression ratio for simulation (between 40% and 75% reduction)
        const reductionRatio = Math.random() * (0.75 - 0.40) + 0.40;
        const compressedSize = Math.round(originalSize * (1 - reductionRatio));
        
        // Generate fake binary data for preview
        const compressedData = Array.from({ length: 200 }, () => Math.round(Math.random())).join('');

        const compressionResult: CompressionResultData = {
            originalSize,
            compressedSize,
            ratio: reductionRatio,
            compressedData,
            fileName: file.name
        };
        // --- END SIMULATION LOGIC ---

        setResult(compressionResult);
        setIsLoading(false);
    }, [file]);

    return (
        <div className="min-h-screen flex flex-col items-center justify-center p-4 text-slate-800">
            <main className="w-full max-w-2xl mx-auto">
                <header className="text-center mb-8">
                    <h1 className="text-4xl font-bold text-slate-900">Huffman Compression Tool</h1>
                    <p className="text-slate-600 mt-2">Upload a .txt file to simulate Huffman compression.</p>
                </header>

                <div className="bg-white p-6 sm:p-8 rounded-xl shadow-md space-y-6">
                    <FileUploader
                        onFileSelect={handleFileSelect}
                        selectedFile={file}
                        onFileRemove={handleFileRemove}
                    />

                    <button
                        onClick={handleCompress}
                        disabled={!file || isLoading}
                        className="w-full flex items-center justify-center bg-blue-600 text-white font-semibold py-3 px-4 rounded-lg hover:bg-blue-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 disabled:bg-slate-300 disabled:cursor-not-allowed transition-all duration-300"
                    >
                        {isLoading ? (
                            <>
                                <LoaderIcon className="animate-spin mr-2 h-5 w-5" />
                                Compressing...
                            </>
                        ) : 'Compress File'}
                    </button>

                    {result && (
                        <CompressionResult data={result} />
                    )}
                </div>
                
                <footer className="text-center mt-8 text-sm text-slate-500">
                    <p>This is a UI simulation. The actual compression logic is not implemented.</p>
                </footer>
            </main>
        </div>
    );
};

export default App;
