/*
    Ophidia Primitives
    Copyright (C) 2012-2025 CMCC Foundation

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

#include "oph_core_fdi.h"
#include "math.h"

int _yisleap(int year)
{
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int _get_yday(int day, int month, int year)
{
	static const int days[2][13] = {
		{0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
		{0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
	};
	int leap = _yisleap(year);

	return days[leap][month] + day;
}

double _get_daylength(int day, int month, int year, double lat)
{
	int doy = _get_yday(day, month, year);

	return 24 / M_PI * acos(-tan(lat * M_PI / 180) * tan(0.409 * sin((2 * M_PI / 365) * doy - 1.39)));
}

int oph_fdi_configuration_setup(oph_fdi_configuration * conf)
{
	if (!conf) {
		fprintf(stderr, "Unable to setup configuration: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	conf->fwi_dc_lf.jan = -1.6;
	conf->fwi_dc_lf.feb = -1.6;
	conf->fwi_dc_lf.mar = -1.6;
	conf->fwi_dc_lf.apr = 0.9;
	conf->fwi_dc_lf.may = 3.8;
	conf->fwi_dc_lf.jun = 5.8;
	conf->fwi_dc_lf.jul = 6.4;
	conf->fwi_dc_lf.aug = 5.0;
	conf->fwi_dc_lf.sep = 2.4;
	conf->fwi_dc_lf.oct = 0.4;
	conf->fwi_dc_lf.nov = -1.6;
	conf->fwi_dc_lf.dec = -1.6;


	conf->fwi_1 = 147.2;
	conf->fwi_2 = 101;
	conf->fwi_3 = 59.5;
	conf->fwi_4 = 0.5;
	conf->fwi_5 = 150;
	conf->fwi_6 = 42.5;
	conf->fwi_7 = -100;
	conf->fwi_8 = 251;
	conf->fwi_9 = -6.93;
	conf->fwi_10 = 0.0015;
	conf->fwi_11 = 2;
	conf->fwi_12 = 0.5;
	conf->fwi_13 = 0.942;
	conf->fwi_14 = 0.679;
	conf->fwi_15 = 11;
	conf->fwi_16 = 100;
	conf->fwi_17 = 10;
	conf->fwi_18 = 0.18;
	conf->fwi_19 = 21.1;
	conf->fwi_20 = -0.115;
	conf->fwi_21 = 0.618;
	conf->fwi_22 = 0.753;
	conf->fwi_23 = 10;
	conf->fwi_24 = 0.424;
	conf->fwi_25 = 1.7;
	conf->fwi_26 = 0.0694;
	conf->fwi_27 = 0.5;
	conf->fwi_28 = 8;
	conf->fwi_29 = 0.581;
	conf->fwi_30 = 0.0365;
	conf->fwi_31 = 59.5;
	conf->fwi_32 = 250;
	conf->fwi_33 = 147.2;


	conf->fwi_34 = 1.5;
	conf->fwi_35 = 0.92;
	conf->fwi_36 = 1.27;
	conf->fwi_37 = 20;
	conf->fwi_38 = 5.6348;
	conf->fwi_39 = 43.43;
	conf->fwi_40 = 33;
	conf->fwi_41 = 100;
	conf->fwi_42 = 0.5;
	conf->fwi_43 = 0.3;
	conf->fwi_44 = 65;
	conf->fwi_45 = 14;
	conf->fwi_46 = 1.3;
	conf->fwi_47 = 6.2;
	conf->fwi_48 = 17.2;
	conf->fwi_49 = 1000;
	conf->fwi_50 = 48.77;
	conf->fwi_51 = 244.72;
	conf->fwi_52 = 43.43;
	conf->fwi_53 = 20;
	conf->fwi_54 = 1.894;
	conf->fwi_55 = 1.1;
	conf->fwi_56 = 100;
	conf->fwi_57 = 0.000001;
	conf->fwi_58 = 100;
	conf->fwi_59 = 0;	// L_e to be set according to month


	conf->fwi_60 = 2.8;
	conf->fwi_61 = 0.83;
	conf->fwi_62 = 1.27;
	conf->fwi_63 = 800;
	conf->fwi_64 = 400;
	conf->fwi_65 = 3.937;
	conf->fwi_66 = 400;
	conf->fwi_67 = 800;
	conf->fwi_68 = 0.36;
	conf->fwi_69 = 2.8;
	conf->fwi_70 = 0.5;
	conf->fwi_71 = 0;	// L_f to be set according to month


	conf->fwi_72 = 0.05039;
	conf->fwi_73 = 91.9;
	conf->fwi_74 = -0.1386;
	conf->fwi_75 = 5.31;
	conf->fwi_76 = 49300000;
	conf->fwi_77 = 0.208;


	conf->fwi_78 = 0.4;
	conf->fwi_79 = 0.8;
	conf->fwi_80 = 0.92;
	conf->fwi_81 = 0.0114;
	conf->fwi_82 = 1.7;


	conf->fwi_83 = 80;
	conf->fwi_84 = 0.626;
	conf->fwi_85 = 0.809;
	conf->fwi_86 = 2;
	conf->fwi_87 = 1000;
	conf->fwi_88 = 25;
	conf->fwi_89 = 108.64;
	conf->fwi_90 = -0.023;
	conf->fwi_91 = 0.1;
	conf->fwi_92 = 1;
	conf->fwi_93 = 2.72;
	conf->fwi_94 = 0.434;
	conf->fwi_95 = 0.647;


	conf->fwi_96 = 0.0272;
	conf->fwi_97 = 1.77;


	conf->ffwi_1 = 1;
	conf->ffwi_2 = 2;


	conf->ffwi_3 = 2;
	conf->ffwi_4 = 30;
	conf->ffwi_5 = 1.5;
	conf->ffwi_6 = 2;
	conf->ffwi_7 = 0.5;
	conf->ffwi_8 = 3;


	conf->ffwi_9 = 10;
	conf->ffwi_10 = 0.03229;
	conf->ffwi_11 = 0.281073;
	conf->ffwi_12 = 0.000578;
	conf->ffwi_13 = 50;
	conf->ffwi_14 = 2.22749;
	conf->ffwi_15 = 0.160107;
	conf->ffwi_16 = 0.014784;
	conf->ffwi_17 = 21.0606;
	conf->ffwi_18 = 0.005565;
	conf->ffwi_19 = 2;
	conf->ffwi_20 = 0.00035;
	conf->ffwi_21 = 0.483199;


	conf->ffwi_22 = (1.0 / 0.3002);
	conf->ffwi_23 = 0.5;
	conf->ffwi_24 = 30;


	conf->ifi_1 = 1.12;
	conf->ifi_2 = 0.261;
	conf->ifi_3 = 2501;
	conf->ifi_4 = 2.361;
	conf->ifi_5 = 0.1;
	conf->ifi_6 = 5;


	conf->ifi_7 = 0.6;
	conf->ifi_8 = -0.02;
	conf->ifi_9 = 0.3;
	conf->ifi_10 = 0.05;
	conf->ifi_11 = 25;
	conf->ifi_12 = -5;
	conf->ifi_13 = 0.2;
	conf->ifi_14 = 0.08;
	conf->ifi_15 = 2.205;
	conf->ifi_16 = 0.4;
	conf->ifi_17 = 5;


	conf->ifi_18 = 400;
	conf->ifi_19 = 0.24;
	conf->ifi_20 = 800;
	conf->ifi_21 = 0.32;
	conf->ifi_22 = 1;


	conf->ifi_23 = 45;
	conf->ifi_24 = 13;
	conf->ifi_25 = 10;
	conf->ifi_26 = 0.25;

	return OPH_FDI_SUCCESS;
}

int oph_fdi_configuration_fwi_dmc_le_setup(int day, int month, int year, double lat, oph_fdi_configuration * conf)
{
	if (!conf) {
		fprintf(stderr, "Unable to setup configuration: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	conf->fwi_59 = _get_daylength(day, month, year, lat);

	return OPH_FDI_SUCCESS;
}

int oph_fdi_configuration_fwi_dc_lf_setup(int month, oph_fdi_configuration * conf)
{
	if (!conf) {
		fprintf(stderr, "Unable to setup configuration: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	switch (month) {
		case 1:
			conf->fwi_71 = conf->fwi_dc_lf.jan;
			break;
		case 2:
			conf->fwi_71 = conf->fwi_dc_lf.feb;
			break;
		case 3:
			conf->fwi_71 = conf->fwi_dc_lf.mar;
			break;
		case 4:
			conf->fwi_71 = conf->fwi_dc_lf.apr;
			break;
		case 5:
			conf->fwi_71 = conf->fwi_dc_lf.may;
			break;
		case 6:
			conf->fwi_71 = conf->fwi_dc_lf.jun;
			break;
		case 7:
			conf->fwi_71 = conf->fwi_dc_lf.jul;
			break;
		case 8:
			conf->fwi_71 = conf->fwi_dc_lf.aug;
			break;
		case 9:
			conf->fwi_71 = conf->fwi_dc_lf.sep;
			break;
		case 10:
			conf->fwi_71 = conf->fwi_dc_lf.oct;
			break;
		case 11:
			conf->fwi_71 = conf->fwi_dc_lf.nov;
			break;
		case 12:
			conf->fwi_71 = conf->fwi_dc_lf.dec;
			break;
		default:
			fprintf(stderr, "Invalid month\n");
			return OPH_FDI_ERROR_BAD_PARAM;
	}

	return OPH_FDI_SUCCESS;
}

int oph_fdi_fwi_ffmc(double rain, double temperature, double humidity, double wind, double prev_ffmc, oph_fdi_configuration * conf, double *ffmc)
{
	if (!conf || !ffmc) {
		fprintf(stderr, "Unable to compute ffmc: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(rain) || isnan(temperature) || isnan(humidity) || isnan(wind) || isnan(prev_ffmc)) {
		*ffmc = NAN;
		return OPH_FDI_SUCCESS;
	}

	double m_o = conf->fwi_1 * ((conf->fwi_2 - prev_ffmc) / (conf->fwi_3 + prev_ffmc));

	if (rain > conf->fwi_4) {
		double r_f = rain - conf->fwi_4;
		double m_r;
		if (m_o <= conf->fwi_5) {
			m_r = m_o + conf->fwi_6 * r_f * exp(conf->fwi_7 / (conf->fwi_8 - m_o)) * (1 - exp(conf->fwi_9 / r_f));
			if (m_r > conf->fwi_32)
				m_r = conf->fwi_32;
		} else {
			m_r =
			    m_o + conf->fwi_6 * r_f * exp(conf->fwi_7 / (conf->fwi_8 - m_o)) * (1 - exp(conf->fwi_9 / r_f)) + conf->fwi_10 * pow(m_o - conf->fwi_5, conf->fwi_11) * pow(r_f,
																							conf->fwi_12);
			if (m_r > conf->fwi_32)
				m_r = conf->fwi_32;
		}
		m_o = m_r;
	}

	double E_d = conf->fwi_13 * pow(humidity,
					conf->fwi_14) + conf->fwi_15 * exp((humidity - conf->fwi_16) / conf->fwi_17) + conf->fwi_18 * (conf->fwi_19 - temperature) * (1 - exp(conf->fwi_20 * humidity));
	double m;

	if (m_o > E_d) {
		double k_o = conf->fwi_24 * (1 - pow(humidity / 100, conf->fwi_25)) + conf->fwi_26 * pow(wind, conf->fwi_27) * (1 - pow(humidity / 100, conf->fwi_28));
		double k_d = k_o * conf->fwi_29 * exp(conf->fwi_30 * temperature);
		m = E_d + (m_o - E_d) * pow(10, -k_d);
	} else if (m_o < E_d) {
		double E_w = conf->fwi_21 * pow(humidity,
						conf->fwi_22) + conf->fwi_23 * exp((humidity - conf->fwi_16) / conf->fwi_17) + conf->fwi_18 * (conf->fwi_19 - temperature) * (1 -
																					      exp(conf->fwi_20 *
																						  humidity));
		if (m_o < E_w) {
			double k_l = conf->fwi_24 * (1 - pow((100 - humidity) / 100, conf->fwi_25)) + conf->fwi_26 * pow(wind, conf->fwi_27) * (1 - pow((100 - humidity) / 100, conf->fwi_28));
			double k_w = k_l * conf->fwi_29 * exp(conf->fwi_30 * temperature);
			m = E_w - (E_w - m_o) * pow(10, -k_w);
		} else {
			m = m_o;
		}
	} else {
		m = m_o;
	}

	*ffmc = conf->fwi_31 * (conf->fwi_32 - m) / (conf->fwi_33 + m);

	return OPH_FDI_SUCCESS;
}

int oph_fdi_fwi_dmc(int day, int month, int year, double lat, double rain, double temperature, double humidity, double prev_dmc, oph_fdi_configuration * conf, double *dmc)
{
	if (!conf || !dmc) {
		fprintf(stderr, "Unable to compute dmc: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(rain) || isnan(temperature) || isnan(humidity) || isnan(prev_dmc)) {
		*dmc = NAN;
		return OPH_FDI_SUCCESS;
	}

	if (oph_fdi_configuration_fwi_dmc_le_setup(day, month, year, lat, conf))
		return OPH_FDI_ERROR_BAD_PARAM;

	double P_o = prev_dmc;

	if (rain > conf->fwi_34) {
		double r_e = conf->fwi_35 * rain - conf->fwi_36;
		double M_o = conf->fwi_37 + exp(conf->fwi_38 - prev_dmc / conf->fwi_39);
		double b;
		if (prev_dmc <= conf->fwi_40) {
			b = conf->fwi_41 / (conf->fwi_42 + conf->fwi_43 * prev_dmc);
		} else if (conf->fwi_40 < prev_dmc && prev_dmc <= conf->fwi_44) {
			b = conf->fwi_45 - conf->fwi_46 * log(prev_dmc);
		} else {
			b = conf->fwi_47 * log(prev_dmc) - conf->fwi_48;
		}
		double M_r = M_o + (conf->fwi_49 * r_e) / (conf->fwi_50 + b * r_e);
		if (M_r > 300)
			M_r = 300;
		double P_r = conf->fwi_51 - conf->fwi_52 * log(M_r - conf->fwi_53);
		if (P_r < 0)
			P_r = 0;
		P_o = P_r;
	}

	double T = temperature;
	if (temperature < -conf->fwi_55)
		T = -conf->fwi_55;
	double K = conf->fwi_54 * (T + conf->fwi_55) * (conf->fwi_56 - humidity) * conf->fwi_59 * conf->fwi_57;

	*dmc = P_o + conf->fwi_58 * K;

	return OPH_FDI_SUCCESS;
}

int oph_fdi_fwi_dc(int month, double rain, double temperature, double prev_dc, oph_fdi_configuration * conf, double *dc)
{
	if (!conf || !dc) {
		fprintf(stderr, "Unable to compute dc: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(rain) || isnan(temperature) || isnan(prev_dc)) {
		*dc = NAN;
		return OPH_FDI_SUCCESS;
	}

	if (oph_fdi_configuration_fwi_dc_lf_setup(month, conf))
		return OPH_FDI_ERROR_BAD_PARAM;

	double D_o = prev_dc;

	if (rain > conf->fwi_60) {
		double r_d = conf->fwi_61 * rain - conf->fwi_62;
		double Q_o = conf->fwi_63 * exp(-prev_dc / conf->fwi_64);
		double Q_r = Q_o + conf->fwi_65 * r_d;
		double D_r = conf->fwi_66 * log(conf->fwi_67 / Q_r);
		if (D_r < 0)
			D_r = 0;
		D_o = D_r;
	}

	double T = temperature;
	if (temperature < -conf->fwi_69)
		T = -conf->fwi_69;
	double V = conf->fwi_68 * (T + conf->fwi_69) + conf->fwi_71;
	if (V < 0)
		V = 0;

	*dc = D_o + conf->fwi_70 * V;

	return OPH_FDI_SUCCESS;
}

int oph_fdi_fwi_isi_from_ffmc(double ffmc, double wind, oph_fdi_configuration * conf, double *isi)
{
	if (!conf || !isi) {
		fprintf(stderr, "Unable to compute isi: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(ffmc) || isnan(wind)) {
		*isi = NAN;
		return OPH_FDI_SUCCESS;
	}

	double m = (conf->fwi_31 * conf->fwi_32 - ffmc * conf->fwi_33) / (ffmc + conf->fwi_31);
	double f_W = exp(conf->fwi_72 * wind);
	double f_F = conf->fwi_73 * exp(conf->fwi_74 * m) * (1 + pow(m, conf->fwi_75) / (conf->fwi_76));
	*isi = conf->fwi_77 * f_W * f_F;

	return OPH_FDI_SUCCESS;
}

int oph_fdi_fwi_isi(double rain, double temperature, double humidity, double wind, double prev_ffmc, oph_fdi_configuration * conf, double *isi)
{
	if (!conf || !isi) {
		fprintf(stderr, "Unable to compute isi: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(rain) || isnan(temperature) || isnan(humidity) || isnan(wind) || isnan(prev_ffmc)) {
		*isi = NAN;
		return OPH_FDI_SUCCESS;
	}

	double ffmc;
	if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, prev_ffmc, conf, &ffmc))
		return OPH_FDI_ERROR_GENERIC;

	return oph_fdi_fwi_isi_from_ffmc(ffmc, wind, conf, isi);
}

int oph_fdi_fwi_bui_from_dmc_and_dc(double dmc, double dc, oph_fdi_configuration * conf, double *bui)
{
	if (!conf || !bui) {
		fprintf(stderr, "Unable to compute bui: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(dmc) || isnan(dc)) {
		*bui = NAN;
		return OPH_FDI_SUCCESS;
	}

	if (dmc <= conf->fwi_78 * dc) {
		*bui = (conf->fwi_79 * dmc * dc) / (dmc + conf->fwi_78 * dc);
	} else {
		*bui = dmc - (1 - (conf->fwi_79 * dc) / (dmc + conf->fwi_78 * dc)) * (conf->fwi_80 + pow(conf->fwi_81 * dmc, conf->fwi_82));
	}

	return OPH_FDI_SUCCESS;
}

int oph_fdi_fwi_bui(int day, int month, int year, double lat, double rain, double temperature, double humidity, double prev_dmc, double prev_dc, oph_fdi_configuration * conf, double *bui)
{
	if (!conf || !bui) {
		fprintf(stderr, "Unable to compute bui: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(rain) || isnan(temperature) || isnan(humidity) || isnan(prev_dmc) || isnan(prev_dc)) {
		*bui = NAN;
		return OPH_FDI_SUCCESS;
	}

	double dmc, dc;
	if (oph_fdi_fwi_dmc(day, month, year, lat, rain, temperature, humidity, prev_dmc, conf, &dmc))
		return OPH_FDI_ERROR_GENERIC;
	if (oph_fdi_fwi_dc(month, rain, temperature, prev_dc, conf, &dc))
		return OPH_FDI_ERROR_GENERIC;

	return oph_fdi_fwi_bui_from_dmc_and_dc(dmc, dc, conf, bui);
}

int oph_fdi_fwi_from_isi_and_bui(double isi, double bui, oph_fdi_configuration * conf, double *fwi)
{
	if (!conf || !fwi) {
		fprintf(stderr, "Unable to compute fwi: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(isi) || isnan(bui)) {
		*fwi = NAN;
		return OPH_FDI_SUCCESS;
	}

	double f_D;
	if (bui <= conf->fwi_83) {
		f_D = conf->fwi_84 * pow(bui, conf->fwi_85) + conf->fwi_86;
	} else {
		f_D = conf->fwi_87 / (conf->fwi_88 + conf->fwi_89 * exp(conf->fwi_90 * bui));
	}

	double B = conf->fwi_91 * isi * f_D;

	if (B > conf->fwi_92) {
		*fwi = exp(conf->fwi_93 * pow(conf->fwi_94 * log(B), conf->fwi_95));
	} else {
		*fwi = B;
	}

	return OPH_FDI_SUCCESS;
}

int oph_fdi_fwi_main(int day, int month, int year, double lat, double rain, double temperature, double humidity, double wind, double prev_ffmc, double prev_dmc, double prev_dc,
		     oph_fdi_configuration * conf, double *fwi)
{
	if (!conf || !fwi) {
		fprintf(stderr, "Unable to compute fwi: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(rain) || isnan(temperature) || isnan(humidity) || isnan(wind) || isnan(prev_ffmc) || isnan(prev_dmc) || isnan(prev_dc)) {
		*fwi = NAN;
		return OPH_FDI_SUCCESS;
	}

	double isi, bui;
	if (oph_fdi_fwi_isi(rain, temperature, humidity, wind, prev_ffmc, conf, &isi))
		return OPH_FDI_ERROR_GENERIC;
	if (oph_fdi_fwi_bui(day, month, year, lat, rain, temperature, humidity, prev_dmc, prev_dc, conf, &bui))
		return OPH_FDI_ERROR_GENERIC;

	return oph_fdi_fwi_from_isi_and_bui(isi, bui, conf, fwi);
}

int oph_fdi_fwi_dsr_from_fwi(double fwi, oph_fdi_configuration * conf, double *dsr)
{
	if (!conf || !dsr) {
		fprintf(stderr, "Unable to compute dsr: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(fwi)) {
		*dsr = NAN;
		return OPH_FDI_SUCCESS;
	}

	*dsr = conf->fwi_96 * pow(fwi, conf->fwi_97);

	return OPH_FDI_SUCCESS;
}

int oph_fdi_fwi_dsr(int day, int month, int year, double lat, double rain, double temperature, double humidity, double wind, double prev_ffmc, double prev_dmc, double prev_dc,
		    oph_fdi_configuration * conf, double *dsr)
{
	if (!conf || !dsr) {
		fprintf(stderr, "Unable to compute dsr: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(rain) || isnan(temperature) || isnan(humidity) || isnan(wind) || isnan(prev_ffmc) || isnan(prev_dmc) || isnan(prev_dc)) {
		*dsr = NAN;
		return OPH_FDI_SUCCESS;
	}

	double fwi;
	if (oph_fdi_fwi_main(day, month, year, lat, rain, temperature, humidity, wind, prev_ffmc, prev_dmc, prev_dc, conf, &fwi))
		return OPH_FDI_ERROR_GENERIC;

	return oph_fdi_fwi_dsr_from_fwi(fwi, conf, dsr);
}

int oph_fdi_ffwi_rs(double temperature, double humidity, double wind, oph_fdi_configuration * conf, double *rs)
{
	if (!conf || !rs) {
		fprintf(stderr, "Unable to compute rs: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(temperature) || isnan(humidity) || isnan(wind)) {
		*rs = NAN;
		return OPH_FDI_SUCCESS;
	}

	double winds = wind * 0.621371192;

	double n;
	if (oph_fdi_ffwi_rso(temperature, humidity, conf, &n))
		return OPH_FDI_ERROR_GENERIC;

	*rs = n * (1 + conf->ffwi_1 * pow(winds, conf->ffwi_2));

	return OPH_FDI_SUCCESS;
}

int oph_fdi_ffwi_rso(double temperature, double humidity, oph_fdi_configuration * conf, double *rso)
{
	if (!conf || !rso) {
		fprintf(stderr, "Unable to compute rso: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(temperature) || isnan(humidity)) {
		*rso = NAN;
		return OPH_FDI_SUCCESS;
	}

	double m;
	if (oph_fdi_ffwi_emc(temperature, humidity, conf, &m))
		return OPH_FDI_ERROR_GENERIC;

	*rso = 1 - conf->ffwi_3 * (m / conf->ffwi_4) + conf->ffwi_5 * pow(m / conf->ffwi_4, conf->ffwi_6) - conf->ffwi_7 * pow(m / conf->ffwi_4, conf->ffwi_8);

	return OPH_FDI_SUCCESS;
}

int oph_fdi_ffwi_er(double temperature, double humidity, oph_fdi_configuration * conf, double *er)
{
	if (!conf || !er) {
		fprintf(stderr, "Unable to compute er: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(temperature) || isnan(humidity)) {
		*er = NAN;
		return OPH_FDI_SUCCESS;
	}

	return oph_fdi_ffwi_rso(temperature, humidity, conf, er);
}

int oph_fdi_ffwi_emc(double temperature, double humidity, oph_fdi_configuration * conf, double *emc)
{
	if (!conf || !emc) {
		fprintf(stderr, "Unable to compute emc: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(temperature) || isnan(humidity)) {
		*emc = NAN;
		return OPH_FDI_SUCCESS;
	}

	double temp = temperature * 1.8 + 32;

	double m;
	if (humidity < conf->ffwi_9) {
		m = conf->ffwi_10 + conf->ffwi_11 * humidity - conf->ffwi_12 * humidity * temp;
	} else if (conf->ffwi_9 <= humidity && humidity <= conf->ffwi_13) {
		m = conf->ffwi_14 + conf->ffwi_15 * humidity - conf->ffwi_16 * temp;
	} else {
		m = conf->ffwi_17 + conf->ffwi_18 * pow(humidity, conf->ffwi_19) - conf->ffwi_20 * humidity * temp - conf->ffwi_21 * humidity;
	}
	if (m < 0)
		m = 0;

	*emc = m;

	return OPH_FDI_SUCCESS;
}

int oph_fdi_ffwi_main(double temperature, double humidity, double wind, oph_fdi_configuration * conf, double *ffwi)
{
	if (!conf || !ffwi) {
		fprintf(stderr, "Unable to compute ffwi: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(temperature) || isnan(humidity) || isnan(wind)) {
		*ffwi = NAN;
		return OPH_FDI_SUCCESS;
	}

	double winds = wind * 0.621371192;

	double U = winds;
	if (U > conf->ffwi_24)
		U = conf->ffwi_24;

	double n;
	if (oph_fdi_ffwi_rso(temperature, humidity, conf, &n))
		return OPH_FDI_ERROR_GENERIC;

	*ffwi = conf->ffwi_22 * pow(pow(n, 2) * (1 + conf->ffwi_1 * pow(U, conf->ffwi_2)), conf->ffwi_23);

	return OPH_FDI_SUCCESS;
}

int oph_fdi_ifi_dc(double radiation_mean, double temperature_mean, double rain, oph_fdi_configuration * conf, double *dc)
{
	if (!conf || !dc) {
		fprintf(stderr, "Unable to compute dc: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(radiation_mean) || isnan(temperature_mean) || isnan(rain)) {
		*dc = NAN;
		return OPH_FDI_SUCCESS;
	}

	double DC;
	DC = conf->ifi_1 * (exp(conf->ifi_2 * ((radiation_mean * temperature_mean) / (conf->ifi_3 - conf->ifi_4 * temperature_mean))) / (1 + sqrt(rain)));
	if (DC < conf->ifi_5)
		DC = conf->ifi_5;
	if (DC > conf->ifi_6)
		DC = conf->ifi_6;

	*dc = DC;

	return OPH_FDI_SUCCESS;
}

int oph_fdi_ifi_mc(double temperature_max, double wind_mean, double humidity_min, oph_fdi_configuration * conf, double *mc)
{
	if (!conf || !mc) {
		fprintf(stderr, "Unable to compute mc: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(temperature_max) || isnan(wind_mean) || isnan(humidity_min)) {
		*mc = NAN;
		return OPH_FDI_SUCCESS;
	}

	double f_RHn = conf->ifi_7 * exp(conf->ifi_8 * humidity_min);
	double f_Tx = conf->ifi_9 * exp(conf->ifi_10 * temperature_max);

	double f_Tx_w;
	if (temperature_max >= conf->ifi_11) {
		f_Tx_w = 1;
	} else {
		if (temperature_max < conf->ifi_12) {
			f_Tx_w = 0;
		} else {
			f_Tx_w = cos((conf->ifi_11 - temperature_max) / (conf->ifi_11 - conf->ifi_12) * (M_PI / 2));
		}
	}

	double f_Wint;
	if (conf->ifi_13 * exp(conf->ifi_14 * wind_mean) > conf->ifi_15) {
		f_Wint = conf->ifi_15;
	} else {
		f_Wint = conf->ifi_13 * exp(conf->ifi_14 * wind_mean);
	}

	double f_W = f_Tx_w * f_Wint;
	double MC = f_RHn + f_Tx + f_W;
	if (MC < conf->ifi_16)
		MC = conf->ifi_16;
	if (MC > conf->ifi_17)
		MC = conf->ifi_17;

	*mc = MC;

	return OPH_FDI_SUCCESS;
}

int oph_fdi_ifi_r(double radiation_max, oph_fdi_configuration * conf, double *r)
{
	if (!conf || !r) {
		fprintf(stderr, "Unable to compute r: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(radiation_max)) {
		*r = NAN;
		return OPH_FDI_SUCCESS;
	}

	if (radiation_max < conf->ifi_18) {
		*r = conf->ifi_19;
	} else {
		if (radiation_max <= conf->ifi_20) {
			*r = conf->ifi_21;
		} else {
			*r = conf->ifi_22;
		}
	}

	return OPH_FDI_SUCCESS;
}

int oph_fdi_ifi_fc(double temperature_mean, double humidity_mean, oph_fdi_configuration * conf, double *fc)
{
	if (!conf || !fc) {
		fprintf(stderr, "Unable to compute fc: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(temperature_mean) || isnan(humidity_mean)) {
		*fc = NAN;
		return OPH_FDI_SUCCESS;
	}

	double a = conf->ifi_25 - conf->ifi_26 * (temperature_mean - humidity_mean);
	if (a < 0)
		a = 0;
	if (a > conf->ifi_23)
		a = conf->ifi_23;

	*fc = (conf->ifi_23 - a) / (conf->ifi_24);

	return OPH_FDI_SUCCESS;
}

int oph_fdi_ifi_from_dc_mc_r_fc(double dc, double mc, double r, double fc, double *ifi)
{
	if (!ifi) {
		fprintf(stderr, "Unable to compute ifi: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(dc) || isnan(mc) || isnan(r) || isnan(fc)) {
		*ifi = NAN;
		return OPH_FDI_SUCCESS;
	}

	*ifi = dc + mc + r + fc;

	return OPH_FDI_SUCCESS;
}

int oph_fdi_ifi_main(double radiation_mean, double radiation_max, double temperature_mean, double temperature_max, double rain, double wind_mean, double humidity_mean, double humidity_min,
		     oph_fdi_configuration * conf, double *ifi)
{
	if (!conf || !ifi) {
		fprintf(stderr, "Unable to compute ifi: null input value\n");
		return OPH_FDI_ERROR_BAD_PARAM;
	}

	if (isnan(radiation_mean) || isnan(radiation_max) || isnan(temperature_mean) || isnan(temperature_max) || isnan(rain) || isnan(wind_mean) || isnan(humidity_mean) || isnan(humidity_min)) {
		*ifi = NAN;
		return OPH_FDI_SUCCESS;
	}

	double dc;
	if (oph_fdi_ifi_dc(radiation_mean, temperature_mean, rain, conf, &dc))
		return OPH_FDI_ERROR_GENERIC;

	double mc;
	if (oph_fdi_ifi_mc(temperature_max, wind_mean, humidity_min, conf, &mc))
		return OPH_FDI_ERROR_GENERIC;

	double r;
	if (oph_fdi_ifi_r(radiation_max, conf, &r))
		return OPH_FDI_ERROR_GENERIC;

	double fc;
	if (oph_fdi_ifi_fc(temperature_mean, humidity_mean, conf, &fc))
		return OPH_FDI_ERROR_GENERIC;

	return oph_fdi_ifi_from_dc_mc_r_fc(dc, mc, r, fc, ifi);
}
