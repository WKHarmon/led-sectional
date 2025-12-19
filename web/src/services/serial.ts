import type { SerialCommand, SerialResponse, DeviceConfig, DeviceStatus } from '../types/config';

const BAUD_RATE = 115200;
const RESPONSE_TIMEOUT = 5000; // 5 seconds

export interface SerialConnection {
  port: SerialPort;
  reader: ReadableStreamDefaultReader<Uint8Array> | null;
  writer: WritableStreamDefaultWriter<Uint8Array> | null;
  isConnected: boolean;
}

export type SerialLogCallback = (message: string, type: 'tx' | 'rx' | 'info' | 'error') => void;

class SerialService {
  private connection: SerialConnection | null = null;
  private logCallback: SerialLogCallback | null = null;
  private responseBuffer = '';
  private responseResolver: ((value: string) => void) | null = null;
  private readLoopPromise: Promise<void> | null = null;
  private isReading = false;

  setLogCallback(callback: SerialLogCallback | null) {
    this.logCallback = callback;
  }

  private log(message: string, type: 'tx' | 'rx' | 'info' | 'error') {
    if (this.logCallback) {
      this.logCallback(message, type);
    }
  }

  async isSupported(): Promise<boolean> {
    return 'serial' in navigator;
  }

  async connect(): Promise<boolean> {
    if (!await this.isSupported()) {
      this.log('Web Serial API not supported in this browser', 'error');
      return false;
    }

    try {
      // Request port from user
      const port = await navigator.serial.requestPort();

      // Open with baud rate
      await port.open({ baudRate: BAUD_RATE });

      this.connection = {
        port,
        reader: null,
        writer: null,
        isConnected: true,
      };

      // Set up writer
      if (port.writable) {
        this.connection.writer = port.writable.getWriter();
      }

      // Start read loop
      this.startReadLoop();

      this.log(`Connected at ${BAUD_RATE} baud`, 'info');
      return true;
    } catch (error) {
      const message = error instanceof Error ? error.message : 'Unknown error';
      this.log(`Connection failed: ${message}`, 'error');
      return false;
    }
  }

  async disconnect(): Promise<void> {
    if (!this.connection) return;

    const conn = this.connection;
    this.connection = null;  // Clear early to prevent race conditions
    this.isReading = false;

    try {
      // Release reader
      if (conn.reader) {
        try {
          await conn.reader.cancel();
          conn.reader.releaseLock();
        } catch (e) {
          // Reader may already be released
        }
      }

      // Release writer
      if (conn.writer) {
        try {
          await conn.writer.close();
        } catch (e) {
          // Writer may already be closed
        }
      }

      // Wait for read loop to finish
      if (this.readLoopPromise) {
        try {
          await this.readLoopPromise;
        } catch (e) {
          // Read loop may have errored
        }
      }

      // Close port
      try {
        await conn.port.close();
      } catch (e) {
        // Port may already be closed
      }

      this.log('Disconnected', 'info');
    } catch (error) {
      const message = error instanceof Error ? error.message : 'Unknown error';
      this.log(`Disconnect error: ${message}`, 'error');
    }
  }

  isConnected(): boolean {
    return this.connection?.isConnected ?? false;
  }

  private async startReadLoop(): Promise<void> {
    if (!this.connection?.port.readable) return;

    this.isReading = true;
    this.connection.reader = this.connection.port.readable.getReader();

    this.readLoopPromise = (async () => {
      const decoder = new TextDecoder();

      try {
        while (this.isReading && this.connection?.reader) {
          const { value, done } = await this.connection.reader.read();

          if (done) break;

          const text = decoder.decode(value);
          this.handleIncomingData(text);
        }
      } catch (error) {
        if (this.isReading) {
          const message = error instanceof Error ? error.message : 'Unknown error';
          this.log(`Read error: ${message}`, 'error');
        }
      } finally {
        if (this.connection?.reader) {
          this.connection.reader.releaseLock();
          this.connection.reader = null;
        }
      }
    })();
  }

  private handleIncomingData(text: string): void {
    // Add to buffer
    this.responseBuffer += text;

    // Log raw data
    for (const char of text) {
      if (char === '\n' || char === '\r') continue;
    }

    // Check for complete JSON responses (newline terminated)
    const lines = this.responseBuffer.split('\n');

    // Process complete lines
    for (let i = 0; i < lines.length - 1; i++) {
      const line = lines[i].trim();
      if (line) {
        this.log(line, 'rx');

        // Check if it's a JSON response we're waiting for
        if (this.responseResolver && line.startsWith('{')) {
          this.responseResolver(line);
          this.responseResolver = null;
        }
      }
    }

    // Keep incomplete line in buffer
    this.responseBuffer = lines[lines.length - 1];
  }

