package com.medi

import android.content.Context
import android.media.midi.MidiDevice
import android.media.midi.MidiDeviceInfo
import android.media.midi.MidiDeviceStatus
import android.media.midi.MidiInputPort
import android.media.midi.MidiManager
import android.media.midi.MidiOutputPort
import android.media.midi.MidiReceiver
import android.os.Build
import android.util.Log
import androidx.annotation.RequiresApi
import com.facebook.react.bridge.Arguments
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.bridge.WritableArray
import com.facebook.react.bridge.WritableMap
import com.facebook.react.bridge.WritableNativeArray
import com.facebook.react.module.annotations.ReactModule

@ReactModule(name = MediModule.NAME)
class MediModule(reactContext: ReactApplicationContext) :
  NativeMediSpec(reactContext) {

  private var midiManager: MidiManager? = null
  private val openDevices = mutableMapOf<String, MidiDevice>()
  private val openPorts = mutableMapOf<String, Any>()
  private var sysexEnabled = false

  override fun getName(): String {
    return NAME
  }

  @RequiresApi(Build.VERSION_CODES.M)
  override fun prepareMIDIClient(sysex: Boolean, promise: com.facebook.react.bridge.Promise) {
    if (midiManager != null) {
      Log.d(TAG, "MIDI client already prepared")
      promise.resolve(null)
      return
    }

    midiManager = reactApplicationContext.getSystemService(Context.MIDI_SERVICE) as? MidiManager
    sysexEnabled = sysex

    if (midiManager == null) {
      Log.e(TAG, "Failed to get MIDI Manager - MIDI not supported on this device")
      promise.reject("MIDI_NOT_SUPPORTED", "MIDI not supported on this device")
      return
    }

    // Register device callback for state changes (for now just logging)
    midiManager?.registerDeviceCallback(
      MidiManager.TRANSPORT_MIDI_BYTE_STREAM,
      reactApplicationContext.mainExecutor,
      object : MidiManager.DeviceCallback() {
        override fun onDeviceAdded(device: MidiDeviceInfo) {
          Log.d(TAG, "[MIDI State] Device added: ${device.properties.getString(MidiDeviceInfo.PROPERTY_NAME)}")
        }

        override fun onDeviceRemoved(device: MidiDeviceInfo) {
          Log.d(TAG, "[MIDI State] Device removed: ${device.properties.getString(MidiDeviceInfo.PROPERTY_NAME)}")
        }

        override fun onDeviceStatusChanged(status: MidiDeviceStatus) {
          Log.d(TAG, "[MIDI State] Device status changed: ${status.deviceInfo.properties.getString(MidiDeviceInfo.PROPERTY_NAME)}")
        }
      }
    )

    Log.d(TAG, "MIDI client prepared with sysex: $sysex")

    // On Android, devices are available immediately after getting the MidiManager
    // Unlike iOS, we don't need to wait for async initialization
    // But we add a small delay to ensure the callback is fully registered
    // android.os.Handler(android.os.Looper.getMainLooper()).postDelayed({
    promise.resolve(null)
  }

  @RequiresApi(Build.VERSION_CODES.M)
  override fun getSources(): WritableArray {
    val sources = WritableNativeArray()
    val devices = midiManager?.getDevicesForTransport(MidiManager.TRANSPORT_MIDI_BYTE_STREAM)?.toList() ?: emptyList()

    for (device in devices) {
      // Sources are devices that OUTPUT MIDI data (TYPE_OUTPUT from device perspective)
      // We want to receive from these, so we call them "input" type in Web MIDI API terms
      for (portInfo in device.ports) {
        if (portInfo.type == MidiDeviceInfo.PortInfo.TYPE_OUTPUT) {
          sources.pushMap(createPortInfo(device, portInfo, "input"))
        }
      }
    }

    Log.d(TAG, "Found ${sources.size()} MIDI sources")
    return sources
  }

  @RequiresApi(Build.VERSION_CODES.M)
  override fun getDestinations(): WritableArray {
    val destinations = WritableNativeArray()
    val devices = midiManager?.getDevicesForTransport(MidiManager.TRANSPORT_MIDI_BYTE_STREAM)?.toList() ?: emptyList()

    for (device in devices) {
      // Destinations are devices that INPUT MIDI data (TYPE_INPUT from device perspective)
      // We want to send to these, so we call them "output" type in Web MIDI API terms
      for (portInfo in device.ports) {
        if (portInfo.type == MidiDeviceInfo.PortInfo.TYPE_INPUT) {
          destinations.pushMap(createPortInfo(device, portInfo, "output"))
        }
      }
    }

    Log.d(TAG, "Found ${destinations.size()} MIDI destinations")
    return destinations
  }

  @RequiresApi(Build.VERSION_CODES.M)
  override fun openPort(portId: String): Boolean {
    if (midiManager == null) {
      Log.e(TAG, "MIDI Manager not initialized")
      return false
    }

    if (openPorts.containsKey(portId)) {
      Log.d(TAG, "Port already open: $portId")
      return true
    }

    val parts = portId.split("_")
    if (parts.size != 3) {
      Log.e(TAG, "Invalid port ID format: $portId")
      return false
    }

    val type = parts[0]
    val deviceId = parts[1].toIntOrNull() ?: return false
    val portIndex = parts[2].toIntOrNull() ?: return false

    val devices = midiManager?.getDevicesForTransport(MidiManager.TRANSPORT_MIDI_BYTE_STREAM)?.toList() ?: emptyList()
    val deviceInfo = devices.firstOrNull { it.id == deviceId }
    if (deviceInfo == null) {
      Log.e(TAG, "Device not found for ID: $deviceId")
      return false
    }

    Log.d(TAG, "Attempting to open port: $portId (type: $type, device: $deviceId, port: $portIndex)")

    // Type "input" means we receive MIDI from this source (device OUTPUT port)
    // Type "output" means we send MIDI to this destination (device INPUT port)

    if (type == "input") {
      // Open device and connect to its OUTPUT port to receive MIDI data
      midiManager?.openDevice(deviceInfo, { openedDevice ->
        if (openedDevice == null) {
          Log.e(TAG, "Failed to open device for port: $portId")
          return@openDevice
        }

        Log.d(TAG, "Device opened successfully for port: $portId")

        val outputPort = openedDevice.openOutputPort(portIndex)
        if (outputPort == null) {
          Log.e(TAG, "Failed to open output port $portIndex on device: $portId")
          openedDevice.close()
          return@openDevice
        }

        Log.d(TAG, "Output port opened successfully: $portId")

        // Set up MIDI receiver to emit messages to JS
        outputPort.connect(object : MidiReceiver() {
          override fun onSend(msg: ByteArray, offset: Int, count: Int, timestamp: Long) {
            var i = 0
            while (i < count) {
              val statusByte = msg[offset + i].toInt() and 0xFF

              // Determine message length based on status byte
              val messageLength = when {
                statusByte >= 0xF0 -> {
                  // System messages
                  when (statusByte) {
                    0xF0 -> { // SysEx start
                      // Find SysEx end (0xF7)
                      var sysexEnd = i + 1
                      while (sysexEnd < count && (msg[offset + sysexEnd].toInt() and 0xFF) != 0xF7) {
                        sysexEnd++
                      }
                      if (sysexEnd < count) sysexEnd + 1 - i else count - i
                    }
                    0xF1, 0xF3 -> 2 // MTC Quarter Frame, Song Select
                    0xF2 -> 3 // Song Position Pointer
                    else -> 1 // 0xF6 Tune Request, 0xF7 SysEx end, 0xF8-0xFF Real-time
                  }
                }
                statusByte >= 0xC0 && statusByte < 0xE0 -> 2 // Program Change, Channel Pressure
                statusByte >= 0x80 -> 3 // Note Off, Note On, Poly Pressure, Control Change, Pitch Bend
                else -> 1 // Running status or invalid
              }

              // Extract the message
              val data = Arguments.createArray()
              for (j in 0 until minOf(messageLength, count - i)) {
                data.pushInt(msg[offset + i + j].toInt() and 0xFF)
              }

              val event = Arguments.createMap().apply {
                putString("portId", portId)
                putArray("data", data)
                putDouble("timestamp", System.currentTimeMillis().toDouble())
              }
              emitOnMidiMessage(event)

              i += messageLength
            }
          }
        })

        openDevices[portId] = openedDevice
        openPorts[portId] = outputPort
        Log.d(TAG, "Successfully opened and connected input port: $portId")
      }, null)
    } else {
      // For output ports (destinations), we would open the INPUT port to send data
      // But for now, we just mark it as open since we're not implementing sending yet
      Log.d(TAG, "Output port marked as open: $portId (sending not yet implemented)")
      return true
    }

    return true
  }

  @RequiresApi(Build.VERSION_CODES.M)
  override fun closePort(portId: String): Boolean {
    val port = openPorts.remove(portId)
    val device = openDevices.remove(portId)

    if (port == null && device == null) {
      Log.d(TAG, "Port not open: $portId")
      return false
    }

    try {
      when (port) {
        is MidiOutputPort -> port.close()
        is MidiInputPort -> port.close()
      }
      device?.close()
      Log.d(TAG, "Closed port: $portId")
      return true
    } catch (e: Exception) {
      Log.e(TAG, "Error closing port: $portId", e)
      return false
    }
  }

  @RequiresApi(Build.VERSION_CODES.M)
  private fun createPortInfo(
    device: MidiDeviceInfo,
    portInfo: MidiDeviceInfo.PortInfo,
    type: String
  ): WritableMap {
    val info = Arguments.createMap()
    val portId = "${type}_${device.id}_${portInfo.portNumber}"

    info.putString("id", portId)
    info.putString("name", device.properties.getString(MidiDeviceInfo.PROPERTY_NAME))
    info.putString("manufacturer", device.properties.getString(MidiDeviceInfo.PROPERTY_MANUFACTURER))
    info.putString("type", type)
    info.putString("version", device.properties.getString(MidiDeviceInfo.PROPERTY_VERSION))
    info.putString("state", "connected")
    info.putString("connection", if (openPorts.containsKey(portId)) "open" else "closed")

    Log.d(TAG, "Created MIDI port info: $info")
    return info
  }

  override fun invalidate() {
    super.invalidate()
    // Clean up all open ports and devices
    openPorts.values.forEach { port ->
      try {
        when (port) {
          is MidiOutputPort -> port.close()
          is MidiInputPort -> port.close()
        }
      } catch (e: Exception) {
        Log.e(TAG, "Error closing port during cleanup", e)
      }
    }
    openDevices.values.forEach { device ->
      try {
        device.close()
      } catch (e: Exception) {
        Log.e(TAG, "Error closing device during cleanup", e)
      }
    }
    openPorts.clear()
    openDevices.clear()
  }

  companion object {
    const val NAME = "Medi"
    private const val TAG = "MediModule"
  }
}
