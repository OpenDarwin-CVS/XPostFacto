data 'TxSt' (19938, "Application 10, plain, { 0, 0, 65535 }") {
	$"0076 000A 0000 0000 FFFF 0161"                                                                      /* .v.Â....ÿÿ.a */
};

data 'TxSt' (27392, "Application 10, plain") {
	$"009C 000A 0000 0000 0000 0161"                                                                      /* .œ.Â.......a */
};

resource 'View' (1010, "Additional Settings", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 50 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'wind',
			4976,
			"XPFSettingsWindow",
			'WIND',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 1 elements */
				/* [1] */
				BehaviorSignatureAndClassname {
					noID,
					"XPFHelpBehavior",
					Behavior {
						enabled,
						noIdle
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
				512,
				/* [2] */
				658
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
				noGrowDocProc,
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
				center,
				doesntFloat,
				doesntHideOnSuspend,
				generateActivates,
				filler,
				1010,
				28
			},
			27
		},
		/* [2] */
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
				20,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				42
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
				justRight,
				1010,
				3
			},
			NoSubviews
		},
		/* [3] */
		ViewSignatureAndClassname {
			'stat',
			138,
			"",
			'exdb',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 1 elements */
				/* [1] */
				BehaviorSignatureAndClassname {
					noID,
					"XPFOpenURLBehavior",
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
				468,
				/* [2] */
				75
			},
			{	/* array: 2 elements */
				/* [1] */
				14,
				/* [2] */
				133
			},
			sizeVariable,
			sizeVariable,
			shown,
			doesntWantToBeTarget,
			doesntHandleCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			handlesHelp,
			letsSubViewsHandleHelp,
			2001,
			10,
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
				19938,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justSystem,
				1010,
				13
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
				204,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				48
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
				justRight,
				1010,
				14
			},
			NoSubviews
		},
		/* [5] */
		ViewSignatureAndClassname {
			'stat',
			138,
			"",
			'exof',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 1 elements */
				/* [1] */
				BehaviorSignatureAndClassname {
					noID,
					"XPFOpenURLBehavior",
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
				142,
				/* [2] */
				388
			},
			{	/* array: 2 elements */
				/* [1] */
				14,
				/* [2] */
				166
			},
			sizeVariable,
			sizeVariable,
			shown,
			doesntWantToBeTarget,
			doesntHandleCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			handlesHelp,
			letsSubViewsHandleHelp,
			2001,
			20,
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
				19938,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justSystem,
				1010,
				18
			},
			NoSubviews
		},
		/* [6] */
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
				20,
				/* [2] */
				292
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				100
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
				justRight,
				1010,
				19
			},
			NoSubviews
		},
		/* [7] */
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
				178,
				/* [2] */
				292
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				31
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
				justRight,
				1010,
				22
			},
			NoSubviews
		},
		/* [8] */
		ViewSignatureAndClassname {
			'sepr',
			66,
			"",
			'sepr',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				23,
				/* [2] */
				64
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				188
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
			Separator {

			},
			NoSubviews
		},
		/* [9] */
		ViewSignatureAndClassname {
			'sepr',
			66,
			"",
			'sep0',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				207,
				/* [2] */
				70
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				182
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
			Separator {

			},
			NoSubviews
		},
		/* [10] */
		ViewSignatureAndClassname {
			'sepr',
			66,
			"",
			'sep2',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				28,
				/* [2] */
				394
			},
			{	/* array: 2 elements */
				/* [1] */
				9,
				/* [2] */
				244
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
			Separator {

			},
			NoSubviews
		},
		/* [11] */
		ViewSignatureAndClassname {
			'sepr',
			66,
			"",
			'sep3',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				186,
				/* [2] */
				325
			},
			{	/* array: 2 elements */
				/* [1] */
				9,
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
			Separator {

			},
			NoSubviews
		},
		/* [12] */
		ViewSignatureAndClassname {
			'sepr',
			66,
			"",
			'sep1',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				32,
				/* [2] */
				264
			},
			{	/* array: 2 elements */
				/* [1] */
				450,
				/* [2] */
				12
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
			Separator {

			},
			NoSubviews
		},
		/* [13] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'sta4',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				112,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				42
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
				justRight,
				1010,
				25
			},
			NoSubviews
		},
		/* [14] */
		ViewSignatureAndClassname {
			'sepr',
			66,
			"",
			'sep5',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				117,
				/* [2] */
				64
			},
			{	/* array: 2 elements */
				/* [1] */
				14,
				/* [2] */
				188
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
			Separator {

			},
			NoSubviews
		},
		/* [15] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'sta5',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				274,
				/* [2] */
				292
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				65
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
				justRight,
				1010,
				27
			},
			NoSubviews
		},
		/* [16] */
		ViewSignatureAndClassname {
			'sepr',
			66,
			"",
			'sep6',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				281,
				/* [2] */
				359
			},
			{	/* array: 2 elements */
				/* [1] */
				11,
				/* [2] */
				279
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
			Separator {

			},
			NoSubviews
		},
		/* [17] */
		ViewSignatureAndClassname {
			'stat',
			115,
			"XPFSettingsText",
			'sta6',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				302,
				/* [2] */
				328
			},
			{	/* array: 2 elements */
				/* [1] */
				123,
				/* [2] */
				274
			},
			sizeVariable,
			sizeVariable,
			shown,
			doesntWantToBeTarget,
			doesntHandleCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			handlesHelp,
			letsSubViewsHandleHelp,
			2001,
			13,
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
				27392,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justSystem,
				-1,
				1
			},
			NoSubviews
		},
		/* [18] */
		ViewSignatureAndClassname {
			'view',
			993,
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
				232,
				/* [2] */
				56
			},
			{	/* array: 2 elements */
				/* [1] */
				226,
				/* [2] */
				160
			},
			sizeVariable,
			sizeVariable,
			shown,
			doesntWantToBeTarget,
			doesntHandleCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			handlesHelp,
			letsSubViewsHandleHelp,
			2001,
			4,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			View {

			},
			9
		},
		/* [19] */
		ViewSignatureAndClassname {
			'chkb',
			97,
			"",
			'debr',
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
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
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
			0,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			CheckBox {
				mCheckBoxHit,
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
				off,
				1010,
				4
			},
			NoSubviews
		},
		/* [20] */
		ViewSignatureAndClassname {
			'chkb',
			97,
			"",
			'depr',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				26,
				/* [2] */
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
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
			0,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			CheckBox {
				mCheckBoxHit,
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
				off,
				1010,
				5
			},
			NoSubviews
		},
		/* [21] */
		ViewSignatureAndClassname {
			'chkb',
			97,
			"",
			'denm',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				52,
				/* [2] */
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
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
			0,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			CheckBox {
				mCheckBoxHit,
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
				off,
				1010,
				6
			},
			NoSubviews
		},
		/* [22] */
		ViewSignatureAndClassname {
			'chkb',
			97,
			"",
			'dekp',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				78,
				/* [2] */
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
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
			0,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			CheckBox {
				mCheckBoxHit,
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
				off,
				1010,
				7
			},
			NoSubviews
		},
		/* [23] */
		ViewSignatureAndClassname {
			'chkb',
			97,
			"",
			'dudd',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				104,
				/* [2] */
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
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
			0,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			CheckBox {
				mCheckBoxHit,
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
				off,
				1010,
				8
			},
			NoSubviews
		},
		/* [24] */
		ViewSignatureAndClassname {
			'chkb',
			97,
			"",
			'dedi',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				130,
				/* [2] */
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
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
			0,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			CheckBox {
				mCheckBoxHit,
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
				off,
				1010,
				9
			},
			NoSubviews
		},
		/* [25] */
		ViewSignatureAndClassname {
			'chkb',
			97,
			"",
			'deda',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				156,
				/* [2] */
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
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
			0,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			CheckBox {
				mCheckBoxHit,
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
				off,
				1010,
				10
			},
			NoSubviews
		},
		/* [26] */
		ViewSignatureAndClassname {
			'chkb',
			97,
			"",
			'deso',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				182,
				/* [2] */
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
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
			0,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			CheckBox {
				mCheckBoxHit,
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
				off,
				1010,
				11
			},
			NoSubviews
		},
		/* [27] */
		ViewSignatureAndClassname {
			'chkb',
			97,
			"",
			'desp',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				208,
				/* [2] */
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
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
			CheckBox {
				mCheckBoxHit,
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
				off,
				1010,
				12
			},
			NoSubviews
		},
		/* [28] */
		ViewSignatureAndClassname {
			'chkb',
			115,
			"XPFHelpTagCheckbox",
			'chkb',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				476,
				/* [2] */
				604
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				34
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
			CheckBox {
				mCheckBoxHit,
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
				off,
				1010,
				30
			},
			NoSubviews
		},
		/* [29] */
		ViewSignatureAndClassname {
			'view',
			169,
			"",
			'vie0',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				48,
				/* [2] */
				56
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
			},
			sizeVariable,
			sizeVariable,
			shown,
			doesntWantToBeTarget,
			doesntHandleCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			handlesHelp,
			letsSubViewsHandleHelp,
			2001,
			3,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			View {

			},
			1
		},
		/* [30] */
		ViewSignatureAndClassname {
			'chkb',
			97,
			"",
			'verm',
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
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
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
			CheckBox {
				mCheckBoxHit,
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
				off,
				1010,
				1
			},
			NoSubviews
		},
		/* [31] */
		ViewSignatureAndClassname {
			'view',
			169,
			"",
			'vie1',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				74,
				/* [2] */
				56
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
			},
			sizeVariable,
			sizeVariable,
			shown,
			doesntWantToBeTarget,
			doesntHandleCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			handlesHelp,
			letsSubViewsHandleHelp,
			2001,
			2,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			View {

			},
			1
		},
		/* [32] */
		ViewSignatureAndClassname {
			'chkb',
			97,
			"",
			'sinm',
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
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
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
			CheckBox {
				mCheckBoxHit,
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
				off,
				1010,
				2
			},
			NoSubviews
		},
		/* [33] */
		ViewSignatureAndClassname {
			'view',
			169,
			"",
			'vie2',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				140,
				/* [2] */
				56
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
			},
			sizeVariable,
			sizeVariable,
			shown,
			doesntWantToBeTarget,
			doesntHandleCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			handlesHelp,
			letsSubViewsHandleHelp,
			2001,
			24,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			View {

			},
			1
		},
		/* [34] */
		ViewSignatureAndClassname {
			'chkb',
			97,
			"",
			'romn',
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
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
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
			CheckBox {
				mCheckBoxHit,
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
				off,
				1010,
				26
			},
			NoSubviews
		},
		/* [35] */
		ViewSignatureAndClassname {
			'view',
			169,
			"",
			'vie3',
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
				56
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
			},
			sizeVariable,
			sizeVariable,
			shown,
			doesntWantToBeTarget,
			doesntHandleCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			handlesHelp,
			letsSubViewsHandleHelp,
			2001,
			25,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			View {

			},
			1
		},
		/* [36] */
		ViewSignatureAndClassname {
			'chkb',
			97,
			"",
			'uprp',
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
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				160
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
			CheckBox {
				mCheckBoxHit,
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
				off,
				1010,
				29
			},
			NoSubviews
		},
		/* [37] */
		ViewSignatureAndClassname {
			'view',
			169,
			"",
			'vie4',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				48,
				/* [2] */
				436
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				166
			},
			sizeVariable,
			sizeVariable,
			shown,
			doesntWantToBeTarget,
			doesntHandleCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			handlesHelp,
			letsSubViewsHandleHelp,
			2001,
			5,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			View {

			},
			1
		},
		/* [38] */
		ViewSignatureAndClassname {
			'chkb',
			97,
			"",
			'auto',
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
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				166
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
			CheckBox {
				mCheckBoxHit,
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
				off,
				1010,
				15
			},
			NoSubviews
		},
		/* [39] */
		ViewSignatureAndClassname {
			'view',
			294,
			"",
			'vie5',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				78,
				/* [2] */
				328
			},
			{	/* array: 2 elements */
				/* [1] */
				22,
				/* [2] */
				274
			},
			sizeVariable,
			sizeVariable,
			shown,
			doesntWantToBeTarget,
			doesntHandleCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			handlesHelp,
			letsSubViewsHandleHelp,
			2001,
			6,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			View {

			},
			2
		},
		/* [40] */
		ViewSignatureAndClassname {
			'popp',
			116,
			"TPopup",
			'inpd',
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
				108
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				166
			},
			sizeFixed,
			sizeFixed,
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
			Popup {
				mPopupHit,
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
				0,
				1,
				0,
				plain,
				justSystem,
				dontUseAddResMenu,
				noID,
				-1,
				1
			},
			NoSubviews
		},
		/* [41] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'intx',
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
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				100
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
				justRight,
				1010,
				16
			},
			NoSubviews
		},
		/* [42] */
		ViewSignatureAndClassname {
			'view',
			294,
			"",
			'vie6',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				110,
				/* [2] */
				327
			},
			{	/* array: 2 elements */
				/* [1] */
				22,
				/* [2] */
				274
			},
			sizeVariable,
			sizeVariable,
			shown,
			doesntWantToBeTarget,
			doesntHandleCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			handlesHelp,
			letsSubViewsHandleHelp,
			2001,
			7,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			View {

			},
			2
		},
		/* [43] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'outx',
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
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				100
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
				justRight,
				1010,
				17
			},
			NoSubviews
		},
		/* [44] */
		ViewSignatureAndClassname {
			'popp',
			116,
			"TPopup",
			'outd',
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
				108
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				166
			},
			sizeFixed,
			sizeFixed,
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
			Popup {
				mPopupHit,
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
				0,
				1,
				0,
				plain,
				justSystem,
				dontUseAddResMenu,
				noID,
				-1,
				1
			},
			NoSubviews
		},
		/* [45] */
		ViewSignatureAndClassname {
			'view',
			169,
			"",
			'vie7',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				206,
				/* [2] */
				436
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				166
			},
			sizeVariable,
			sizeVariable,
			shown,
			doesntWantToBeTarget,
			doesntHandleCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			handlesHelp,
			letsSubViewsHandleHelp,
			2001,
			8,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			View {

			},
			1
		},
		/* [46] */
		ViewSignatureAndClassname {
			'chkb',
			97,
			"",
			'cach',
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
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				166
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
			CheckBox {
				mCheckBoxHit,
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
				off,
				1010,
				21
			},
			NoSubviews
		},
		/* [47] */
		ViewSignatureAndClassname {
			'view',
			471,
			"",
			'vie8',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				236,
				/* [2] */
				360
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				242
			},
			sizeVariable,
			sizeVariable,
			shown,
			doesntWantToBeTarget,
			doesntHandleCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			handlesHelp,
			letsSubViewsHandleHelp,
			2001,
			9,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			View {

			},
			3
		},
		/* [48] */
		ViewSignatureAndClassname {
			'stat',
			176,
			"",
			'thrt',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				0,
				/* [2] */
				216
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				26
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
						'bfad',
						"TBorderFrameAdorner",
						'bfad',
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
				justCenter,
				1010,
				20
			},
			NoSubviews
		},
		/* [49] */
		ViewSignatureAndClassname {
			'slid',
			111,
			"",
			'thro',
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
				76
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				132
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
			Slider {
				mControlHit,
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
				0,
				rightJustify,
				0,
				24,
				0,
				0
			},
			NoSubviews
		},
		/* [50] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'thtx',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				0,
				/* [2] */
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				70
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
				justRight,
				1010,
				20
			},
			NoSubviews
		}
	}
};

