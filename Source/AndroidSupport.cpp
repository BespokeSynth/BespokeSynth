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
#include <cstdio>
#include <cstring>

namespace
{
   const char* kAssetPrefix = "assets/";
   //the build adds this to .gz files, since the android packager would otherwise decompress them (see app/build.gradle)
   const char* kKeptGzSuffix = ".gz.keep";

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

         auto relativePath = entry->filename.substring((int)strlen(kAssetPrefix));
         if (relativePath.endsWith(kKeptGzSuffix))
            relativePath = relativePath.dropLastCharacters((int)strlen(".keep"));
         auto target = destination.getChildFile(relativePath);
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

   //also keep it in a file, since logcat only holds recent messages, and isn't available while the device is unplugged
   static juce::CriticalSection sLogFileLock;
   static FILE* sLogFile = nullptr;
   static bool sTriedOpening = false;
   juce::ScopedLock lock(sLogFileLock);
   if (!sTriedOpening)
   {
      sTriedOpening = true;
      auto dataDir = GetDataDir();
      dataDir.createDirectory();
      auto logFile = dataDir.getChildFile("log.txt");
      if (logFile.existsAsFile()) //keep the previous session's log around too
         logFile.moveFileTo(dataDir.getChildFile("log_previous.txt"));
      sLogFile = fopen(logFile.getFullPathName().toRawUTF8(), "w");
   }
   if (sLogFile != nullptr)
   {
      fprintf(sLogFile, "%s\n", message);
      fflush(sLogFile);
   }
}

bool AndroidSupport::IsDocument(const juce::URL& url)
{
   return url.getScheme() == "content";
}

juce::String AndroidSupport::GetDocumentName(const juce::URL& document)
{
   auto name = juce::AndroidDocument::fromDocument(document).getInfo().getName();
   if (name.isEmpty()) //document ids usually end with the file name, eg primary%3ADocuments%2Fsong.bsk
      name = juce::URL::removeEscapeChars(document.getFileName()).fromLastOccurrenceOf("/", false, false).fromLastOccurrenceOf(":", false, false);
   if (name.isEmpty())
      name = "document";
   return name;
}

juce::File AndroidSupport::GetDocumentMirror(const juce::URL& document)
{
   return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("documents").getChildFile(juce::File::createLegalFileName(GetDocumentName(document)));
}

juce::Result AndroidSupport::CopyDocumentToMirror(const juce::URL& document)
{
   //rather than URL::createInputStream(), which first tries to look the document up as a file, and asserts when that
   //needs storage permissions
   auto input = juce::AndroidDocument::fromDocument(document).createInputStream();
   if (input == nullptr)
      return juce::Result::fail("couldn't open " + document.toString(false));

   auto mirror = GetDocumentMirror(document);
   mirror.getParentDirectory().createDirectory();
   juce::FileOutputStream output(mirror);
   if (!output.openedOk())
      return output.getStatus();
   output.setPosition(0);
   output.truncate();
   output.writeFromInputStream(*input, -1);
   output.flush();
   return output.getStatus();
}

