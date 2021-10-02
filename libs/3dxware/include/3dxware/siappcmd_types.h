#ifndef siappcmd_types_H_INCLUDED_
#define siappcmd_types_H_INCLUDED_
/* siappcmd_types.h */
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
/* $Id: siappcmd_types.h 11232 2015-02-19 14:56:09Z mbonk $
/*
*/
/************************************************************************************
/* File Description:

    This header file describes the variable types used in the 3dconnexion interface 
    that allows a user to assign an arbitrary action to a 3dconnexion device button.
    
    Data structures are described in detail below.
***********************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


typedef enum siActionNodeType_e
{
  SI_ACTIONSET_NODE=0
  , SI_CATEGORY_NODE
  , SI_ACTION_NODE
} SiActionNodeType_t;

/*------------------------------------+---------------------------------------

    SiActionNode_t 

    The application passes a pointer to a structure of type SiActionNode_t to
    the function SiAppCmdWriteActionSet

    A set of actions is composed of a linked list of SiActionNode_t structures. 
    Sibling nodes are linked by the next field of the structure and child nodes 
    by the children field. The root node of the tree represents the name of the 
    action set while the leaf nodes of the tree represent the actions that can be 
    assigned to buttons and invoked by the user. The intermediate nodes represent 
    categories and sub-categories for the actions. An example of this would be the 
    menu item structure in a menu bar. The menus in the menu bar would be 
    represented by the SiActionNode_t structures with type SI_CATEGORY_NODE pointed 
    to by each successively linked next field and the first menu item of each menu 
    represented by the structure pointed to by their child fields (the rest of the 
    menu items in each menu would again be linked by the next fields).   

    id
        The id field specifies a UTF8 string identifier for the action set, 
        category, or action represented by the node. The field is always non-NULL. 
        This string needs to remain constant across application sessions and more 
        or less constant across application releases. The id is used by the 
        application to identify an action.

    label
        The label field specifies a UTF8 localized/internationalized description 
        for the action set, category, or action represented by the node. The label 
        field can be NULL for the root and intermediate category nodes that are not 
        explicitly presented to users. All leaf (action) and intermediate nodes 
        containing leaf nodes have non-NULL labels. If the application only has a 
        single action tree set, then the label of the root (context) node can also 
        be NULL.

    type
        The type field specifies one of the following values.
            SI_ACTIONSET_NODE
            SI_CATEGORY_NODE
            SI_ACTION_NODE

        The root node (and only the root node) of the tree always has type
        SI_ACTIONSET_NODE. Only the leaf nodes of the tree have type SI_ACTION_NODE. 
        All intermediate nodes have type SI_CATEGORY_NODE.


--------------------------------------+-------------------------------------*/
typedef struct siActionNode_s
{
    struct siActionNode_s       *next;
    struct siActionNode_s       *children;
    const char                  *id;
    const char                  *label;
    SiActionNodeType_t          type;
} SiActionNode_t;


#ifdef __cplusplus
} // extern "C"
#endif

#endif  /* siappcmd_types_H_INCLUDED_  */
