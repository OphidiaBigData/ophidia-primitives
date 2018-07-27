/*
    Ophidia Primitives
    Copyright (C) 2012-2018 CMCC Foundation

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPH_FDI_H
#define OPH_FDI_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include "oph_core.h"

/**
 * @brief Return value in case of success
 */
#define OPH_FDI_SUCCESS 0

/**
 * @brief Return value in case of a generic error
 */
#define OPH_FDI_ERROR_GENERIC 1

/**
 * @brief Return value in case of a memory allocation error
 */
#define OPH_FDI_ERROR_MEMORY 2

/**
 * @brief Return value in case of an incorrect parameter
 */
#define OPH_FDI_ERROR_BAD_PARAM 3


/**
 * @brief Lookup table: day length factors (L_f) for DC.
 *
 * Standard Configuration:
 * 	 jan = -1.6;
 * 	 feb = -1.6;
 * 	 mar = -1.6;
 * 	 apr = 0.9;
 * 	 may = 3.8;
 * 	 jun = 5.8;
 * 	 jul = 6.4;
 * 	 aug = 5.0;
 * 	 sep = 2.4;
 * 	 oct = 0.4;
 * 	 nov = -1.6;
 * 	 dec = -1.6;
 */
typedef struct _oph_fdi_dc_lf {
	double jan;
	double feb;
	double mar;
	double apr;
	double may;
	double jun;
	double jul;
	double aug;
	double sep;
	double oct;
	double nov;
	double dec;
} oph_fdi_dc_lf;

/**
 * @brief Configuration of parameters for all index functions.
 */