resource 'STR#' (1010, "Additional Settings", purgeable) {
	{	/* array StringArray: 30 elements */
		/* [1] */
		"Verbose",
		/* [2] */
		"Single User",
		/* [3] */
		"Mode",
		/* [4] */
		"Early Breakpoint",
		/* [5] */
		"Debug Print",
		/* [6] */
		"Enable NMI",
		/* [7] */
		"Enable kprintf",
		/* [8] */
		"Use DDB",
		/* [9] */
		"Extra Diagnostics",
		/* [10] */
		"Use Debugger ARP",
		/* [11] */
		"Use Old GDB",
		/* [12] */
		"Show Panic Text",
		/* [13] */
		"Explain Debugging Flags",
		/* [14] */
		"Debug",
		/* [15] */
		"auto-boot?",
		/* [16] */
		"input-device:",
		/* [17] */
		"output-device:",
		/* [18] */
		"Explain Open Firmware Settings",
		/* [19] */
		"Open Firmware",
		/* [20] */
		"Throttle:",
		/* [21] */
		"Enable L2/L3 Cache",
		/* [22] */
		"CPU",
		/* [23] */
		" ",
		/* [24] */
		"Help",
		/* [25] */
		"Video",
		/* [26] */
		"Use old NDRVs",
		/* [27] */
		"Summary",
		/* [28] */
		"Additional Settings",
		/* [29] */
		"Use PatchedRagePro",
		/* [30] */
		"?"
	}
};

