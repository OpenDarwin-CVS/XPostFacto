/*

Copyright (c) 2003
Other World Computing
All rights reserved

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer as the first lines of
   each file.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

3. Redistributions in binary form must retain the link to Other World
   Computing's web site in the application's "About Box."

This software is provided by Other World Computing ``as is'' and any express or implied
warranties, including, but not limited to, the implied warranties of
merchantability and fitness for a particular purpose are disclaimed. In no event
shall Ryan Rempel or Other World Computing be liable for any direct, indirect,
incidental, special, exemplary, or consequential damages (including, but not
limited to, procurement of substitute goods or services; loss of use, data, or
profits; or business interruption) however caused and on any theory of
liability, whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software, even if
advised of the possibility of such damage.

*/

#ifndef __XPFUPDATE_H__
#define __XPFUPDATE_H__

class MountedVolume;
class XPFUpdateItem;

enum XPFUpdateAction {
	kActionNone,
	kActionDelete,
	kActionInstall
};

typedef TemplateAutoList_AC <XPFUpdateItem> XPFUpdateItemList;
typedef TemplateAutoList_AC <XPFUpdateItem>::Iterator XPFUpdateListIterator;

class XPFUpdate : public MDependable_AC
{
	public:
	
		XPFUpdate (MountedVolume *target, MountedVolume *helper = NULL, MountedVolume *fInstallCD = NULL);
		
		MountedVolume *getTarget () {return fTarget;}
		MountedVolume *getHelper () {return fHelper;}
		MountedVolume *getInstallCD () {return fInstallCD;}
		
		XPFUpdateItem *getItemWithResourceID (ResNumber resourceID);
		XPFUpdateItem *getItemWithResourceName (CStr255_AC name);
		
		XPFUpdateItemList *getItemList () {return &fItemList;}
		
		bool getRequiresAction ();
				
	private:
	
		MountedVolume *fTarget;
		MountedVolume *fHelper;
		MountedVolume *fInstallCD;
		XPFUpdateItemList fItemList;
		
};

class XPFUpdateItem : public MDependable_AC
{

	public:
		XPFUpdateItem (XPFUpdate *update);
	
		bool getEnabled () {return fEnabled;}
		void setEnabled (bool enabled);
		
		UInt32 getInstalledVersion () {return fInstalledVersion;}
		UInt32 getAvailableVersion () {return fAvailableVersion;}
		
		XPFUpdateAction getAction ();
		bool getIsQualified ();
		
		CStr255_AC getResourceName () {return fResourceName;}
		ResNumber getResourceID () {return fResourceID;}
		
		virtual void doUpdate () = 0;
		virtual void uninstall () = 0;
				
	private:
		bool fEnabled;
		
	protected:
		void parsePListString (char *plist, char *search, char *result);
		void getResourceInfo (ResType resType, SInt16 resIndex);
		void getFileInfo (FSRef *file);

		XPFUpdate *fUpdate;
	
		UInt32 fInstalledVersion;
		UInt32 fAvailableVersion;
		
		int fMinBuild;
		int fMaxBuild;
		
		ResNumber fResourceID;
		CStr255_AC fResourceName;
		
};

class XPFExtensionsUpdate : public XPFUpdateItem
{
	
	public:
	
		XPFExtensionsUpdate (XPFUpdate *update, SInt16 resIndex);
		virtual void doUpdate ();
		virtual void uninstall ();
		
};


class XPFBootXUpdate : public XPFUpdateItem
{

	public:
	
		XPFBootXUpdate (XPFUpdate *update);
		virtual void doUpdate ();
		virtual void uninstall ();

};

class XPFStartupItemUpdate : public XPFUpdateItem
{

	public:
	
		XPFStartupItemUpdate (XPFUpdate *update, SInt16 resIndex);
		virtual void doUpdate ();
		virtual void uninstall ();
		
	private:
	
		void moveToTmp ();

};

#endif
