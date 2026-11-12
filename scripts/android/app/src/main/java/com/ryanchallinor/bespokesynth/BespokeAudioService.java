/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

package com.ryanchallinor.bespokesynth;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ServiceInfo;
import android.os.Build;
import android.os.IBinder;

// A foreground service that runs while bespoke does, started and stopped by native code (see AndroidSupport.cpp).
// Without it, android treats bespoke as a cached background app whenever another activity covers it (including the
// system file picker, or switching apps), and restricts it to the slow cpu cores, so the audio can't keep up
public final class BespokeAudioService extends Service
{
    private static final String kChannelId = "bespoke_audio";
    private static final int kNotificationId = 1;

    public static void start (Context context)
    {
        Intent intent = new Intent (context, BespokeAudioService.class);
        if (Build.VERSION.SDK_INT >= 26)
            context.startForegroundService (intent);
        else
            context.startService (intent);
    }

    public static void stop (Context context)
    {
        context.stopService (new Intent (context, BespokeAudioService.class));
    }

    @Override
    public int onStartCommand (Intent intent, int flags, int startId)
    {
        Notification notification = buildNotification();
        if (Build.VERSION.SDK_INT >= 29)
            startForeground (kNotificationId, notification, ServiceInfo.FOREGROUND_SERVICE_TYPE_MEDIA_PLAYBACK);
        else
            startForeground (kNotificationId, notification);
        return START_NOT_STICKY; // if the process dies, there's no audio for this to keep going
    }

    @Override
    public IBinder onBind (Intent intent)
    {
        return null;
    }

    private Notification buildNotification()
    {
        Notification.Builder builder;
        if (Build.VERSION.SDK_INT >= 26)
        {
            NotificationChannel channel = new NotificationChannel (kChannelId, "Audio", NotificationManager.IMPORTANCE_LOW);
            channel.setDescription ("Shown while bespoke is running, so it can keep playing in the background");
            getSystemService (NotificationManager.class).createNotificationChannel (channel);
            builder = new Notification.Builder (this, kChannelId);
        }
        else
        {
            builder = new Notification.Builder (this);
        }

        // tapping the notification brings bespoke back
        Intent launch = getPackageManager().getLaunchIntentForPackage (getPackageName());
        if (launch != null)
            builder.setContentIntent (PendingIntent.getActivity (this, 0, launch, PendingIntent.FLAG_IMMUTABLE));

        return builder.setContentTitle ("Bespoke")
                      .setContentText ("Audio is running")
                      .setSmallIcon (getApplicationInfo().icon)
                      .setOngoing (true)
                      .build();
    }
}
