import { useState } from 'react';
import type { DeviceStatus } from '../types/config';

interface WiFiConfigProps {
  status: DeviceStatus | null;
  onSetWifi: (ssid: string, password: string) => Promise<boolean>;
  onResetWifi: () => Promise<boolean>;
  isLoading: boolean;
}

export function WiFiConfig({
  status,
  onSetWifi,
  onResetWifi,
  isLoading,
}: WiFiConfigProps) {
  const [ssid, setSsid] = useState('');
  const [password, setPassword] = useState('');
  const [showPassword, setShowPassword] = useState(false);
  const [showResetConfirm, setShowResetConfirm] = useState(false);
  const [message, setMessage] = useState<{ type: 'success' | 'error'; text: string } | null>(null);

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setMessage(null);

    if (!ssid.trim()) {
      setMessage({ type: 'error', text: 'Please enter a WiFi network name' });
      return;
    }

    const success = await onSetWifi(ssid.trim(), password);
    if (success) {
      setMessage({ type: 'success', text: 'WiFi credentials updated. Device will reboot.' });
      setSsid('');
      setPassword('');
    } else {
      setMessage({ type: 'error', text: 'Failed to update WiFi credentials' });
    }
  };

  const handleReset = async () => {
    setMessage(null);
    const success = await onResetWifi();
    if (success) {
      setMessage({ type: 'success', text: 'WiFi settings cleared. Device will reboot into AP mode.' });
    } else {
      setMessage({ type: 'error', text: 'Failed to reset WiFi settings' });
    }
    setShowResetConfirm(false);
  };

  return (
    <div className="space-y-6">
      {/* Fresh device banner - needs WiFi config */}
      {status?.needs_wifi_config && (
        <div className="bg-yellow-900/50 border border-yellow-700 rounded-lg p-4">
          <div className="flex items-start gap-3">
            <svg className="w-6 h-6 text-yellow-400 mt-0.5 flex-shrink-0" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z" />
            </svg>
            <div>
              <h3 className="font-semibold text-yellow-400">WiFi Setup Required</h3>
              <p className="text-sm text-gray-300 mt-1">
                This device needs WiFi credentials. Enter your network details below to connect.
                The device will reboot and connect to your network.
              </p>
            </div>
          </div>
        </div>
      )}

      {/* Current Status */}
      {status && !status.needs_wifi_config && (
        <div className="bg-gray-800 rounded-lg p-4">
          <h3 className="font-semibold mb-3">Current Connection</h3>
          <div className="space-y-2 text-sm">
            <div className="flex justify-between">
              <span className="text-gray-400">Status</span>
              <span className={status.wifi_connected ? 'text-green-400' : 'text-red-400'}>
                {status.wifi_connected ? 'Connected' : 'Not Connected'}
              </span>
            </div>
            {status.wifi_connected && (
              <>
                <div className="flex justify-between">
                  <span className="text-gray-400">Network</span>
                  <span>{status.wifi_ssid}</span>
                </div>
                <div className="flex justify-between">
                  <span className="text-gray-400">IP Address</span>
                  <span className="font-mono">{status.ip_address}</span>
                </div>
              </>
            )}
          </div>
        </div>
      )}

      {/* WiFi Form */}
      <form onSubmit={handleSubmit} className="space-y-4">
        <h3 className="font-semibold">Configure WiFi</h3>

        <div>
          <label className="block text-sm font-medium text-gray-400 mb-1">
            Network Name (SSID)
          </label>
          <input
            type="text"
            value={ssid}
            onChange={(e) => setSsid(e.target.value)}
            placeholder="Enter WiFi network name"
            className="w-full bg-gray-700 rounded-lg px-3 py-2"
            disabled={isLoading}
          />
        </div>

        <div>
          <label className="block text-sm font-medium text-gray-400 mb-1">
            Password
          </label>
          <div className="relative">
            <input
              type={showPassword ? 'text' : 'password'}
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              placeholder="Enter WiFi password"
              className="w-full bg-gray-700 rounded-lg px-3 py-2 pr-10"
              disabled={isLoading}
            />
            <button
              type="button"
              onClick={() => setShowPassword(!showPassword)}
              className="absolute right-2 top-1/2 -translate-y-1/2 text-gray-400 hover:text-gray-300"
            >
              {showPassword ? (
                <svg className="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M13.875 18.825A10.05 10.05 0 0112 19c-4.478 0-8.268-2.943-9.543-7a9.97 9.97 0 011.563-3.029m5.858.908a3 3 0 114.243 4.243M9.878 9.878l4.242 4.242M9.88 9.88l-3.29-3.29m7.532 7.532l3.29 3.29M3 3l3.59 3.59m0 0A9.953 9.953 0 0112 5c4.478 0 8.268 2.943 9.543 7a10.025 10.025 0 01-4.132 5.411m0 0L21 21" />
                </svg>
              ) : (
                <svg className="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M15 12a3 3 0 11-6 0 3 3 0 016 0z" />
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M2.458 12C3.732 7.943 7.523 5 12 5c4.478 0 8.268 2.943 9.542 7-1.274 4.057-5.064 7-9.542 7-4.477 0-8.268-2.943-9.542-7z" />
                </svg>
              )}
            </button>
          </div>
        </div>

        {message && (
          <div
            className={`text-sm p-3 rounded-lg ${
              message.type === 'success'
                ? 'bg-green-900/50 text-green-400'
                : 'bg-red-900/50 text-red-400'
            }`}
          >
            {message.text}
          </div>
        )}

        <button
          type="submit"
          disabled={isLoading}
          className="w-full px-4 py-2 bg-blue-600 hover:bg-blue-700 rounded-lg font-medium disabled:opacity-50"
        >
          {isLoading ? 'Saving...' : 'Save WiFi Settings'}
        </button>
      </form>

      {/* Reset WiFi */}
      <div className="pt-4 border-t border-gray-700">
        <h3 className="font-semibold mb-3">Reset WiFi</h3>
        <p className="text-sm text-gray-400 mb-3">
          Clear stored WiFi credentials and reboot. You'll need to reconnect via Web Serial to configure WiFi again.
        </p>

        {!showResetConfirm ? (
          <button
            onClick={() => setShowResetConfirm(true)}
            disabled={isLoading}
            className="px-4 py-2 bg-yellow-600 hover:bg-yellow-700 rounded-lg font-medium disabled:opacity-50"
          >
            Reset WiFi Settings
          </button>
        ) : (
          <div className="bg-yellow-900/50 border border-yellow-700 rounded-lg p-4">
            <p className="text-sm mb-3">
              The device will forget the current WiFi network and reboot. You'll need to reconnect via Web Serial to configure WiFi again. Continue?
            </p>
            <div className="flex gap-2">
              <button
                onClick={handleReset}
                disabled={isLoading}
                className="flex-1 px-4 py-2 bg-yellow-600 hover:bg-yellow-700 rounded-lg font-medium disabled:opacity-50"
              >
                Yes, Reset
              </button>
              <button
                onClick={() => setShowResetConfirm(false)}
                className="flex-1 px-4 py-2 bg-gray-600 hover:bg-gray-700 rounded-lg font-medium"
              >
                Cancel
              </button>
            </div>
          </div>
        )}
      </div>
    </div>
  );
}
