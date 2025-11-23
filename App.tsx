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
    const [mode, setMode] = useState<'compress' | 'decompress'>('compress');

    const handleFileSelect = useCallback((selectedFile: File) => {
        setFile(selectedFile);
        setError(null);
        // Détection automatique du mode selon l'extension
        if (selectedFile.name.endsWith('.huff')) {
            setMode('decompress');
        } else {
            setMode('compress');
        }
    }, []);

    const handleFileRemove = useCallback(() => {
        setFile(null);
        setError(null);
    }, []);

    const handleProcess = useCallback(async () => {
        if (!file) return;

        setIsLoading(true);
        setError(null);

        const formData = new FormData();
        formData.append('textFile', file);

        // Choix dynamique de l'URL et de la route
        const endpoint = mode === 'compress' ? '/compress' : '/decompress';
        const baseUrl = 'https://huffman-backend.onrender.com'; 

        try {
            const response = await fetch(`${baseUrl}${endpoint}`, {
                method: 'POST',
                body: formData,
            });

            if (!response.ok) {
                const errorText = await response.text();
                throw new Error(`Server error: ${response.status} - ${errorText}`);
            }

            const blob = await response.blob();
            const url = window.URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.style.display = 'none';
            a.href = url;

            // Gestion du nom de fichier selon le mode
            if (mode === 'compress') {
                const [fileName] = file.name.split('.');
                a.download = `${fileName}.huff`;
            } else {
                // Retirer .huff si présent, et s'assurer d'une extension texte
                let name = file.name.replace('.huff', '');
                if (!name.includes('.')) name += '.txt';
                a.download = name;
            }
            
            document.body.appendChild(a);
            a.click();
            window.URL.revokeObjectURL(url);
            document.body.removeChild(a);

        } catch (err) {
            const errorMessage = err instanceof Error ? err.message : 'An unknown error occurred.';
            setError(`${mode === 'compress' ? 'Compression' : 'Decompression'} failed. Backend details: ${errorMessage}`);
        } finally {
            setIsLoading(false);
        }
    }, [file, mode]);

    return (
        <div className="min-h-screen flex flex-col items-center justify-center p-4 text-slate-800">
            <main className="w-full max-w-2xl mx-auto">
                <header className="text-center mb-8">
                    <h1 className="text-4xl font-bold text-slate-900">Mohamed Elfene Compression's Tool</h1>
                    <p className="text-slate-600 mt-2">
                        {mode === 'compress' 
                            ? "Upload a .txt file to compress it." 
                            : "Upload a .huff file to decompress it."}
                    </p>
                </header>

                <div className="bg-white p-6 sm:p-8 rounded-xl shadow-md space-y-6">
                    {/* Boutons de choix de mode */}
                    <div className="flex justify-center space-x-4 mb-4">
                        <button 
                            onClick={() => { setMode('compress'); setFile(null); setError(null); }}
                            className={`px-4 py-2 rounded-full text-sm font-medium transition-colors ${
                                mode === 'compress' ? 'bg-blue-100 text-blue-700 border border-blue-200' : 'text-slate-500 hover:bg-slate-100'
                            }`}
                        >
                            Compression
                        </button>
                        <button 
                            onClick={() => { setMode('decompress'); setFile(null); setError(null); }}
                            className={`px-4 py-2 rounded-full text-sm font-medium transition-colors ${
                                mode === 'decompress' ? 'bg-green-100 text-green-700 border border-green-200' : 'text-slate-500 hover:bg-slate-100'
                            }`}
                        >
                            Décompression
                        </button>
                    </div>

                    <FileUploader
                        onFileSelect={handleFileSelect}
                        selectedFile={file}
                        onFileRemove={handleFileRemove}
                    />

                    <button
                        onClick={handleProcess}
                        disabled={!file || isLoading}
                        className={`w-full flex items-center justify-center font-semibold py-3 px-4 rounded-lg focus:outline-none focus:ring-2 focus:ring-offset-2 transition-all duration-300 disabled:bg-slate-300 disabled:cursor-not-allowed ${
                            mode === 'compress' 
                            ? 'bg-blue-600 hover:bg-blue-700 focus:ring-blue-500 text-white' 
                            : 'bg-green-600 hover:bg-green-700 focus:ring-green-500 text-white'
                        }`}
                    >
                        {isLoading ? (
                            <>
                                <LoaderIcon className="animate-spin mr-2 h-5 w-5" />
                                Processing...
                            </>
                        ) : (mode === 'compress' ? 'Compress & Download' : 'Decompress & Download')}
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