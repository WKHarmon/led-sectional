import { useEffect, useState, useRef, useCallback } from 'react';

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
}

export function FirmwareInstall({ isSerialConnected = false, onDisconnect }: FirmwareInstallProps) {
  const [isSupported, setIsSupported] = useState(true);
  const [isDisconnecting, setIsDisconnecting] = useState(false);
  const buttonRef = useRef<HTMLElement>(null);

  useEffect(() => {
    // Check if Web Serial is supported
    setIsSupported('serial' in navigator);
  }, []);

  // Handle click on install button - disconnect first if connected
  const handleInstallClick = useCallback(async (e: React.MouseEvent) => {
    if (isSerialConnected && onDisconnect) {
      e.preventDefault();
      e.stopPropagation();
      setIsDisconnecting(true);
      try {
        await onDisconnect();
        // Small delay to ensure port is fully released
        await new Promise(resolve => setTimeout(resolve, 500));
        // Now trigger the actual install button
        const installButton = buttonRef.current?.shadowRoot?.querySelector('button') ||
                              buttonRef.current?.querySelector('button[slot="activate"]');
        if (installButton) {
          (installButton as HTMLButtonElement).click();
        }
      } catch (err) {
        console.error('Failed to disconnect:', err);
      } finally {
        setIsDisconnecting(false);
      }
    }
  }, [isSerialConnected, onDisconnect]);

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

  return (
    <div className="space-y-6">
      <div>
        <h3 className="text-lg font-semibold mb-2">Install Firmware</h3>
        <p className="text-gray-400 text-sm mb-4">
          Flash the LED Sectional firmware directly from your browser. Make sure your device is connected via USB and no other program is using the serial port.
        </p>
      </div>

      {/* Connection warning */}
      {isSerialConnected && (
        <div className="bg-yellow-900/50 border border-yellow-700 rounded-lg p-4 mb-4">
          <p className="text-yellow-400 text-sm">
            <strong>Note:</strong> A serial connection is active. It will be disconnected automatically when you click Install Firmware.
          </p>
        </div>
      )}

      {/* ESP Web Tools button */}
      <div className="bg-gray-800 rounded-lg p-6 text-center" onClick={handleInstallClick}>
        <esp-web-install-button manifest="firmware/manifest.json" ref={buttonRef}>
          <button
            slot="activate"
            className="px-6 py-3 bg-blue-600 hover:bg-blue-700 rounded-lg font-medium text-lg disabled:opacity-50"
            disabled={isDisconnecting}
          >
            {isDisconnecting ? 'Disconnecting...' : 'Install Firmware'}
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
