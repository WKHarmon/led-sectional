import { useState, useRef, useEffect } from 'react';
import type { SerialLogEntry } from '../hooks/useSerial';

interface SerialMonitorProps {
  logs: SerialLogEntry[];
  onSendRaw: (text: string) => void;
  onClear: () => void;
  isConnected: boolean;
}

export function SerialMonitor({ logs, onSendRaw, onClear, isConnected }: SerialMonitorProps) {
  const [input, setInput] = useState('');
  const [autoScroll, setAutoScroll] = useState(true);
  const logsEndRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (autoScroll && logsEndRef.current) {
      logsEndRef.current.scrollIntoView({ behavior: 'smooth' });
    }
  }, [logs, autoScroll]);

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    if (input.trim() && isConnected) {
      onSendRaw(input);
      setInput('');
    }
  };

  const getLogColor = (type: SerialLogEntry['type']) => {
    switch (type) {
      case 'tx': return 'text-blue-400';
      case 'rx': return 'text-green-400';
      case 'error': return 'text-red-400';
      case 'info': return 'text-gray-400';
      default: return 'text-gray-300';
    }
  };

  const getLogPrefix = (type: SerialLogEntry['type']) => {
    switch (type) {
      case 'tx': return '→';
      case 'rx': return '←';
      case 'error': return '✗';
      case 'info': return 'ℹ';
      default: return ' ';
    }
  };

  const formatTime = (date: Date) => {
    return date.toLocaleTimeString('en-US', {
      hour12: false,
      hour: '2-digit',
      minute: '2-digit',
      second: '2-digit',
    });
  };

  return (
    <div className="flex flex-col">
      {/* Toolbar */}
      <div className="flex items-center justify-between mb-2">
        <div className="flex items-center gap-4">
          <h3 className="font-semibold">Serial Monitor</h3>
          <label className="flex items-center gap-2 text-sm text-gray-400">
            <input
              type="checkbox"
              checked={autoScroll}
              onChange={(e) => setAutoScroll(e.target.checked)}
              className="rounded"
            />
            Auto-scroll
          </label>
        </div>
        <button
          onClick={onClear}
          className="px-3 py-1 text-sm bg-gray-700 hover:bg-gray-600 rounded"
        >
          Clear
        </button>
      </div>

      {/* Log output */}
      <div className="bg-gray-950 rounded-lg p-3 font-mono text-sm overflow-y-auto min-h-[200px] max-h-[40vh]">
        {logs.length === 0 ? (
          <div className="text-gray-600 text-center py-8">
            {isConnected ? 'Waiting for data...' : 'Connect to a device to view serial output'}
          </div>
        ) : (
          <>
            {logs.map((log, i) => (
              <div key={i} className="flex gap-2 hover:bg-gray-900/50">
                <span className="text-gray-600 shrink-0">{formatTime(log.timestamp)}</span>
                <span className={`shrink-0 ${getLogColor(log.type)}`}>
                  {getLogPrefix(log.type)}
                </span>
                <span className={`break-all ${getLogColor(log.type)}`}>
                  {log.message}
                </span>
              </div>
            ))}
            <div ref={logsEndRef} />
          </>
        )}
      </div>

      {/* Input */}
      <form onSubmit={handleSubmit} className="mt-2 flex gap-2">
        <input
          type="text"
          value={input}
          onChange={(e) => setInput(e.target.value)}
          placeholder={isConnected ? 'Type command and press Enter...' : 'Connect to send commands'}
          disabled={!isConnected}
          className="flex-1 bg-gray-700 rounded-lg px-3 py-2 font-mono text-sm disabled:opacity-50"
        />
        <button
          type="submit"
          disabled={!isConnected || !input.trim()}
          className="px-4 py-2 bg-blue-600 hover:bg-blue-700 rounded-lg font-medium disabled:opacity-50 disabled:cursor-not-allowed"
        >
          Send
        </button>
      </form>

      {/* Quick commands */}
      <div className="mt-2 flex flex-wrap gap-2">
        <span className="text-xs text-gray-500">Quick:</span>
        {[
          { label: 'Get Config', cmd: '{"cmd":"get_config"}' },
          { label: 'Get Status', cmd: '{"cmd":"get_status"}' },
          { label: 'Fetch METARs', cmd: '{"cmd":"fetch_metars"}' },
          { label: 'Test LEDs', cmd: '{"cmd":"test_leds"}' },
          { label: 'Reboot', cmd: '{"cmd":"reboot"}' },
        ].map(({ label, cmd }) => (
          <button
            key={cmd}
            onClick={() => onSendRaw(cmd)}
            disabled={!isConnected}
            className="px-2 py-1 text-xs bg-gray-700 hover:bg-gray-600 rounded disabled:opacity-50"
          >
            {label}
          </button>
        ))}
      </div>
    </div>
  );
}