juce::Result AndroidSupport::WriteToDocument(const juce::URL& document, const juce::File& source)
{
   juce::FileInputStream input(source);
   if (!input.openedOk())
      return input.getStatus();

   auto* env = juce::getEnv();
   auto context = juce::getAppContext();
   if (env == nullptr || context.get() == nullptr)
      return juce::Result::fail("no android context");

   auto checkException = [env]()
   {
      if (!env->ExceptionCheck())
         return false;
      env->ExceptionClear();
      return true;
   };

   //opened with ContentResolver.openOutputStream(uri, "wt") rather than JUCE's URL::createOutputStream(), which uses
   //mode "w": some document providers don't truncate in that mode, which would leave the end of a longer previous
   //version of the file in place
   juce::LocalRef<jclass> contextClass(env->GetObjectClass(context.get()));
   jmethodID getContentResolver = env->GetMethodID(contextClass.get(), "getContentResolver", "()Landroid/content/ContentResolver;");
   juce::LocalRef<jobject> resolver(env->CallObjectMethod(context.get(), getContentResolver));
   if (checkException() || resolver.get() == nullptr)
      return juce::Result::fail("no content resolver");

   juce::LocalRef<jclass> uriClass(env->FindClass("android/net/Uri"));
   jmethodID parse = env->GetStaticMethodID(uriClass.get(), "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
   juce::LocalRef<jobject> uri(env->CallStaticObjectMethod(uriClass.get(), parse, juce::javaString(document.toString(true)).get()));
   if (checkException() || uri.get() == nullptr)
      return juce::Result::fail("bad document url");

   juce::LocalRef<jclass> resolverClass(env->GetObjectClass(resolver.get()));
   jmethodID openOutputStream = env->GetMethodID(resolverClass.get(), "openOutputStream", "(Landroid/net/Uri;Ljava/lang/String;)Ljava/io/OutputStream;");
   juce::LocalRef<jobject> stream(env->CallObjectMethod(resolver.get(), openOutputStream, uri.get(), juce::javaString("wt").get()));
   if (checkException() || stream.get() == nullptr)
      return juce::Result::fail("couldn't open the document for writing");

   juce::LocalRef<jclass> streamClass(env->GetObjectClass(stream.get()));
   jmethodID write = env->GetMethodID(streamClass.get(), "write", "([BII)V");
   jmethodID close = env->GetMethodID(streamClass.get(), "close", "()V");

   constexpr int kChunkSize = 65536;
   juce::HeapBlock<char> buffer(kChunkSize);
   juce::LocalRef<jbyteArray> javaBuffer(env->NewByteArray(kChunkSize));
   bool ok = true;
   while (ok && !input.isExhausted())
   {
      int bytesRead = input.read(buffer.get(), kChunkSize);
      if (bytesRead <= 0)
         break;
      env->SetByteArrayRegion(javaBuffer.get(), 0, bytesRead, reinterpret_cast<const jbyte*>(buffer.get()));
      env->CallVoidMethod(stream.get(), write, javaBuffer.get(), 0, bytesRead);
      ok = !checkException();
   }

   env->CallVoidMethod(stream.get(), close); //flushes, so this can fail too
   ok = !checkException() && ok;

   return ok ? juce::Result::ok() : juce::Result::fail("couldn't write to the document");
}

namespace
{
   //calls a static method of com.ryanchallinor.bespokesynth.BespokeUsb. app classes have to be loaded through the
   //app's class loader, since JNI's FindClass only sees system classes on threads that java didn't start
   jclass GetBespokeUsbClass(JNIEnv* env, jobject context)
   {
      juce::LocalRef<jclass> contextClass(env->GetObjectClass(context));
      jmethodID getClassLoader = env->GetMethodID(contextClass.get(), "getClassLoader", "()Ljava/lang/ClassLoader;");
      juce::LocalRef<jobject> classLoader(env->CallObjectMethod(context, getClassLoader));
      juce::LocalRef<jclass> classLoaderClass(env->GetObjectClass(classLoader.get()));
      jmethodID loadClass = env->GetMethodID(classLoaderClass.get(), "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
      auto usbClass = (jclass)env->CallObjectMethod(classLoader.get(), loadClass, juce::javaString("com.ryanchallinor.bespokesynth.BespokeUsb").get());
      if (env->ExceptionCheck())
      {
         env->ExceptionClear();
         return nullptr;
      }
      return usbClass;
   }
}

int BespokeAndroidUsbOpen(int vendorId, int productId, char* error, int errorSize)
{
   auto setError = [error, errorSize](const char* message)
   {
      if (error != nullptr && errorSize > 0)
         snprintf(error, errorSize, "%s", message);
   };

   auto* env = juce::getEnv();
   auto context = juce::getAppContext();
   if (env == nullptr || context.get() == nullptr)
   {
      setError("no android context");
      return -1;
   }

   juce::LocalRef<jclass> usbClass(GetBespokeUsbClass(env, context.get()));
   if (usbClass.get() == nullptr)
   {
      setError("couldn't load BespokeUsb");
      return -1;
   }

   jmethodID open = env->GetStaticMethodID(usbClass.get(), "open", "(Landroid/content/Context;II)I");
   int result = env->CallStaticIntMethod(usbClass.get(), open, context.get(), vendorId, productId);
   if (env->ExceptionCheck())
   {
      env->ExceptionClear();
      setError("usb error");
      return -1;
   }

   if (result >= 0)
      return result;

   //see the error codes in BespokeUsb.java
   switch (result)
   {
      case -1: setError("device not found"); break;
      case -2: setError("waiting for usb permission"); break;
      case -3: setError("usb access denied, reconnect to ask again"); break;
      case -4: setError("couldn't open usb device"); break;
      default: setError("usb error"); break;
   }
   return -1;
}
