resource 'STR#' (1001, "Grid Window", purgeable) {
	{	/* array StringArray: 13 elements */
		/* [1] */
		"Install from CD to",
		/* [2] */
		"Install",
		/* [3] */
		"Use CD:",
		/* [4] */
		"Update",
		/* [5] */
		"Select Target Volume:",
		/* [6] */
		"Get InfoÉ",
		/* [7] */
		"Help:",
		/* [8] */
		"NVRAM Settings:",
		/* [9] */
		"OptionsÉ",
		/* [10] */
		"Restart from",
		/* [11] */
		"Helper:",
		/* [12] */
		"Restart",
		/* [13] */
		"XPostFacto"
	}
};

resource 'STR#' (1200, "VolumeDisplay", purgeable) {
	{	/* array StringArray: 1 elements */
		/* [1] */
		"A volume name"
	}
};

data 'TxSt' (5312, "Application 12, bold") {
	$"019C 000C 0000 0000 0000 0161"                                                                      /* .œ.........a */
};

data 'TxSt' (27392, "Application 10, plain") {
	$"009C 000A 0000 0000 0000 0161"                                                                      /* .œ.Â.......a */
};

resource 'View' (1001, "Grid Window", purgeable) {
	MAThreeOhView {

	},
	{	/* array ViewArray: 24 elements */
		/* [1] */
		ViewSignatureAndClassname {
			'wind',
			3459,
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
				544,
				/* [2] */
				768
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
				13
			},
			6
		},
		/* [2] */
		ViewSignatureAndClassname {
			'clsp',
			598,
			"XPFVolumeCluster",
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
				392,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				92,
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
				60,
				/* [2] */
				254
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
				28,
				/* [2] */
				74
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				202
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
				29,
				/* [2] */
				12
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				54
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
			731,
			"XPFVolumeCluster",
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
				20,
				/* [2] */
				400
			},
			{	/* array: 2 elements */
				/* [1] */
				228,
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
				196,
				/* [2] */
				246
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
				4
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
				24,
				/* [2] */
				12
			},
			{	/* array: 2 elements */
				/* [1] */
				160,
				/* [2] */
				324
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
				2,
				/* [2] */
				2
			},
			{	/* array: 2 elements */
				/* [1] */
				156,
				/* [2] */
				304
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
				1,
				/* [2] */
				306
			},
			{	/* array: 2 elements */
				/* [1] */
				158,
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
				20,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				256,
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
				5,
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
				24,
				/* [2] */
				12
			},
			{	/* array: 2 elements */
				/* [1] */
				188,
				/* [2] */
				332
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
				2,
				/* [2] */
				2
			},
			{	/* array: 2 elements */
				/* [1] */
				184,
				/* [2] */
				312
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
				1,
				/* [2] */
				314
			},
			{	/* array: 2 elements */
				/* [1] */
				186,
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
				224,
				/* [2] */
				254
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
				6
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
				420,
				/* [2] */
				400
			},
			{	/* array: 2 elements */
				/* [1] */
				104,
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
				7,
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
				12
			},
			{	/* array: 2 elements */
				/* [1] */
				68,
				/* [2] */
				324
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
				260,
				/* [2] */
				400
			},
			{	/* array: 2 elements */
				/* [1] */
				148,
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
				8,
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
				24,
				/* [2] */
				12
			},
			{	/* array: 2 elements */
				/* [1] */
				80,
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
			'clsp',
			595,
			"XPFVolumeCluster",
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
				288,
				/* [2] */
				20
			},
			{	/* array: 2 elements */
				/* [1] */
				92,
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
				10,
				noID
			},
			3
		},
		/* [22] */
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
				28,
				/* [2] */
				74
			},
			{	/* array: 2 elements */
				/* [1] */
				20,
				/* [2] */
				202
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
		/* [23] */
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
				29,
				/* [2] */
				12
			},
			{	/* array: 2 elements */
				/* [1] */
				16,
				/* [2] */
				54
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
				11
			},
			NoSubviews
		},
		/* [24] */
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
				60,
				/* [2] */
				254
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
				12
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
			400,
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
				312
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
				262
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
				262
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
		}
	}
};

