data 'TxSt' (27392, "Application 10, plain") {
	$"009C 000A 0000 0000 0000 0161"                                                                      /* .œ.Â.......a */
};

resource 'View' (1020, "Progress Window", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 4 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'wind',
			493,
			"XPFProgressWindow",
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
						noID,
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
				98,
				/* [2] */
				413
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
				1020,
				2
			},
			3
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
				20,
				/* [2] */
				324
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
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
				1020,
				1
			},
			NoSubviews
		},
		/* [3] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'stut',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				50,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				28,
				/* [2] */
				280
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
		/* [4] */
		ViewSignatureAndClassname {
			'prog',
			105,
			"",
			'prog',
			enabled,
			0,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				24,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				14,
				/* [2] */
				280
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
			ProgressIndicator {
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
				0,
				0,
				notDeterminate
			},
			NoSubviews
		}
	}
};

resource 'STR#' (1020, "Progress Window", purgeable) {
	{	/* array StringArray: 2 elements */
		/* [1] */
		"Stop",
		/* [2] */
		"Progress Window"
	}
};

