package com.swmansion.audioapi.system

import android.Manifest
import android.annotation.SuppressLint
import android.app.Notification
import android.app.PendingIntent
import android.app.Service
import android.content.Intent
import android.content.pm.ServiceInfo
import android.content.res.Resources
import android.os.Build
import android.os.IBinder
import android.provider.ContactsContract
import android.support.v4.media.session.PlaybackStateCompat
import android.util.Log
import android.view.KeyEvent
import androidx.annotation.RequiresPermission
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat
import com.facebook.react.bridge.ReactApplicationContext
import com.swmansion.audioapi.R
import java.lang.ref.WeakReference

class MediaNotificationManager(
  private val reactContext: WeakReference<ReactApplicationContext>,
) {
  private var currentNotification: Notification? = null
  private var isRecordingStyle: Boolean = true
  private var smallIcon: Int = R.drawable.logo
  private var customIcon: Int = 0

  private var play: NotificationCompat.Action? = null
  private var pause: NotificationCompat.Action? = null
  private var stop: NotificationCompat.Action? = null
  private var next: NotificationCompat.Action? = null
  private var previous: NotificationCompat.Action? = null
  private var skipForward: NotificationCompat.Action? = null
  private var skipBackward: NotificationCompat.Action? = null

  companion object {
    const val REMOVE_NOTIFICATION: String = "audio_manager_remove_notification"
    const val PACKAGE_NAME: String = "com.swmansion.audioapi.system"
    const val MEDIA_BUTTON: String = "audio_manager_media_button"

    const val CUSTOM_ACTION: String = "audio_manager_custom_action"
    const val EXTRA_CUSTOM_ACTION_ID: String = "extra_custom_action_id"
  }

  enum class ForegroundAction {
    START_FOREGROUND,
    STOP_FOREGROUND,
    ;

    companion object {
      fun fromAction(action: String?): ForegroundAction? = entries.firstOrNull { it.name == action }
    }
  }

  @SuppressLint("RestrictedApi")
  @Synchronized
  fun prepareNotification(
    builder: NotificationCompat.Builder,
    isPlaying: Boolean,
  ): Notification {
    if (!isRecordingStyle) {
      builder.mActions.clear()

      if (previous != null) {
        builder.addAction(previous)
      }

      if (skipBackward != null) {
        builder.addAction(skipBackward)
      }

      if (play != null && !isPlaying) {
        builder.addAction(play)
      }

      if (pause != null && isPlaying) {
        builder.addAction(pause)
      }

      if (stop != null) {
        builder.addAction(stop)
      }

      if (next != null) {
        builder.addAction(next)
      }

      if (skipForward != null) {
        builder.addAction(skipForward)
      }

      builder.setSmallIcon(if (customIcon != 0) customIcon else smallIcon)
    }

    val packageName: String? = reactContext.get()?.packageName
    val openApp: Intent? = reactContext.get()?.packageManager?.getLaunchIntentForPackage(packageName!!)
    try {
      builder.setContentIntent(
        PendingIntent.getActivity(
          reactContext.get(),
          0,
          openApp,
          PendingIntent.FLAG_IMMUTABLE,
        ),
      )
    } catch (e: Exception) {
      Log.w("AudioManagerModule", "Error creating content intent: ${e.message}")
    }

    val remove = Intent(REMOVE_NOTIFICATION)
    remove.putExtra(PACKAGE_NAME, reactContext.get()?.applicationInfo?.packageName)
    builder.setDeleteIntent(
      PendingIntent.getBroadcast(
        reactContext.get(),
        0,
        remove,
        PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT,
      ),
    )
    currentNotification = builder.build()
    return currentNotification!!
  }

  @Synchronized
  fun setRecordingStyle(isRecording: Boolean) {
    this.isRecordingStyle = isRecording
  }

  @RequiresPermission(Manifest.permission.POST_NOTIFICATIONS)
  @Synchronized
  fun updateNotification(
    builder: NotificationCompat.Builder?,
    isPlaying: Boolean,
  ) {
    NotificationManagerCompat.from(reactContext.get()!!).notify(
      MediaSessionManager.NOTIFICATION_ID,
      prepareNotification(builder!!, isPlaying),
    )
  }

  fun cancelNotification() {
    NotificationManagerCompat.from(reactContext.get()!!).cancel(MediaSessionManager.NOTIFICATION_ID)
    currentNotification = null
  }

  @Synchronized
  fun updateActions(mask: Long) {
    play = createAction("play", "Play", mask, PlaybackStateCompat.ACTION_PLAY, play)
    pause = createAction("pause", "Pause", mask, PlaybackStateCompat.ACTION_PAUSE, pause)
    stop = createAction("stop", "Stop", mask, PlaybackStateCompat.ACTION_STOP, stop)
    next = createAction("next", "Next", mask, PlaybackStateCompat.ACTION_SKIP_TO_NEXT, next)
    previous = createAction("previous", "Previous", mask, PlaybackStateCompat.ACTION_SKIP_TO_PREVIOUS, previous)
    skipForward = createAction("skip_forward_15", "Skip Forward", mask, PlaybackStateCompat.ACTION_FAST_FORWARD, skipForward)
    skipBackward = createAction("skip_backward_15", "Skip Backward", mask, PlaybackStateCompat.ACTION_REWIND, skipBackward)
  }

  fun pendingIntentForAction(actionId: String): PendingIntent {
    val intent = Intent(CUSTOM_ACTION)
    intent.putExtra(EXTRA_CUSTOM_ACTION_ID, actionId)
    intent.putExtra(ContactsContract.Directory.PACKAGE_NAME, reactContext.get()?.packageName)
    val requestCode = actionId.hashCode()
    return PendingIntent.getBroadcast(
      reactContext.get(),
      requestCode,
      intent,
      PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT,
    )
  }

  private fun createAction(
    iconName: String,
    title: String,
    mask: Long,
    action: Long,
    oldAction: NotificationCompat.Action?,
  ): NotificationCompat.Action? {
    if ((mask and action) == 0L) {
      return null
    }

    if (oldAction != null) {
      return oldAction
    }

    val r: Resources? = reactContext.get()?.resources
    val packageName: String? = reactContext.get()?.packageName
    val icon = r?.getIdentifier(iconName, "drawable", packageName)

    val keyCode = PlaybackStateCompat.toKeyCode(action)
    val intent = Intent(MEDIA_BUTTON)
    intent.putExtra(Intent.EXTRA_KEY_EVENT, KeyEvent(KeyEvent.ACTION_DOWN, keyCode))
    intent.putExtra(ContactsContract.Directory.PACKAGE_NAME, packageName)
    val i =
      PendingIntent.getBroadcast(
        reactContext.get(),
        keyCode,
        intent,
        PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT,
      )

    return NotificationCompat.Action(icon!!, title, i)
  }

  class AudioForegroundService : Service() {
    private var notification: Notification? = null
    private var isServiceStarted = false
    private val serviceLock = Any()

    override fun onBind(intent: Intent): IBinder? = null

    private fun startForegroundService() {
      synchronized(serviceLock) {
        if (!isServiceStarted) {
          try {
            val notificationToStartWith = MediaSessionManager.mediaNotificationManager.currentNotification

            val finalNotification =
              notificationToStartWith ?: run {
                val fallbackBuilder =
                  NotificationCompat
                    .Builder(this, MediaSessionManager.CHANNEL_ID)
                    .setSmallIcon(MediaSessionManager.mediaNotificationManager.smallIcon)
                    .setContentTitle("Audio Service")
                MediaSessionManager.mediaNotificationManager.prepareNotification(fallbackBuilder, false)
              }

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
              startForeground(
                MediaSessionManager.NOTIFICATION_ID,
                finalNotification,
                ServiceInfo.FOREGROUND_SERVICE_TYPE_MANIFEST,
              )
            } else {
              startForeground(
                MediaSessionManager.NOTIFICATION_ID,
                finalNotification,
              )
            }
            isServiceStarted = true
          } catch (ex: Exception) {
            Log.e("AudioManagerModule", "Error starting foreground service: ${ex.message}")
            stopSelf()
          }
        }
      }
    }

    override fun onStartCommand(
      intent: Intent?,
      flags: Int,
      startId: Int,
    ): Int {
      val action = ForegroundAction.fromAction(intent?.action)

      when (action) {
        ForegroundAction.START_FOREGROUND -> startForegroundService()
        ForegroundAction.STOP_FOREGROUND -> stopForegroundService()
        else -> startForegroundService()
      }

      return START_NOT_STICKY
    }

    private fun stopForegroundService() {
      synchronized(serviceLock) {
        if (isServiceStarted) {
          if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            stopForeground(STOP_FOREGROUND_REMOVE)
          }
          isServiceStarted = false
          stopSelf()
        }
      }
    }

    override fun onTaskRemoved(rootIntent: Intent?) {
      super.onTaskRemoved(rootIntent)
      stopForegroundService()
    }

    override fun onDestroy() {
      synchronized(serviceLock) {
        // notification = null
        isServiceStarted = false
      }
      super.onDestroy()
    }
  }
}