typedef struct _oph_fdi_configuration {
	/**
	 * @name Lookup tables for DMC and DC
	 */
	/*@{ */
	oph_fdi_dc_lf fwi_dc_lf;
	/*@} */
	/**
	 * @name FWI parameters (FFMC)
	 *
	 * (1) m_o = fwi_1 * ((fwi_2 - F_o)/(fwi_3 + F_o))
	 * (2) if r_o > fwi_4 then r_f = r_o - fwi_4
	 * (3) if m_o <= fwi_5 then m_r = m_o + fwi_6 * r_f * (exp(fwi_7/(fwi_8 - m_o))) * (1 - exp(fwi_9 / r_f))
	 * (4) if m_o > fwi_5 then m_r = m_o + fwi_6 * r_f * (exp(fwi_7/(fwi_8 - m_o))) * (1 - exp(fwi_9 / r_f)) + fwi_10 * (m_o - fwi_5)^fwi_11 * (r_f)^fwi_12
	 * (5) E_d = fwi_13 * H^fwi_14 + fwi_15*exp((H-fwi_16)/fwi_17) + fwi_18*(fwi_19 - T)*(1 - exp(fwi_20*H))
	 * (6) E_w = fwi_21 * H^fwi_22 + fwi_23*exp((H-fwi_16)/fwi_17) + fwi_18*(fwi_19 - T)*(1 - exp(fwi_20*H))
	 * (7) k_o = fwi_24 * (1 - (H/100)^fwi_25) + fwi_26 * W^fwi_27 * (1 - (H/100)^fwi_28)
	 * (8) k_d = k_o * fwi_29 * exp(fwi_30 * T)
	 * (9) k_l = fwi_24 * (1 - ((100 - H)/100)^fwi_25) + fwi_26 * W^fwi_27 * (1 - ((100 - H)/100)^fwi_28)
	 * (10) k_w = k_l * fwi_29 * exp(fwi_30 * T)
	 * (11) m = E_d + (m_o - E_d)*10^(-k_d)
	 * (12) m = E_w - (E_w - m_o)*10^(-k_w)
	 * (13) F = fwi_31 * (fwi_32 - m)/(fwi_33 + m)
	 * if m_r > fwi_32 then m_r = fwi_32
	 *
	 * Standard Configuration:
	 * 	 fwi_1 = 147.2;
	 * 	 fwi_2 = 101;
	 * 	 fwi_3 = 59.5;
	 * 	 fwi_4 = 0.5;
	 * 	 fwi_5 = 150;
	 * 	 fwi_6 = 42.5;
	 * 	 fwi_7 = -100;
	 * 	 fwi_8 = 251;
	 * 	 fwi_9 = -6.93;
	 * 	 fwi_10 = 0.0015;
	 * 	 fwi_11 = 2;
	 * 	 fwi_12 = 0.5;
	 * 	 fwi_13 = 0.942;
	 * 	 fwi_14 = 0.679;
	 * 	 fwi_15 = 11;
	 * 	 fwi_16 = 100;
	 * 	 fwi_17 = 10;
	 * 	 fwi_18 = 0.18;
	 * 	 fwi_19 = 21.1;
	 * 	 fwi_20 = -0.115;
	 * 	 fwi_21 = 0.618;
	 * 	 fwi_22 = 0.753;
	 * 	 fwi_23 = 10;
	 * 	 fwi_24 = 0.424;
	 * 	 fwi_25 = 1.7;
	 * 	 fwi_26 = 0.0694;
	 * 	 fwi_27 = 0.5;
	 * 	 fwi_28 = 8;
	 * 	 fwi_29 = 0.581;
	 * 	 fwi_30 = 0.0365;
	 * 	 fwi_31 = 59.5;
	 * 	 fwi_32 = 250;
	 * 	 fwi_33 = 147.2;
	 */
	/*@{ */
	double fwi_1;
	double fwi_2;
	double fwi_3;
	double fwi_4;
	double fwi_5;
	double fwi_6;
	double fwi_7;
	double fwi_8;
	double fwi_9;
	double fwi_10;
	double fwi_11;
	double fwi_12;
	double fwi_13;
	double fwi_14;
	double fwi_15;
	double fwi_16;
	double fwi_17;
	double fwi_18;
	double fwi_19;
	double fwi_20;
	double fwi_21;
	double fwi_22;
	double fwi_23;
	double fwi_24;
	double fwi_25;
	double fwi_26;
	double fwi_27;
	double fwi_28;
	double fwi_29;
	double fwi_30;
	double fwi_31;
	double fwi_32;
	double fwi_33;
	/*@} */
	/**
	 * @name FWI parameters (DMC)
	 *
	 * (14) if r_o > fwi_34 then r_e = fwi_35 * r_o - fwi_36
	 * (15) M_o = fwi_37 + exp((fwi_38 - P_o)/fwi_39)
	 * (16) if P_o <= fwi_40 then b = fwi_41 / (fwi_42 + fwi_43 * P_o)
	 * (17) if fwi_40 < P_o <= fwi_44 then b = fwi_45 - fwi_46 * ln(P_o)
	 * (18) if P_o > fwi_44 then b = fwi_47 * ln(P_o) - fwi_48
	 * (19) M_r = M_o + (fwi_49 * r_e)/(fwi_50 + b * r_e)
	 * (20) P_r = fwi_51 - fwi_52 * ln(M_r - fwi_53)
	 * (21) K = fwi_54 * (T + fwi_55) * (fwi_56 - H) * L_e * fwi_57
	 * (22) P = P_o (or P_r) + fwi_58 * K
	 * L_e = fwi_59
	 * if P_r < 0 then P_r = 0
	 * if T < -fwi_55 then T = -fwi_55
	 *
	 * Standard Configuration:
	 * 	 fwi_34 = 1.5;
	 * 	 fwi_35 = 0.92;
	 * 	 fwi_36 = 1.27;
	 * 	 fwi_37 = 20;
	 * 	 fwi_38 = 5.6348;
	 * 	 fwi_39 = 43.43;
	 * 	 fwi_40 = 33;
	 * 	 fwi_41 = 100;
	 * 	 fwi_42 = 0.5;
	 * 	 fwi_43 = 0.3;
	 * 	 fwi_44 = 65;
	 * 	 fwi_45 = 14;
	 * 	 fwi_46 = 1.3;
	 * 	 fwi_47 = 6.2;
	 * 	 fwi_48 = 17.2;
	 * 	 fwi_49 = 1000;
	 * 	 fwi_50 = 48.77;
	 * 	 fwi_51 = 244.72;
	 * 	 fwi_52 = 43.43;
	 * 	 fwi_53 = 20;
	 * 	 fwi_54 = 1.894;
	 * 	 fwi_55 = 1.1;
	 * 	 fwi_56 = 100;
	 * 	 fwi_57 = 0.000001;
	 * 	 fwi_58 = 100;
	 * 	 fwi_59 = 0; L_e to be set according to month
	 */
	/*@{ */
	double fwi_34;
	double fwi_35;
	double fwi_36;
	double fwi_37;
	double fwi_38;
	double fwi_39;
	double fwi_40;
	double fwi_41;
	double fwi_42;
	double fwi_43;
	double fwi_44;
	double fwi_45;
	double fwi_46;
	double fwi_47;
	double fwi_48;
	double fwi_49;
	double fwi_50;
	double fwi_51;
	double fwi_52;
	double fwi_53;
	double fwi_54;
	double fwi_55;
	double fwi_56;
	double fwi_57;
	double fwi_58;
	double fwi_59;
	/*@} */
	/**
	 * @name FWI parameters (DC)
	 *
	 * (23) if r_o > fwi_60 then r_d = fwi_61 * r_o - fwi_62
	 * (24) Q_o = fwi_63 * exp(-D_o / fwi_64)
	 * (25) Q_r = Q_o + fwi_65 * r_d
	 * (26) D_r = fwi_66 * ln(fwi_67 / Q_r)
	 * (27) V = fwi_68 * (T + fwi_69) + L_f
	 * (28) D = D_o + fwi_70 * V
	 * L_f = fwi_71
	 * if D_r < 0 then D_r = 0
	 * if T < -fwi_69 then T = -fwi_69
	 * if V < 0 then V = 0
	 *
	 * Standard Configuration:
	 * 	 fwi_60 = 2.8;
	 * 	 fwi_61 = 0.83;
	 * 	 fwi_62 = 1.27;
	 * 	 fwi_63 = 800;
	 * 	 fwi_64 = 400;
	 * 	 fwi_65 = 3.937;
	 * 	 fwi_66 = 400;
	 * 	 fwi_67 = 800;
	 * 	 fwi_68 = 0.36;
	 * 	 fwi_69 = 2.8;
	 * 	 fwi_70 = 0.5;
	 * 	 fwi_71 = 0; L_f to be set according to month
	 */
	/*@{ */
	double fwi_60;
	double fwi_61;
	double fwi_62;
	double fwi_63;
	double fwi_64;
	double fwi_65;
	double fwi_66;
	double fwi_67;
	double fwi_68;
	double fwi_69;
	double fwi_70;
	double fwi_71;
	/*@} */
	/**
	 * @name FWI parameters (ISI)
	 *
	 * (29) f(W) = exp(fwi_72 * W)
	 * (30) f(F) = fwi_73 * exp(fwi_74 * m) * (1 + (m^fwi_75)/(fwi_76))
	 * (31) R = fwi_77 * f(W) * f(F)
	 *
	 * Standard Configuration:
	 * 	 fwi_72 = 0.05039;
	 * 	 fwi_73 = 91.9;
	 * 	 fwi_74 = -0.1386;
	 * 	 fwi_75 = 5.31;
	 * 	 fwi_76 = 49300000;
	 * 	 fwi_77 = 0.208;
	 */
	/*@{ */
	double fwi_72;
	double fwi_73;
	double fwi_74;
	double fwi_75;
	double fwi_76;
	double fwi_77;
	/*@} */
	/**
	 * @name FWI parameters (BUI)
	 *
	 * (32) if P <= fwi_78 * D then U = (fwi_79 * P * D) / (P + fwi_78 * D)
	 * (33) if P > fwi_78 * D then U = P - (1 - (fwi_79 * D)/(P + fwi_78 * D)) * (fwi_80 + (fwi_81 * P)^fwi_82)
	 *
	 * Standard Configuration:
	 * 	 fwi_78 = 0.4;
	 * 	 fwi_79 = 0.8;
	 * 	 fwi_80 = 0.92;
	 * 	 fwi_81 = 0.0114;
	 * 	 fwi_82 = 1.7;
	 */
	/*@{ */
	double fwi_78;
	double fwi_79;
	double fwi_80;
	double fwi_81;
	double fwi_82;
	/*@} */
	/**
	 * @name FWI parameters (FWI)
	 *
	 * (34) if U <= fwi_83 then f(D) = fwi_84 * (U)^fwi_85 + fwi_86
	 * (35) if U > fwi_83 then f(D) = fwi_87 / (fwi_88 + fwi_89 * exp(fwi_90 * U))
	 * (36) B = fwi_91 * R * f(D)
	 * (37) if B > fwi_92 then ln(S) = fwi_93 * (fwi_94 * ln(B))^fwi_95
	 * (38) if B <= fwi_92 then S = B
	 *
	 * Standard Configuration:
	 * 	 fwi_83 = 80;
	 * 	 fwi_84 = 0.626;
	 * 	 fwi_85 = 0.809;
	 * 	 fwi_86 = 2;
	 * 	 fwi_87 = 1000;
	 * 	 fwi_88 = 25;
	 * 	 fwi_89 = 108.64;
	 * 	 fwi_90 = -0.023;
	 * 	 fwi_91 = 0.1;
	 * 	 fwi_92 = 1;
	 * 	 fwi_93 = 2.72;
	 * 	 fwi_94 = 0.434;
	 * 	 fwi_95 = 0.647;
	 */
	/*@{ */
	double fwi_83;
	double fwi_84;
	double fwi_85;
	double fwi_86;
	double fwi_87;
	double fwi_88;
	double fwi_89;
	double fwi_90;
	double fwi_91;
	double fwi_92;
	double fwi_93;
	double fwi_94;
	double fwi_95;
	/*@} */
	/**
	 * @name FWI parameters (DSR)
	 *
	 * (39) DSR = fwi_96 * (S)^fwi_97
	 *
	 * Standard Configuration:
	 * 	 fwi_96 = 0.0272;
	 * 	 fwi_97 = 1.77;
	 */
	/*@{ */
	double fwi_96;
	double fwi_97;
	/*@} */
	/**
	 * @name FFWI parameters (Rate of Spread)
	 *
	 * R_s = n * (1 + ffwi_1 * U^ffwi_2)
	 * if U > ffwi_24 then U = ffwi_24
	 *
	 * Standard Configuration:
	 * 	 ffwi_1 = 1;
	 * 	 ffwi_2 = 2;
	 */
	/*@{ */
	double ffwi_1;
	double ffwi_2;
	/*@} */
	/**
	 * @name FFWI parameters (Zero Wind Rate of Speed and Energy Release)
	 *
	 * n = 1 - ffwi_3 * (m / ffwi_4) + ffwi_5 * (m / ffwi_4)^ffwi_6 - ffwi_7 * (m / ffwi_4)^ffwi_8
	 *
	 * Standard Configuration:
	 * 	 ffwi_3 = 2;
	 * 	 ffwi_4 = 30;
	 * 	 ffwi_5 = 1.5;
	 * 	 ffwi_6 = 2;
	 * 	 ffwi_7 = 0.5;
	 * 	 ffwi_8 = 3;
	 */
	/*@{ */
	double ffwi_3;
	double ffwi_4;
	double ffwi_5;
	double ffwi_6;
	double ffwi_7;
	double ffwi_8;
	/*@} */
	/**
	 * @name FFWI parameters (Equilibrium Moisture Content)
	 *
	 * if h < ffwi_9 then m = ffwi_10 + ffwi_11 * h - ffwi_12 * h * T
	 * if ffwi_9 <= h <= ffwi_13 then m = ffwi_14 + ffwi_15 * h - ffwi_16* T
	 * if h > ffwi_13 then m = ffwi_17 + ffwi_18 * h^ffwi_19 - ffwi_20 * h * T - ffwi_21 * h
	 * if m < 0 then m = 0
	 *
	 * Standard Configuration:
	 * 	 ffwi_9 = 10;
	 * 	 ffwi_10 = 0.03229;
	 * 	 ffwi_11 = 0.281073;
	 * 	 ffwi_12 = 0.000578;
	 * 	 ffwi_13 = 50;
	 * 	 ffwi_14 = 2.22749;
	 * 	 ffwi_15 = 0.160107;
	 * 	 ffwi_16 = 0.014784;
	 * 	 ffwi_17 = 21.0606;
	 * 	 ffwi_18 = 0.005565;
	 * 	 ffwi_19 = 2;
	 * 	 ffwi_20 = 0.00035;
	 * 	 ffwi_21 = 0.483199;
	 */
	/*@{ */
	double ffwi_9;
	double ffwi_10;
	double ffwi_11;
	double ffwi_12;
	double ffwi_13;
	double ffwi_14;
	double ffwi_15;
	double ffwi_16;
	double ffwi_17;
	double ffwi_18;
	double ffwi_19;
	double ffwi_20;
	double ffwi_21;
	/*@} */
	/**
	 * @name FFWI parameters (FFWI)
	 *
	 * FFWI = ffwi_22 * [n^2 * (1 + ffwi_1 * U^ffwi_2)]^ffwi_23
	 * if U > ffwi_24 then U = ffwi_24
	 *
	 * Standard Configuration:
	 * 	 ffwi_22 = (1.0 / 0.3002);
	 * 	 ffwi_23 = 0.5;
	 * 	 ffwi_24 = 30;
	 */
	/*@{ */
	double ffwi_22;
	double ffwi_23;
	double ffwi_24;
	/*@} */
	/**
	 * @name IFI parameters (DC)
	 *
	 * DC = ifi_1 * (exp(ifi_2 * ((R_g * T)/(ifi_3 - ifi_4 * T))) / (1 + sqrt(P_a)) )
	 * if DC < ifi_5 then DC = ifi_5
	 * if DC > ifi_6 then DC = ifi_6
	 *
	 * Standard Configuration:
	 * 	 ifi_1 = 1.12;
	 * 	 ifi_2 = 0.261;
	 * 	 ifi_3 = 2501;
	 * 	 ifi_4 = 2.361;
	 * 	 ifi_5 = 0.1;
	 * 	 ifi_6 = 5;
	 */
	/*@{ */
	double ifi_1;
	double ifi_2;
	double ifi_3;
	double ifi_4;
	double ifi_5;
	double ifi_6;
	/*@} */
	/**
	 * @name IFI parameters (MC)
	 *
	 * MC = f(RHn) + f(Tx) + f(W)
	 * f(RHn) = ifi_7 * exp(ifi_8 * RHn)
	 * f(Tx) = ifi_9 * exp(ifi_10 * Tx)
	 * f(W) = f(Tx,w) * f(Wint)
	 * if Tx >= ifi_11 then f(Tx,w) = 1 else ( if Tx < ifi_12 then f(Tx,w) = 0 else f(Tx,w) = cos((ifi_11-Tx)/(ifi_11-ifi_12)*pi/2) )
	 * if ifi_13*exp(ifi_14*W) > ifi_15 then f(Wint) = ifi_15 else f(Wint) = ifi_13*exp(ifi_14*Wint)
	 * if MC < ifi_16 then MC = ifi_16
	 * if MC > ifi_17 then MC = ifi_17
	 *
	 * Standard Configuration:
	 * 	 ifi_7 = 0.6;
	 * 	 ifi_8 = -0.02;
	 * 	 ifi_9 = 0.3;
	 * 	 ifi_10 = 0.05;
	 * 	 ifi_11 = 25;
	 * 	 ifi_12 = -5;
	 * 	 ifi_13 = 0.2;
	 * 	 ifi_14 = 0.08;
	 * 	 ifi_15 = 2.205;
	 * 	 ifi_16 = 0.4;
	 * 	 ifi_17 = 5;
	 */
	/*@{ */
	double ifi_7;
	double ifi_8;
	double ifi_9;
	double ifi_10;
	double ifi_11;
	double ifi_12;
	double ifi_13;
	double ifi_14;
	double ifi_15;
	double ifi_16;
	double ifi_17;
	/*@} */
	/**
	 * @name IFI parameters (R)
	 *
	 * if RSx < ifi_18 then R = ifi_19 else ( if RSx <= ifi_20 then R = ifi_21 else R = ifi_22 )
	 *
	 * Standard Configuration:
	 * 	 ifi_18 = 400;
	 * 	 ifi_19 = 0.24;
	 * 	 ifi_20 = 800;
	 * 	 ifi_21 = 0.32;
	 * 	 ifi_22 = 1;
	 */
	/*@{ */
	double ifi_18;
	double ifi_19;
	double ifi_20;
	double ifi_21;
	double ifi_22;
	/*@} */
	/**
	 * @name IFI parameters (FC)
	 *
	 * FC = (ifi_23 - a) / ifi_24
	 * a = ifi_25-ifi_26*(Tmed-RHmed)
	 * if a < 0 then a = 0
	 * if a > ifi_23 then a = ifi_23
	 *
	 * Standard Configuration:
	 * 	 ifi_23 = 45;
	 * 	 ifi_24 = 13;
	 * 	 ifi_25 = 10;
	 * 	 ifi_26 = 0.25;
	 */
	/*@{ */
	double ifi_23;
	double ifi_24;
	double ifi_25;
	double ifi_26;
	/*@} */
} oph_fdi_configuration;

