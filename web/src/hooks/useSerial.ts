import { useState, useCallback, useEffect, useRef } from 'react';
import { serialService, type SerialLogCallback } from '../services/serial';
import type { DeviceConfig, DeviceStatus } from '../types/config';

export interface SerialLogEntry {
  timestamp: Date;
  message: string;
  type: 'tx' | 'rx' | 'info' | 'error';
}

export function useSerial() {
  const [isConnected, setIsConnected] = useState(false);
  const [isSupported, setIsSupported] = useState(true);
  const [config, setConfig] = useState<DeviceConfig | null>(null);
  const [status, setStatus] = useState<DeviceStatus | null>(null);
  const [logs, setLogs] = useState<SerialLogEntry[]>([]);
  const [isLoading, setIsLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  // Ref to track if a command is in progress (more reliable than state for async checks)
  const commandInProgressRef = useRef(false);

  const logCallbackRef = useRef<SerialLogCallback>((message, type) => {
    setLogs(prev => [...prev, { timestamp: new Date(), message, type }]);
  });

  useEffect(() => {
    serialService.isSupported().then(setIsSupported);
    serialService.setLogCallback(logCallbackRef.current);

    return () => {
      serialService.setLogCallback(null);
    };
  }, []);

  const connect = useCallback(async () => {
    setError(null);
    setIsLoading(true);
    try {
      const success = await serialService.connect();
      setIsConnected(success);
      if (success) {
        // Auto-fetch config and status on connect (sequentially to avoid response conflicts)
        try {
          const cfg = await serialService.getConfig();
          setConfig(cfg);
          const sts = await serialService.getStatus();
          setStatus(sts);
        } catch (e) {
          console.error('Failed to fetch initial data:', e);
        }
      }
      return success;
    } catch (e) {
      const message = e instanceof Error ? e.message : 'Connection failed';
      setError(message);
      return false;
    } finally {
      setIsLoading(false);
    }
  }, []);

  const disconnect = useCallback(async () => {
    setIsLoading(true);
    try {
      await serialService.disconnect();
      setIsConnected(false);
      setConfig(null);
      setStatus(null);
    } finally {
      setIsLoading(false);
    }
  }, []);

  const refreshConfig = useCallback(async () => {
    if (!isConnected) return;
    setIsLoading(true);
    setError(null);
    try {
      const cfg = await serialService.getConfig();
      setConfig(cfg);
    } catch (e) {
      const message = e instanceof Error ? e.message : 'Failed to refresh config';
      setError(message);
    } finally {
      setIsLoading(false);
    }
  }, [isConnected]);

  const refreshStatus = useCallback(async () => {
    if (!isConnected) return;
    try {
      const sts = await serialService.getStatus();
      setStatus(sts);
    } catch (e) {
      console.error('Failed to refresh status:', e);
    }
  }, [isConnected]);

  const saveConfig = useCallback(async (newConfig: Partial<DeviceConfig>) => {
    if (!isConnected) return false;
    commandInProgressRef.current = true;
    setIsLoading(true);
    setError(null);
    try {
      await serialService.setConfig(newConfig);
      // Refresh config to get updated values
      const cfg = await serialService.getConfig();
      setConfig(cfg);

      // If airports were changed, trigger a METAR fetch
      if (newConfig.airports) {
        try {
          await serialService.fetchMetars();
        } catch (e) {
          // Don't fail the save if METAR fetch fails
          console.error('Failed to fetch METARs after save:', e);
        }
      }

      return true;
    } catch (e) {
      const message = e instanceof Error ? e.message : 'Failed to save config';
      setError(message);
      return false;
    } finally {
      commandInProgressRef.current = false;
      setIsLoading(false);
    }
  }, [isConnected]);

  const setWifi = useCallback(async (ssid: string, password: string) => {
    if (!isConnected) return false;
    setIsLoading(true);
    setError(null);
    try {
      await serialService.setWifi(ssid, password);
      return true;
    } catch (e) {
      const message = e instanceof Error ? e.message : 'Failed to set WiFi';
      setError(message);
      return false;
    } finally {
      setIsLoading(false);
    }
  }, [isConnected]);

  const resetWifi = useCallback(async () => {
    if (!isConnected) return false;
    setIsLoading(true);
    setError(null);
    try {
      await serialService.resetWifi();
      return true;
    } catch (e) {
      const message = e instanceof Error ? e.message : 'Failed to reset WiFi';
      setError(message);
      return false;
    } finally {
      setIsLoading(false);
    }
  }, [isConnected]);

  const reboot = useCallback(async () => {
    if (!isConnected) return false;
    setIsLoading(true);
    try {
      await serialService.reboot();
      return true;
    } catch (e) {
      const message = e instanceof Error ? e.message : 'Failed to reboot';
      setError(message);
      return false;
    } finally {
      setIsLoading(false);
    }
  }, [isConnected]);

  const factoryReset = useCallback(async () => {
    if (!isConnected) return false;
    setIsLoading(true);
    try {
      await serialService.factoryReset();
      return true;
    } catch (e) {
      const message = e instanceof Error ? e.message : 'Failed to factory reset';
      setError(message);
      return false;
    } finally {
      setIsLoading(false);
    }
  }, [isConnected]);

  const testLeds = useCallback(async () => {
    if (!isConnected) return false;
    try {
      await serialService.testLeds();
      return true;
    } catch (e) {
      const message = e instanceof Error ? e.message : 'Failed to test LEDs';
      setError(message);
      return false;
    }
  }, [isConnected]);

  const sendRaw = useCallback(async (text: string) => {
    if (!isConnected) return;
    try {
      await serialService.sendRaw(text);
    } catch (e) {
      const message = e instanceof Error ? e.message : 'Failed to send';
      setError(message);
    }
  }, [isConnected]);

  const clearLogs = useCallback(() => {
    setLogs([]);
  }, []);

  const clearError = useCallback(() => {
    setError(null);
  }, []);

  // Check if a command is currently in progress (for preventing concurrent commands)
  const isCommandInProgress = useCallback(() => commandInProgressRef.current, []);

  return {
    isConnected,
    isSupported,
    isLoading,
    error,
    config,
    status,
    logs,
    connect,
    disconnect,
    refreshConfig,
    refreshStatus,
    saveConfig,
    setWifi,
    resetWifi,
    reboot,
    factoryReset,
    testLeds,
    sendRaw,
    clearLogs,
    clearError,
    isCommandInProgress,
  };
}
