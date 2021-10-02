/* SiCfg.h -- Configuration Saving Functions Header File
 *
 * These functions are used to save and retrieve application configuration info
 * in the registry.   
 *
 * Strings are TCHARs.  The defaut library is built with WCHARs.  Src code is provided
 * to rebuild it with ANSI chars if your application uses chars.
 *
 * Anything can be saved because everything is a string.
 *
 * We suggest that the following conventions be used:
 * 
 * appName is the name of the application (e.g., "GUISync_SDK")
 * modeName is the name of the application mode (e.g., "Sketch")
 * configName is the name the user gives to the specific configuration, or a default, 
 *		(e.g., "Electrical", 
 *             "Mechanical 1",
 *             "Steve 1",
 *             "Small Assembly Work")
 * settingName can optionally indicate the 3Dconnexion device # and the parameter name:
 *		(e.g., "29_Button 4" is SpacePilot button number 4,
 *             "Button 4" is a device independent button 4)
 * settingValue should indicate the command source and a value that is assigned to this button (or whatever):
 *      (e.g., "D_23" means Driver function 23,
 *             "D_-3" means Driver macro 3,
 *              A_1005 means Application function 1005
 *      )
 */

SpwReturnValue SiCfgSaveSetting( const TCHAR *appName, const TCHAR *modeName, const TCHAR *configName, const TCHAR *settingName, const TCHAR *settingValue );
SpwReturnValue SiCfgGetSetting( const TCHAR *appName, const TCHAR *modeName, const TCHAR *configName, const TCHAR *settingName, TCHAR *settingValue, SPWuint32 *pmaxValueLen );
SpwReturnValue SiCfgGetModes( const TCHAR *appName, TCHAR *modeName, SPWuint32 *pmaxNameLen );
SpwReturnValue SiCfgGetModesNext( const TCHAR *appName, TCHAR *modeName, SPWuint32 *pmaxNameLen );
SpwReturnValue SiCfgGetNames( const TCHAR *appName, const TCHAR *modeName, TCHAR *configName, SPWuint32 *pmaxNameLen );
SpwReturnValue SiCfgGetNamesNext( const TCHAR *appName, const TCHAR *modeName, TCHAR *configName, SPWuint32 *pmaxNameLen );
