//--------------------------------------------------------------------------------------------------
// INCLUDE FILES 
//--------------------------------------------------------------------------------------------------

// Standard Includes 

// MacAppTypes.r must be first
#ifndef __MacAppTypes__
#include "MacAppTypes.r"
#endif

#ifndef __Balloons__
#include "Balloons.r"
#endif

#ifndef __ViewTypes__
#include "ViewTypes.r"
#endif

#ifndef __FILETYPESANDCREATORS_R__
#include "FileTypesAndCreators.r"
#endif

#include "XPostFacto.h"

#include "AEUserTermTypes.r"
#include "AERegistry.r"

resource kAETerminologyExtension (0, 
#if qNames
	"English Terminology",
#endif
	purgeable) {
	0x1,
	0x0,
	english,
	roman,
	{	/* array Suites: 4 elements */

//========================================================================================
// Required Suite
//========================================================================================

		/* [1] */
		"Required Suite",
		"Terms that every application should support",
		kAERequiredSuite,
		1,
		1,
//----------------------------------------------------------------------------------------
// Events
//----------------------------------------------------------------------------------------
		{	/* array Events: 0 elements */
		},
//----------------------------------------------------------------------------------------
// Classes
//----------------------------------------------------------------------------------------
		{	/* array Classes: 0 elements */
		},
//----------------------------------------------------------------------------------------
// ComparisonOps
//----------------------------------------------------------------------------------------
		{	/* array ComparisonOps: 0 elements */
		},
//----------------------------------------------------------------------------------------
// Enumerations
//----------------------------------------------------------------------------------------
		{	/* array Enumerations: 0 elements */
		},

//========================================================================================
// Standard Suite
//========================================================================================

		/* [2] */
		"Standard Suite",
		"Common terms for most applications",
		'CoRe',
		1,
		1,
//----------------------------------------------------------------------------------------
// Events
//----------------------------------------------------------------------------------------
		{	/* array Events: 14 elements */
			/* [1] */
		"close", "Close an object", kAECoreSuite, kAEClose, noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"the object to close",
			directParamRequired, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 2 elements */
				/* [1] */
			"saving", keyAESaveOptions, enumSaveOptions,
				"specifies whether changes should be saved before closing",
				optional, singleItem, enumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular,
				/* [2] */
			"saving in", keyAEFile, typeAlias,
				"the file in which to save the object",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular
			},
			/* [2] */
		"count", "Return the number of elements of a particular class within an object",
			kAECoreSuite, kAECountElements,
			typeLongInteger,
			"the number of elements",
			replyRequired, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"the object whose elements are to be counted",
			directParamRequired, singleItem, notEnumerated, doesntChangeState, Reserved12,
			{	/* array OtherParams: 1 elements */
				/* [1] */
			"each", keyAEObjectClass, cType,
				"the class of the elements to be counted",
				required, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular
			},
			/* [3] */
		"data size", "Return the size in bytes of an object", kAECoreSuite, kAEGetDataSize,
			cLongInteger,
			"the size of the object in bytes",
			replyRequired, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"the object whose data size is to be returned",
			directParamRequired, singleItem, notEnumerated, doesntChangeState, Reserved12,
			{	/* array OtherParams: 1 elements */
				/* [1] */
			"as", keyAERequestedType, cType,
				"the data type for which the size is calculated",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular
			},
			/* [4] */
		"delete", "Delete an element from an object", kAECoreSuite, kAEDelete,
			noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"the element to delete",
			directParamRequired, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 0 elements */
			},
			/* [5] */
		"duplicate", "Duplicate objects(s)", kAECoreSuite, kAEClone,
			typeObjectSpecifier,
			"",
			replyRequired, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"the object(s) to duplicate",
			directParamRequired, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 1 elements */
				/* [1] */
			"to", keyAEInsertHere, typeInsertionLoc,
				"the new location for the object(s)",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular
			},
			/* [6] */
		"exists", "Verify if an object exists", kAECoreSuite, kAEDoObjectsExist,
			typeBoolean,
			"true if it exists, false if not",
			replyRequired, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"the object in question",
			directParamRequired, singleItem, notEnumerated, doesntChangeState, Reserved12,
			{	/* array OtherParams: 0 elements */
			},
			/* [7] */
		"make", "Make a new element", kAECoreSuite, kAECreateElement,
			typeObjectSpecifier,
			"to the new object(s)",
			replyRequired, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			noParams,
			"",
			directParamOptional, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 4 elements */
				/* [1] */
			"new", keyAEObjectClass, cType,
				"the class of the new element.",
				required, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular,
				/* [2] */
			"at", keyAEInsertHere, typeInsertionLoc,
				"the location at which to insert the element",
				required, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular,
				/* [3] */
			"with data", keyAEData, typeWildCard,
				"the initial data for the element",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular,
				/* [4] */
			"with properties", keyAEPropData, 'reco',
				"the initial values for the properties of the element",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular
			},
			/* [8] */
		"move", "Move object(s) to a new location", kAECoreSuite, kAEMove,
			typeObjectSpecifier,
			"to the object(s) after they have been moved",
			replyRequired, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"the object(s) to move",
			directParamRequired, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 1 elements */
				/* [1] */
			"to", keyAEInsertHere, typeInsertionLoc,
				"the new location for the object(s)",
				required, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular
			},
			/* [9] */
		"open",
			"Open the specified object(s)", kCoreEventClass, kAEOpen, noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"list of objects to open",
			directParamRequired, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 0 elements */
			},
			/* [10] */
		"print",
			"Print the specified object(s)", kCoreEventClass, kAEPrint, noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"list of objects to print",
			directParamRequired, singleItem, notEnumerated, doesntChangeState, Reserved12,
			{	/* array OtherParams: 0 elements */
			},
			/* [11] */
		"quit",
			"Quit an application program", kCoreEventClass, kAEQuitApplication, noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			noParams,
			"",
			directParamOptional, singleItem, notEnumerated, doesntChangeState, Reserved12,
			{	/* array OtherParams: 0 elements */
			},
			/* [12] */
		"save",
			"Save an object", kAECoreSuite, kAESave, noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"the object to save",
			directParamRequired, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 3 elements */
				/* [1] */
			"in", keyAEFile, typeAlias,
				"the file in which to save the object",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular,
				/* [2] */
			"as", keyAEFileType, cType,
				"the file type of the document in which to save the data",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular,
				/* [3] */
			"makeCopy", keyAESaveCopy, cBoolean,
				"save a copy of the document without effecting the original",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular
			},
			/* [13] */
		"get",
			"Get the data for an object", kAECoreSuite, kAEGetData, typeWildCard,
			"the data from the object",
			replyRequired, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"the object whose data is to be returned",
			directParamRequired, singleItem, notEnumerated, doesntChangeState, Reserved12,
			{	/* array OtherParams: 1 elements */
				/* [1] */
			"as", keyAERequestedType, cType,
				"the desired types for the data, in order of preference",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular
			},
			/* [14] */
		"set",
			"Set an object's data", kAECoreSuite, kAESetData, noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"the object to change",
			directParamRequired, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 1 elements */
				/* [1] */
			"to", keyAEData, typeWildCard,
				"the new value",
				required, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular
			}
		},
