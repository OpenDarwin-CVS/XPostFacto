data 'TxSt' (19938, "Application 10, plain, { 0, 0, 65535 }") {
	$"0076 000A 0000 0000 FFFF 0161"                                                                      /* .v.Â....ÿÿ.a */
};

data 'TxSt' (27392, "Application 10, plain") {
	$"009C 000A 0000 0000 0000 0161"                                                                      /* .œ.Â.......a */
};

resource 'STR#' (1010, "Additional Settings", purgeable) {
	{	/* array StringArray: 26 elements */
		/* [1] */
		"Mode",
		/* [2] */
		"Verbose",
		/* [3] */
		"Single User",
		/* [4] */
		"Debug",
		/* [5] */
		"Early Breakpoint",
		/* [6] */
		"Debug Print",
		/* [7] */
		"Enable NMI",
		/* [8] */
		"Enable kprintf",
		/* [9] */
		"Use DDB",
		/* [10] */
		"Extra Diagnostics",
		/* [11] */
		"Use Debugger ARP",
		/* [12] */
		"Use Old GDB",
		/* [13] */
		"Show Panic Text",
		/* [14] */
		"Explain Debugging Flags",
		/* [15] */
		"Open Firmware",
		/* [16] */
		"auto-boot?",
		/* [17] */
		"Missing Popup",
		/* [18] */
		"input-device:",
		/* [19] */
		"output-device:",
		/* [20] */
		"Explain Open Firmware Settings",
		/* [21] */
		"CPU",
		/* [22] */
		"Throttle:",
		/* [23] */
		"Enable L2/L3 Cache",
		/* [24] */
		"Help",
		/* [25] */
		"",
		/* [26] */
		"Additional Settings"
	}
};