/**
 * @brief Initial setup for all parameters.
 * @param[in,out] conf Struct for containing all parameters
 * @return 0 on success
 */
int oph_fdi_configuration_setup(oph_fdi_configuration * conf);

/**
 * @brief Setup of DMC L_e.
 * @param[in] month Month as a number between 1 and 12
 * @param[in,out] conf Struct for containing all parameters
 * @return 0 on success
 */
int oph_fdi_configuration_fwi_dmc_le_setup(int day, int month, int year, double lat, oph_fdi_configuration * conf);

/**
 * @brief Setup of DC L_f.
 * @param[in] month Month as a number between 1 and 12
 * @param[in,out] conf Struct for containing all parameters
 * @return 0 on success
 */
int oph_fdi_configuration_fwi_dc_lf_setup(int month, oph_fdi_configuration * conf);

/**
 * @name Fire Weather Index (FWI) functions
 */
/*@{*/

/**
 * @brief Compute the Fine Fuel Moisture Code (FFMC).
 * @param[in] rain Accumulated rainfall of 24 hours
 * @param[in] temperature Dry-bulb temperature at noon
 * @param[in] humidity Relative humidity at noon
 * @param[in] wind Open wind speed at noon
 * @param[in] prev_ffmc FFMC of the previous day
 * @param[in] conf libfdi configuration
 * @param[out] ffmc Output index
 * @return 0 on success
 */
