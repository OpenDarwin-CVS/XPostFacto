resource 'STR#' (1200, "VolumeDisplay", purgeable) {
	{	/* array StringArray: 1 elements */
		/* [1] */
		"A volume name"
	}
};

resource 'STR#' (1001, "Grid Window", purgeable) {
	{	/* array StringArray: 12 elements */
		/* [1] */
		"Install from CDÉ",
		/* [2] */
		"Select Install CD",
		/* [3] */
		"Restart NowÉ",
		/* [4] */
		"Mac OS 9",
		/* [5] */
		"Mac OS X",
		/* [6] */
		"Restart in:",
		/* [7] */
		"Helper:",
		/* [8] */
		"Select Volume",
		/* [9] */
		"OptionsÉ",
		/* [10] */
		"NVRAM Options",
		/* [11] */
		"Help",
		/* [12] */
		"XPostFacto"
	}
};

data 'TxSt' (5312, "Application 12, bold") {
	$"019C 000C 0000 0000 0000 0161"                                                                      /* .œ.........a */
};

data 'TxSt' (27392, "Application 10, plain") {
	$"009C 000A 0000 0000 0000 0161"                                                                      /* .œ.Â.......a */
};

resource 'View' (1200, "VolumeDisplay", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 5 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'view',
			550,
			"XPFVolumeDisplay",
			'view',
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
				40,
				/* [2] */
				256
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
			View {

			},
			4
		},
		/* [2] */
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
				4,
				/* [2] */
				40
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				176
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
				5312,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justSystem,
				1200,
				1
			},
			NoSubviews
		},
		/* [3] */
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
				4,
				/* [2] */
				4
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
		/* [4] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'vols',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				22,
				/* [2] */
				40
			},
			{	/* array: 2 elements */
				/* [1] */
				14,
				/* [2] */
				176
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
		/* [5] */
		ViewSignatureAndClassname {
			'icon',
			144,
			"XPFWarningIcon",
			'warn',
			notEnabled,
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
				4,
				/* [2] */
				220
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
		}
	}
};

