data 'TxSt' (16903, "System 10, plain") {
	$"007F 000A 0000 0000 0000 00"                                                                        /* ...Â....... */
};

data 'TxSt' (17832, "System 12, bold") {
	$"0182 000C 0000 0000 0000 00"                                                                        /* .‚......... */
};

data 'TxSt' (27392, "Application 10, plain") {
	$"009C 000A 0000 0000 0000 0161"                                                                      /* .œ.Â.......a */
};

resource 'STR#' (1001, "Grid Window", purgeable) {
	{	/* array StringArray: 14 elements */
		/* [1] */
		"Install from CD to Target Volume:",
		/* [2] */
		"Install",
		/* [3] */
		"Use CD:",
		/* [4] */
		"Update Target Volume:",
		/* [5] */
		"Update",
		/* [6] */
		"Select Target Volume:",
		/* [7] */
		"Get InfoÉ",
		/* [8] */
		"Help",
		/* [9] */
		"NVRAM Settings:",
		/* [10] */
		"OptionsÉ",
		/* [11] */
		"Restart from Target Volume:",
		/* [12] */
		"Helper:",
		/* [13] */
		"Restart",
		/* [14] */
		"XPostFacto"
	}
};

resource 'STR#' (1200, "VolumeDisplay", purgeable) {
	{	/* array StringArray: 1 elements */
		/* [1] */
		"A volume name"
	}
};

