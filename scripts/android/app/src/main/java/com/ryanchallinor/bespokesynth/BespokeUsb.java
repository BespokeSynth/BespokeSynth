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

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Build;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;

// Opens USB devices for native code (see AndroidSupport.cpp), which hands the returned file descriptor to libusb.
// Android apps can't list or open USB devices themselves, and need the user's permission for each device.
// Used for the Ableton Move and Push 2 displays, which aren't MIDI, so Android's MIDI support doesn't cover them.
public final class BespokeUsb
{
    public static final int kNotFound = -1;
    public static final int kWaitingForPermission = -2;
    public static final int kPermissionDenied = -3;
    public static final int kOpenFailed = -4;

    private static final String kPermissionAction = "com.ryanchallinor.bespokesynth.USB_PERMISSION";

    private static final ArrayList<UsbDeviceConnection> sConnections = new ArrayList<>(); // keeps their descriptors open
    private static final HashSet<String> sPendingRequests = new HashSet<>(); // by device name
    private static final HashSet<String> sDeniedDevices = new HashSet<>(); // by device name, until it's unplugged
    private static boolean sReceiverRegistered = false;

    // returns a file descriptor for the device, or one of the k* error codes.
    // if the app doesn't have permission for the device yet, this asks the user, and returns kWaitingForPermission
    // until they answer, so callers should retry
    public static synchronized int open (Context context, int vendorId, int productId)
    {
        UsbManager manager = (UsbManager) context.getSystemService (Context.USB_SERVICE);
        if (manager == null)
            return kNotFound;

        HashMap<String, UsbDevice> devices = manager.getDeviceList();
        sDeniedDevices.retainAll (devices.keySet()); // forget denials for devices that were unplugged
        sPendingRequests.retainAll (devices.keySet());

        for (UsbDevice device : devices.values())
        {
            if (device.getVendorId() != vendorId || device.getProductId() != productId)
                continue;

            if (! manager.hasPermission (device))
            {
                if (sDeniedDevices.contains (device.getDeviceName()))
                    return kPermissionDenied;

                if (! sPendingRequests.contains (device.getDeviceName()))
                    requestPermission (context, manager, device);

                return kWaitingForPermission;
            }

            UsbDeviceConnection connection = manager.openDevice (device);
            if (connection == null)
                return kOpenFailed;

            sConnections.add (connection); // the descriptor stays valid for as long as the connection is open
            return connection.getFileDescriptor();
        }

        return kNotFound;
    }

    private static void requestPermission (Context context, UsbManager manager, UsbDevice device)
    {
        if (! sReceiverRegistered)
        {
            BroadcastReceiver receiver = new BroadcastReceiver()
            {
                @Override
                public void onReceive (Context receiverContext, Intent intent)
                {
                    UsbDevice answeredDevice = intent.getParcelableExtra (UsbManager.EXTRA_DEVICE);
                    if (answeredDevice == null)
                        return;

                    synchronized (BespokeUsb.class)
                    {
                        sPendingRequests.remove (answeredDevice.getDeviceName());
                        if (! intent.getBooleanExtra (UsbManager.EXTRA_PERMISSION_GRANTED, false))
                            sDeniedDevices.add (answeredDevice.getDeviceName());
                    }
                }
            };

            IntentFilter filter = new IntentFilter (kPermissionAction);
            if (Build.VERSION.SDK_INT >= 33)
                context.registerReceiver (receiver, filter, Context.RECEIVER_NOT_EXPORTED);
            else
                context.registerReceiver (receiver, filter);
            sReceiverRegistered = true;
        }

        // the system adds the device and the answer to this intent, so it has to be mutable. it's also explicit
        // (setPackage), which android 14 requires for mutable pending intents
        Intent intent = new Intent (kPermissionAction).setPackage (context.getPackageName());
        int flags = Build.VERSION.SDK_INT >= 31 ? PendingIntent.FLAG_MUTABLE : 0;
        manager.requestPermission (device, PendingIntent.getBroadcast (context, 0, intent, flags));
        sPendingRequests.add (device.getDeviceName());
    }
}
