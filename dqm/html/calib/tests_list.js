var TREE_ITEMS = [	
	['CFEB02', 'Pedestals and Noise ',
		['R01', 'Overall Pedestals'],
		['R02', 'Overall Noise'],
		['R03', 'RMS of SCA Pedestals'],
		['R04', 'P12 Pedestals'],
		['R05', 'P12 Noise'],
		['R06', 'P345 Pedestals'],
		['R07', 'P345 Noise'],
		['R08', 'C(12)(12) Covariance matrix element'],
		['R09', 'C(12)3 Covariance matrix element'],
		['R10', 'C(12)4 Covariance matrix element'],
		['R11', 'C(12)5 Covariance matrix element'],
		['R12', 'C33 Covariance matrix element'],
		['R13', 'C34 Covariance matrix element'],
		['R14', 'C35 Covariance matrix element'],
		['R15', 'C44 Covariance matrix element'],
		['R16', 'C45 Covariance matrix element'],
		['R17', 'C55 Covariance matrix element'],
		['V00', 'CSC Format Errors'],
		['V01', 'Signal Line'],
		['V02', 'Q4 with dynamic pedestal subtraction'],
		['V03', 'SCA Cell Occupancy'],
		['V04', 'SCA Block Occupancy'],
		['V05', 'Max SCA Charge']
	],
	['CFEB03','Pulse Response',
		['R01', 'Pulse Maximum Amplitude'],
		['R02', 'Pulse Peak Time Position'],
		['R03', 'Pulse FHWM'],
		['R04', 'Left crosstalk'],
		['R05', 'Right crosstalk'],
	        ['R06', 'Left crosstalk slope'],
		['R07', 'Left crosstalk intercept'],
		['R08', 'Left crosstalk RMS'],
		['R09', 'Right crosstalk slope'],
		['R10', 'Right crosstalk intercept'],
		['R11', 'Right crosstalk RMS'],
		['V00', 'CSC Format Errors'],
                ['V01', 'Number of Events per Setting'],
                ['V02', 'Pulsed Signal line shape'],
                ['V03', 'Crosstalk signal line'],
                ['V04', 'L1A Increment'],
		['V05', 'Convoluted Central Strip'],
		['V06', 'Convoluted Left Strip'],
		['V07', 'Convoluted Right Strip'],
		['V08', 'Convoluted Left Ratio'],
                ['V09', 'Convoluted Right Ratio'],
                ['V10', 'Convoluted (Right-Left)/2']
		
	],
	['CFEB04','Gains',
		['R01', 'Gain slope'],
		['R03', 'Gain non-linearity'],
		['R04', 'Saturation'],
		['R05', 'Normalized gains'],
		['R06', 'Test Pulse Peak Timing'],
		['V00', 'CSC Format Errors'],
		['V01', 'Number of Events per Setting'],
		['V02', 'Signal line shape'],
		['V03', 'Calibration curve'],
		['V04', 'L1A Increment'],
		['V05', 'Fit Residuals'],
		['V06', 'Corrected Peak Time']
		
	],

	['AFEB05','Connectivity',
                ['R01', 'Efficiency'],
                ['R02', 'Pair-Plane Crosstalk'],
                ['R03', 'Non-Pair-Plane Crosstalk'],
		['V00', 'CSC Format Errors'],
                ['V01', 'Wire Groups Occupancy']
	],

	['AFEB06','Thresholds',
		['R01', 'Noise for 20 fC'],
		['R02', 'Thresholds for 20 fC'],
                ['R04', 'Thresholds Offset for 20 fC'],
		['R05', 'Noise for 40 fC'],
                ['R06', 'Thresholds for 40 fC'],
                ['R08', 'Thresholds Offset for 40 fC'],
		['R09', 'Thresholds Slopes'],
		['R11', 'Calculated Thresholds for 20 fC'],
		['R12', 'Calculated Offsets for 20 fC'],
		['R13', 'AFEB Thresholds for 20 fC'],
                ['V00', 'CSC Format Errors'],
                ['V01', 'Wire Groups Occupancy'],
		['V02', 'Raw Hit Time Bin Occupancy']
	],
	['AFEB07','Delays',
		['R01', 'Intercept'],
                ['R02', 'Slope'],
                ['R03', 'Chi-squared of fit'],
		['R04', 'Summary AFEB Results'],
		['V00', 'CSC Format Errors'],
                ['V01', 'Wire Groups Occupancy'],
                ['V02', 'Raw Hit Time Bin Occupancy'],
		['V03', 'Average Time per Delay Setting'],
		['V04', 'Active Time Bins per Delay Setting']
	]
		
]