//----------------------------------------------------------------------------------------
// Classes
//----------------------------------------------------------------------------------------
		{	/* array Classes: 16 elements */
			/* [1] */
		"application", cApplication,
			"An application program",
			{	/* array Properties: 8 elements */
				/* [1] */
			"clipboard", pClipboard, typeWildCard,
				"the clipboard", reserved,
				listOfItems,
				notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [2] */
			"frontmost", pIsFrontProcess, typeBoolean,
				"Is this the frontmost application?", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [3] */
			"name", pName, typeIntlText,
				"the name", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [4] */
			"selection", pUserSelection, cSelection,
				"the selection visible to the user", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [5] */
			"version", pVersion, cVersion,
				"the version number of the application", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [6] */
			"class", pClass, cType,
				"the class", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular
			},
			{	/* array Elements: 2 elements */
				/* [1] */
				cDocument,
				{	/* array KeyForms: 2 elements */
					/* [1] */
					formAbsolutePosition,
					/* [2] */
					formName
				},
				/* [2] */
				cWindow,
				{	/* array KeyForms: 2 elements */
					/* [1] */
					formAbsolutePosition,
					/* [2] */
					formName
				},
				/* [3] */
				cPalette,
				{	/* array KeyForms: 2 elements */
					/* [1] */
					formAbsolutePosition,
					/* [2] */
					formName
				}
			},
			/* [2] */
			"applications",
			cApplication,
			"Every application",
			{	/* array Properties: 1 elements */
				/* [1] */
			"", kAESpecialClassProperties, cType,
				"", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, plural
			},
			{	/* array Elements: 0 elements */
			},
			/* [3] */
		"character", cChar,
			"A character",
			{	/* array Properties: 9 elements */
				/* [1] */
			"color", pColor, cRGBColor,
				"the color", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [2] */
			"font", keyAEFont, cText,
				"the name of the font", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [3] */
			"size", pPointSize, cFixed,
				"the size in points", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [4] */
			"writing code", pScriptTag, cIntlWritingCode,
				"the script system and language", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [5] */
			"style", pTextStyles, cTextStyles,
				"the text style", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [6] */
			"uniform styles", pUniformStyles, cTextStyles,
				"the text style", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [7] */
			"class", pClass, cType,
				"the class", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular
			},
			{	/* array Elements: 2 elements */
				/* [1] */
				cChar,
				{	/* array KeyForms: 1 elements */
					/* [1] */
					formAbsolutePosition
				},
				/* [2] */
				cText,
				{	/* array KeyForms: 1 elements */
					/* [1] */
					formAbsolutePosition
				}
			},
			/* [4] */
		"characters", cChar,
			"Every character",
			{	/* array Properties: 1 elements */
				/* [1] */
			"", kAESpecialClassProperties, cType,
				"", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, plural
			},
			{	/* array Elements: 0 elements */
			},
			/* [5] */
		"document", cDocument,
			"A document",
			{	/* array Properties: 6 elements */
				/* [1] */
			"modified", pIsModified, cBoolean,
				"Has the document been modified since the last save?", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [2] */
			"name", pName, cIntlText,
				"the name", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [3] */
			"selection", pSelection, cSelection,
				"the selection visible to the user", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [4] */
			"class", pClass, cType,
				"the class", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular
			},
			{	/* array Elements: 1 elements */
				/* [1] */
				cFile,
				{	/* array KeyForms: 2 elements */
					/* [1] */
					formAbsolutePosition,
					/* [2] */
					formName
				}
			},
			/* [6] */
		"documents", cDocument,
			"Every document",
			{	/* array Properties: 1 elements */
				/* [1] */
			"", kAESpecialClassProperties, cType,
				"", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, plural
			},
			{	/* array Elements: 0 elements */
			},
			/* [7] */
		"file", cFile,
			"A file",
			{	/* array Properties: 5 elements */
				/* [1] */
			"stationery", pIsStationeryPad, cBoolean,
				"Is the file a stationery file?", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [2] */
			"name", pName, cIntlText,
				"the name", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [3] */
			"class", pClass, cType,
				"the class", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular
			},
			{	/* array Elements: 0 elements */
			},
			/* [8] */
		"files", cFile,
			"Every file",
			{	/* array Properties: 1 elements */
				/* [1] */
			"", kAESpecialClassProperties, cType,
				"", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, plural
			},
			{	/* array Elements: 0 elements */
			},
			/* [9] */
		"selection-object", cSelection,
			"the selection visible to the user",
			{	/* array Properties: 4 elements */
				/* [1] */
			"contents", pContents, cType,
				"the contents of the selection", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [2] */
			"class", pClass, cType,
				"the class", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular
			},
			{	/* array Elements: 0 elements */
			},
			/* [10] */
		"text", cText,
			"Text",
			{	/* array Properties: 10 elements */
				/* [1] */
			"", kAESpecialClassProperties, cType,
				"", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, plural,
				/* [2] */
			"color", pColor, cRGBColor,
				"the color of the first character", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [3] */
			"font", pFont, cText,
				"the name of the font of the first character", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [4] */
			"size", pPointSize, cFixed,
				"the size in points of the first character", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [5] */
			"writing code", pScriptTag, cIntlWritingCode,
				"the script system and language of the first character", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [6] */
			"style", pTextStyles, cTextStyles,
				"the text style of the first character", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [7] */
			"uniform styles", pUniformStyles, cTextStyles,
				"the text styles that are uniform throughout the text", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [8] */
			"class", pClass, cType,
				"the class", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular
			},
			{	/* array Elements: 2 elements */
				/* [1] */
			cChar,
				{	/* array KeyForms: 1 elements */
					/* [1] */
					formAbsolutePosition
				},
				/* [2] */
			cText,
				{	/* array KeyForms: 1 elements */
					/* [1] */
					formAbsolutePosition
				}
			},
			/* [11] */
			"text style info",
			cTextStyles,
			"On and Off styles of text run",
			{	/* array Properties: 2 elements */
				/* [1] */
			"on styles", keyAEOnStyles, enumStyle,
				"the styles that are on for the text", reserved,
				listOfItems, enumerated,
				readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [2] */
			"off styles", keyAEOffStyles, enumStyle,
				"the styles that are off for the text", reserved,
				listOfItems, enumerated,
				readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular
			},
			{	/* array Elements: 0 elements */
			},
			/* [12] */
			"text style infos",
			cTextStyles,
			"every text style info",
			{	/* array Properties: 1 elements */
				/* [1] */
			"", kAESpecialClassProperties, cType,
				"", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, plural
			},
			{	/* array Elements: 0 elements */
			},
			/* [13] */
		"window", cWindow,
			"A window",
			{	/* array Properties: 15 elements */
				/* [1] */
			"bounds", pBounds, cQDRectangle,
				"the boundary rectangle for the window", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [2] */
			"closeable", pHasCloseBox, cBoolean,
				"Does the window have a close box?", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [3] */
			"titled", pHasTitleBar, cBoolean,
				"Does the window have a title bar?", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [4] */
			"index", pIndex, cLongInteger,
				"the number of the window", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [5] */
			"modal", pIsModal, cBoolean,
				"Is the window modal?", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [6] */
			"resizable", pIsResizable, cBoolean,
				"Is the window resizable?", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [7] */
			"zoomable", pIsZoomable, cBoolean,
				"Is the window zoomable?", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [8] */
			"zoomed", pIsZoomed, cBoolean,
				"Is the window zoomed?", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [9] */
			"name", pName, cIntlText,
				"the title of the window", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [10] */
			"selection", pSelection, cSelection,
				"the selection visible to the user", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [11] */
			"class", pClass, cType,
				"the class", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular
			},
			{	/* array Elements: 1 elements */
				/* [1] */
				cDocument,
				{	/* array KeyForms: 2 elements */
					/* [1] */
					formAbsolutePosition,
					/* [2] */
					formName
				}
			},
			/* [14] */
		"windows", cWindow,
			"Every window",
			{	/* array Properties: 1 elements */
				/* [1] */
			"", kAESpecialClassProperties, cType,
				"", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, plural
			},
			{	/* array Elements: 0 elements */
			},
			/* [15] */
		"palette", cPalette,
			"A floating palette",
			{	/* array Properties: 15 elements */
				/* [1] */
			"bounds", pBounds, cQDRectangle,
				"the boundary rectangle for the palette", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [2] */
			"closeable", pHasCloseBox, cBoolean,
				"Does the palette have a close box?", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [3] */
			"titled", pHasTitleBar, cBoolean,
				"Does the palette have a title bar?", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [4] */
			"index", pIndex, cLongInteger,
				"the number of the palette", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [5] */
			"floating", pIsFloating, cBoolean,
				"Does the palette float?", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [6] */
			"resizable", pIsResizable, cBoolean,
				"Is the palette resizable?", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [7] */
			"zoomable", pIsZoomable, cBoolean,
				"Is the palette zoomable?", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [8] */
			"zoomed", pIsZoomed, cBoolean,
				"Is the palette zoomed?", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [9] */
			"name", pName, cIntlText,
				"the title of the palette", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [10] */
			"selection", pSelection, cSelection,
				"the selection visible to the user", reserved,
				singleItem, notEnumerated, readWrite, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [11] */
			"class", pClass, cType,
				"the class", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular
			},
			{	/* array Elements: 0 elements */
			},
			/* [16] */
		"palettes", cPalette,
			"Every palette",
			{	/* array Properties: 1 elements */
				/* [1] */
			"", kAESpecialClassProperties, cType,
				"", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, plural
			},
			{	/* array Elements: 0 elements */
			},
			/* [17] */
		"insertion point", cInsertionPoint,
			"An insertion location between two objects",
			{	/* array Properties: 3 elements */
			"class", pClass, cType,
				"the class", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular
			},
			{	/* array Elements: 0 elements */
			},
			/* [18] */
		"insertion points", cInsertionPoint,
			"Every insertion location",
			{	/* array Properties: 1 elements */
				/* [1] */
				"",
				kAESpecialClassProperties,
				cType,
				"", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, plural
			},
			{	/* array Elements: 0 elements */
			}
		},
