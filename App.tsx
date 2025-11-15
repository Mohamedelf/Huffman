
import React, { useState, useCallback } from 'react';
import FileUploader from './components/FileUploader';

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
    const [isLoading, setIsLoading] = useState<boolean>(false);
    const [error, setError] = useState<string | null>(null);

    const handleFileSelect = useCallback((selectedFile: File) => {
        setFile(selectedFile);
        setError(null);
    }, []);

    const handleFileRemove = useCallback(() => {
        setFile(null);
        setError(null);
    }, []);

    const handleCompressAndDownload = useCallback(async () => {
        if (!file) return;

        setIsLoading(true);
        setError(null);

        const formData = new FormData();
        formData.append('textFile', file);

        try {
            // This endpoint should be your Node.js server, which will call the C binary
            const response = await fetch('http://localhost:8080/compress', {
                method: 'POST',
                body: formData,
            });

            if (!response.ok) {
                const errorText = await response.text();
                throw new Error(`Server error: ${response.status} - ${errorText}`);
            }

            // The backend now sends the compressed file directly
            const blob = await response.blob();
            const url = window.URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.style.display = 'none';
            a.href = url;

            const [fileName] = file.name.split('.');
            a.download = `${fileName}.huff`;
            
            document.body.appendChild(a);
            a.click();
            window.URL.revokeObjectURL(url);
            document.body.removeChild(a);

        } catch (err) {
            const errorMessage = err instanceof Error ? err.message : 'An unknown error occurred.';
            console.error("Compression request failed:", errorMessage);
            setError(`Compression failed. Please ensure the backend is running. Details: ${errorMessage}`);
        } finally {
            setIsLoading(false);
        }
    }, [file]);

    return (
        <div className="min-h-screen flex flex-col items-center justify-center p-4 text-slate-800">
            <main className="w-full max-w-2xl mx-auto">
                <header className="text-center mb-8">
                    <h1 className="text-4xl font-bold text-slate-900">Huffman Compression Tool</h1>
                    <p className="text-slate-600 mt-2">Upload a .txt file, and my C program will compress it for you.</p>
                </header>

                <div className="bg-white p-6 sm:p-8 rounded-xl shadow-md space-y-6">
                    <FileUploader
                        onFileSelect={handleFileSelect}
                        selectedFile={file}
                        onFileRemove={handleFileRemove}
                    />

                    <button
                        onClick={handleCompressAndDownload}
                        disabled={!file || isLoading}
                        className="w-full flex items-center justify-center bg-blue-600 text-white font-semibold py-3 px-4 rounded-lg hover:bg-blue-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 disabled:bg-slate-300 disabled:cursor-not-allowed transition-all duration-300"
                    >
                        {isLoading ? (
                            <>
                                <LoaderIcon className="animate-spin mr-2 h-5 w-5" />
                                Compressing...
                            </>
                        ) : 'Compress & Download File'}
                    </button>
                    
                    {error && (
                        <div className="bg-red-100 border border-red-400 text-red-700 px-4 py-3 rounded-lg relative" role="alert">
                            <strong className="font-bold">Error: </strong>
                            <span className="block sm:inline">{error}</span>
                        </div>
                    )}
                </div>
                
                <footer className="text-center mt-8 text-sm text-slate-500">
                    <p>UI by Mohamed El Fene - Backend powered by Node.js + C</p>
                </footer>
            </main>
        </div>
    );
};

export default App;
