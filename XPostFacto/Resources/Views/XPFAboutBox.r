data 'TxSt' (8715, "System 12, plain, { 0, 0, 65535 }") {
	$"0088 000C 0000 0000 FFFF 00"                                                                        /* .ˆ......ÿÿ. */
};

data 'TxSt' (27392, "Application 10, plain") {
	$"009C 000A 0000 0000 0000 0161"                                                                      /* .œ.Â.......a */
};

resource 'View' (1005, "About Box", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 8 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'wind',
			985,
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
				462,
				/* [2] */
				634
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
			4
		},
		/* [2] */
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
				95,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				320
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
		/* [3] */
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
				59,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				34,
				/* [2] */
				313
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
		},
		/* [4] */
		ViewSignatureAndClassname {
			'pict',
			131,
			"TPicture",
			'owcp',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 1 elements */
				/* [1] */
				BehaviorSignatureAndClassname {
					noID,
					"TBehavior",
					Behavior {
						enabled,
						noIdle
					}
				}
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				355
			},
			{	/* array: 2 elements */
				/* [1] */
				134,
				/* [2] */
				259
			},
			sizeVariable,
			sizeVariable,
			shown,
			doesntWantToBeTarget,
			handlesCursor,
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
			Picture {
				mPictureHit,
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
				1008
			},
			NoSubviews
		},
		/* [5] */
		ViewSignatureAndClassname {
			'view',
			491,
			"",
			'view',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				166,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				276,
				/* [2] */
				594
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
			AdornerListSignatureAndClassname {
				NoIdentifier,
				"TAdornerList",
				8,
				AdornerElementSizeShift,
				8,
				{	/* array AdornerElementArray: 2 elements */
					/* [1] */
					AdornFirst,
					AdornerSignatureAndClassname {
						'wgad',
						"TWhiteBackgroundAdorner",
						'wgad',
						freeOnDeletion,
						$""
					},
					/* [2] */
					DrawView,
					AdornerLocalObject {
						DrawAdorner
					}
				}
			},
			emptyUserArea,
			View {

			},
			2
		},
		/* [6] */
		ViewSignatureAndClassname {
			'scrl',
			228,
			"TScroller",
			'scrl',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				2,
				/* [2] */
				2
			},
			{	/* array: 2 elements */
				/* [1] */
				272,
				/* [2] */
				576
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
			Scroller {
				'vcrl',
				noID,
				{	/* array: 2 elements */
					/* [1] */
					270,
					/* [2] */
					576
				},
				{	/* array: 2 elements */
					/* [1] */
					13,
					/* [2] */
					16
				},
				VertConstrain,
				noHorzConstrain,
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
				respondsToFKeys
			},
			1
		},
		/* [7] */
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
				2,
				/* [2] */
				2
			},
			{	/* array: 2 elements */
				/* [1] */
				268,
				/* [2] */
				572
			},
			sizeVariable,
			sizeRelSuperView,
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
		/* [8] */
		ViewSignatureAndClassname {
			'ssbr',
			105,
			"",
			'vcrl',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				1,
				/* [2] */
				578
			},
			{	/* array: 2 elements */
				/* [1] */
				274,
				/* [2] */
				16
			},
			sizeVariable,
			sizeVariable,
			shown,
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
			NoAdorners {

			},
			emptyUserArea,
			ScrollerScrollBar {
				mVScrollBarHit,
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
				v,
				0,
				0,
				0
			},
			NoSubviews
		}
	}
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