//----------------------------------------------------------------------------------------
// ComparisonOps
//----------------------------------------------------------------------------------------
		{	/* array ComparisonOps: 8 elements */
			"starts with", kAEBeginsWith, "Starts with",
			"contains", kAEContains, "Contains",
			"ends with", kAEEndsWith, "Ends with",
			"=", kAEEquals, "Equal",
			">", kAEGreaterThan, "Greater than",
			"³", kAEGreaterThanEquals, "Greater than or equal to",
			"<", kAELessThan, "Less than",
			"²", kAELessThanEquals, "Less than or equal to"
		},
//----------------------------------------------------------------------------------------
// Enumerations
//----------------------------------------------------------------------------------------
		{	/* array Enumerations: 2 elements */
			/* [1] */
			enumSaveOptions,
			{	/* array Enumerators: 3 elements */
				"yes", kAEYes, "Save objects now",
				"no", kAENo, "Do not save objects",
				"ask", kAEAsk, "Ask the user whether to save"
			},
			/* [2] */
			enumKeyForm,
			{	/* array Enumerators: 3 elements */
				"index", formAbsolutePosition, "keyform designating indexed access",
				"named", formName, "keyform designating named access",
				"ID", formUniqueID, "keyform designating access by unique identifer"
			}
		},

//========================================================================================
// Miscellaneous Standards
//========================================================================================

		/* [3] */
	"Miscellaneous Standards",
		"Miscellaneous standard events and classes",
		'misc',
		1,
		1,
