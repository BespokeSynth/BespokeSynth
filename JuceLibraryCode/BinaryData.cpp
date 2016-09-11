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
"* Introduction:\r\n"
"  =============\r\n"
"\r\n"
"JSON (JavaScript Object Notation) is a lightweight data-interchange format. \r\n"
"It can represent integer, real number, string, an ordered sequence of \r\n"
"value, and a collection of name/value pairs.\r\n"
"\r\n"
"JsonCpp is a simple API to manipulate JSON value, handle serialization \r\n"
"and unserialization to string.\r\n"
"\r\n"
"It can also preserve existing comment in unserialization/serialization steps,\r\n"
"making it a convenient format to store user input files.\r\n"
"\r\n"
"Unserialization parsing is user friendly and provides precise error reports.\r\n"
"\r\n"
"\r\n"
"* Building/Testing:\r\n"
"  =================\r\n"
"\r\n"
"JsonCpp uses Scons (http://www.scons.org) as a build system. Scons requires\r\n"
"python to be installed (http://www.python.org).\r\n"
"\r\n"
"You download scons-local distribution from the following url:\r\n"
"http://sourceforge.net/project/showfiles.php?group_id=30337&package_id=67375\r\n"
"\r\n"
"Unzip it in the directory where you found this README file. scons.py Should be \r\n"
"at the same level as README.\r\n"
"\r\n"
"python scons.py platform=PLTFRM [TARGET]\r\n"
"where PLTFRM may be one of:\r\n"
"\tsuncc Sun C++ (Solaris)\r\n"
"\tvacpp Visual Age C++ (AIX)\r\n"
"\tmingw \r\n"
"\tmsvc6 Microsoft Visual Studio 6 service pack 5-6\r\n"
"\tmsvc70 Microsoft Visual Studio 2002\r\n"
"\tmsvc71 Microsoft Visual Studio 2003\r\n"
"\tmsvc80 Microsoft Visual Studio 2005\r\n"
"\tlinux-gcc Gnu C++ (linux, also reported to work for Mac OS X)\r\n"
"\t\r\n"
"adding platform is fairly simple. You need to change the Sconstruct file \r\n"
"to do so.\r\n"
"\t\r\n"
"and TARGET may be:\r\n"
"\tcheck: build library and run unit tests.\r\n"
"\r\n"
"    \r\n"
"* Running the test manually:\r\n"
"  ==========================\r\n"
"\r\n"
"cd test\r\n"
"# This will run the Reader/Writer tests\r\n"
"python runjsontests.py \"path to jsontest.exe\"\r\n"
"\r\n"
"# This will run the Reader/Writer tests, using JSONChecker test suite\r\n"
"# (http://www.json.org/JSON_checker/).\r\n"
"# Notes: not all tests pass: JsonCpp is too lenient (for example,\r\n"
"# it allows an integer to start with '0'). The goal is to improve\r\n"
"# strict mode parsing to get all tests to pass.\r\n"
"python runjsontests.py --with-json-checker \"path to jsontest.exe\"\r\n"
"\r\n"
"# This will run the unit tests (mostly Value)\r\n"
"python rununittests.py \"path to test_lib_json.exe\"\r\n"
"\r\n"
"You can run the tests using valgrind:\r\n"
"python rununittests.py --valgrind \"path to test_lib_json.exe\"\r\n"
"\r\n"
"\r\n"
"* Building the documentation:\r\n"
"  ===========================\r\n"
"\r\n"
"Run the python script doxybuild.py from the top directory:\r\n"
"\r\n"
"python doxybuild.py --open --with-dot\r\n"
"\r\n"
"See doxybuild.py --help for options. \r\n"
"\r\n"
"\r\n"
"* Adding a reader/writer test:\r\n"
"  ============================\r\n"
"\r\n"
"To add a test, you need to create two files in test/data:\r\n"
"- a TESTNAME.json file, that contains the input document in JSON format.\r\n"
"- a TESTNAME.expected file, that contains a flatened representation of \r\n"
"  the input document.\r\n"
"  \r\n"
"TESTNAME.expected file format:\r\n"
"- each line represents a JSON element of the element tree represented \r\n"
"  by the input document.\r\n"
"- each line has two parts: the path to access the element separated from\r\n"
"  the element value by '='. Array and object values are always empty \r\n"
"  (e.g. represented by either [] or {}).\r\n"
"- element path: '.' represented the root element, and is used to separate \r\n"
"  object members. [N] is used to specify the value of an array element\r\n"
"  at index N.\r\n"
"See test_complex_01.json and test_complex_01.expected to better understand\r\n"
"element path.\r\n"
"\r\n"
"\r\n"
"* Understanding reader/writer test output:\r\n"
"  ========================================\r\n"
"\r\n"
"When a test is run, output files are generated aside the input test files. \r\n"
"Below is a short description of the content of each file:\r\n"
"\r\n"
"- test_complex_01.json: input JSON document\r\n"
"- test_complex_01.expected: flattened JSON element tree used to check if \r\n"
"    parsing was corrected.\r\n"
"\r\n"
"- test_complex_01.actual: flattened JSON element tree produced by \r\n"
"    jsontest.exe from reading test_complex_01.json\r\n"
"- test_complex_01.rewrite: JSON document written by jsontest.exe using the\r\n"
"    Json::Value parsed from test_complex_01.json and serialized using\r\n"
"    Json::StyledWritter.\r\n"
"- test_complex_01.actual-rewrite: flattened JSON element tree produced by \r\n"
"    jsontest.exe from reading test_complex_01.rewrite.\r\n"
"test_complex_01.process-output: jsontest.exe output, typically useful to\r\n"
"    understand parsing error.\r\n";

const char* README_txt = (const char*) temp_binary_data_1;

//================== version ==================
static const unsigned char temp_binary_data_2[] =
"0.5.0";

const char* version = (const char*) temp_binary_data_2;


const char* getNamedResource (const char*, int&) throw();
const char* getNamedResource (const char* resourceNameUTF8, int& numBytes) throw()
{
    unsigned int hash = 0;
    if (resourceNameUTF8 != 0)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0xa2ea79ba:  numBytes = 154; return sconscript;
        case 0x2aaab85f:  numBytes = 4240; return README_txt;
        case 0x14f51cd8:  numBytes = 5; return version;
        default: break;
    }

    numBytes = 0;
    return 0;
}

const char* namedResourceList[] =
{
    "sconscript",
    "README_txt",
    "version"
};

}