resource 'View' (1010, "Additional Settings", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 29 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'wind',
			3859,
			"XPFSettingsWindow",
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
				402,
				/* [2] */
				535
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
				26
			},
			5
		},
		/* [2] */
		ViewSignatureAndClassname {
			'clsp',
			396,
			"",
			'clsp',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 1 elements */
				/* [1] */
				BehaviorSignatureAndClassname {
					'club',
					"",
					ClusterBehavior {
						enabled,
						noIdle
					}
				}
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				15,
				/* [2] */
				23
			},
			{	/* array: 2 elements */
				/* [1] */
				71,
				/* [2] */
				193
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
			PrimaryCluster {
				mClusterHit,
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
				1010,
				1,
				noID
			},
			2
		},
		/* [3] */
		ViewSignatureAndClassname {
			'chkb',
			132,
			"",
			'verm',
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
				23,
				/* [2] */
				15
			},
			{	/* array: 2 elements */
				/* [1] */
				19,
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
		/* [4] */
		ViewSignatureAndClassname {
			'chkb',
			132,
			"",
			'sinm',
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
				45,
				/* [2] */
				15
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				137
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
				3
			},
			NoSubviews
		},
		/* [5] */
		ViewSignatureAndClassname {
			'clsp',
			1261,
			"",
			'debg',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 2 elements */
				/* [1] */
				BehaviorSignatureAndClassname {
					'club',
					"",
					ClusterBehavior {
						enabled,
						noIdle
					}
				},
				/* [2] */
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
				23
			},
			{	/* array: 2 elements */
				/* [1] */
				282,
				/* [2] */
				194
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
			PrimaryCluster {
				mClusterHit,
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
				1010,
				4,
				noID
			},
			10
		},
		/* [6] */
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
				25,
				/* [2] */
				13
			},
			{	/* array: 2 elements */
				/* [1] */
				24,
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
				5
			},
			NoSubviews
		},
		/* [7] */
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
				50,
				/* [2] */
				13
			},
			{	/* array: 2 elements */
				/* [1] */
				24,
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
				6
			},
			NoSubviews
		},
		/* [8] */
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
				75,
				/* [2] */
				13
			},
			{	/* array: 2 elements */
				/* [1] */
				24,
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
				7
			},
			NoSubviews
		},
		/* [9] */
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
				100,
				/* [2] */
				13
			},
			{	/* array: 2 elements */
				/* [1] */
				24,
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
				8
			},
			NoSubviews
		},
		/* [10] */
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
				125,
				/* [2] */
				13
			},
			{	/* array: 2 elements */
				/* [1] */
				24,
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
				9
			},
			NoSubviews
		},
		/* [11] */
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
				150,
				/* [2] */
				13
			},
			{	/* array: 2 elements */
				/* [1] */
				24,
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
				10
			},
			NoSubviews
		},
		/* [12] */
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
				175,
				/* [2] */
				13
			},
			{	/* array: 2 elements */
				/* [1] */
				24,
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
				11
			},
			NoSubviews
		},
		/* [13] */
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
				200,
				/* [2] */
				13
			},
			{	/* array: 2 elements */
				/* [1] */
				24,
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
		/* [14] */
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
				225,
				/* [2] */
				13
			},
			{	/* array: 2 elements */
				/* [1] */
				24,
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
				13
			},
			NoSubviews
		},
		/* [15] */
		ViewSignatureAndClassname {
			'stat',
			173,
			"",
			'exdb',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 2 elements */
				/* [1] */
				BehaviorSignatureAndClassname {
					noID,
					"XPFOpenURLBehavior",
					Behavior {
						enabled,
						noIdle
					}
				},
				/* [2] */
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
				257,
				/* [2] */
				32
			},
			{	/* array: 2 elements */
				/* [1] */
				17,
				/* [2] */
				133
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
				19938,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justSystem,
				1010,
				14
			},
			NoSubviews
		},
		/* [16] */
		ViewSignatureAndClassname {
			'clsp',
			1033,
			"",
			'cls1',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 1 elements */
				/* [1] */
				BehaviorSignatureAndClassname {
					'club',
					"",
					ClusterBehavior {
						enabled,
						noIdle
					}
				}
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				15,
				/* [2] */
				243
			},
			{	/* array: 2 elements */
				/* [1] */
				127,
				/* [2] */
				272
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
			PrimaryCluster {
				mClusterHit,
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
				1010,
				15,
				noID
			},
			6
		},
		/* [17] */
		ViewSignatureAndClassname {
			'chkb',
			132,
			"",
			'auto',
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
				26,
				/* [2] */
				114
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				123
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
				16
			},
			NoSubviews
		},
		/* [18] */
		ViewSignatureAndClassname {
			'popp',
			151,
			"TPopup",
			'inpd',
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
				114
			},
			{	/* array: 2 elements */
				/* [1] */
				19,
				/* [2] */
				144
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
				-1,
				dontPreferOutline,
				0,
				1,
				0,
				plain,
				justSystem,
				dontUseAddResMenu,
				noID,
				1010,
				17
			},
			NoSubviews
		},
		/* [19] */
		ViewSignatureAndClassname {
			'popp',
			151,
			"TPopup",
			'outd',
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
				77,
				/* [2] */
				115
			},
			{	/* array: 2 elements */
				/* [1] */
				19,
				/* [2] */
				143
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
				-1,
				dontPreferOutline,
				0,
				1,
				0,
				plain,
				justSystem,
				dontUseAddResMenu,
				noID,
				1010,
				17
			},
			NoSubviews
		},
		/* [20] */
		ViewSignatureAndClassname {
			'stat',
			135,
			"",
			'intx',
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
				49,
				/* [2] */
				3
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				106
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
				-1,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justRight,
				1010,
				18
			},
			NoSubviews
		},
		/* [21] */
		ViewSignatureAndClassname {
			'stat',
			135,
			"",
			'outx',
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
				78,
				/* [2] */
				1
			},
			{	/* array: 2 elements */
				/* [1] */
				17,
				/* [2] */
				109
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
		/* [22] */
		ViewSignatureAndClassname {
			'stat',
			173,
			"",
			'exof',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 2 elements */
				/* [1] */
				BehaviorSignatureAndClassname {
					noID,
					"XPFOpenURLBehavior",
					Behavior {
						enabled,
						noIdle
					}
				},
				/* [2] */
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
				105,
				/* [2] */
				62
			},
			{	/* array: 2 elements */
				/* [1] */
				17,
				/* [2] */
				166
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
				19938,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justSystem,
				1010,
				20
			},
			NoSubviews
		},
		/* [23] */
		ViewSignatureAndClassname {
			'clsp',
			768,
			"",
			'cls2',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 1 elements */
				/* [1] */
				BehaviorSignatureAndClassname {
					'club',
					"",
					ClusterBehavior {
						enabled,
						noIdle
					}
				}
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				156,
				/* [2] */
				244
			},
			{	/* array: 2 elements */
				/* [1] */
				83,
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
			doesntHandleHelp,
			letsSubViewsHandleHelp,
			noHelpID,
			1,
			NoDrawingEnvironment {

			},
			NoAdorners {

			},
			emptyUserArea,
			PrimaryCluster {
				mClusterHit,
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
				1010,
				21,
				noID
			},
			4
		},
		/* [24] */
		ViewSignatureAndClassname {
			'slid',
			146,
			"",
			'thro',
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
				50,
				/* [2] */
				92
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				141
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
		/* [25] */
		ViewSignatureAndClassname {
			'stat',
			135,
			"",
			'thtx',
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
				50,
				/* [2] */
				10
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				70
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
		/* [26] */
		ViewSignatureAndClassname {
			'chkb',
			132,
			"",
			'cach',
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
				23,
				/* [2] */
				91
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				157
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
				23
			},
			NoSubviews
		},
		/* [27] */
		ViewSignatureAndClassname {
			'stat',
			211,
			"",
			'thrt',
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
				50,
				/* [2] */
				237
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				25
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
				22
			},
			NoSubviews
		},
		/* [28] */
		ViewSignatureAndClassname {
			'clsp',
			226,
			"",
			'cls3',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 1 elements */
				/* [1] */
				BehaviorSignatureAndClassname {
					'club',
					"",
					ClusterBehavior {
						enabled,
						noIdle
					}
				}
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				249,
				/* [2] */
				245
			},
			{	/* array: 2 elements */
				/* [1] */
				136,
				/* [2] */
				263
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
			PrimaryCluster {
				mClusterHit,
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
				1010,
				24,
				noID
			},
			1
		},
		/* [29] */
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
				23,
				/* [2] */
				13
			},
			{	/* array: 2 elements */
				/* [1] */
				102,
				/* [2] */
				239
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
				1010,
				25
			},
			NoSubviews
		}
	}
};