//----------------------------------------------------------------------------------------
// Events
//----------------------------------------------------------------------------------------
		{	/* array Events: 12.1 elements */
			/* [11.1] */
		"revert", "Revert an object to its last saved state", kAEMiscStandards, kAERevert,
			noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"list of object(s) to revert",
			directParamRequired, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 0 elements */
			},
			/* [1] */
		"cut", "Cut an object to the clipboard", kAEMiscStandards, kAECut,
			noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			noParams,
			"",
			directParamOptional, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 0 elements */
			},
			/* [2] */
		"copy", "Copy an object to the clipboard", kAEMiscStandards, kAECopy,
			noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			noParams,
			"",
			directParamOptional, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 0 elements */
			},
			/* [3] */
		"paste", "Paste an object from the clipboard", kAEMiscStandards, kAEPaste,
			noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			noParams,
			"",
			directParamOptional, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 0 elements */
			},
			/* [4] */
		"redo", "Reverse the action of the immediately preceeding undo", kAEMiscStandards, kAERedo,
			noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			noParams,
			"",
			directParamOptional, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 0 elements */
			},
			/* [5] */
		"undo", "Undo the action of the previous event or user interaction", kAEMiscStandards, kAEUndo,
			noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			noParams,
			"",
			directParamOptional, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 0 elements */