resource 'View' (1001, "Grid Window", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 33 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'wind',
			4037,
			"XPFGridWindow",
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
				538,
				/* [2] */
				588
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
				closesDocument,
				openWithDocument,
				dontAdaptToScreen,
				dontStagger,
				forceOnScreen,
				center,
				doesntFloat,
				doesntHideOnSuspend,
				generateActivates,
				filler,
				1001,
				12
			},
			3
		},
		/* [2] */
		ViewSignatureAndClassname {
			'view',
			1094,
			"",
			'invw',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				220,
				/* [2] */
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				132,
				/* [2] */
				588
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
			6
		},
		/* [3] */
		ViewSignatureAndClassname {
			'butn',
			147,
			"XPFInstallButton",
			'inst',
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
				102,
				/* [2] */
				410
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				132
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
			Button {
				mButtonHit,
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
				1001,
				1
			},
			NoSubviews
		},
		/* [4] */
		ViewSignatureAndClassname {
			'view',
			453,
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
				40,
				/* [2] */
				44
			},
			{	/* array: 2 elements */
				/* [1] */
				84,
				/* [2] */
				276
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
		/* [5] */
		ViewSignatureAndClassname {
			'scrl',
			154,
			"XPFInstallCDList",
			'bol0',
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
				2,
				/* [2] */
				2
			},
			{	/* array: 2 elements */
				/* [1] */
				80,
				/* [2] */
				256
			},
			sizeRelSuperView,
			sizeRelSuperView,
			shown,
			doesntWantToBeTarget,
			handlesCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			doesntHandleHelp,
			letsSubViewsHandleHelp,
			2000,
			13,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			Scroller {
				'voli',
				noID,
				{	/* array: 2 elements */
					/* [1] */
					0,
					/* [2] */
					0
				},
				{	/* array: 2 elements */
					/* [1] */
					16,
					/* [2] */
					16
				},
				noVertConstrain,
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
		/* [6] */
		ViewSignatureAndClassname {
			'ssbr',
			105,
			"",
			'vol0',
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
				258
			},
			{	/* array: 2 elements */
				/* [1] */
				82,
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
		},
		/* [7] */
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
				16,
				/* [2] */
				125
			},
			{	/* array: 2 elements */
				/* [1] */
				8,
				/* [2] */
				443
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
		/* [8] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'incs',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				8,
				/* [2] */
				9
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				114
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
				1001,
				2
			},
			NoSubviews
		},
		/* [9] */
		ViewSignatureAndClassname {
			'icon',
			109,
			"XPFWarningIcon",
			'inwa',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				40,
				/* [2] */
				341
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
		/* [10] */
		ViewSignatureAndClassname {
			'stat',
			117,
			"XPFInstallWarning",
			'sta6',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				40,
				/* [2] */
				382
			},
			{	/* array: 2 elements */
				/* [1] */
				32,
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
		/* [11] */
		ViewSignatureAndClassname {
			'view',
			1891,
			"",
			'revw',
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
				220,
				/* [2] */
				588
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
			11
		},
		/* [12] */
		ViewSignatureAndClassname {
			'butn',
			147,
			"XPFRestartButton",
			'rest',
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
				190,
				/* [2] */
				410
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				132
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
			Button {
				mButtonHit,
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
				1001,
				3
			},
			NoSubviews
		},
		/* [13] */
		ViewSignatureAndClassname {
			'view',
			462,
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
				48,
				/* [2] */
				44
			},
			{	/* array: 2 elements */
				/* [1] */
				164,
				/* [2] */
				276
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
		/* [14] */
		ViewSignatureAndClassname {
			'scrl',
			163,
			"XPFTargetVolumeList",
			'boli',
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
				2,
				/* [2] */
				2
			},
			{	/* array: 2 elements */
				/* [1] */
				160,
				/* [2] */
				256
			},
			sizeRelSuperView,
			sizeRelSuperView,
			shown,
			doesntWantToBeTarget,
			handlesCursor,
			letsSubViewsHandleCursor,
			noCursorID,
			doesntHandleHelp,
			letsSubViewsHandleHelp,
			2000,
			13,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			Scroller {
				'voli',
				noID,
				{	/* array: 2 elements */
					/* [1] */
					0,
					/* [2] */
					0
				},
				{	/* array: 2 elements */
					/* [1] */
					16,
					/* [2] */
					16
				},
				noVertConstrain,
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
		/* [15] */
		ViewSignatureAndClassname {
			'ssbr',
			105,
			"",
			'voli',
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
				258
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
		},
		/* [16] */
		ViewSignatureAndClassname {
			'radb',
			147,
			"XPFMacOS9Button",
			'mos9',
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
				74,
				/* [2] */
				418
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				126
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
			Radio {
				mRadioHit,
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
				1001,
				4
			},
			NoSubviews
		},
		/* [17] */
		ViewSignatureAndClassname {
			'radb',
			147,
			"XPFMacOSXButton",
			'mosx',
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
				48,
				/* [2] */
				418
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				126
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
			Radio {
				mRadioHit,
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
				1001,
				5
			},
			NoSubviews
		},
		/* [18] */
		ViewSignatureAndClassname {
			'stat',
			135,
			"",
			'rsst',
			notEnabled,
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
				49,
				/* [2] */
				336
			},
			{	/* array: 2 elements */
				/* [1] */
				22,
				/* [2] */
				74
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
				1001,
				6
			},
			NoSubviews
		},
		/* [19] */
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
				107,
				/* [2] */
				418
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				126
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
		/* [20] */
		ViewSignatureAndClassname {
			'stat',
			135,
			"",
			'hlps',
			notEnabled,
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
				108,
				/* [2] */
				337
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				72
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
				1001,
				7
			},
			NoSubviews
		},
		/* [21] */
		ViewSignatureAndClassname {
			'stat',
			135,
			"",
			'voln',
			notEnabled,
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
				16,
				/* [2] */
				7
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				102
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
				1001,
				8
			},
			NoSubviews
		},
		/* [22] */
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
				23,
				/* [2] */
				111
			},
			{	/* array: 2 elements */
				/* [1] */
				10,
				/* [2] */
				457
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
		/* [23] */
		ViewSignatureAndClassname {
			'icon',
			109,
			"XPFWarningIcon",
			'rewa',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				142,
				/* [2] */
				341
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
		/* [24] */
		ViewSignatureAndClassname {
			'stat',
			117,
			"XPFRestartWarning",
			'sta0',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				142,
				/* [2] */
				382
			},
			{	/* array: 2 elements */
				/* [1] */
				32,
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
		/* [25] */
		ViewSignatureAndClassname {
			'view',
			893,
			"",
			'bovw',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				352,
				/* [2] */
				0
			},
			{	/* array: 2 elements */
				/* [1] */
				186,
				/* [2] */
				588
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
			8
		},
		/* [26] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'hlpt',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				40,
				/* [2] */
				376
			},
			{	/* array: 2 elements */
				/* [1] */
				126,
				/* [2] */
				168
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
		/* [27] */
		ViewSignatureAndClassname {
			'stat',
			150,
			"XPFSettingsText",
			'nvrm',
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
				40,
				/* [2] */
				44
			},
			{	/* array: 2 elements */
				/* [1] */
				94,
				/* [2] */
				252
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
		/* [28] */
		ViewSignatureAndClassname {
			'butn',
			131,
			"",
			'opti',
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
				146,
				/* [2] */
				206
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				90
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
			Button {
				1436,
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
				1001,
				9
			},
			NoSubviews
		},
		/* [29] */
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
				16,
				/* [2] */
				126
			},
			{	/* array: 2 elements */
				/* [1] */
				9,
				/* [2] */
				194
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
		/* [30] */
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
				8,
				/* [2] */
				10
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				113
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
				1001,
				10
			},
			NoSubviews
		},
		/* [31] */
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
				8,
				/* [2] */
				352
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				38
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
				1001,
				11
			},
			NoSubviews
		},
		/* [32] */
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
				16,
				/* [2] */
				392
			},
			{	/* array: 2 elements */
				/* [1] */
				8,
				/* [2] */
				176
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
		/* [33] */
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
				20,
				/* [2] */
				331
			},
			{	/* array: 2 elements */
				/* [1] */
				146,
				/* [2] */
				13
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

