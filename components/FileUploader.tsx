import React, { useState } from 'react';
// CORRECTION : Import depuis le nouveau fichier utils
import { formatBytes } from '../utils';

// --- SVG ICONS ---
const UploadIcon: React.FC<React.SVGProps<SVGSVGElement>> = (props) => (
    <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round" {...props}>
        <path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4" />
        <polyline points="17 8 12 3 7 8" />
        <line x1="12" x2="12" y1="3" y2="15" />
    </svg>
);

const FileIcon: React.FC<React.SVGProps<SVGSVGElement>> = (props) => (
    <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round" {...props}>
        <path d="M14.5 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V7.5L14.5 2z" />
        <polyline points="14 2 14 8 20 8" />
    </svg>
);

const XIcon: React.FC<React.SVGProps<SVGSVGElement>> = (props) => (
    <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round" {...props}>
        <line x1="18" x2="6" y1="6" y2="18" />
        <line x1="6" x2="18" y1="6" y2="18" />
    </svg>
);

interface FileUploaderProps {
    onFileSelect: (file: File) => void;
    selectedFile: File | null;
    onFileRemove: () => void;
}

const FileUploader: React.FC<FileUploaderProps> = ({ onFileSelect, selectedFile, onFileRemove }) => {
    const [isDragging, setIsDragging] = useState(false);

    const handleFileChange = (e: React.ChangeEvent<HTMLInputElement>) => {
        const file = e.target.files?.[0];
        if (file) {
            onFileSelect(file);
        }
    };

    const handleDragEnter = (e: React.DragEvent<HTMLDivElement>) => {
        e.preventDefault();
        e.stopPropagation();
        setIsDragging(true);
    };

    const handleDragLeave = (e: React.DragEvent<HTMLDivElement>) => {
        e.preventDefault();
        e.stopPropagation();
        setIsDragging(false);
    };
    
    const handleDragOver = (e: React.DragEvent<HTMLDivElement>) => {
        e.preventDefault();
        e.stopPropagation();
    };

    const handleDrop = (e: React.DragEvent<HTMLDivElement>) => {
        e.preventDefault();
        e.stopPropagation();
        setIsDragging(false);

        const file = e.dataTransfer.files?.[0];
        // Validation mise à jour pour accepter .txt ET .huff
        if (file && (file.type === 'text/plain' || file.name.endsWith('.huff'))) {
            onFileSelect(file);
        } else {
            alert("Please upload a .txt or .huff file.");
        }
    };
    
    return (
        <div>
            <label htmlFor="file-upload" className="sr-only">Choose a file</label>
            {/* Input accept mis à jour */}
            <input id="file-upload" name="file-upload" type="file" className="sr-only" onChange={handleFileChange} accept=".txt,.huff" />
            
            {!selectedFile ? (
                <div
                    onDragEnter={handleDragEnter}
                    onDragLeave={handleDragLeave}
                    onDragOver={handleDragOver}
                    onDrop={handleDrop}
                    onClick={() => document.getElementById('file-upload')?.click()}
                    className={`flex justify-center items-center w-full px-6 py-10 border-2 border-dashed rounded-lg cursor-pointer transition-colors duration-300
                        ${isDragging ? 'border-blue-500 bg-blue-50' : 'border-slate-300 bg-slate-50 hover:border-slate-400'}`}
                >
                    <div className="text-center">
                        <UploadIcon className="mx-auto h-12 w-12 text-slate-400" />
                        <p className="mt-2 text-sm text-slate-600">
                            <span className="font-semibold text-blue-600">Click to upload</span> or drag and drop
                        </p>
                        <p className="text-xs text-slate-500">TXT or HUFF files</p>
                    </div>
                </div>
            ) : (
                <div className="flex items-center justify-between w-full p-4 border border-slate-200 rounded-lg bg-white">
                    <div className="flex items-center space-x-3">
                        <FileIcon className="h-8 w-8 text-blue-500" />
                        <div>
                            <p className="font-medium text-slate-800 truncate">{selectedFile.name}</p>
                            <p className="text-sm text-slate-500">{formatBytes(selectedFile.size)}</p>
                        </div>
                    </div>
                    <button 
                        onClick={onFileRemove}
                        className="p-1 rounded-full text-slate-500 hover:bg-slate-100 hover:text-slate-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500"
                        aria-label="Remove file"
                    >
                        <XIcon className="h-5 w-5" />
                    </button>
                </div>
            )}
        </div>
    );
};

export default FileUploader;