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

#ifndef __XPFUPDATECOMMANDS_H__
#define __XPFUPDATECOMMANDS_H__

#include "XPFThreadedCommand.h"

class XPFInstallBootXCommand : public XPFThreadedCommand {

	public:
	
		XPFInstallBootXCommand (XPFUpdate *update) : XPFThreadedCommand (update) {}
				
		void DoItInProgressWindow ();
		
};	

class XPFInstallExtensionsCommand : public XPFThreadedCommand {

	public:
	
		XPFInstallExtensionsCommand (XPFUpdate *update) : XPFThreadedCommand (update) {}
				
		void DoItInProgressWindow ();
							
};	

class XPFInstallStartupCommand : public XPFThreadedCommand {

	public:

		XPFInstallStartupCommand (XPFUpdate *update) : XPFThreadedCommand (update) {}
									
		void DoItInProgressWindow ();

};

class XPFRecopyHelperFilesCommand : public XPFThreadedCommand {

	public:

		XPFRecopyHelperFilesCommand (XPFUpdate *update) : XPFThreadedCommand (update) {}
									
		void DoItInProgressWindow ();

};	

class XPFBlessMacOS9SystemFolderCommand : public XPFThreadedCommand {

	public:
	
		XPFBlessMacOS9SystemFolderCommand (XPFUpdate *update) : XPFThreadedCommand (update) {}
									
		void DoItInProgressWindow ();
	
};

class XPFUpdateCommand : public XPFThreadedCommand {

	public:
		
		XPFUpdateCommand (XPFUpdate *update) : XPFThreadedCommand (update) {}
								
		void DoItInProgressWindow ();					
			
};	

class XPFUninstallCommand : public XPFThreadedCommand {

	public:
	
		XPFUninstallCommand (XPFUpdate *update) : XPFThreadedCommand (update) {}
									
		void DoItInProgressWindow ();

};

class XPFEmptyCacheCommand : public XPFThreadedCommand {

	public:
	
		XPFEmptyCacheCommand (XPFUpdate *update) : XPFThreadedCommand (update) {}
									
		void DoItInProgressWindow ();

};

class XPFCheckPermissionsCommand : public XPFThreadedCommand {

	public:
	
		XPFCheckPermissionsCommand (XPFUpdate *update) : XPFThreadedCommand (update) {}
									
		void DoItInProgressWindow ();
		
	private:
	
		OSErr checkPermissions (FSRef *rootDirectory);

};

class XPFFixSymlinksCommand : public XPFThreadedCommand {

	public:
	
		XPFFixSymlinksCommand (XPFUpdate *update) : XPFThreadedCommand (update) {}
									
		void DoItInProgressWindow ();
		
};


#endif
