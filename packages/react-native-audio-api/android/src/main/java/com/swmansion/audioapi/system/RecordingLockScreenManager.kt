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
  private var title: String? = null
  private var description: String? = null
  private val iconName: String = "logo"
  private val iconSource: String = "drawable"

  private var nb: NotificationCompat.Builder =
    NotificationCompat.Builder(reactContext.get()!!, MediaSessionManager.CHANNEL_ID)

  init {
    val resId = reactContext.get()!!.resources.getIdentifier(iconName, iconSource, reactContext.get()!!.packageName)
    val color = Color.rgb(255, 0, 0)

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

    val resId = reactContext.get()!!.resources.getIdentifier(iconName, iconSource, reactContext.get()!!.packageName)
    val color = Color.rgb(255, 0, 0)

    nb.setSmallIcon(resId)
    nb.setColorized(true)
    nb.setColor(color)
    nb.setUsesChronometer(true)
    nb.setContentTitle(title ?: "Recording")
    nb.setContentText(description ?: "Recording in progress")

    mediaSession.get()?.setActive(true)
    mediaNotificationManager.get()?.updateNotification(nb, isRecording)
  }

  fun resetRecordingInfo() {
    isRecording = false
    mediaNotificationManager.get()?.cancelNotification()
    mediaSession.get()?.setActive(false)
  }

  fun enableRemoteCommand(
    name: String,
    enabled: Boolean,
  ) {}
}
