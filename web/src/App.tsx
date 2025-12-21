import { useState, useEffect, useCallback, useRef } from 'react';
import { useSerial } from './hooks/useSerial';
import { ConnectionStatus } from './components/ConnectionStatus';
import { AirportConfig } from './components/AirportConfig';
import { DeviceSettings } from './components/DeviceSettings';
import { WiFiConfig } from './components/WiFiConfig';
import { SerialMonitor } from './components/SerialMonitor';
import { FirmwareInstall } from './components/FirmwareInstall';
import {
  loadFromLocalStorage,
  saveToLocalStorage,
  loadFromUrl,
  getShareableUrl,
  mergeWithDefaults,
  clearLocalStorage,
} from './services/storage';
import type { DeviceConfig } from './types/config';
import { DEFAULT_CONFIG } from './types/config';

type Tab = 'airports' | 'settings' | 'wifi' | 'monitor' | 'firmware';

function App() {
  const [activeTab, setActiveTab] = useState<Tab>('airports');
  const [offlineConfig, setOfflineConfig] = useState<DeviceConfig>(DEFAULT_CONFIG);
  const [savedLocalAirports, setSavedLocalAirports] = useState<string[] | null>(null); // Preserved local airports
  const [shareUrl, setShareUrl] = useState<string | null>(null);
  const [showShareModal, setShowShareModal] = useState(false);
  const [showUploadModal, setShowUploadModal] = useState(false);
  const [pendingUploadConfig, setPendingUploadConfig] = useState<DeviceConfig | null>(null);

  const {
    isConnected,
    isSupported,
    isLoading,
    error,
    config: deviceConfig,
    status,
    logs,
    connect,
    disconnect,
    refreshStatus,
    saveConfig: saveToDevice,
    setWifi,
    resetWifi,
    reboot,
    factoryReset,
    testLeds,
    sendRaw,
    clearLogs,
    clearError,
    isCommandInProgress,
  } = useSerial();

  // Load config from URL or localStorage on mount
  useEffect(() => {
    const urlConfig = loadFromUrl();
    if (urlConfig) {
      setOfflineConfig(mergeWithDefaults(urlConfig));
      // Clear URL params after loading
      window.history.replaceState({}, '', window.location.pathname);
      return;
    }

    const localConfig = loadFromLocalStorage();
    if (localConfig) {
      setOfflineConfig(mergeWithDefaults(localConfig));
    }
    // If nothing saved, we already have DEFAULT_CONFIG from initial state
  }, []);

  // Track if we've auto-navigated to WiFi tab for this connection
  const hasAutoNavigatedToWifiRef = useRef(false);

  // Auto-navigate to WiFi tab when device needs WiFi configuration (only once per connection)
  useEffect(() => {
    if (isConnected && status?.needs_wifi_config === true && !hasAutoNavigatedToWifiRef.current) {
      console.log('[WiFi Check] Navigating to WiFi tab (first time)');
      hasAutoNavigatedToWifiRef.current = true;
      setActiveTab('wifi');
    }
  }, [isConnected, status]);

  // Periodically refresh status when connected to detect state changes (e.g., after WiFi configured)
  useEffect(() => {
    if (!isConnected) return;

    const interval = setInterval(() => {
      // Skip refresh if another command is in progress
      if (!isCommandInProgress()) {
        refreshStatus();
      }
    }, 10000); // Refresh every 10 seconds

    return () => clearInterval(interval);
  }, [isConnected, refreshStatus, isCommandInProgress]);

  // Track if we've done the initial sync for this connection
  const hasSyncedRef = useRef(false);

  // Reset sync flags when disconnected
  useEffect(() => {
    if (!isConnected) {
      hasSyncedRef.current = false;
      hasAutoNavigatedToWifiRef.current = false;
      setSavedLocalAirports(null);
    }
  }, [isConnected]);

  // Handle device config sync when connected
  useEffect(() => {
    if (!isConnected || !deviceConfig || hasSyncedRef.current) {
      return;
    }

    hasSyncedRef.current = true;

    const deviceAirports = deviceConfig.airports || [];

    // Read directly from localStorage to get pre-connection config
    const storedConfig = loadFromLocalStorage();
    const localAirports = storedConfig?.airports || [];

    console.log('[Config Sync] Device airports:', deviceAirports);
    console.log('[Config Sync] LocalStorage airports:', localAirports);

    // Check if device appears to be fresh (has default airports or empty)
    const isDeviceFresh = deviceAirports.length === 0 ||
      JSON.stringify(deviceAirports) === JSON.stringify(DEFAULT_CONFIG.airports);

    // Check if we have a meaningful saved config
    const hasSavedConfig = localAirports.length > 0 &&
      JSON.stringify(localAirports) !== JSON.stringify(DEFAULT_CONFIG.airports);

    // Check if local differs from device
    const localDiffersFromDevice = JSON.stringify(localAirports) !== JSON.stringify(deviceAirports);

    console.log('[Config Sync] isDeviceFresh:', isDeviceFresh, 'hasSavedConfig:', hasSavedConfig, 'localDiffersFromDevice:', localDiffersFromDevice);

    // Preserve local airports if they differ from device (for upload button)
    // Do this BEFORE any localStorage overwrites
    if (hasSavedConfig && localDiffersFromDevice) {
      console.log('[Config Sync] Setting savedLocalAirports:', localAirports.length, 'airports');
      setSavedLocalAirports([...localAirports]);
      // DON'T overwrite localStorage yet - let user decide via upload button
      setOfflineConfig(deviceConfig);
      return; // Exit early to preserve localStorage
    }

    if (isDeviceFresh && hasSavedConfig && storedConfig) {
      // Offer to upload saved config via modal
      setPendingUploadConfig(mergeWithDefaults(storedConfig));
      setShowUploadModal(true);
    } else {
      // Normal sync: device config becomes the source of truth
      setOfflineConfig(deviceConfig);
      saveToLocalStorage(deviceConfig);
    }
  }, [deviceConfig, isConnected]);

  // Handle upload modal confirmation
  const handleUploadSavedConfig = useCallback(async () => {
    if (pendingUploadConfig) {
      const success = await saveToDevice(pendingUploadConfig);
      if (success) {
        saveToLocalStorage(pendingUploadConfig);
      }
      setShowUploadModal(false);
      setPendingUploadConfig(null);
    }
  }, [pendingUploadConfig, saveToDevice]);

  // Handle upload modal dismissal (use device config instead)
  const handleKeepDeviceConfig = useCallback(() => {
    if (deviceConfig) {
      setOfflineConfig(deviceConfig);
      saveToLocalStorage(deviceConfig);
    }
    setShowUploadModal(false);
    setPendingUploadConfig(null);
  }, [deviceConfig]);

  // Use device config if connected (and not showing upload prompt), otherwise use offline config
  const activeConfig = (isConnected && deviceConfig && !showUploadModal) ? deviceConfig : offlineConfig;

  // Save handler that works in both modes
  const handleSaveConfig = useCallback(async (newConfig: Partial<DeviceConfig>) => {
    if (isConnected && !showUploadModal) {
      const success = await saveToDevice(newConfig);
      if (success) {
        saveToLocalStorage({ ...activeConfig, ...newConfig });
      }
      return success;
    } else {
      // Offline mode - just save locally
      const updated = { ...offlineConfig, ...newConfig };
      setOfflineConfig(updated);
      saveToLocalStorage(updated);
      return true;
    }
  }, [isConnected, showUploadModal, saveToDevice, activeConfig, offlineConfig]);

  // Upload local config to device
  const handleUploadLocalConfig = useCallback(async () => {
    if (isConnected && savedLocalAirports && savedLocalAirports.length > 0) {
      const success = await saveToDevice({ airports: savedLocalAirports });
      if (success) {
        // Local and device are now synced
        setSavedLocalAirports(null);
        saveToLocalStorage({ ...activeConfig, airports: savedLocalAirports });
      }
      return success;
    }
    return false;
  }, [isConnected, savedLocalAirports, saveToDevice, activeConfig]);

  // Check if we have saved local airports that differ from device
  const hasLocalAirportsToUpload = isConnected && savedLocalAirports && savedLocalAirports.length > 0;

  // Generate shareable URL
  const handleShare = useCallback(() => {
    if (activeConfig) {
      const url = getShareableUrl(activeConfig);
      setShareUrl(url);
      setShowShareModal(true);
    }
  }, [activeConfig]);

  // Copy URL to clipboard
  const handleCopyUrl = useCallback(() => {
    if (shareUrl) {
      navigator.clipboard.writeText(shareUrl);
    }
  }, [shareUrl]);

  // Clear local storage
  const handleClearLocal = useCallback(() => {
    clearLocalStorage();
    setOfflineConfig(DEFAULT_CONFIG);
  }, []);

  // Check if device needs WiFi configuration
  const needsWifiConfig = isConnected && status?.needs_wifi_config === true;

  const tabs: { id: Tab; label: string; icon: string; requiresDevice?: boolean; requiresWifi?: boolean }[] = [
    { id: 'airports', label: 'Airports', icon: '✈️', requiresWifi: true },
    { id: 'settings', label: 'Settings', icon: '⚙️', requiresWifi: true },
    { id: 'wifi', label: 'WiFi', icon: '📶', requiresDevice: true },
    { id: 'monitor', label: 'Monitor', icon: '📟', requiresDevice: true },
    { id: 'firmware', label: 'Firmware', icon: '💾' },
  ];

  return (
    <div className="min-h-screen bg-gray-900">
      {/* Header */}
      <header className="bg-gray-800 border-b border-gray-700">
        <div className="max-w-6xl mx-auto px-4 py-4">
          <div className="flex items-center justify-between">
            <div>
              <h1 className="text-2xl font-bold">LED Sectional</h1>
              <p className="text-gray-400 text-sm">Configuration Tool</p>
            </div>
            <div className="flex items-center gap-4">
              {/* Share button */}
              <button
                onClick={handleShare}
                disabled={!activeConfig}
                className="text-gray-400 hover:text-gray-300 disabled:opacity-50"
                title="Share configuration"
              >
                <svg className="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M8.684 13.342C8.886 12.938 9 12.482 9 12c0-.482-.114-.938-.316-1.342m0 2.684a3 3 0 110-2.684m0 2.684l6.632 3.316m-6.632-6l6.632-3.316m0 0a3 3 0 105.367-2.684 3 3 0 00-5.367 2.684zm0 9.316a3 3 0 105.368 2.684 3 3 0 00-5.368-2.684z" />
                </svg>
              </button>
              <a
                href="https://github.com/WKHarmon/led-sectional"
                target="_blank"
                rel="noopener noreferrer"
                className="text-gray-400 hover:text-gray-300"
              >
                <svg className="w-6 h-6" fill="currentColor" viewBox="0 0 24 24">
                  <path
                    fillRule="evenodd"
                    clipRule="evenodd"
                    d="M12 2C6.477 2 2 6.477 2 12c0 4.42 2.865 8.17 6.839 9.49.5.092.682-.217.682-.482 0-.237-.008-.866-.013-1.7-2.782.604-3.369-1.34-3.369-1.34-.454-1.156-1.11-1.464-1.11-1.464-.908-.62.069-.608.069-.608 1.003.07 1.531 1.03 1.531 1.03.892 1.529 2.341 1.087 2.91.832.092-.647.35-1.088.636-1.338-2.22-.253-4.555-1.11-4.555-4.943 0-1.091.39-1.984 1.029-2.683-.103-.253-.446-1.27.098-2.647 0 0 .84-.269 2.75 1.025A9.578 9.578 0 0112 6.836c.85.004 1.705.114 2.504.336 1.909-1.294 2.747-1.025 2.747-1.025.546 1.377.203 2.394.1 2.647.64.699 1.028 1.592 1.028 2.683 0 3.842-2.339 4.687-4.566 4.935.359.309.678.919.678 1.852 0 1.336-.012 2.415-.012 2.743 0 .267.18.578.688.48C19.138 20.167 22 16.418 22 12c0-5.523-4.477-10-10-10z"
                  />
                </svg>
              </a>
            </div>
          </div>
        </div>
      </header>

      <main className="max-w-6xl mx-auto px-4 py-6">
        {/* Offline mode banner */}
        {!isConnected && (
          <div className="mb-4 bg-blue-900/50 border border-blue-700 rounded-lg p-4">
            <div className="flex items-center justify-between">
              <div>
                <span className="text-blue-400 font-medium">Offline Mode</span>
                <span className="text-gray-400 ml-2">
                  {offlineConfig.airports.length > 0 &&
                   JSON.stringify(offlineConfig.airports) !== JSON.stringify(DEFAULT_CONFIG.airports)
                    ? 'Your configuration will be saved locally. Connect a device to upload.'
                    : 'Configure your airports now, then flash and connect your device to upload.'}
                </span>
              </div>
              {offlineConfig.airports.length > 0 &&
               JSON.stringify(offlineConfig.airports) !== JSON.stringify(DEFAULT_CONFIG.airports) && (
                <button
                  onClick={handleClearLocal}
                  className="text-blue-400 hover:text-blue-300 text-sm whitespace-nowrap ml-4"
                >
                  Clear Local Data
                </button>
              )}
            </div>
          </div>
        )}

        {/* Error banner */}
        {error && (
          <div className="mb-4 bg-red-900/50 border border-red-700 rounded-lg p-4 flex items-center justify-between">
            <span className="text-red-400">{error}</span>
            <button onClick={clearError} className="text-red-400 hover:text-red-300">
              <svg className="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M6 18L18 6M6 6l12 12" />
              </svg>
            </button>
          </div>
        )}

        <div className="grid grid-cols-1 lg:grid-cols-4 gap-6">
          {/* Sidebar */}
          <div className="lg:col-span-1 space-y-4">
            {/* Connection status */}
            <ConnectionStatus
              isConnected={isConnected}
              isSupported={isSupported}
              isLoading={isLoading}
              status={status}
              onConnect={connect}
              onDisconnect={disconnect}
              onRefreshStatus={refreshStatus}
            />

            {/* Tab navigation */}
            <nav className="bg-gray-800 rounded-lg p-2 space-y-1">
              {tabs.map((tab) => {
                const isDisabled = (tab.requiresDevice && !isConnected) ||
                                   (tab.requiresWifi && needsWifiConfig);
                const disabledReason = tab.requiresDevice && !isConnected
                  ? 'Device required'
                  : tab.requiresWifi && needsWifiConfig
                    ? 'WiFi setup required'
                    : null;

                return (
                  <button
                    key={tab.id}
                    onClick={() => setActiveTab(tab.id)}
                    disabled={isDisabled}
                    className={`w-full flex items-center gap-3 px-3 py-2 rounded-lg text-left transition-colors ${
                      activeTab === tab.id
                        ? 'bg-blue-600 text-white'
                        : isDisabled
                          ? 'text-gray-600 cursor-not-allowed'
                          : 'text-gray-400 hover:text-gray-200 hover:bg-gray-700'
                    }`}
                  >
                    <span>{tab.icon}</span>
                    <span>{tab.label}</span>
                    {disabledReason && (
                      <span className="ml-auto text-xs text-gray-600">{disabledReason}</span>
                    )}
                  </button>
                );
              })}
            </nav>
          </div>

          {/* Main content */}
          <div className="lg:col-span-3">
            <div className="bg-gray-800 rounded-lg p-6">
              {activeTab === 'airports' && (
                <AirportConfig
                  config={activeConfig}
                  onSave={handleSaveConfig}
                  isLoading={isLoading}
                  isConnected={isConnected}
                  localAirports={hasLocalAirportsToUpload ? savedLocalAirports : undefined}
                  onUploadLocal={hasLocalAirportsToUpload ? handleUploadLocalConfig : undefined}
                />
              )}

              {activeTab === 'settings' && (
                <DeviceSettings
                  config={activeConfig}
                  onSave={handleSaveConfig}
                  onTestLeds={testLeds}
                  onReboot={reboot}
                  onFactoryReset={factoryReset}
                  isLoading={isLoading}
                  isConnected={isConnected}
                />
              )}

              {activeTab === 'wifi' && (
                <WiFiConfig
                  status={status}
                  onSetWifi={setWifi}
                  onResetWifi={resetWifi}
                  isLoading={isLoading}
                />
              )}

              {activeTab === 'monitor' && (
                <SerialMonitor
                  logs={logs}
                  onSendRaw={sendRaw}
                  onClear={clearLogs}
                  isConnected={isConnected}
                />
              )}

              {activeTab === 'firmware' && (
                <FirmwareInstall
                  isSerialConnected={isConnected}
                  onDisconnect={disconnect}
                />
              )}
            </div>
          </div>
        </div>
      </main>

      {/* Share Modal */}
      {showShareModal && (
        <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 p-4">
          <div className="bg-gray-800 rounded-lg p-6 max-w-lg w-full">
            <h3 className="text-lg font-semibold mb-4">Share Configuration</h3>
            <p className="text-gray-400 text-sm mb-4">
              Share this URL to let others import your airport configuration:
            </p>
            <div className="flex gap-2 mb-4">
              <input
                type="text"
                value={shareUrl || ''}
                readOnly
                className="flex-1 bg-gray-700 rounded-lg px-3 py-2 text-sm font-mono"
              />
              <button
                onClick={handleCopyUrl}
                className="px-4 py-2 bg-blue-600 hover:bg-blue-700 rounded-lg font-medium"
              >
                Copy
              </button>
            </div>
            <button
              onClick={() => setShowShareModal(false)}
              className="w-full px-4 py-2 bg-gray-700 hover:bg-gray-600 rounded-lg"
            >
              Close
            </button>
          </div>
        </div>
      )}

      {/* Upload Saved Config Modal */}
      {showUploadModal && pendingUploadConfig && (
        <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 p-4">
          <div className="bg-gray-800 rounded-lg p-6 max-w-lg w-full">
            <h3 className="text-lg font-semibold mb-4">Upload Saved Configuration?</h3>
            <p className="text-gray-400 text-sm mb-4">
              You have a saved configuration with {pendingUploadConfig.airports.length} airports.
              Would you like to upload it to this device?
            </p>
            <div className="bg-gray-700 rounded-lg p-3 mb-4 max-h-32 overflow-y-auto">
              <div className="flex flex-wrap gap-1">
                {pendingUploadConfig.airports.slice(0, 20).map((airport, i) => (
                  <span key={i} className="px-2 py-0.5 bg-gray-600 rounded text-xs font-mono">
                    {airport}
                  </span>
                ))}
                {pendingUploadConfig.airports.length > 20 && (
                  <span className="px-2 py-0.5 text-gray-400 text-xs">
                    +{pendingUploadConfig.airports.length - 20} more
                  </span>
                )}
              </div>
            </div>
            <div className="flex gap-2">
              <button
                onClick={handleUploadSavedConfig}
                disabled={isLoading}
                className="flex-1 px-4 py-2 bg-green-600 hover:bg-green-700 rounded-lg font-medium disabled:opacity-50"
              >
                {isLoading ? 'Uploading...' : 'Upload Saved Config'}
              </button>
              <button
                onClick={handleKeepDeviceConfig}
                className="flex-1 px-4 py-2 bg-gray-600 hover:bg-gray-700 rounded-lg font-medium"
              >
                Keep Device Config
              </button>
            </div>
          </div>
        </div>
      )}

      {/* Footer */}
      <footer className="border-t border-gray-800 mt-8">
        <div className="max-w-6xl mx-auto px-4 py-4 text-center text-gray-500 text-sm">
          LED Sectional Configuration Tool •{' '}
          <a
            href="https://github.com/WKHarmon/led-sectional"
            className="text-blue-400 hover:text-blue-300"
          >
            View on GitHub
          </a>
        </div>
      </footer>
    </div>
  );
}

export default App;
