#ifndef siappcmd_H_INCLUDED_
#define siappcmd_H_INCLUDED_
/* siappcmd.h */
/*
* Copyright notice:
* (c) 2013-2015 3Dconnexion. All rights reserved. 
* 
* This file and source code are an integral part of the "3Dconnexion
* Software Developer Kit", including all accompanying documentation,
* and is protected by intellectual property laws. All use of the
* 3Dconnexion Software Developer Kit and this file is subject to the License
* Agreement found in the "LicenseAgreementSDK.txt" file.
* All rights not expressly granted by 3Dconnexion are reserved.
*
*/
/************************************************************************************
/* File History
/*
/* $Id: siappcmd.h 11374 2015-05-04 09:11:12Z mbonk $
/*
*/
/************************************************************************************
/* File Description:

    This 3Dconnexion application extension is a set of methods that allows users to 
    press buttons on a 3dconnexion device to invoke arbitrary actions.
    
    A 3dconnexion utility is responsible for supporting user customization of 3D Input 
    device buttons. The application is responsible for passing application action 
    information to the 3dconnexion library, and for invoking actions identified by the 
    3dconnexion application extension.
    
    The application invokes the following library functions.
    

    SiAppCmdWriteActionSet

        The application calls this function to write a context sensitive set of actions
        to the 3dconnexion library. The action set is passed as a tree consisting of 
        categories and actions. An application may have either only one set of actions 
        through the whole life time of the application, or may change the set of actions 
        depending on the current working environment or context.

    SiAppCmdActivateActionSet

        The application calls this function immediately after enabling an action set,
        passing the id for an action tree previously written to the 3dconnexion library. 
        If the application enables a new action set, it will also need to write the whole 
        action tree (using SiAppCmdWriteActions) to the 3dconnexion library. 


    Data structures are described in siappcmd_types.h
    The functions are described in detail below.
***********************************************************************************************/
#include <si.h>
#include <siappcmd_types.h>
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------+---------------------------------------

    SiAppCmdWriteActionSet

    SiAppCmdWriteActionSet is invoked by the application to communicate a set of actions
    (application commands) to the 3Dconnexion library. The library caches the action information 
    to support user customization of action-button mappings.

    The application must pass the entire action tree on each invocation of SiAppCmdWriteActionSet 
    for a particular environment or context.
     
    Parameters:
          Sihdl           - active 3dware connection
          SiActionNode_t* - action_tree (memory owned by the function caller)

    Result:
          SPW_NO_ERROR
          SI_BAD_HANDLE


--------------------------------------+-------------------------------------*/

enum SpwRetVal SiAppCmdWriteActionSet (SiHdl hdl, const SiActionNode_t *action_tree);


/*------------------------------------+---------------------------------------

    SiAppCmdActivateActionSet

    SiAppCmdActivateActionSet is invoked  by the application after enabling an action set.
    The action tree passed in consists of a single action set node identifying the 
    the action set.

    The function activates the action set named in the id.

    Parameters:
      Sihdl           - active 3dware connection
      char*           - action set id (memory owned by the function caller)

    Result:
      SPW_NO_ERROR
      SI_BAD_HANDLE
      SI_BAD_VALUE

--------------------------------------+-------------------------------------*/

enum SpwRetVal SiAppCmdActivateActionSet (SiHdl hdl, const char *action_set_id);


#ifdef __cplusplus
} // extern "C"
#endif

#endif  /* siappcmd_H_INCLUDED_  */
