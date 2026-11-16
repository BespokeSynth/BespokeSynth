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

// Receives documents that other apps open with bespoke, and passes them on to BespokeActivity in bespoke's own task.
// Apps like the files app open documents with startActivityForResult, which puts the opened activity in the app's own
// task even when it's singleTask, so if BespokeActivity received them directly, bespoke would end up running inside
// (and labeled as) the other app. This activity has no window, and finishes right away
public class BespokeOpenActivity extends Activity
{
    @Override
    protected void onCreate (Bundle savedInstanceState)
    {
        super.onCreate (savedInstanceState);

        Uri document = getIntent() != null ? getIntent().getData() : null;
        if (document != null)
        {
            Intent open = new Intent (Intent.ACTION_VIEW, document, this, BespokeActivity.class);
            // the permission to read the document came with this activity's intent, and would go away when this
            // finishes, so pass it on to BespokeActivity
            open.addFlags (Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_GRANT_READ_URI_PERMISSION);
            startActivity (open);
        }

        finish();
    }
}
