data 'TxSt' (27392, "Application 10, plain") {
	$"009C 000A 0000 0000 0000 0161"                                                                      /* .œ.Â.......a */
};

resource 'STR#' (1020, "Progress Window", purgeable) {
	{	/* array StringArray: 4 elements */
		/* [1] */
		"Description",
		/* [2] */
		"Cancel",
		/* [3] */
		"Status",
		/* [4] */
		"OK"
	}
};

resource 'View' (1020, "Progress Window", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 6 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'wind',
			812,
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
						'okay',
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
				159,
				/* [2] */
				338
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
				-1,
				1
			},
			5
		},
		/* [2] */
		ViewSignatureAndClassname {
			'stat',
			100,
			"",
			'desc',
			notEnabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				19,
				/* [2] */
				29
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
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
				1020,
				1
			},
			NoSubviews
		},
		/* [3] */
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
				121,
				/* [2] */
				148
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				71
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
				2
			},
			NoSubviews
		},
		/* [4] */
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
				76,
				/* [2] */
				29
			},
			{	/* array: 2 elements */
				/* [1] */
				31,
				/* [2] */
				281
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
				1020,
				3
			},
			NoSubviews
		},
		/* [5] */
		ViewSignatureAndClassname {
			'butn',
			207,
			"",
			'okay',
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
				232
			},
			{	/* array: 2 elements */
				/* [1] */
				28,
				/* [2] */
				81
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
				9,
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
				1020,
				4
			},
			NoSubviews
		},
		/* [6] */
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
				47,
				/* [2] */
				27
			},
			{	/* array: 2 elements */
				/* [1] */
				14,
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
