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

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;

// Bespoke's activity. JUCE drives it through JuceApp's lifecycle callbacks, like a plain Activity; this subclass only
// adds catching documents that other apps open with bespoke, which arrive through BespokeOpenActivity. native code
// collects them with takeOpenedDocument() (see AndroidSupport.cpp)
public class BespokeActivity extends Activity
{
    private static Uri sOpenedDocument;

    @Override
    protected void onCreate (Bundle savedInstanceState)
    {
        super.onCreate (savedInstanceState);
        if (savedInstanceState == null) // otherwise this is the activity being recreated, with an intent already handled
            catchOpenedDocument (getIntent());
    }

    // the activity is singleTask, so opening a document while bespoke is running comes here instead of onCreate
    @Override
    protected void onNewIntent (Intent intent)
    {
        super.onNewIntent (intent);
        setIntent (intent);
        catchOpenedDocument (intent);
    }

    private static synchronized void catchOpenedDocument (Intent intent)
    {
        if (intent != null && Intent.ACTION_VIEW.equals (intent.getAction()) && intent.getData() != null)
            sOpenedDocument = intent.getData();
    }

    // returns the URI of the document most recently opened with bespoke, once, or null if there isn't a new one
    public static synchronized String takeOpenedDocument()
    {
        Uri document = sOpenedDocument;
        sOpenedDocument = null;
        return document != null ? document.toString() : null;
    }
}
