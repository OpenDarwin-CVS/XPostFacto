resource 'STR#' (1030, "Restart", purgeable) {
	{	/* array StringArray: 6 elements */
		/* [1] */
		"Are you sure you want to change your startup settings?",
		/* [2] */
		"Your computer will start up using $OS$ on $VOLUME$.",
		/* [3] */
		"Change",
		/* [4] */
		"Cancel",
		/* [5] */
		"Don't Change",
		/* [6] */
		"Confirm Changes"
	}
};

resource 'STR#' (1032, "Install", purgeable) {
	{	/* array StringArray: 6 elements */
		/* [1] */
		"Are you sure you want to install Mac OS X now?",
		/* [2] */
		"Your computer will restart from $VOLUME$, and install $OS$ to $TARGET$.",
		/* [3] */
		"Install",
		/* [4] */
		"Cancel",
		/* [5] */
		"Quit",
		/* [6] */
		"Confirm Install"
	}
};

resource 'STR#' (1033, "Restart Now", purgeable) {
	{	/* array StringArray: 3 elements */
		/* [1] */
		"Are you sure you want to restart your computer now?",
		/* [2] */
		"Restart",
		/* [3] */
		"Confirm Restart"
	}
};

resource 'STR#' (1034, "Install Now", purgeable) {
	{	/* array StringArray: 4 elements */
		/* [1] */
		"Are you sure you want to install Mac OS X now?",
		/* [2] */
		"Your computer will restart from $VOLUME$, and install $OS$ to $TARGET$.",
		/* [3] */
		"Install",
		/* [4] */
		"Confirm Install"
	}
};

data 'TxSt' (27392, "Application 10, plain") {
	$"0081 000A 0000 0000 0000 0161"                                                                      /* .Å.¬.......a */
};

resource 'View' (1030, "Restart", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 7 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'wind',
			892,
			"",
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
						'chan',
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
				150,
				/* [2] */
				518
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
				1030,
				6
			},
			6
		},
		/* [2] */
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
		/* [3] */
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
				21,
				/* [2] */
				117
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				373
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
				1030,
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
				47,
				/* [2] */
				116
			},
			{	/* array: 2 elements */
				/* [1] */
				31,
				/* [2] */
				376
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
				1030,
				2
			},
			NoSubviews
		},
		/* [5] */
		ViewSignatureAndClassname {
			'butn',
			207,
			"",
			'chan',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				93,
				/* [2] */
				400
			},
			{	/* array: 2 elements */
				/* [1] */
				28,
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
				1030,
				3
			},
			NoSubviews
		},
		/* [6] */
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
				97,
				/* [2] */
				302
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
				1030,
				4
			},
			NoSubviews
		},
		/* [7] */
		ViewSignatureAndClassname {
			'butn',
			96,
			"",
			'dont',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				97,
				/* [2] */
				115
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				128
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
				1030,
				5
			},
			NoSubviews
		}
	}
};

resource 'View' (1032, "Install", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 7 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'wind',
			892,
			"",
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
						'chan',
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
				150,
				/* [2] */
				518
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
				1032,
				6
			},
			6
		},
		/* [2] */
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
		/* [3] */
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
				21,
				/* [2] */
				117
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				373
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
				1032,
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
				47,
				/* [2] */
				116
			},
			{	/* array: 2 elements */
				/* [1] */
				31,
				/* [2] */
				376
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
				1032,
				2
			},
			NoSubviews
		},
		/* [5] */
		ViewSignatureAndClassname {
			'butn',
			207,
			"",
			'chan',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				93,
				/* [2] */
				400
			},
			{	/* array: 2 elements */
				/* [1] */
				28,
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
				1032,
				3
			},
			NoSubviews
		},
		/* [6] */
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
				97,
				/* [2] */
				302
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
				1032,
				4
			},
			NoSubviews
		},
		/* [7] */
		ViewSignatureAndClassname {
			'butn',
			96,
			"",
			'dont',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				97,
				/* [2] */
				115
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				95
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
				1032,
				5
			},
			NoSubviews
		}
	}
};

resource 'View' (1033, "Restart Now", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 6 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'wind',
			790,
			"",
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
						'chan',
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
				150,
				/* [2] */
				518
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
				1033,
				3
			},
			5
		},
		/* [2] */
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
		/* [3] */
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
				21,
				/* [2] */
				117
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				373
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
				1033,
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
				47,
				/* [2] */
				116
			},
			{	/* array: 2 elements */
				/* [1] */
				31,
				/* [2] */
				376
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
				1030,
				2
			},
			NoSubviews
		},
		/* [5] */
		ViewSignatureAndClassname {
			'butn',
			207,
			"",
			'chan',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				93,
				/* [2] */
				400
			},
			{	/* array: 2 elements */
				/* [1] */
				28,
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
				1033,
				2
			},
			NoSubviews
		},
		/* [6] */
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
				97,
				/* [2] */
				302
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
				1030,
				4
			},
			NoSubviews
		}
	}
};

resource 'View' (1034, "Install Now", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 6 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'wind',
			790,
			"",
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
						'chan',
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
				150,
				/* [2] */
				518
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
				1034,
				4
			},
			5
		},
		/* [2] */
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
		/* [3] */
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
				21,
				/* [2] */
				117
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				373
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
				1034,
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
				47,
				/* [2] */
				116
			},
			{	/* array: 2 elements */
				/* [1] */
				31,
				/* [2] */
				376
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
				1034,
				2
			},
			NoSubviews
		},
		/* [5] */
		ViewSignatureAndClassname {
			'butn',
			207,
			"",
			'chan',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				93,
				/* [2] */
				400
			},
			{	/* array: 2 elements */
				/* [1] */
				28,
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
				1034,
				3
			},
			NoSubviews
		},
		/* [6] */
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
				97,
				/* [2] */
				302
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
				1030,
				4
			},
			NoSubviews
		}
	}
};