#if 0
			},
			/* [6] */
		"begin transaction", "Begin a transaction", kAEMiscStandards, kAEBeginTransaction,
			noReply,
			"the transaction id",
			replyRequired, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			noParams,
			"",
			directParamRequired, singleItem, notEnumerated, doesntChangeState, Reserved12,
			{	/* array OtherParams: 0 elements */
			},
			/* [7] */
		"end transaction", "End a transaction", kAEMiscStandards, kAEEndTransaction,
			noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			noParams,
			"",
			directParamRequired, singleItem, notEnumerated, doesntChangeState, Reserved12,
			{	/* array OtherParams: 0 elements */
			},
			/* [8] */
		"create publisher", "Create a publisher", kAEMiscStandards, kAECreatePublisher,
			noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"object(s) to publish (default value is the user selection)",
			directParamOptional, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 1 elements */
				/* [1] */
				"edition",
				keyAEEditionFileLoc,
				typeAlias,
				"Alias to the edition container file for the publisher",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular
			},
			/* [9] */
		"edit graphic",
			"let the user edit the specified drawing area",
			kAEMiscStandards, kAEEditGraphic,
			typeDrawingArea,
			"the edited drawing area",
			replyRequired, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"the drawing area to be edited",
			directParamRequired, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 0 elements */
			},
			/* [10] */
		"image graphic", "convert a graphic from one format to another",
			kAEMiscStandards, kAEImageGraphic,
			typeWildCard,
			"the graphic that is returned",
			replyRequired, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"the graphic to be converted or formatted",
			directParamRequired, singleItem, notEnumerated, doesntChangeState, Reserved12,
			{	/* array OtherParams: 10 elements */
				/* [1] */
			"antialiasing", keyAEDoAntiAlias, typeBoolean,
				"use antialising when imaging the graphic",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular,
				/* [2] */
			"dithering", keyAEDoDithered, typeBoolean,
				"use dithering when imaging the graphic",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular,
				/* [3] */
			"rotate", keyAEDoRotate, typeRotation,
				"description of how to rotate the graphic",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular,
				/* [4] */
			"scale", keyAEDoScale, typeFixed,
				"the factor by which to scale the image",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular,
				/* [5] */
			"translation", keyAEDoTranslate, typeQDPoint,
				"offset by which the image should be moved",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular,
				/* [6] */
			"flip horizontal", keyAEFlipHorizontal, typeBoolean,
				"if true, flip the graphic horizontally",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular,
				/* [7] */
			"flip vertically", keyAEFlipVertical, typeBoolean,
				"if true, flip the image vertically",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular,
				/* [8] */
			"quality", keyAEImageQuality, enumQuality,
				"the image quality of the resulting graphic",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular,
				/* [9] */
			"format", keyAERenderAs, typeType,
				"graphics format in which to return the resulting image",
				required, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular,
				/* [10] */
			"structured graphic", keyAETryAsStructGraf, typeBoolean,
				"if true, return an object graphic (a graphic composed of individual "
				"graphic objects that can be manipulated separately) rather than a "
				"pixel map",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular
			},
			/* [11] */
		"is uniform",
			"is the value of the specified property the same for all objects in the set?",
			kAEMiscStandards, kAEIsUniform,
			typeBoolean,
			"indicates whether all the objects in the set have the same value for the "
			"specified property",
			replyRequired, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"the set of objects to be checked for uniformity",
			directParamRequired, singleItem, notEnumerated, doesntChangeState, Reserved12,
			{	/* array OtherParams: 1 elements */
				/* [1] */
			"in", keyAEProperty, typeType,
				"the property ID for the property to be checked",
				required, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular
			},
			/* [12] */
		"show", "Bring an object into view", kAEMiscStandards, kAEMakeObjectsVisible,
			noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, Reserved8, verbEvent, Reserved3,
			typeObjectSpecifier,
			"the object(s) to make visible",
			directParamRequired, singleItem, notEnumerated, changesState, Reserved12,
			{	/* array OtherParams: 2 elements */
				/* [1] */
			"in", keyAEWindow, typeObjectSpecifier,
				"the window in which to display the object(s)",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular,
				/* [2] */
			"at", keyAEShowWhere, typeQDPoint,
				"the point (in window coordinates) at which to display the top-left corner"
				" of the top-left object",
				optional, singleItem, notEnumerated, Reserved9, prepositionParam, notFeminine, notMasculine, singular
#endif
			}
		},
