package com.swmansion.audioapi.system

import android.app.Notification
import android.graphics.Color
import android.support.v4.media.session.MediaSessionCompat
import android.util.Log
import androidx.core.app.NotificationCompat
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.bridge.ReadableMap
import java.lang.ref.WeakReference

class RecordingLockScreenManager(
  private val reactContext: WeakReference<ReactApplicationContext>,
  private val mediaSession: WeakReference<MediaSessionCompat>,
  private val mediaNotificationManager: WeakReference<MediaNotificationManager>,
) {
  private var isRecording: Boolean = false
  private var isPaused: Boolean = false
  private var title: String? = null
  private var description: String? = null
  private val iconName: String = "logo"
  private val iconSource: String = "drawable"
  private val buttonListeners = mutableMapOf<String, (String) -> Unit>()

  companion object {
    private const val PAUSE_RESUME_REQUEST_CODE = 1001
    private const val STOP_REQUEST_CODE = 1002
  }

  private var nb: NotificationCompat.Builder =
    NotificationCompat.Builder(reactContext.get()!!, MediaSessionManager.CHANNEL_ID)

  init {
    val resId = reactContext.get()!!.resources.getIdentifier(iconName, iconSource, reactContext.get()!!.packageName)
    val color = parseColor(null) // Default red

    nb.setSmallIcon(resId)
    nb.setColorized(true)
    nb.setColor(color)
    nb.setVisibility(NotificationCompat.VISIBILITY_PUBLIC)
    nb.setPriority(NotificationCompat.PRIORITY_HIGH)
    nb.setCategory(Notification.CATEGORY_SERVICE)
    nb.setOnlyAlertOnce(true)
    nb.setForegroundServiceBehavior(NotificationCompat.FOREGROUND_SERVICE_IMMEDIATE)
    nb.setOngoing(true)
  }

  fun setRecordingInfo(info: ReadableMap?) {
    if (info == null) {
      return
    }

    if (info.hasKey("title")) {
      title = info.getString("title")
    }

    if (info.hasKey("description")) {
      description = info.getString("description")
    }

    if (!isRecording) {
      isRecording = true
      isPaused = false
    }

    val resId = reactContext.get()!!.resources.getIdentifier(iconName, iconSource, reactContext.get()!!.packageName)
    val color = parseColor(info.getString("notificationColor"))

    nb.setSmallIcon(resId)
    nb.setColorized(true)
    nb.setColor(color)
    nb.setUsesChronometer(isRecording && !isPaused)
    nb.setContentTitle(title ?: "Recording")
    nb.setContentText(
      when {
        isPaused -> "Recording paused"
        isRecording -> "Recording in progress"
        else -> "Ready to record"
      },
    )

    nb.clearActions()

    val pauseResumeAction =
      androidx.core.app.NotificationCompat.Action
        .Builder(
          android.R.drawable.ic_media_pause,
          if (isPaused) "Resume" else "Pause",
          android.app.PendingIntent.getBroadcast(
            reactContext.get(),
            PAUSE_RESUME_REQUEST_CODE,
            android.content.Intent("com.swmansion.audioapi.RECORDING_BUTTON_CLICK").apply {
              putExtra("buttonId", "pause_resume")
            },
            android.app.PendingIntent.FLAG_UPDATE_CURRENT or android.app.PendingIntent.FLAG_IMMUTABLE,
          ),
        ).build()
    nb.addAction(pauseResumeAction)

    val stopAction =
      androidx.core.app.NotificationCompat.Action
        .Builder(
          android.R.drawable.ic_media_stop,
          "Stop",
          android.app.PendingIntent.getBroadcast(
            reactContext.get(),
            STOP_REQUEST_CODE,
            android.content.Intent("com.swmansion.audioapi.RECORDING_BUTTON_CLICK").apply {
              putExtra("buttonId", "stop")
            },
            android.app.PendingIntent.FLAG_UPDATE_CURRENT or android.app.PendingIntent.FLAG_IMMUTABLE,
          ),
        ).build()
    nb.addAction(stopAction)

    mediaSession.get()?.setActive(true)
    mediaNotificationManager.get()?.updateNotification(nb, isRecording)
  }

  fun resetRecordingInfo() {
    isRecording = false
    isPaused = false
    mediaNotificationManager.get()?.cancelNotification()
    mediaSession.get()?.setActive(false)
  }

  fun enableRemoteCommand(
    name: String,
    enabled: Boolean,
  ) {}

  fun addButtonListener(
    buttonId: String,
    listener: (String) -> Unit,
  ) {
    buttonListeners[buttonId] = listener
  }

  fun removeButtonListener(buttonId: String) {
    buttonListeners.remove(buttonId)
  }

  fun onButtonClicked(buttonId: String) {
    when (buttonId) {
      "pause_resume" -> {
        if (isRecording) {
          isPaused = !isPaused
          setRecordingInfo(createCurrentInfoMap())
        }
      }
      "stop" -> {
        isRecording = false
        isPaused = false
        buttonListeners[buttonId]?.invoke(buttonId)
      }
      else -> {
        buttonListeners[buttonId]?.invoke(buttonId)
      }
    }
  }

  fun pauseRecording() {
    if (isRecording && !isPaused) {
      isPaused = true
      setRecordingInfo(createCurrentInfoMap())
    }
  }

  fun resumeRecording() {
    if (isRecording && isPaused) {
      isPaused = false
      setRecordingInfo(createCurrentInfoMap())
    }
  }

  private fun createCurrentInfoMap(): ReadableMap? {
    val arguments =
      com.facebook.react.bridge.Arguments
        .createMap()
    title?.let { arguments.putString("title", it) }
    description?.let { arguments.putString("description", it) }
    return arguments
  }

  private fun parseColor(colorString: String?): Int =
    when {
      colorString?.startsWith("#") == true -> {
        try {
          Color.parseColor(colorString)
        } catch (e: IllegalArgumentException) {
          Color.rgb(255, 0, 0)
        }
      }
      colorString == "red" -> Color.RED
      colorString == "blue" -> Color.BLUE
      colorString == "green" -> Color.GREEN
      colorString == "orange" -> Color.rgb(255, 165, 0)
      colorString == "purple" -> Color.rgb(128, 0, 128)
      colorString == "yellow" -> Color.YELLOW
      colorString == "black" -> Color.BLACK
      colorString == "white" -> Color.WHITE
      colorString == "gray" -> Color.GRAY
      else -> Color.rgb(255, 0, 0)
    }
}
