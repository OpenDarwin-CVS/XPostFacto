data 'TxSt' (17832, "Application 10, plain") {
	$"00A7 000A 0000 0000 0000 0161"                                                                      /* .§.Â.......a */
};

data 'TxSt' (8715, "Application 9, bold") {
	$"0100 0009 0000 0000 0000 0161"                                                                      /* ...Æ.......a */
};

resource 'STR#' (1040, "Update Window", purgeable) {
	{	/* array StringArray: 7 elements */
		/* [1] */
		"Cancel",
		/* [2] */
		"Update",
		/* [3] */
		"Installed",
		/* [4] */
		"Available",
		/* [5] */
		"Action",
		/* [6] */
		"The following updates are available for $VOLUME$.",
		/* [7] */
		"XPostFacto Update"
	}
};

resource 'STR#' (1041, "Update List", purgeable) {
	{	/* array StringArray: 4 elements */
		/* [1] */
		"Item",
		/* [2] */
		"Installed",
		/* [3] */
		"Avail",
		/* [4] */
		"Action"
	}
};

resource 'View' (1040, "Update Window", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 11 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'wind',
			1440,
			"XPFUpdateWindow",
			'WIND',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 1 elements */
				/* [1] */
				BehaviorSignatureAndClassname {
					'dlgb',
					"",
					DialogBehavior {
						enabled,
						noIdle,
						modal,
						'upda',
						'canc'
					}
				}
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
				320,
				/* [2] */
				544
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
				movableDBoxProc,
				noID,
				noGoAwayBox,
				notResizable,
				ignoreFirstClick,
				freeOnClosing,
				disposeOnFree,
				doesntCloseDocument,
				dontOpenWithDocument,
				dontAdaptToScreen,
				dontStagger,
				forceOnScreen,
				center,
				doesntFloat,
				doesntHideOnSuspend,
				generateActivates,
				filler,
				1040,
				7
			},
			8
		},
		/* [2] */
		ViewSignatureAndClassname {
			'butn',
			96,
			"",
			'canc',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				276,
				/* [2] */
				316
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				76
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
			Button {
				mDismiss,
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
				1040,
				1
			},
			NoSubviews
		},
		/* [3] */
		ViewSignatureAndClassname {
			'butn',
			207,
			"",
			'upda',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				272,
				/* [2] */
				416
			},
			{	/* array: 2 elements */
				/* [1] */
				28,
				/* [2] */
				108
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
			DrawingEnvironmentClassIDAndClassname {
				10,
				"TDrawingEnvironment",
				{3, 3},
				8,
				$"FFFF FFFF FFFF FFFF",
				{	/* array: 3 elements */
					/* [1] */
					0,
					/* [2] */
					0,
					/* [3] */
					0
				},
				{	/* array: 3 elements */
					/* [1] */
					65535,
					/* [2] */
					65535,
					/* [3] */
					65535
				}
			},
			AdornerListSignatureAndClassname {
				NoIdentifier,
				"TAdornerList",
				8,
				AdornerElementSizeShift,
				8,
				{	/* array AdornerElementArray: 2 elements */
					/* [1] */
					DrawView,
					AdornerLocalObject {
						DrawAdorner
					},
					/* [2] */
					AdornAfter,
					AdornerSignatureAndClassname {
						'rrct',
						"",
						'rrct',
						freeOnDeletion,
						$""
					}
				}
			},
			emptyUserArea,
			Button {
				mDismiss,
				notHilited,
				notDimmed,
				sizeable,
				{	/* array: 4 elements */
					/* [1] */
					4,
					/* [2] */
					4,
					/* [3] */
					4,
					/* [4] */
					4
				},
				-1,
				dontPreferOutline,
				1040,
				2
			},
			NoSubviews
		},
		/* [4] */
		ViewSignatureAndClassname {
			'icon',
			100,
			"TIcon",
			'icon',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				21,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				64,
				/* [2] */
				64
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
			Icon {
				mIconHit,
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
				preferColor,
				128
			},
			NoSubviews
		},
		/* [5] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'stat',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				71,
				/* [2] */
				300
			},
			{	/* array: 2 elements */
				/* [1] */
				14,
				/* [2] */
				50
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
				17832,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justSystem,
				1040,
				3
			},
			NoSubviews
		},
		/* [6] */
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
				71,
				/* [2] */
				371
			},
			{	/* array: 2 elements */
				/* [1] */
				14,
				/* [2] */
				59
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
				17832,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justSystem,
				1040,
				4
			},
			NoSubviews
		},
		/* [7] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'sta1',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				71,
				/* [2] */
				440
			},
			{	/* array: 2 elements */
				/* [1] */
				14,
				/* [2] */
				53
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
				17832,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justSystem,
				1040,
				5
			},
			NoSubviews
		},
		/* [8] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'sta2',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				21,
				/* [2] */
				102
			},
			{	/* array: 2 elements */
				/* [1] */
				36,
				/* [2] */
				415
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
				1040,
				6
			},
			NoSubviews
		},
		/* [9] */
		ViewSignatureAndClassname {
			'view',
			417,
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
				86,
				/* [2] */
				102
			},
			{	/* array: 2 elements */
				/* [1] */
				166,
				/* [2] */
				419
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
				{	/* array AdornerElementArray: 3 elements */
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
					},
					/* [3] */
					AdornLast,
					AdornerSignatureAndClassname {
						'bfad',
						"TBorderFrameAdorner",
						'bfad',
						freeOnDeletion,
						$""
					}
				}
			},
			emptyUserArea,
			View {

			},
			2
		},
		/* [10] */
		ViewSignatureAndClassname {
			'scrl',
			118,
			"TScroller",
			'list',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				3,
				/* [2] */
				4
			},
			{	/* array: 2 elements */
				/* [1] */
				160,
				/* [2] */
				397
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
				'vist',
				noID,
				{	/* array: 2 elements */
					/* [1] */
					0,
					/* [2] */
					0
				},
				{	/* array: 2 elements */
					/* [1] */
					20,
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
			NoSubviews
		},
		/* [11] */
		ViewSignatureAndClassname {
			'ssbr',
			105,
			"",
			'vist',
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
				401
			},
			{	/* array: 2 elements */
				/* [1] */
				162,
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

resource 'View' (1041, "Update List", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 5 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'view',
			490,
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
				0,
				/* [2] */
				-1
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				402
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
			View {

			},
			4
		},
		/* [2] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'item',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				2,
				/* [2] */
				3
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				185
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
				130,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justSystem,
				1041,
				1
			},
			NoSubviews
		},
		/* [3] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'inst',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				2,
				/* [2] */
				194
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				61
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
				130,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justSystem,
				1041,
				2
			},
			NoSubviews
		},
		/* [4] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'avai',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				2,
				/* [2] */
				264
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				69
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
				130,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justSystem,
				1041,
				3
			},
			NoSubviews
		},
		/* [5] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'acti',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				2,
				/* [2] */
				337
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				61
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
				130,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justSystem,
				1041,
				4
			},
			NoSubviews
		}
	}
};