int oph_fdi_fwi_ffmc(double rain, double temperature, double humidity, double wind, double prev_ffmc, oph_fdi_configuration * conf, double *ffmc);

/**
 * @brief Compute the Duff Moisture Code (DMC).
 * @param[in] day Day as a number
 * @param[in] month Month as a number between 1 and 12
 * @param[in] year Year as a 4-digit number
 * @param[in] lat Latitude
 * @param[in] rain Accumulated rainfall of 24 hours
 * @param[in] temperature Dry-bulb temperature at noon
 * @param[in] humidity Relative humidity at noon
 * @param[in] prev_dmc Previous day DMC
 * @param[in] conf libfdi configuration
 * @param[out] dmc Output index
 * @return 0 on success
 */
int oph_fdi_fwi_dmc(int day, int month, int year, double lat, double rain, double temperature, double humidity, double prev_dmc, oph_fdi_configuration * conf, double *dmc);

/**
 * @brief Compute the Drought Code (DC).
 * @param[in] month Month as a number between 1 and 12
 * @param[in] rain Accumulated rainfall of 24 hours
 * @param[in] temperature Dry-bulb temperature at noon
 * @param[in] prev_dc Previous day DC
 * @param[in] conf libfdi configuration
 * @param[out] dc Output index
 * @return 0 on success
 */