//----------------------------------------------------------------------------------------
// Classes
//----------------------------------------------------------------------------------------
		{	/* array Classes: 4 elements */
			/* [1] */
			"menu",
			'cmnu',
			"A menu",
			{	/* array Properties: 5 elements */
				/* [1] */
			"class", 'pcls', 'type',
				"The class", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [2] */
			"menu ID", 'mnid', 'shor',
				"the menu ID for the menu", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [5] */
			"name", 'pnam', 'itxt',
				"the name", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular
			},
			{	/* array Elements: 1 elements */
				/* [1] */
				'cmen',
				{	/* array KeyForms: 2 elements */
					/* [1] */
					'indx',
					/* [2] */
					'name'
				}
			},
			/* [2] */
		"menus", 'cmnu',
			"Every menu",
			{	/* array Properties: 1 elements */
				/* [1] */
			"", kAESpecialClassProperties, 'type',
				"", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, plural
			},
			{	/* array Elements: 0 elements */
			},
			/* [3] */
		"menu item", 'cmen',
			"A menu item",
			{	/* array Properties: 6 elements */
				/* [1] */
			"class", 'pcls', 'type',
				"the class", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [2] */
			"enabled", 'enbl', 'bool',
				"Is the menu item is enabled?", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [5] */
			"item number", 'itmn', 'shor',
				"the menu item number", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular,
				/* [6] */
			"name", 'pnam', 'itxt',
				"The name", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, singular
			},
			{	/* array Elements: 0 elements */
			},
			/* [4] */
		"menu items", 'cmen',
			"Every menu item",
			{	/* array Properties: 1 elements */
				/* [1] */
			"", kAESpecialClassProperties, 'type',
				"", reserved,
				singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, plural
			},
			{	/* array Elements: 0 elements */
			}
		},
//----------------------------------------------------------------------------------------
// ComparisonOps
//----------------------------------------------------------------------------------------
		{	/* array ComparisonOps: 0 elements */
		},
//----------------------------------------------------------------------------------------
// Enumerations
//----------------------------------------------------------------------------------------
		{	/* array Enumerations: 1 element */
			/* [1] */
			enumQuality,
			{	/* array Enumerators: 3 elements */
				"draft", kAEFast, "image the graphic as quickly as possible",
				"regular", kAERegular, "image the graphic normally",
				"high", kAEHiQuality, "image the graphic at the highest quality possible"
			}
		},
	}
};
