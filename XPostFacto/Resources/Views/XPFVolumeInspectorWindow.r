data 'TxSt' (5591, "Application 10, plain") {
	$"0039 000A 0000 0000 0000 0161"                                                                      /* .9.Â.......a */
};

resource 'View' (1110, "Volume Inspector", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 19 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'wind',
			2156,
			"XPFVolumeInspectorWindow",
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
				452,
				/* [2] */
				420
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
				1110,
				1
			},
			18
		},
		/* [2] */
		ViewSignatureAndClassname {
			'popp',
			133,
			"XPFVolumeInspectorPopup",
			'volu',
			enabled,
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
				20,
				/* [2] */
				200
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
		/* [3] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'voln',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				52,
				/* [2] */
				190
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				200
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
				1110,
				2
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
				52,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				162
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
				justRight,
				1110,
				3
			},
			NoSubviews
		},
		/* [5] */
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
				76,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				162
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
				justRight,
				1110,
				4
			},
			NoSubviews
		},
		/* [6] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'ofnm',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				76,
				/* [2] */
				190
			},
			{	/* array: 2 elements */
				/* [1] */
				32,
				/* [2] */
				200
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
				1110,
				5
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
				116,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				162
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
				justRight,
				1110,
				6
			},
			NoSubviews
		},
		/* [8] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'mosv',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				116,
				/* [2] */
				190
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				200
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
				1110,
				7
			},
			NoSubviews
		},
		/* [9] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'sta3',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				140,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				162
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
				justRight,
				1110,
				8
			},
			NoSubviews
		},
		/* [10] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'boov',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				140,
				/* [2] */
				190
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				200
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
				1110,
				9
			},
			NoSubviews
		},
		/* [11] */
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
				164,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				162
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
				justRight,
				1110,
				10
			},
			NoSubviews
		},
		/* [12] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'mos9',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				164,
				/* [2] */
				190
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				200
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
				1110,
				11
			},
			NoSubviews
		},
		/* [13] */
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
				188,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				162
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
				justRight,
				1110,
				12
			},
			NoSubviews
		},
		/* [14] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'blsf',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				188,
				/* [2] */
				190
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				200
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
				1110,
				13
			},
			NoSubviews
		},
		/* [15] */
		ViewSignatureAndClassname {
			'stat',
			129,
			"",
			'hlps',
			notEnabled,
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
				216,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				162
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
				justRight,
				1110,
				16
			},
			NoSubviews
		},
		/* [16] */
		ViewSignatureAndClassname {
			'popp',
			159,
			"XPFHelperPopup",
			'hlpd',
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
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				214,
				/* [2] */
				190
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				200
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
				130,
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
		/* [17] */
		ViewSignatureAndClassname {
			'icon',
			109,
			"XPFWarningIcon",
			'warn',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				246,
				/* [2] */
				150
			},
			{	/* array: 2 elements */
				/* [1] */
				32,
				/* [2] */
				32
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
				0
			},
			NoSubviews
		},
		/* [18] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'wart',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				246,
				/* [2] */
				190
			},
			{	/* array: 2 elements */
				/* [1] */
				184,
				/* [2] */
				200
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
				1110,
				17
			},
			NoSubviews
		},
		/* [19] */
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
				32,
				/* [2] */
				228
			},
			{	/* array: 2 elements */
				/* [1] */
				8,
				/* [2] */
				174
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
		}
	}
};

resource 'STR#' (1110, "Volume Inspector", purgeable) {
	{	/* array StringArray: 17 elements */
		/* [1] */
		"Volume Inspector",
		/* [2] */
		"Volume name",
		/* [3] */
		"Volume Name:",
		/* [4] */
		"Open Firmware Name:",
		/* [5] */
		"Open Firmware Name",
		/* [6] */
		"Mac OS X Version:",
		/* [7] */
		"Mac OS X Version",
		/* [8] */
		"BootX Version:",
		/* [9] */
		"BootX Version",
		/* [10] */
		"Mac OS 9 System Folder ID:",
		/* [11] */
		"Mac OS 9 System Folder",
		/* [12] */
		"Blessed Folder ID:",
		/* [13] */
		"Blessed Folder ID",
		/* [14] */
		"Help:",
		/* [15] */
		" ",
		/* [16] */
		"Helper:",
		/* [17] */
		"Warning Text"
	}
};