int oph_fdi_fwi_dc(int month, double rain, double temperature, double prev_dc, oph_fdi_configuration * conf, double *dc);

/**
 * @brief Compute the Initial Spread Index (ISI) (from FFMC)
 * @param[in] ffmc Fine Fuel Moisture Code (FFMC)
 * @param[in] wind Open wind speed at noon
 * @param[in] conf libfdi configuration
 * @param[out] isi Output index
 * @return 0 on success
 */
int oph_fdi_fwi_isi_from_ffmc(double ffmc, double wind, oph_fdi_configuration * conf, double *isi);
/**
 * @brief Compute the Initial Spread Index (ISI).
 * @param[in] rain Accumulated rainfall of 24 hours
 * @param[in] temperature Dry-bulb temperature at noon
 * @param[in] humidity Relative humidity at noon
 * @param[in] wind Open wind speed at noon
 * @param[in] prev_ffmc FFMC of the previous day
 * @param[in] conf libfdi configuration
 * @param[out] isi Output index
 * @return 0 on success
 */
int oph_fdi_fwi_isi(double rain, double temperature, double humidity, double wind, double prev_ffmc, oph_fdi_configuration * conf, double *isi);

/**
 * @brief Compute the Build Up Index (BUI). (from DMC and DC)
 * @param[in] dmc Duff Moisture Code (DMC)
 * @param[in] dc Drought Code (DC)
 * @param[in] conf libfdi configuration
 * @param[out] bui Output index
 * @return 0 on success
 */
