import { useState } from 'react';

interface LegacyImportProps {
  onImport: (airports: string[]) => void;
}

/**
 * Parses legacy C++ airport configuration format
 * Handles: std::vector<String> airports({"LIFR", "IFR", ...});
 * Or just: {"LIFR", "IFR", ...}
 * Also handles comments like: "KMRY", // 8
 */
function parseLegacyAirports(input: string): string[] {
  // First, try to extract content between curly braces
  const braceMatch = input.match(/\{([^}]+)\}/s);
  if (!braceMatch) return [];

  const content = braceMatch[1];

  // Match quoted strings, handling comments
  const airports: string[] = [];
  const linePattern = /"([^"]+)"/g;
  let match;

  while ((match = linePattern.exec(content)) !== null) {
    airports.push(match[1]);
  }

  return airports;
}

export function LegacyImport({ onImport }: LegacyImportProps) {
  const [input, setInput] = useState('');
  const [preview, setPreview] = useState<string[] | null>(null);
  const [error, setError] = useState<string | null>(null);

  const handleParse = () => {
    setError(null);
    setPreview(null);

    if (!input.trim()) {
      setError('Please paste your airport configuration');
      return;
    }

    const parsed = parseLegacyAirports(input);

    if (parsed.length === 0) {
      setError('Could not parse airports. Make sure the format includes quoted strings like "KSFO"');
      return;
    }

    setPreview(parsed);
  };

  const handleImport = () => {
    if (preview && preview.length > 0) {
      onImport(preview);
      setInput('');
      setPreview(null);
    }
  };

  const handleClear = () => {
    setInput('');
    setPreview(null);
    setError(null);
  };

  return (
    <div className="space-y-4">
      <div>
        <label className="block text-sm font-medium text-gray-400 mb-2">
          Paste your legacy airport configuration
        </label>
        <textarea
          value={input}
          onChange={(e) => setInput(e.target.value)}
          placeholder={`Paste your C++ code here, e.g.:

std::vector<String> airports({
  "LIFR", // 1
  "IFR", // 2
  "MVFR", // 3
  "KMRY", // 4
  ...
});`}
          className="w-full h-48 bg-gray-700 rounded-lg p-3 font-mono text-sm resize-none"
        />
      </div>

      {error && (
        <div className="text-red-400 text-sm">{error}</div>
      )}

      <div className="flex gap-2">
        <button
          onClick={handleParse}
          className="px-4 py-2 bg-blue-600 hover:bg-blue-700 rounded-lg font-medium"
        >
          Parse
        </button>
        <button
          onClick={handleClear}
          className="px-4 py-2 bg-gray-600 hover:bg-gray-700 rounded-lg font-medium"
        >
          Clear
        </button>
      </div>

      {preview && (
        <div className="bg-gray-800 rounded-lg p-4">
          <h4 className="font-medium mb-2">Preview ({preview.length} airports)</h4>
          <div className="max-h-48 overflow-y-auto">
            <div className="flex flex-wrap gap-1">
              {preview.map((code, i) => (
                <span
                  key={i}
                  className="px-2 py-1 bg-gray-700 rounded text-sm font-mono"
                >
                  {i + 1}: {code}
                </span>
              ))}
            </div>
          </div>
          <button
            onClick={handleImport}
            className="mt-4 px-4 py-2 bg-green-600 hover:bg-green-700 rounded-lg font-medium"
          >
            Import {preview.length} Airports
          </button>
        </div>
      )}
    </div>
  );
}