resource 'View' (1001, "Grid Window", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 25 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'wind',
			3554,
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
				560,
				/* [2] */
				760
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
				14
			},
			7
		},
		/* [2] */
		ViewSignatureAndClassname {
			'clsp',
			582,
			"",
			'incl',
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
				88,
				/* [2] */
				382
			},
			{	/* array: 2 elements */
				/* [1] */
				57,
				/* [2] */
				345
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
				1001,
				1,
				noID
			},
			3
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
				22,
				/* [2] */
				241
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				93
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
				2
			},
			NoSubviews
		},
		/* [4] */
		ViewSignatureAndClassname {
			'popp',
			162,
			"XPFInstallCDPopup",
			'incd',
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
				88
			},
			{	/* array: 2 elements */
				/* [1] */
				21,
				/* [2] */
				138
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
				-1,
				1
			},
			NoSubviews
		},
		/* [5] */
		ViewSignatureAndClassname {
			'stat',
			135,
			"",
			'incs',
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
				24,
				/* [2] */
				9
			},
			{	/* array: 2 elements */
				/* [1] */
				19,
				/* [2] */
				67
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
				1001,
				3
			},
			NoSubviews
		},
		/* [6] */
		ViewSignatureAndClassname {
			'clsp',
			715,
			"",
			'upsl',
			notEnabled,
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
				163,
				/* [2] */
				383
			},
			{	/* array: 2 elements */
				/* [1] */
				235,
				/* [2] */
				348
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
				1001,
				4,
				noID
			},
			2
		},
		/* [7] */
		ViewSignatureAndClassname {
			'butn',
			131,
			"",
			'updb',
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
				203,
				/* [2] */
				259
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				73
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
				5
			},
			NoSubviews
		},
		/* [8] */
		ViewSignatureAndClassname {
			'view',
			452,
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
				20,
				/* [2] */
				8
			},
			{	/* array: 2 elements */
				/* [1] */
				169,
				/* [2] */
				327
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
		/* [9] */
		ViewSignatureAndClassname {
			'scrl',
			153,
			"TScroller",
			'selu',
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
				5,
				/* [2] */
				5
			},
			{	/* array: 2 elements */
				/* [1] */
				160,
				/* [2] */
				308
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
			Scroller {
				'velu',
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
		/* [10] */
		ViewSignatureAndClassname {
			'ssbr',
			105,
			"",
			'velu',
			enabled,
			noIdle,
			{	/* array BehaviorArray: 0 elements */
			},
			MARelease13View {

			},
			{	/* array: 2 elements */
				/* [1] */
				4,
				/* [2] */
				313
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
		/* [11] */
		ViewSignatureAndClassname {
			'clsp',
			719,
			"",
			'selt',
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
				10,
				/* [2] */
				11
			},
			{	/* array: 2 elements */
				/* [1] */
				242,
				/* [2] */
				356
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
				1001,
				6,
				noID
			},
			2
		},
		/* [12] */
		ViewSignatureAndClassname {
			'view',
			456,
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
				21,
				/* [2] */
				6
			},
			{	/* array: 2 elements */
				/* [1] */
				187,
				/* [2] */
				342
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
		/* [13] */
		ViewSignatureAndClassname {
			'scrl',
			157,
			"XPFVolumeList",
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
				3,
				/* [2] */
				3
			},
			{	/* array: 2 elements */
				/* [1] */
				181,
				/* [2] */
				322
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
		/* [14] */
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
				2,
				/* [2] */
				325
			},
			{	/* array: 2 elements */
				/* [1] */
				183,
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
		/* [15] */
		ViewSignatureAndClassname {
			'butn',
			131,
			"",
			'info',
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
				212,
				/* [2] */
				251
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				84
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
				7
			},
			NoSubviews
		},
		/* [16] */
		ViewSignatureAndClassname {
			'clsp',
			226,
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
				403,
				/* [2] */
				387
			},
			{	/* array: 2 elements */
				/* [1] */
				145,
				/* [2] */
				335
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
				1001,
				8,
				noID
			},
			1
		},
		/* [17] */
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
				24,
				/* [2] */
				21
			},
			{	/* array: 2 elements */
				/* [1] */
				108,
				/* [2] */
				302
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
		/* [18] */
		ViewSignatureAndClassname {
			'clsp',
			413,
			"",
			'opcl',
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
				259,
				/* [2] */
				13
			},
			{	/* array: 2 elements */
				/* [1] */
				146,
				/* [2] */
				350
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
				1001,
				9,
				noID
			},
			2
		},
		/* [19] */
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
				116,
				/* [2] */
				246
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				88
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
				10
			},
			NoSubviews
		},
		/* [20] */
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
				23,
				/* [2] */
				10
			},
			{	/* array: 2 elements */
				/* [1] */
				89,
				/* [2] */
				324
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
		/* [21] */
		ViewSignatureAndClassname {
			'pict',
			137,
			"TPicture",
			'owcp',
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
				413,
				/* [2] */
				47
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
		/* [22] */
		ViewSignatureAndClassname {
			'clsp',
			579,
			"",
			'recl',
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
				10,
				/* [2] */
				381
			},
			{	/* array: 2 elements */
				/* [1] */
				61,
				/* [2] */
				348
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
				1001,
				11,
				noID
			},
			3
		},
		/* [23] */
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
				26,
				/* [2] */
				77
			},
			{	/* array: 2 elements */
				/* [1] */
				21,
				/* [2] */
				131
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
				-1,
				1
			},
			NoSubviews
		},
		/* [24] */
		ViewSignatureAndClassname {
			'stat',
			135,
			"",
			'hlps',
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
				27,
				/* [2] */
				9
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				60
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
				1001,
				12
			},
			NoSubviews
		},
		/* [25] */
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
				26,
				/* [2] */
				238
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				97
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
				13
			},
			NoSubviews
		}
	}
};

resource 'View' (1200, "VolumeDisplay", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 4 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'view',
			435,
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
				320
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
			3
		},
		/* [2] */
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
				4,
				/* [2] */
				40
			},
			{	/* array: 2 elements */
				/* [1] */
				18,
				/* [2] */
				273
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
				17832,
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
				16903,
				dontPreferOutline,
				autoWrap,
				dontEraseFirst,
				justSystem,
				-1,
				1
			},
			NoSubviews
		}
	}
};