int oph_fdi_fwi_bui_from_dmc_and_dc(double dmc, double dc, oph_fdi_configuration * conf, double *bui);
/**
 * @brief Compute the Build Up Index (BUI).
 * @param[in] day Day as a number
 * @param[in] month Month as a number between 1 and 12
 * @param[in] year Year as a 4-digit number
 * @param[in] lat Latitude
 * @param[in] rain Accumulated rainfall of 24 hours
 * @param[in] temperature Dry-bulb temperature at noon
 * @param[in] humidity Relative humidity at noon
 * @param[in] prev_dmc Previous day DMC
 * @param[in] prev_dc Previous day DC
 * @param[in] conf libfdi configuration
 * @param[out] bui Output index
 * @return 0 on success
 */
int oph_fdi_fwi_bui(int day, int month, int year, double lat, double rain, double temperature, double humidity, double prev_dmc, double prev_dc, oph_fdi_configuration * conf, double *bui);

/**
 * @brief Compute the Fire Weather Index (FWI). (from ISI and BUI)
 * @param[in] isi Initial Spread Index (ISI)
 * @param[in] bui Build Up Index (BUI)
 * @param[in] conf libfdi configuration
 * @param[out] fwi Output index
 * @return 0 on success
 */
int oph_fdi_fwi_from_isi_and_bui(double isi, double bui, oph_fdi_configuration * conf, double *fwi);
/**
 * @brief Compute the Fire Weather Index (FWI).
 * @param[in] day Day as a number
 * @param[in] month Month as a number between 1 and 12
 * @param[in] year Year as a 4-digit number
 * @param[in] lat Latitude
 * @param[in] rain Accumulated rainfall of 24 hours
 * @param[in] temperature Dry-bulb temperature at noon
 * @param[in] humidity Relative humidity at noon
 * @param[in] wind Open wind speed at noon
 * @param[in] prev_ffmc FFMC of the previous day
 * @param[in] prev_dmc Previous day DMC
 * @param[in] prev_dc Previous day DC
 * @param[in] conf libfdi configuration
 * @param[out] fwi Output index
 * @return 0 on success
 */
