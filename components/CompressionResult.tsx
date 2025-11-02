
import React from 'react';
import { CompressionResultData, formatBytes } from '../App';

// --- SVG ICONS ---
const DownloadIcon: React.FC<React.SVGProps<SVGSVGElement>> = (props) => (
    <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round" {...props}>
        <path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4" />
        <polyline points="7 10 12 15 17 10" />
        <line x1="12" x2="12" y1="15" y2="3" />
    </svg>
);

interface CompressionResultProps {
    data: CompressionResultData;
}

const StatCard: React.FC<{ label: string; value: string; className?: string }> = ({ label, value, className }) => (
    <div className={`p-4 bg-slate-50 rounded-lg text-center ${className}`}>
        <p className="text-sm text-slate-500">{label}</p>
        <p className="text-xl font-semibold text-slate-900">{value}</p>
    </div>
);

const CompressionResult: React.FC<CompressionResultProps> = ({ data }) => {

    const handleDownload = () => {
        // TODO: This should download the actual compressed file from the backend
        const blob = new Blob([data.compressedData], { type: 'text/plain' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        const [fileName, ] = data.fileName.split('.');
        a.download = `${fileName}_compressed.huff`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    };

    return (
        <div className="animate-fade-in-scale space-y-6">
            <style>{`
                @keyframes fadeInScale {
                    from { opacity: 0; transform: scale(0.95); }
                    to { opacity: 1; transform: scale(1); }
                }
                .animate-fade-in-scale {
                    animation: fadeInScale 0.5s ease-in-out forwards;
                }
            `}</style>
            
            <h2 className="text-lg font-semibold text-center text-slate-800">Compression Results</h2>
            
            <div className="grid grid-cols-1 sm:grid-cols-3 gap-4">
                <StatCard label="Original Size" value={formatBytes(data.originalSize)} />
                <StatCard label="Compressed Size" value={formatBytes(data.compressedSize)} className="text-green-600"/>
                <StatCard label="Reduction" value={`${(data.ratio * 100).toFixed(1)}%`} className="text-blue-600"/>
            </div>

            <div>
                <label htmlFor="compressed-data" className="block text-sm font-medium text-slate-700 mb-1">
                    Compressed Data Preview
                </label>
                <textarea
                    id="compressed-data"
                    readOnly
                    value={data.compressedData + '...'}
                    className="w-full h-24 p-2 font-mono text-xs bg-slate-100 border border-slate-200 rounded-md focus:ring-blue-500 focus:border-blue-500"
                />
            </div>
            
            <button
                onClick={handleDownload}
                className="w-full flex items-center justify-center bg-green-600 text-white font-semibold py-3 px-4 rounded-lg hover:bg-green-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-green-500 transition-all duration-300"
            >
                <DownloadIcon className="mr-2 h-5 w-5" />
                Download Compressed File
            </button>
        </div>
    );
};

export default CompressionResult;
