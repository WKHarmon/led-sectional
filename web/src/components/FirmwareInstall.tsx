import { useEffect, useState, useCallback } from 'react';

// Declare ESP Web Tools types
declare global {
  namespace JSX {
    interface IntrinsicElements {
      'esp-web-install-button': React.DetailedHTMLProps<
        React.HTMLAttributes<HTMLElement> & {
          manifest?: string;
        },
        HTMLElement
      >;
    }
  }
}

interface FirmwareInstallProps {
  isSerialConnected?: boolean;
  onDisconnect?: () => Promise<void>;
  deviceVersion?: string;
  latestVersion?: string | null;
}

export function FirmwareInstall({
  isSerialConnected = false,
  onDisconnect,
  deviceVersion,
  latestVersion,
}: FirmwareInstallProps) {
  const [isSupported, setIsSupported] = useState(true);
  const [isDisconnecting, setIsDisconnecting] = useState(false);
  const [justDisconnected, setJustDisconnected] = useState(false);
  const [installState, setInstallState] = useState<string | null>(null);

  useEffect(() => {
    // Check if Web Serial is supported
    setIsSupported('serial' in navigator);
  }, []);

  // Watch for ESP Web Tools dialog and auto-close on completion
  useEffect(() => {
    // Use MutationObserver to detect when the dialog appears and monitor its state
    const observer = new MutationObserver((mutations) => {
      for (const mutation of mutations) {
        for (const node of mutation.addedNodes) {
          if (node instanceof HTMLElement && node.tagName.toLowerCase() === 'ewt-install-dialog') {
            console.log('[ESP Web Tools] Dialog detected');

            // Watch this dialog for state changes
            const dialogObserver = new MutationObserver(() => {
              // Check for completion state by looking for ewt-page-message with label containing "complete"
              const shadowRoot = node.shadowRoot;
              if (shadowRoot) {
                const pageMessage = shadowRoot.querySelector('ewt-page-message[label*="complete" i]');
                if (pageMessage) {
                  console.log('[ESP Web Tools] Installation complete detected');
                  setInstallState('FINISHED');

                  // Click "Next" to go back to initial dialog, then close
                  setTimeout(() => {
                    const shadowRoot = node.shadowRoot;
                    if (shadowRoot) {
                      // Find and click the "Next" button
                      const nextButton = shadowRoot.querySelector('ew-text-button');
                      if (nextButton instanceof HTMLElement) {
                        console.log('[ESP Web Tools] Clicking Next button');
                        nextButton.click();

                        // After clicking Next, wait then click the close button (ew-icon-button in headline)
                        setTimeout(() => {
                          const closeButton = shadowRoot.querySelector('ew-icon-button[slot="headline"]');
                          if (closeButton instanceof HTMLElement) {
                            console.log('[ESP Web Tools] Clicking close button');
                            closeButton.click();
                          }
                        }, 300);
                      }
                    }
                  }, 500);

                  dialogObserver.disconnect();
                }
              }
            });

            // Observe the dialog's shadow DOM for changes
            if (node.shadowRoot) {
              dialogObserver.observe(node.shadowRoot, {
                childList: true,
                subtree: true,
                characterData: true
              });
            } else {
              // Shadow root might not be available immediately, wait for it
              const shadowWatcher = setInterval(() => {
                if (node.shadowRoot) {
                  dialogObserver.observe(node.shadowRoot, {
                    childList: true,
                    subtree: true,
                    characterData: true
                  });
                  clearInterval(shadowWatcher);
                }
              }, 100);

              // Clean up after 30 seconds
              setTimeout(() => clearInterval(shadowWatcher), 30000);
            }
          }
        }
      }
    });

    observer.observe(document.body, { childList: true });

    return () => observer.disconnect();
  }, []);

  // Reset justDisconnected when connection state changes
  useEffect(() => {
    if (isSerialConnected) {
      setJustDisconnected(false);
    }
  }, [isSerialConnected]);

  // Handle disconnect button click
  const handleDisconnect = useCallback(async () => {
    if (onDisconnect) {
      setIsDisconnecting(true);
      try {
        await onDisconnect();
        setJustDisconnected(true);
      } catch (err) {
        console.error('Failed to disconnect:', err);
      } finally {
        setIsDisconnecting(false);
      }
    }
  }, [onDisconnect]);

  if (!isSupported) {
    return (
      <div className="bg-red-900/50 border border-red-700 rounded-lg p-4">
        <h3 className="text-red-400 font-semibold">Browser Not Supported</h3>
        <p className="text-sm text-gray-400 mt-1">
          Your browser doesn't support Web Serial. Please use Chrome, Edge, or Opera on desktop to flash firmware.
        </p>
      </div>
    );
  }

  // Check if firmware is up to date
  const isUpToDate = deviceVersion && latestVersion && deviceVersion !== 'dev'
    ? deviceVersion === latestVersion || compareVersions(deviceVersion, latestVersion) >= 0
    : false;

  // Simple version comparison
  function compareVersions(a: string, b: string): number {
    const aParts = a.split('.').map(Number);
    const bParts = b.split('.').map(Number);
    for (let i = 0; i < Math.max(aParts.length, bParts.length); i++) {
      const aVal = aParts[i] || 0;
      const bVal = bParts[i] || 0;
      if (aVal < bVal) return -1;
      if (aVal > bVal) return 1;
    }
    return 0;
  }

  return (
    <div className="space-y-6">
      <div>
        <h3 className="text-lg font-semibold mb-2">Install Firmware</h3>
        <p className="text-gray-400 text-sm mb-2">
          Flash the LED Sectional firmware directly from your browser. Make sure your device is connected via USB and no other program is using the serial port.
        </p>
        <p className="text-gray-500 text-sm">
          <em>Note: You only need to install firmware once per device. Return here to install updates when new versions are released.</em>
        </p>
      </div>

      {/* Version status */}
      {(deviceVersion || latestVersion) && (
        <div className={`rounded-lg p-4 ${
          isUpToDate
            ? 'bg-green-900/30 border border-green-700'
            : deviceVersion
              ? 'bg-yellow-900/30 border border-yellow-700'
              : 'bg-gray-700/50 border border-gray-600'
        }`}>
          <div className="flex items-center justify-between">
            <div className="space-y-1">
              {deviceVersion && (
                <div className="text-sm">
                  <span className="text-gray-400">Device version: </span>
                  <span className={`font-mono ${deviceVersion === 'dev' ? 'text-yellow-400' : 'text-white'}`}>
                    {deviceVersion === 'dev' ? 'Development build' : `v${deviceVersion}`}
                  </span>
                </div>
              )}
              {latestVersion && (
                <div className="text-sm">
                  <span className="text-gray-400">Latest version: </span>
                  <span className="font-mono text-white">v{latestVersion}</span>
                </div>
              )}
            </div>
            <div className="text-right">
              {isUpToDate ? (
                <span className="text-green-400 font-medium">Up to date</span>
              ) : deviceVersion ? (
                <span className="text-yellow-400 font-medium">Update available</span>
              ) : !isSerialConnected ? (
                <span className="text-gray-400 text-sm">Connect device to check version</span>
              ) : null}
            </div>
          </div>
        </div>
      )}

      {/* Connection status and actions */}
      {isSerialConnected && (
        <div className="bg-yellow-900/50 border border-yellow-700 rounded-lg p-4">
          <p className="text-yellow-400 text-sm mb-3">
            <strong>Note:</strong> A serial connection is active. Please disconnect before installing firmware.
          </p>
          <button
            onClick={handleDisconnect}
            disabled={isDisconnecting}
            className="px-4 py-2 bg-yellow-600 hover:bg-yellow-700 rounded-lg font-medium disabled:opacity-50"
          >
            {isDisconnecting ? 'Disconnecting...' : 'Disconnect Device'}
          </button>
        </div>
      )}

      {/* Just disconnected message */}
      {justDisconnected && !isSerialConnected && (
        <div className="bg-green-900/50 border border-green-700 rounded-lg p-4">
          <p className="text-green-400 text-sm">
            Device disconnected. Click <strong>Install Firmware</strong> below to continue.
          </p>
        </div>
      )}

      {/* Install state feedback */}
      {installState === 'FINISHED' && (
        <div className="bg-green-900/50 border border-green-700 rounded-lg p-4">
          <p className="text-green-400 text-sm">
            <strong>Firmware installed successfully!</strong> Click the <strong>Connect</strong> button at the top of the page to configure your device.
          </p>
        </div>
      )}

      {/* ESP Web Tools button */}
      <div className="bg-gray-800 rounded-lg p-6 text-center">
        <esp-web-install-button manifest="firmware/manifest.json">
          <button
            slot="activate"
            className={`px-6 py-3 rounded-lg font-medium text-lg ${
              isSerialConnected
                ? 'bg-gray-600 cursor-not-allowed opacity-50'
                : 'bg-blue-600 hover:bg-blue-700'
            }`}
            disabled={isSerialConnected}
          >
            Install Firmware
          </button>
          <span slot="unsupported">
            <div className="text-red-400">
              Your browser doesn't support Web Serial.
              <br />
              Please use Chrome, Edge, or Opera.
            </div>
          </span>
          <span slot="not-allowed">
            <div className="text-yellow-400">
              Serial port access was denied.
              <br />
              Please grant permission and try again.
            </div>
          </span>
        </esp-web-install-button>
      </div>

      {/* Instructions */}
      <div className="space-y-4">
        <h4 className="font-semibold">Instructions</h4>
        <ol className="list-decimal list-inside space-y-2 text-sm text-gray-400">
          <li>Connect your ESP8266 board to your computer via USB</li>
          <li>If this is the first time, you may need to install USB drivers</li>
          <li>Click the "Install Firmware" button above</li>
          <li>Select the correct serial port when prompted</li>
          <li>Wait for the installation to complete</li>
          <li>The device will reboot and start the WiFi setup portal</li>
        </ol>
      </div>

      {/* Troubleshooting */}
      <div className="space-y-4">
        <h4 className="font-semibold">Troubleshooting</h4>
        <div className="text-sm text-gray-400 space-y-2">
          <p>
            <strong className="text-gray-300">No ports showing up?</strong>
            <br />
            Make sure the USB cable supports data transfer (not charge-only). Try a different cable or USB port.
          </p>
          <p>
            <strong className="text-gray-300">Installation failing?</strong>
            <br />
            Close any other programs that might be using the serial port (Arduino IDE, PlatformIO, serial monitors, etc.)
          </p>
          <p>
            <strong className="text-gray-300">Need USB drivers?</strong>
            <br />
            Most ESP8266 boards use CH340 or CP2102 USB chips. Download drivers from the manufacturer's website if needed.
          </p>
        </div>
      </div>

      {/* Alternative methods */}
      <div className="pt-4 border-t border-gray-700">
        <h4 className="font-semibold mb-2">Alternative Installation Methods</h4>
        <p className="text-sm text-gray-400 mb-3">
          If browser-based flashing doesn't work, you can use these alternatives:
        </p>
        <ul className="text-sm text-gray-400 space-y-1">
          <li>
            • <strong className="text-gray-300">PlatformIO:</strong> Clone the repository and run{' '}
            <code className="bg-gray-700 px-1 rounded">pio run -t upload</code>
          </li>
          <li>
            • <strong className="text-gray-300">Arduino IDE:</strong> Open the project and upload using the IDE
          </li>
          <li>
            • <strong className="text-gray-300">esptool:</strong> Use{' '}
            <code className="bg-gray-700 px-1 rounded">esptool.py write_flash 0x0 firmware.bin</code>
          </li>
        </ul>
      </div>
    </div>
  );
}