int oph_fdi_fwi_main(int day, int month, int year, double lat, double rain, double temperature, double humidity, double wind, double prev_ffmc, double prev_dmc, double prev_dc,
		oph_fdi_configuration * conf, double *fwi);

/**
 * @brief Compute the Daily Severity Rating (DSR). (from FWI)
 * @param[in] fwi Fire Weather Index (FWI)
 * @param[in] conf libfdi configuration
 * @param[out] dsr Output index
 * @return 0 on success
 */
int oph_fdi_fwi_dsr_from_fwi(double fwi, oph_fdi_configuration * conf, double *dsr);
/**
 * @brief Compute the Daily Severity Rating (DSR).
 * @param[in] day Day as a number
 * @param[in] month Month as a number between 1 and 12
 * @param[in] year Year as a 4-digit number
 * @param[in] lat Latitude
 * @param[in] rain Accumulated rainfall of 24 hours
 * @param[in] temperature Dry-bulb temperature at noon
 * @param[in] humidity Relative humidity at noon
 * @param[in] wind Open wind speed at noon
 * @param[in] prev_ffmc FFMC of the previous day
 * @param[in] prev_dmc Previous day DMC
 * @param[in] prev_dc Previous day DC
 * @param[in] conf libfdi configuration
 * @param[out] dsr Output index
 * @return 0 on success
 */
int oph_fdi_fwi_dsr(int day, int month, int year, double lat, double rain, double temperature, double humidity, double wind, double prev_ffmc, double prev_dmc, double prev_dc,
		    oph_fdi_configuration * conf, double *dsr);
/*@}*/


/**
 * @name Fosberg Fire Weather Index (FFWI) functions
 */
/*@{*/

/**
 * @brief Compute the Rate of Spread.
 * @param[in] temperature Dry-bulb temperature at noon
 * @param[in] humidity Relative humidity at noon
 * @param[in] wind Open wind speed at noon
 * @param[in] conf libfdi configuration
 * @param[out] rs Output index
 * @return 0 on success
 */
int oph_fdi_ffwi_rs(double temperature, double humidity, double wind, oph_fdi_configuration * conf, double *rs);

/**
 * @brief Compute the Zero Wind Rate of Speed.
 * @param[in] temperature Dry-bulb temperature at noon
 * @param[in] humidity Relative humidity at noon
 * @param[in] conf libfdi configuration
 * @param[out] rso Output index
 * @return 0 on success
 */
int oph_fdi_ffwi_rso(double temperature, double humidity, oph_fdi_configuration * conf, double *rso);

