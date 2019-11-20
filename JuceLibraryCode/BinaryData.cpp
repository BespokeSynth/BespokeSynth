/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

namespace BinaryData
{

//================== sconscript ==================
static const unsigned char temp_binary_data_0[] =
"Import( 'env buildLibrary' )\r\n"
"\r\n"
"buildLibrary( env, Split( \"\"\"\r\n"
"    json_reader.cpp \r\n"
"    json_value.cpp \r\n"
"    json_writer.cpp\r\n"
"     \"\"\" ),\r\n"
"    'json' )\r\n";

const char* sconscript = (const char*) temp_binary_data_0;

//================== README.txt ==================
static const unsigned char temp_binary_data_1[] =
"* Introduction:\n"
"  =============\n"
"\n"
"JSON (JavaScript Object Notation) is a lightweight data-interchange format. \n"
"It can represent integer, real number, string, an ordered sequence of \n"
"value, and a collection of name/value pairs.\n"
"\n"
"JsonCpp is a simple API to manipulate JSON value, handle serialization \n"
"and unserialization to string.\n"
"\n"
"It can also preserve existing comment in unserialization/serialization steps,\n"
"making it a convenient format to store user input files.\n"
"\n"
"Unserialization parsing is user friendly and provides precise error reports.\n"
"\n"
"\n"
"* Building/Testing:\n"
"  =================\n"
"\n"
"JsonCpp uses Scons (http://www.scons.org) as a build system. Scons requires\n"
"python to be installed (http://www.python.org).\n"
"\n"
"You download scons-local distribution from the following url:\n"
"http://sourceforge.net/project/showfiles.php?group_id=30337&package_id=67375\n"
"\n"
"Unzip it in the directory where you found this README file. scons.py Should be \n"
"at the same level as README.\n"
"\n"
"python scons.py platform=PLTFRM [TARGET]\n"
"where PLTFRM may be one of:\n"
"\tsuncc Sun C++ (Solaris)\n"
"\tvacpp Visual Age C++ (AIX)\n"
"\tmingw \n"
"\tmsvc6 Microsoft Visual Studio 6 service pack 5-6\n"
"\tmsvc70 Microsoft Visual Studio 2002\n"
"\tmsvc71 Microsoft Visual Studio 2003\n"
"\tmsvc80 Microsoft Visual Studio 2005\n"
"\tlinux-gcc Gnu C++ (linux, also reported to work for Mac OS X)\n"
"\t\n"
"adding platform is fairly simple. You need to change the Sconstruct file \n"
"to do so.\n"
"\t\n"
"and TARGET may be:\n"
"\tcheck: build library and run unit tests.\n"
"\n"
"    \n"
"* Running the test manually:\n"
"  ==========================\n"
"\n"
"cd test\n"
"# This will run the Reader/Writer tests\n"
"python runjsontests.py \"path to jsontest.exe\"\n"
"\n"
"# This will run the Reader/Writer tests, using JSONChecker test suite\n"
"# (http://www.json.org/JSON_checker/).\n"
"# Notes: not all tests pass: JsonCpp is too lenient (for example,\n"
"# it allows an integer to start with '0'). The goal is to improve\n"
"# strict mode parsing to get all tests to pass.\n"
"python runjsontests.py --with-json-checker \"path to jsontest.exe\"\n"
"\n"
"# This will run the unit tests (mostly Value)\n"
"python rununittests.py \"path to test_lib_json.exe\"\n"
"\n"
"You can run the tests using valgrind:\n"
"python rununittests.py --valgrind \"path to test_lib_json.exe\"\n"
"\n"
"\n"
"* Building the documentation:\n"
"  ===========================\n"
"\n"
"Run the python script doxybuild.py from the top directory:\n"
"\n"
"python doxybuild.py --open --with-dot\n"
"\n"
"See doxybuild.py --help for options. \n"
"\n"
"\n"
"* Adding a reader/writer test:\n"
"  ============================\n"
"\n"
"To add a test, you need to create two files in test/data:\n"
"- a TESTNAME.json file, that contains the input document in JSON format.\n"
"- a TESTNAME.expected file, that contains a flatened representation of \n"
"  the input document.\n"
"  \n"
"TESTNAME.expected file format:\n"
"- each line represents a JSON element of the element tree represented \n"
"  by the input document.\n"
"- each line has two parts: the path to access the element separated from\n"
"  the element value by '='. Array and object values are always empty \n"
"  (e.g. represented by either [] or {}).\n"
"- element path: '.' represented the root element, and is used to separate \n"
"  object members. [N] is used to specify the value of an array element\n"
"  at index N.\n"
"See test_complex_01.json and test_complex_01.expected to better understand\n"
"element path.\n"
"\n"
"\n"
"* Understanding reader/writer test output:\n"
"  ========================================\n"
"\n"
"When a test is run, output files are generated aside the input test files. \n"
"Below is a short description of the content of each file:\n"
"\n"
"- test_complex_01.json: input JSON document\n"
"- test_complex_01.expected: flattened JSON element tree used to check if \n"
"    parsing was corrected.\n"
"\n"
"- test_complex_01.actual: flattened JSON element tree produced by \n"
"    jsontest.exe from reading test_complex_01.json\n"
"- test_complex_01.rewrite: JSON document written by jsontest.exe using the\n"
"    Json::Value parsed from test_complex_01.json and serialized using\n"
"    Json::StyledWritter.\n"
"- test_complex_01.actual-rewrite: flattened JSON element tree produced by \n"
"    jsontest.exe from reading test_complex_01.rewrite.\n"
"test_complex_01.process-output: jsontest.exe output, typically useful to\n"
"    understand parsing error.\n";

const char* README_txt = (const char*) temp_binary_data_1;

//================== version ==================
static const unsigned char temp_binary_data_2[] =
"0.5.0";

const char* version = (const char*) temp_binary_data_2;


const char* getNamedResource (const char* resourceNameUTF8, int& numBytes)
{
    unsigned int hash = 0;

    if (resourceNameUTF8 != nullptr)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0xa2ea79ba:  numBytes = 154; return sconscript;
        case 0x2aaab85f:  numBytes = 4123; return README_txt;
        case 0x14f51cd8:  numBytes = 5; return version;
        default: break;
    }

    numBytes = 0;
    return nullptr;
}

const char* namedResourceList[] =
{
    "sconscript",
    "README_txt",
    "version"
};

const char* originalFilenames[] =
{
    "sconscript",
    "README.txt",
    "version"
};

const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8)
{
    for (unsigned int i = 0; i < (sizeof (namedResourceList) / sizeof (namedResourceList[0])); ++i)
    {
        if (namedResourceList[i] == resourceNameUTF8)
            return originalFilenames[i];
    }

    return nullptr;
}

}
