data 'TxSt' (8715, "System 12, plain, { 0, 0, 65535 }") {
	$"0088 000C 0000 0000 FFFF 00"                                                                        /* .ˆ......ÿÿ. */
};

data 'TxSt' (27392, "Application 10, plain") {
	$"009C 000A 0000 0000 0000 0161"                                                                      /* .œ.Â.......a */
};

resource 'STR#' (1005, "About Box", purgeable) {
	{	/* array StringArray: 3 elements */
		/* [1] */
		"http://eshop.macsales.com/OSXCenter/XPostFacto/",
		/* [2] */
		"For updates and information about this utility, visit Other World Computing's we"
		"b site at:",
		/* [3] */
		"About XPostFacto"
	}
};

resource 'View' (1005, "About Box", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 4 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'wind',
			461,
			"XPFAboutBox",
			'WIND',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MAThreeOhView {

			},
			{	/* array: 2 elements */
				/* [1] */
				45,
				/* [2] */
				45
			},
			{	/* array: 2 elements */
				/* [1] */
				406,
				/* [2] */
				623
			},
			sizeVariable,
			sizeVariable,
			notShown,
			doesntWantToBeTarget,
			handlesCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			handlesHelp,
			letsSubViewsHandleHelp,
			noHelpID,
			1,
			NoDrawingEnvironment {

			},
			AdornerListSignatureAndClassname {
				NoIdentifier,
				"TAdornerList",
				AdornerElementSize,
				AdornerElementSizeShift,
				DynamicArrayAllocationIncrement,
				{	/* array AdornerElementArray: 3 elements */
					/* [1] */
					AdornFirst,
					AdornerLocalObject {
						EraseAdorner
					},
					/* [2] */
					DrawView,
					AdornerLocalObject {
						DrawAdorner
					},
					/* [3] */
					AdornLast,
					AdornerLocalObject {
						ResizeIconAdorner
					}
				}
			},
			emptyUserArea,
			Window {
				zoomDocProc,
				noID,
				goAwayBox,
				notResizable,
				ignoreFirstClick,
				dontFreeOnClosing,
				disposeOnFree,
				doesntCloseDocument,
				dontOpenWithDocument,
				dontAdaptToScreen,
				dontStagger,
				forceOnScreen,
				dontCenter,
				doesntFloat,
				doesntHideOnSuspend,
				generateActivates,
				filler,
				1005,
				3
			},
			3
		},
		/* [2] */
		ViewSignatureAndClassname {
			'tevw',
			104,
			"TTEView",
			'Copy',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				54,
				/* [2] */
				16
			},
			{	/* array: 2 elements */
				/* [1] */
				338,
				/* [2] */
				594
			},
			sizeVariable,
			sizeVariable,
			shown,
			wantsToBeTarget,
			doesntHandleCursor,
			letsSubViewsHandleCursor,
			1,
			doesntHandleHelp,
			letsSubViewsHandleHelp,
			noHelpID,
			1,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			TEView {
				withoutStyle,
				autoWrap,
				acceptChanges,
				dontFreeText,
				cTyping,
				unlimited,
				{	/* array: 4 elements */
					/* [1] */
					3,
					/* [2] */
					3,
					/* [3] */
					3,
					/* [4] */
					3
				},
				justSystem,
				27392,
				dontPreferOutline
			},
			NoSubviews
		},
		/* [3] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'urlT',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				30,
				/* [2] */
				17
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				564
			},
			sizeVariable,
			sizeVariable,
			shown,
			doesntWantToBeTarget,
			doesntHandleCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			doesntHandleHelp,
			letsSubViewsHandleHelp,
			noHelpID,
			1,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			StaticText {
				mStaticTextHit,
				notHilited,
				notDimmed,
				sizeable,
				{	/* array: 4 elements */
					/* [1] */
					0,
					/* [2] */
					0,
					/* [3] */
					0,
					/* [4] */
					0
				},
				8715,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justSystem,
				1005,
				1
			},
			NoSubviews
		},
		/* [4] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'sta0',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				9,
				/* [2] */
				16
			},
			{	/* array: 2 elements */
				/* [1] */
				19,
				/* [2] */
				577
			},
			sizeVariable,
			sizeVariable,
			shown,
			doesntWantToBeTarget,
			doesntHandleCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			doesntHandleHelp,
			letsSubViewsHandleHelp,
			noHelpID,
			1,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			StaticText {
				mStaticTextHit,
				notHilited,
				notDimmed,
				sizeable,
				{	/* array: 4 elements */
					/* [1] */
					0,
					/* [2] */
					0,
					/* [3] */
					0,
					/* [4] */
					0
				},
				-1,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justSystem,
				1005,
				2
			},
			NoSubviews
		}
	}
};

