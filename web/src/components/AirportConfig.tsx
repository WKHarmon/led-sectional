import { useState } from 'react';
import { AirportList } from './AirportList';
import { LegacyImport } from './LegacyImport';
import type { DeviceConfig } from '../types/config';

// Maximum airports supported by firmware (must match MAX_LEDS in leds.h)
const MAX_AIRPORTS = 300;

interface AirportConfigProps {
  config: DeviceConfig;
  onSave: (config: Partial<DeviceConfig>) => Promise<boolean>;
  isLoading: boolean;
  isConnected?: boolean;
  localAirports?: string[];  // Airports saved locally (when different from device)
  onUploadLocal?: () => Promise<boolean>;  // Upload local config to device
}

export function AirportConfig({
  config,
  onSave,
  isLoading,
  isConnected = false,
  localAirports,
  onUploadLocal,
}: AirportConfigProps) {
  const [airports, setAirports] = useState<string[]>(config.airports || []);
  const [hasChanges, setHasChanges] = useState(false);
  const [showImport, setShowImport] = useState(false);

  // Sync with config when it changes
  const syncedAirports = config.airports || [];
  if (JSON.stringify(syncedAirports) !== JSON.stringify(airports) && !hasChanges) {
    setAirports(syncedAirports);
  }

  // Check if there's a local config that differs from device config
  const hasLocalConfig = localAirports &&
    localAirports.length > 0 &&
    JSON.stringify(localAirports) !== JSON.stringify(syncedAirports);

  const handleChange = (newAirports: string[]) => {
    // Enforce maximum airport limit
    if (newAirports.length > MAX_AIRPORTS) {
      newAirports = newAirports.slice(0, MAX_AIRPORTS);
    }
    setAirports(newAirports);
    setHasChanges(true);
  };

  const isAtLimit = airports.length >= MAX_AIRPORTS;

  const handleImport = (importedAirports: string[]) => {
    setAirports(importedAirports);
    setHasChanges(true);
    setShowImport(false);
  };

  const handleSave = async () => {
    const success = await onSave({ airports });
    if (success) {
      setHasChanges(false);
    }
  };

  const handleDiscard = () => {
    setAirports(config.airports || []);
    setHasChanges(false);
  };

  return (
    <div className="space-y-6">
      {/* Import toggle */}
      <div className="flex justify-end">
        <button
          onClick={() => setShowImport(!showImport)}
          className="text-blue-400 hover:text-blue-300 text-sm"
        >
          {showImport ? 'Hide Import' : 'Import from Legacy Config'}
        </button>
      </div>

      {/* Legacy import section */}
      {showImport && (
        <div className="bg-gray-800 rounded-lg p-4">
          <h3 className="font-semibold mb-3">Import Legacy Configuration</h3>
          <LegacyImport onImport={handleImport} />
        </div>
      )}

      {/* Upload local config banner */}
      {isConnected && hasLocalConfig && onUploadLocal && !hasChanges && (
        <div className="bg-blue-900/50 border border-blue-700 rounded-lg p-4">
          <div className="flex items-center justify-between">
            <div>
              <span className="text-blue-400 font-medium">Local Configuration Available</span>
              <p className="text-gray-400 text-sm mt-1">
                You have {localAirports?.length} airports saved locally that differ from this device.
              </p>
            </div>
            <button
              onClick={onUploadLocal}
              disabled={isLoading}
              className="px-4 py-2 bg-blue-600 hover:bg-blue-700 rounded-lg font-medium disabled:opacity-50 whitespace-nowrap ml-4"
            >
              {isLoading ? 'Uploading...' : 'Upload to Device'}
            </button>
          </div>
        </div>
      )}

      {/* Airport list */}
      <div className="bg-gray-800 rounded-lg p-4">
        <AirportList airports={airports} onChange={handleChange} />
      </div>

      {/* Quick add buttons */}
      <div className="bg-gray-800 rounded-lg p-4">
        <div className="flex items-center justify-between mb-3">
          <h4 className="text-sm font-medium text-gray-400">Quick Add</h4>
          <span className={`text-xs ${isAtLimit ? 'text-yellow-500' : 'text-gray-500'}`}>
            {airports.length} / {MAX_AIRPORTS} airports
          </span>
        </div>
        {isAtLimit && (
          <div className="mb-3 text-yellow-500 text-sm">
            Maximum of {MAX_AIRPORTS} airports reached. Remove some to add more.
          </div>
        )}
        <div className="flex flex-wrap gap-2">
          {['VFR', 'MVFR', 'IFR', 'LIFR', 'WVFR', 'NULL'].map((code) => (
            <button
              key={code}
              onClick={() => handleChange([...airports, code])}
              disabled={isAtLimit}
              className="px-3 py-1 bg-gray-700 hover:bg-gray-600 rounded text-sm font-mono disabled:opacity-50 disabled:cursor-not-allowed"
            >
              + {code}
            </button>
          ))}
        </div>
        <p className="mt-2 text-xs text-gray-500">
          VFR/MVFR/IFR/LIFR/WVFR = Legend colors • NULL = LED disabled
        </p>
      </div>

      {/* Save/Discard bar */}
      {hasChanges && (
        <div className="sticky bottom-0 bg-gray-900 py-4 border-t border-gray-700 flex gap-3">
          <button
            onClick={handleSave}
            disabled={isLoading}
            className="flex-1 px-4 py-3 bg-green-600 hover:bg-green-700 rounded-lg font-medium disabled:opacity-50"
          >
            {isLoading ? 'Saving...' : `Save ${airports.length} Airports`}
          </button>
          <button
            onClick={handleDiscard}
            disabled={isLoading}
            className="px-4 py-3 bg-gray-600 hover:bg-gray-700 rounded-lg font-medium disabled:opacity-50"
          >
            Discard
          </button>
        </div>
      )}
    </div>
  );
}