  async sendCommand(command: SerialCommand): Promise<SerialResponse> {
    if (!this.connection?.writer) {
      throw new Error('Not connected');
    }

    const json = JSON.stringify(command);
    this.log(json, 'tx');

    const encoder = new TextEncoder();
    const data = encoder.encode(json + '\n');

    // Send in chunks to avoid overwhelming the ESP8266 serial buffer
    // ESP8266 hardware serial buffer is 128 bytes, and main loop needs time to drain it
    const CHUNK_SIZE = 32;
    const CHUNK_DELAY_MS = 50;
    for (let i = 0; i < data.length; i += CHUNK_SIZE) {
      const chunk = data.slice(i, Math.min(i + CHUNK_SIZE, data.length));
      await this.connection.writer.write(chunk);
      // Delay between chunks to let ESP8266 main loop drain the serial buffer
      if (i + CHUNK_SIZE < data.length) {
        await new Promise(resolve => setTimeout(resolve, CHUNK_DELAY_MS));
      }
    }

    // Wait for response with timeout
    return new Promise((resolve, reject) => {
      const timeout = setTimeout(() => {
        this.responseResolver = null;
        reject(new Error('Response timeout'));
      }, RESPONSE_TIMEOUT);

      this.responseResolver = (responseText: string) => {
        clearTimeout(timeout);
        try {
          const response = JSON.parse(responseText) as SerialResponse;
          resolve(response);
        } catch {
          reject(new Error('Invalid JSON response'));
        }
      };
    });
  }

  async sendRaw(text: string): Promise<void> {
    if (!this.connection?.writer) {
      throw new Error('Not connected');
    }

    this.log(text, 'tx');
    const encoder = new TextEncoder();
    await this.connection.writer.write(encoder.encode(text + '\n'));
  }

  // High-level commands

  async getConfig(): Promise<DeviceConfig> {
    const response = await this.sendCommand({ cmd: 'get_config' });
    if (response.status !== 'ok' || !response.config) {
      throw new Error(response.message || 'Failed to get config');
    }
    return response.config;
  }

  async setConfig(config: Partial<DeviceConfig>): Promise<void> {
    const response = await this.sendCommand({ cmd: 'set_config', config });
    if (response.status !== 'ok') {
      throw new Error(response.message || 'Failed to save config');
    }
  }

  async getStatus(): Promise<DeviceStatus> {
    const response = await this.sendCommand({ cmd: 'get_status' });
    if (response.status !== 'ok') {
      throw new Error(response.message || 'Failed to get status');
    }
    return response as unknown as DeviceStatus;
  }

  async setWifi(ssid: string, password: string): Promise<void> {
    const response = await this.sendCommand({ cmd: 'set_wifi', ssid, pass: password });
    if (response.status !== 'ok') {
      throw new Error(response.message || 'Failed to set WiFi');
    }
  }

  async resetWifi(): Promise<void> {
    const response = await this.sendCommand({ cmd: 'reset_wifi' });
    if (response.status !== 'ok') {
      throw new Error(response.message || 'Failed to reset WiFi');
    }
  }

  async reboot(): Promise<void> {
    const response = await this.sendCommand({ cmd: 'reboot' });
    if (response.status !== 'ok') {
      throw new Error(response.message || 'Failed to reboot');
    }
  }

  async factoryReset(): Promise<void> {
    const response = await this.sendCommand({ cmd: 'factory_reset' });
    if (response.status !== 'ok') {
      throw new Error(response.message || 'Failed to factory reset');
    }
  }

  async testLeds(): Promise<void> {
    const response = await this.sendCommand({ cmd: 'test_leds' });
    if (response.status !== 'ok') {
      throw new Error(response.message || 'Failed to test LEDs');
    }
  }

  async fetchMetars(): Promise<void> {
    const response = await this.sendCommand({ cmd: 'fetch_metars' });
    if (response.status !== 'ok') {
      throw new Error(response.message || 'Failed to fetch METARs');
    }
  }
}

// Singleton instance
export const serialService = new SerialService();
