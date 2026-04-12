import { useState, useEffect } from 'react';
import type { DeviceConfig } from '../types/config';
import { LightSensorType } from '../types/config';

interface DeviceSettingsProps {
  config: DeviceConfig;
  onSave: (config: Partial<DeviceConfig>) => Promise<boolean>;
  onTestLeds: () => void;
  onReboot: () => void;
  onFactoryReset: () => void;
  isLoading: boolean;
  isConnected?: boolean;
}

export function DeviceSettings({
  config,
  onSave,
  onTestLeds,
  onReboot,
  onFactoryReset,
  isLoading,
  isConnected = false,
}: DeviceSettingsProps) {
  const [localConfig, setLocalConfig] = useState<Partial<DeviceConfig>>({});
  const [hasChanges, setHasChanges] = useState(false);
  const [showFactoryResetConfirm, setShowFactoryResetConfirm] = useState(false);
  const [showAdvanced, setShowAdvanced] = useState(false);

  useEffect(() => {
    setLocalConfig({
      brightness: config.brightness,
      windThreshold: config.windThreshold,
      doLightning: config.doLightning,
      doWinds: config.doWinds,
      windAlternate: config.windAlternate,
      windAlternateInterval: config.windAlternateInterval,
      useLightSensor: config.useLightSensor,
      lightSensorType: config.lightSensorType,
      minBrightness: config.minBrightness,
      maxBrightness: config.maxBrightness,
      minLight: config.minLight,
      maxLight: config.maxLight,
      requestInterval: config.requestInterval,
      loopInterval: config.loopInterval,
      dataPin: config.dataPin,
      colorOrder: config.colorOrder,
      mqttEnabled: config.mqttEnabled,
      mqttBroker: config.mqttBroker,
      mqttPort: config.mqttPort,
      mqttUsername: config.mqttUsername,
      mqttPassword: config.mqttPassword,
      powerOn: config.powerOn,
      noDataColor: config.noDataColor,
    });
    setHasChanges(false);
  }, [config]);

  const updateConfig = <K extends keyof DeviceConfig>(key: K, value: DeviceConfig[K]) => {
    setLocalConfig(prev => ({ ...prev, [key]: value }));
    setHasChanges(true);
  };

  const handleSave = async () => {
    const success = await onSave(localConfig);
    if (success) {
      setHasChanges(false);
    }
  };

  return (
    <div className="space-y-6">
      {/* LED Settings */}
      <section>
        <h3 className="text-lg font-semibold mb-4">LED Settings</h3>
        <div className="space-y-4">
          <div>
            <label className="block text-sm font-medium text-gray-400 mb-1">
              Brightness ({localConfig.brightness})
            </label>
            <input
              type="range"
              min="1"
              max="255"
              value={localConfig.brightness || 20}
              onChange={(e) => updateConfig('brightness', parseInt(e.target.value))}
              className="w-full"
            />
            <div className="flex justify-between text-xs text-gray-500">
              <span>Dim</span>
              <span>Bright</span>
            </div>
          </div>

          <button
            onClick={onTestLeds}
            disabled={isLoading || !isConnected}
            className="px-4 py-2 bg-purple-600 hover:bg-purple-700 rounded-lg font-medium disabled:opacity-50"
            title={!isConnected ? 'Connect a device to test LEDs' : undefined}
          >
            Test LED Pattern
          </button>
          {!isConnected && (
            <p className="text-xs text-gray-500">Connect a device to test LEDs</p>
          )}
        </div>
      </section>

      {/* Weather Display */}
      <section>
        <h3 className="text-lg font-semibold mb-4">Weather Display</h3>
        <div className="space-y-4">
          <div className="flex items-center justify-between">
            <label className="text-sm font-medium text-gray-400">
              Lightning Animation
            </label>
            <button
              onClick={() => updateConfig('doLightning', !localConfig.doLightning)}
              className={`relative inline-flex h-6 w-11 items-center rounded-full transition-colors ${
                localConfig.doLightning ? 'bg-blue-600' : 'bg-gray-600'
              }`}
            >
              <span
                className={`inline-block h-4 w-4 transform rounded-full bg-white transition-transform ${
                  localConfig.doLightning ? 'translate-x-6' : 'translate-x-1'
                }`}
              />
            </button>
          </div>

          <div className="flex items-center justify-between">
            <label className="text-sm font-medium text-gray-400">
              Wind Indication (Yellow for VFR with high winds)
            </label>
            <button
              onClick={() => updateConfig('doWinds', !localConfig.doWinds)}
              className={`relative inline-flex h-6 w-11 items-center rounded-full transition-colors ${
                localConfig.doWinds ? 'bg-blue-600' : 'bg-gray-600'
              }`}
            >
              <span
                className={`inline-block h-4 w-4 transform rounded-full bg-white transition-transform ${
                  localConfig.doWinds ? 'translate-x-6' : 'translate-x-1'
                }`}
              />
            </button>
          </div>

          {localConfig.doWinds && (
            <>
              <div>
                <label className="block text-sm font-medium text-gray-400 mb-1">
                  Wind Threshold ({localConfig.windThreshold} knots)
                </label>
                <input
                  type="range"
                  min="10"
                  max="50"
                  value={localConfig.windThreshold || 25}
                  onChange={(e) => updateConfig('windThreshold', parseInt(e.target.value))}
                  className="w-full"
                />
                <div className="flex justify-between text-xs text-gray-500">
                  <span>10 kt</span>
                  <span>50 kt</span>
                </div>
              </div>

              <div className="flex items-center justify-between">
                <div>
                  <label className="text-sm font-medium text-gray-400">
                    Alternate Colors
                  </label>
                  <p className="text-xs text-gray-500 mt-0.5">
                    Blink between flight category and yellow
                  </p>
                </div>
                <button
                  onClick={() => updateConfig('windAlternate', !localConfig.windAlternate)}
                  className={`relative inline-flex h-6 w-11 items-center rounded-full transition-colors ${
                    localConfig.windAlternate ? 'bg-blue-600' : 'bg-gray-600'
                  }`}
                >
                  <span
                    className={`inline-block h-4 w-4 transform rounded-full bg-white transition-transform ${
                      localConfig.windAlternate ? 'translate-x-6' : 'translate-x-1'
                    }`}
                  />
                </button>
              </div>

              {localConfig.windAlternate && (
                <div>
                  <label className="block text-sm font-medium text-gray-400 mb-1">
                    Blink Interval ({(localConfig.windAlternateInterval || 2000) / 1000} seconds)
                  </label>
                  <input
                    type="range"
                    min="1000"
                    max="10000"
                    step="1000"
                    value={localConfig.windAlternateInterval || 2000}
                    onChange={(e) => updateConfig('windAlternateInterval', parseInt(e.target.value))}
                    className="w-full"
                  />
                  <div className="flex justify-between text-xs text-gray-500">
                    <span>1 sec</span>
                    <span>10 sec</span>
                  </div>
                </div>
              )}
            </>
          )}
        </div>
      </section>

      {/* Light Sensor */}
      <section>
        <h3 className="text-lg font-semibold mb-4">Light Sensor</h3>
        <div className="space-y-4">
          <div className="flex items-center justify-between">
            <label className="text-sm font-medium text-gray-400">
              Use Light Sensor
            </label>
            <button
              onClick={() => {
                const enabling = !localConfig.useLightSensor;
                updateConfig('useLightSensor', enabling);
                // Default to TSL2561 when enabling if no sensor type is set (None = 0 is falsy)
                if (enabling && !localConfig.lightSensorType) {
                  updateConfig('lightSensorType', LightSensorType.TSL2561);
                }
              }}
              className={`relative inline-flex h-6 w-11 items-center rounded-full transition-colors ${
                localConfig.useLightSensor ? 'bg-blue-600' : 'bg-gray-600'
              }`}
            >
              <span
                className={`inline-block h-4 w-4 transform rounded-full bg-white transition-transform ${
                  localConfig.useLightSensor ? 'translate-x-6' : 'translate-x-1'
                }`}
              />
            </button>
          </div>

          {localConfig.useLightSensor && (
            <>
              <div>
                <label className="block text-sm font-medium text-gray-400 mb-1">
                  Sensor Type
                </label>
                <select
                  value={localConfig.lightSensorType || LightSensorType.TSL2561}
                  onChange={(e) => updateConfig('lightSensorType', parseInt(e.target.value) as LightSensorType)}
                  className="w-full bg-gray-700 rounded-lg px-3 py-2"
                >
                  <option value={LightSensorType.TSL2561}>Digital (TSL2561)</option>
                  <option value={LightSensorType.Analog}>Analog (A0) - Older kits only</option>
                </select>
              </div>

              <div className="grid grid-cols-2 gap-4">
                <div>
                  <label className="block text-sm font-medium text-gray-400 mb-1">
                    Min Brightness
                  </label>
                  <input
                    type="number"
                    min="0"
                    max="255"
                    value={localConfig.minBrightness || 20}
                    onChange={(e) => updateConfig('minBrightness', parseInt(e.target.value))}
                    className="w-full bg-gray-700 rounded-lg px-3 py-2"
                  />
                </div>
                <div>
                  <label className="block text-sm font-medium text-gray-400 mb-1">
                    Max Brightness
                  </label>
                  <input
                    type="number"
                    min="0"
                    max="255"
                    value={localConfig.maxBrightness || 30}
                    onChange={(e) => updateConfig('maxBrightness', parseInt(e.target.value))}
                    className="w-full bg-gray-700 rounded-lg px-3 py-2"
                  />
                </div>
                <div>
                  <label className="block text-sm font-medium text-gray-400 mb-1">
                    Min Light
                  </label>
                  <input
                    type="number"
                    min="0"
                    max="1000"
                    value={localConfig.minLight || 16}
                    onChange={(e) => updateConfig('minLight', parseInt(e.target.value))}
                    className="w-full bg-gray-700 rounded-lg px-3 py-2"
                  />
                </div>
                <div>
                  <label className="block text-sm font-medium text-gray-400 mb-1">
                    Max Light
                  </label>
                  <input
                    type="number"
                    min="0"
                    max="1000"
                    value={localConfig.maxLight || 30}
                    onChange={(e) => updateConfig('maxLight', parseInt(e.target.value))}
                    className="w-full bg-gray-700 rounded-lg px-3 py-2"
                  />
                </div>
              </div>
            </>
          )}
        </div>
      </section>

      {/* Timing */}
      <section>
        <h3 className="text-lg font-semibold mb-4">Update Intervals</h3>
        <div className="space-y-4">
          <div>
            <label className="block text-sm font-medium text-gray-400 mb-1">
              METAR Request Interval
            </label>
            <select
              value={localConfig.requestInterval || 900000}
              onChange={(e) => updateConfig('requestInterval', parseInt(e.target.value))}
              className="w-full bg-gray-700 rounded-lg px-3 py-2"
            >
              <option value={300000}>5 minutes</option>
              <option value={600000}>10 minutes</option>
              <option value={900000}>15 minutes (recommended)</option>
              <option value={1800000}>30 minutes</option>
              <option value={3600000}>60 minutes</option>
            </select>
          </div>

          <div>
            <label className="block text-sm font-medium text-gray-400 mb-1">
              Loop Interval (for lightning/sensor)
            </label>
            <select
              value={localConfig.loopInterval || 5000}
              onChange={(e) => updateConfig('loopInterval', parseInt(e.target.value))}
              className="w-full bg-gray-700 rounded-lg px-3 py-2"
            >
              <option value={1000}>1 second</option>
              <option value={2000}>2 seconds</option>
              <option value={5000}>5 seconds (recommended)</option>
              <option value={10000}>10 seconds</option>
            </select>
          </div>
        </div>
      </section>

      {/* Save Changes */}
      {hasChanges && (
        <div className="sticky bottom-0 bg-gray-900 py-4 border-t border-gray-700">
          <button
            onClick={handleSave}
            disabled={isLoading}
            className="w-full px-4 py-3 bg-green-600 hover:bg-green-700 rounded-lg font-medium disabled:opacity-50"
          >
            {isLoading ? 'Saving...' : 'Save Settings'}
          </button>
        </div>
      )}

      {/* Advanced Settings */}
      <section className="pt-4 border-t border-gray-700">
        <button
          onClick={() => setShowAdvanced(!showAdvanced)}
          className="flex items-center justify-between w-full text-left"
        >
          <h3 className="text-lg font-semibold">Advanced Settings</h3>
          <svg
            className={`w-5 h-5 text-gray-400 transition-transform ${showAdvanced ? 'rotate-180' : ''}`}
            fill="none"
            stroke="currentColor"
            viewBox="0 0 24 24"
          >
            <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M19 9l-7 7-7-7" />
          </svg>
        </button>

        {showAdvanced && (
          <div className="mt-4 space-y-6">
            <div>
              <label className="block text-sm font-medium text-gray-400 mb-1">
                LED Data Pin (GPIO)
              </label>
              <select
                value={localConfig.dataPin || 14}
                onChange={(e) => updateConfig('dataPin', parseInt(e.target.value))}
                className="w-full bg-gray-700 rounded-lg px-3 py-2"
              >
                <option value={14}>GPIO 14 (D5) - New kits</option>
                <option value={5}>GPIO 5 (D1) - Old kits</option>
              </select>
              <p className="mt-1 text-xs text-gray-500">
                Only change this if you have an older board with LEDs on a different pin.
              </p>
            </div>

            <div>
              <label className="block text-sm font-medium text-gray-400 mb-1">
                LED Color Order
              </label>
              <select
                value={localConfig.colorOrder || 'RGB'}
                onChange={(e) => updateConfig('colorOrder', e.target.value)}
                className="w-full bg-gray-700 rounded-lg px-3 py-2"
              >
                <option value="RGB">RGB (WS2811)</option>
                <option value="GRB">GRB (WS2812/WS2812B)</option>
                <option value="BRG">BRG</option>
                <option value="RBG">RBG</option>
                <option value="GBR">GBR</option>
                <option value="BGR">BGR</option>
              </select>
              <p className="mt-1 text-xs text-gray-500">
                If colors appear wrong (e.g., red shows as green), try a different order.
              </p>
            </div>

            <div>
              <label className="block text-sm font-medium text-gray-400 mb-1">
                No Weather Data Color
              </label>
              <div className="flex gap-2 items-center">
                <input
                  type="color"
                  value={localConfig.noDataColor || '#000000'}
                  onChange={(e) => updateConfig('noDataColor', e.target.value)}
                  className="h-10 w-14 bg-gray-700 rounded border border-gray-600 cursor-pointer"
                />
                <input
                  type="text"
                  value={localConfig.noDataColor || '#000000'}
                  onChange={(e) => {
                    const val = e.target.value;
                    updateConfig('noDataColor', val);
                  }}
                  placeholder="#000000"
                  maxLength={7}
                  className="flex-1 bg-gray-700 rounded-lg px-3 py-2 font-mono"
                />
              </div>
              <p className="mt-1 text-xs text-gray-500">
                Color shown for airports with no valid weather report. #000000 (black) turns the LED off.
              </p>
            </div>

            {/* MQTT / Home Assistant */}
            <div className="pt-4 border-t border-gray-600">
              <div className="flex items-center justify-between mb-4">
                <label className="text-sm font-medium text-gray-400">
                  Enable MQTT (Home Assistant)
                </label>
                <button
                  onClick={() => updateConfig('mqttEnabled', !localConfig.mqttEnabled)}
                  className={`relative inline-flex h-6 w-11 items-center rounded-full transition-colors ${
                    localConfig.mqttEnabled ? 'bg-blue-600' : 'bg-gray-600'
                  }`}
                >
                  <span
                    className={`inline-block h-4 w-4 transform rounded-full bg-white transition-transform ${
                      localConfig.mqttEnabled ? 'translate-x-6' : 'translate-x-1'
                    }`}
                  />
                </button>
              </div>

              {localConfig.mqttEnabled && (
                <div className="space-y-4">
                  <div>
                    <label className="block text-sm font-medium text-gray-400 mb-1">
                      MQTT Broker
                    </label>
                    <input
                      type="text"
                      placeholder="192.168.1.100 or mqtt.example.com"
                      value={localConfig.mqttBroker || ''}
                      onChange={(e) => updateConfig('mqttBroker', e.target.value)}
                      className="w-full bg-gray-700 rounded-lg px-3 py-2"
                    />
                  </div>

                  <div>
                    <label className="block text-sm font-medium text-gray-400 mb-1">
                      MQTT Port
                    </label>
                    <input
                      type="number"
                      min="1"
                      max="65535"
                      value={localConfig.mqttPort || 1883}
                      onChange={(e) => updateConfig('mqttPort', parseInt(e.target.value))}
                      className="w-full bg-gray-700 rounded-lg px-3 py-2"
                    />
                  </div>

                  <div>
                    <label className="block text-sm font-medium text-gray-400 mb-1">
                      Username (optional)
                    </label>
                    <input
                      type="text"
                      placeholder="Leave empty if no authentication"
                      value={localConfig.mqttUsername || ''}
                      onChange={(e) => updateConfig('mqttUsername', e.target.value)}
                      className="w-full bg-gray-700 rounded-lg px-3 py-2"
                    />
                  </div>

                  <div>
                    <label className="block text-sm font-medium text-gray-400 mb-1">
                      Password (optional)
                    </label>
                    <input
                      type="password"
                      placeholder="Leave empty if no authentication"
                      value={localConfig.mqttPassword || ''}
                      onChange={(e) => updateConfig('mqttPassword', e.target.value)}
                      className="w-full bg-gray-700 rounded-lg px-3 py-2"
                    />
                  </div>

                  <p className="text-xs text-gray-500">
                    The device will automatically appear in Home Assistant via MQTT Discovery.
                    You can control power and brightness from the Home Assistant dashboard.
                  </p>
                </div>
              )}
            </div>
          </div>
        )}
      </section>

      {/* Device Actions */}
      <section className="pt-4 border-t border-gray-700">
        <h3 className="text-lg font-semibold mb-4">Device Actions</h3>
        {!isConnected ? (
          <p className="text-gray-500 text-sm">Connect a device to access device actions.</p>
        ) : (
          <div className="space-y-2">
            <button
              onClick={onReboot}
              disabled={isLoading}
              className="w-full px-4 py-2 bg-yellow-600 hover:bg-yellow-700 rounded-lg font-medium disabled:opacity-50"
            >
              Reboot Device
            </button>

            {!showFactoryResetConfirm ? (
              <button
                onClick={() => setShowFactoryResetConfirm(true)}
                disabled={isLoading}
                className="w-full px-4 py-2 bg-red-600 hover:bg-red-700 rounded-lg font-medium disabled:opacity-50"
              >
                Factory Reset
              </button>
            ) : (
              <div className="bg-red-900/50 border border-red-700 rounded-lg p-4">
                <p className="text-sm mb-3">
                  This will erase all settings including WiFi credentials. Are you sure?
                </p>
                <div className="flex gap-2">
                  <button
                    onClick={() => {
                      onFactoryReset();
                      setShowFactoryResetConfirm(false);
                    }}
                    disabled={isLoading}
                    className="flex-1 px-4 py-2 bg-red-600 hover:bg-red-700 rounded-lg font-medium disabled:opacity-50"
                  >
                    Yes, Reset
                  </button>
                  <button
                    onClick={() => setShowFactoryResetConfirm(false)}
                    className="flex-1 px-4 py-2 bg-gray-600 hover:bg-gray-700 rounded-lg font-medium"
                  >
                    Cancel
                  </button>
                </div>
              </div>
            )}
          </div>
        )}
      </section>
    </div>
  );
}
