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
//
//  AndroidSupport.cpp
//  Bespoke
//

//exposes juce::getEnv(), juce::getAppContext() etc
#define JUCE_CORE_INCLUDE_JNI_HELPERS 1

#include "AndroidSupport.h"

#include <android/log.h>

namespace
{
   const char* kAssetPrefix = "assets/";

   juce::File GetApkFile()
   {
      //on android, JUCE returns the apk path (ApplicationInfo.publicSourceDir) for this
      return juce::File::getSpecialLocation(juce::File::currentApplicationFile);
   }

   bool ExtractAssets(const juce::File& apk, const juce::File& destination)
   {
      juce::ZipFile zip(apk);
      if (zip.getNumEntries() == 0)
         return false;

      bool ok = true;
      for (int i = 0; i < zip.getNumEntries(); ++i)
      {
         const auto* entry = zip.getEntry(i);
         if (entry == nullptr || !entry->filename.startsWith(kAssetPrefix) || entry->filename.endsWithChar('/'))
            continue;

         auto target = destination.getChildFile(entry->filename.substring((int)strlen(kAssetPrefix)));
         target.getParentDirectory().createDirectory();

         std::unique_ptr<juce::InputStream> in(zip.createStreamForEntry(i));
         juce::FileOutputStream out(target);
         if (in == nullptr || !out.openedOk() || !out.setPosition(0) || !out.truncate().wasOk())
         {
            ok = false;
            continue;
         }
         out.writeFromInputStream(*in, -1);
         out.flush();
         if (out.getStatus().failed())
            ok = false;
      }
      return ok;
   }

   juce::File GetExternalFilesDir()
   {
      auto* env = juce::getEnv();
      if (env == nullptr)
         return {};

      auto context = juce::getAppContext();
      if (context.get() == nullptr)
         return {};

      juce::LocalRef<jclass> contextClass(env->GetObjectClass(context.get()));
      jmethodID getExternalFilesDir = env->GetMethodID(contextClass.get(), "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;");
      if (getExternalFilesDir == nullptr)
      {
         env->ExceptionClear();
         return {};
      }

      juce::LocalRef<jobject> dir(env->CallObjectMethod(context.get(), getExternalFilesDir, nullptr));
      if (env->ExceptionCheck())
      {
         env->ExceptionClear();
         return {};
      }
      if (dir.get() == nullptr) //no external storage mounted
         return {};

      juce::LocalRef<jclass> fileClass(env->GetObjectClass(dir.get()));
      jmethodID getAbsolutePath = env->GetMethodID(fileClass.get(), "getAbsolutePath", "()Ljava/lang/String;");
      juce::LocalRef<jstring> path((jstring)env->CallObjectMethod(dir.get(), getAbsolutePath));
      if (env->ExceptionCheck() || path.get() == nullptr)
      {
         env->ExceptionClear();
         return {};
      }

      return juce::File(juce::juceString(path.get()));
   }
}

juce::File AndroidSupport::GetFactoryDir()
{
   static const juce::File sFactoryDir = []
   {
      auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("factory");
      auto apk = GetApkFile();

      //the apk is rewritten by every install, so its timestamp and size identify which assets are unpacked
      juce::String stamp = juce::String(apk.getLastModificationTime().toMilliseconds()) + " " + juce::String(apk.getSize());
      auto stampFile = dir.getChildFile(".apk_stamp");
      if (stampFile.existsAsFile() && stampFile.loadFileAsString() == stamp)
         return dir;

      Log(("unpacking factory files from " + apk.getFullPathName() + " to " + dir.getFullPathName()).toRawUTF8());
      dir.deleteRecursively();
      dir.createDirectory();
      if (ExtractAssets(apk, dir))
         stampFile.replaceWithText(stamp);
      else
         Log("failed to unpack factory files");
      return dir;
   }();
   return sFactoryDir;
}

juce::File AndroidSupport::GetDataDir()
{
   static const juce::File sDataDir = []
   {
      auto external = GetExternalFilesDir();
      if (external != juce::File())
         return external.getChildFile("BespokeSynth");
      return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("BespokeSynth");
   }();
   return sDataDir;
}

void AndroidSupport::Log(const char* message)
{
   __android_log_write(ANDROID_LOG_INFO, "bespoke", message);
}
