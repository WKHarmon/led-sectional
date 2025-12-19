import type { DeviceStatus } from '../types/config';

interface ConnectionStatusProps {
  isConnected: boolean;
  isSupported: boolean;
  isLoading: boolean;
  status: DeviceStatus | null;
  onConnect: () => void;
  onDisconnect: () => void;
  onRefreshStatus: () => void;
}

export function ConnectionStatus({
  isConnected,
  isSupported,
  isLoading,
  status,
  onConnect,
  onDisconnect,
  onRefreshStatus,
}: ConnectionStatusProps) {
  if (!isSupported) {
    return (
      <div className="bg-red-900/50 border border-red-700 rounded-lg p-4">
        <h3 className="text-red-400 font-semibold">Web Serial Not Supported</h3>
        <p className="text-sm text-gray-400 mt-1">
          Your browser doesn't support the Web Serial API. Please use Chrome, Edge, or Opera on desktop.
        </p>
      </div>
    );
  }

  return (
    <div className="bg-gray-800 rounded-lg p-4">
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center gap-3">
          <div
            className={`w-3 h-3 rounded-full ${
              isConnected ? 'bg-green-500 animate-pulse' : 'bg-gray-600'
            }`}
          />
          <span className="font-medium">
            {isConnected ? 'Connected' : 'Disconnected'}
          </span>
        </div>
        <button
          onClick={isConnected ? onDisconnect : onConnect}
          disabled={isLoading}
          className={`px-4 py-2 rounded-lg font-medium transition-colors ${
            isConnected
              ? 'bg-red-600 hover:bg-red-700 disabled:bg-red-800'
              : 'bg-blue-600 hover:bg-blue-700 disabled:bg-blue-800'
          } disabled:opacity-50 disabled:cursor-not-allowed`}
        >
          {isLoading ? 'Working...' : isConnected ? 'Disconnect' : 'Connect'}
        </button>
      </div>

      {isConnected && status && (
        <div className="space-y-2 text-sm">
          <div className="flex justify-between">
            <span className="text-gray-400">WiFi Status</span>
            <span className={status.wifi_connected ? 'text-green-400' : 'text-red-400'}>
              {status.wifi_connected ? `Connected to ${status.wifi_ssid}` : 'Not connected'}
            </span>
          </div>
          {status.wifi_connected && (
            <div className="flex justify-between">
              <span className="text-gray-400">IP Address</span>
              <span>{status.ip_address}</span>
            </div>
          )}
          <div className="flex justify-between">
            <span className="text-gray-400">Airports</span>
            <span>{status.airport_count}</span>
          </div>
          <div className="flex justify-between">
            <span className="text-gray-400">Brightness</span>
            <span>{status.brightness}</span>
          </div>
          <div className="flex justify-between">
            <span className="text-gray-400">Free Memory</span>
            <span>{(status.heap_free / 1024).toFixed(1)} KB</span>
          </div>
          <div className="flex justify-between">
            <span className="text-gray-400">Uptime</span>
            <span>{formatUptime(status.uptime)}</span>
          </div>
          <div className="flex justify-between">
            <span className="text-gray-400">Last METAR Update</span>
            <span>{status.last_metar_update > 0 ? formatUptime(status.uptime - status.last_metar_update) + ' ago' : 'Never'}</span>
          </div>
          <div className="mt-3 pt-3 border-t border-gray-700">
            <button
              onClick={onRefreshStatus}
              className="text-blue-400 hover:text-blue-300 text-sm"
            >
              Refresh Status
            </button>
          </div>
        </div>
      )}
    </div>
  );
}

function formatUptime(seconds: number): string {
  if (seconds < 60) return `${seconds}s`;
  if (seconds < 3600) return `${Math.floor(seconds / 60)}m ${seconds % 60}s`;
  const hours = Math.floor(seconds / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);
  return `${hours}h ${minutes}m`;
}