/**
 * @brief Compute the Energy Release.
 * @param[in] temperature Dry-bulb temperature at noon
 * @param[in] humidity Relative humidity at noon
 * @param[in] conf libfdi configuration
 * @param[out] er Output index
 * @return 0 on success
 */
int oph_fdi_ffwi_er(double temperature, double humidity, oph_fdi_configuration * conf, double *er);

/**
 * @brief Compute the Equilibrium Moisture Content.
 * @param[in] temperature Dry-bulb temperature at noon
 * @param[in] humidity Relative humidity at noon
 * @param[in] conf libfdi configuration
 * @param[out] emc Output index
 * @return 0 on success
 */
int oph_fdi_ffwi_emc(double temperature, double humidity, oph_fdi_configuration * conf, double *emc);

/**
 * @brief Compute the Fosberg Fire Weather Index (FFWI).
 * @param[in] temperature Dry-bulb temperature at noon (C)
 * @param[in] humidity Relative humidity at noon (%)
 * @param[in] wind Open wind speed at noon (km h-1)
 * @param[in] conf libfdi configuration
 * @param[out] ffwi Output index
 * @return 0 on success
 */
int oph_fdi_ffwi_main(double temperature, double humidity, double wind, oph_fdi_configuration * conf, double *ffwi);
/*@}*/


/**
 * @name Integrated Fire Danger Index (IFI) functions
 */
/*@{*/

/**
 * @brief Compute the Drought Code (DC).
 * @param[in] radiation_mean Mean daily radiation
 * @param[in] temperature_mean Mean daily air temperature
 * @param[in] rain Daily rainfall
 * @param[in] conf libfdi configuration
 * @param[out] dc Output index
 * @return 0 on success
 */
int oph_fdi_ifi_dc(double radiation_mean, double temperature_mean, double rain, oph_fdi_configuration * conf, double *dc);

/**
 * @brief Compute the Meteorological Code (MC).
 * @param[in] temperature_max Max daily air temperature
 * @param[in] wind_mean Mean daily wind speed
 * @param[in] humidity_min Min daily relative humidity
 * @param[in] conf libfdi configuration
 * @param[out] mc Output index
 * @return 0 on success
 */
int oph_fdi_ifi_mc(double temperature_max, double wind_mean, double humidity_min, oph_fdi_configuration * conf, double *mc);

/**
 * @brief Compute the R coefficient (R).
 * @param[in] radiation_max Max daily radiation
 * @param[in] conf libfdi configuration
 * @param[out] r Output index
 * @return 0 on success
 */
int oph_fdi_ifi_r(double radiation_max, oph_fdi_configuration * conf, double *r);

/**
 * @brief Compute the Fuel Code (FC).
 * @param[in] temperature_mean Mean air temperature
 * @param[in] humidity_mean Mean humidity
 * @param[in] conf libfdi configuration
 * @param[out] fc Output index
 * @return 0 on success
 */
int oph_fdi_ifi_fc(double temperature_mean, double humidity_mean, oph_fdi_configuration * conf, double *fc);

/**
 * @brief Compute the Integrated Fire Danger Index (IFI). (from DC,MC,R and FC)
 * @param[in] dc Drought Code (DC)
 * @param[in] mc Meteorological Code (MC)
 * @param[in] r R coefficient (R)
 * @param[in] fc Fuel Code (FC)
 * @param[out] ifi Output index
 * @return 0 on success
 */
int oph_fdi_ifi_from_dc_mc_r_fc(double dc, double mc, double r, double fc, double *ifi);
/**
 * @brief Compute the Integrated Fire Danger Index (IFI).
 * @param[in] radiation_mean Mean daily radiation
 * @param[in] radiation_max Max daily radiation
 * @param[in] temperature_mean Mean daily air temperature
 * @param[in] temperature_max Max daily air temperature
 * @param[in] rain Daily rainfall
 * @param[in] wind_mean Mean daily wind speed
 * @param[in] humidity_mean Mean daily relative humidity
 * @param[in] humidity_min Min daily relative humidity
 * @param[in] conf libfdi configuration
 * @param[out] ifi Output index
 * @return 0 on success
 */
int oph_fdi_ifi_main(double radiation_mean, double radiation_max, double temperature_mean, double temperature_max, double rain, double wind_mean, double humidity_mean, double humidity_min,
		oph_fdi_configuration * conf, double *ifi);
/*@}*/

#endif
