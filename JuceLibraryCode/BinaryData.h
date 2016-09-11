/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#ifndef BINARYDATA_H_2660106_INCLUDED
#define BINARYDATA_H_2660106_INCLUDED

namespace BinaryData
{
    extern const char*   sconscript;
    const int            sconscriptSize = 154;

    extern const char*   README_txt;
    const int            README_txtSize = 4240;

    extern const char*   version;
    const int            versionSize = 5;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Number of elements in the namedResourceList array.
    const int namedResourceListSize = 3;

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes) throw();
}

#endif
