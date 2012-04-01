/*
 * UFRaw - Unidentified Flying Raw converter for digital camera images
 *
 * wb_presets.c - White balance preset values for various cameras
 * Copyright 2004-2012 by Udi Fuchs
 *
 * Thanks goes for all the people who sent in the preset values
 * for their cameras.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <glib/gi18n.h>

typedef struct
{
  const char *make;
  const char *model;
  const char *name;
  int tuning;
  double channel[4];
}
wb_data;

/* Column 1 - "make" of the camera.
 * Column 2 - "model" (use the "make" and "model" as provided by DCRaw).
 * Column 3 - WB name.
 * Column 4 - Fine tuning. MUST be in increasing order. 0 for no fine tuning.
 *	      It is enough to give only the extreme values, the other values
 *	      will be interpolated.
 * Column 5 - Channel multipliers.
 *
 * Minolta's ALPHA and MAXXUM models are treated as the DYNAX model.
 *
 * WB name is standardized to one of the following: */
// "Sunlight" and other variation should be switched to this:
static const char Daylight[] = N_("Daylight");
// Probably same as above:
static const char DirectSunlight[] = N_("Direct sunlight");
static const char Cloudy[] = N_("Cloudy");
// "Shadows" should be switched to this:
static const char Shade[] = N_("Shade");
static const char Incandescent[] = N_("Incandescent");
static const char IncandescentWarm[] = N_("Incandescent warm");
// Same as "Incandescent":
static const char Tungsten[] = N_("Tungsten");
static const char Fluorescent[] = N_("Fluorescent");
// In Canon cameras and some newer Nikon cameras:
static const char FluorescentHigh[] = N_("Fluorescent high");
static const char CoolWhiteFluorescent[] = N_("Cool white fluorescent");
static const char WarmWhiteFluorescent[] = N_("Warm white fluorescent");
static const char DaylightFluorescent[] = N_("Daylight fluorescent");
static const char NeutralFluorescent[] = N_("Neutral fluorescent");
static const char WhiteFluorescent[] = N_("White fluorescent");
// In some newer Nikon cameras:
static const char SodiumVaporFluorescent[] = N_("Sodium-vapor fluorescent");
static const char DayWhiteFluorescent[] = N_("Day white fluorescent");
static const char HighTempMercuryVaporFluorescent[] = N_("High temp. mercury-vapor fluorescent");

static const char Flash[] = N_("Flash");
// For Olympus with no real "Flash" preset:
static const char FlashAuto[] = N_("Flash (auto mode)");
static const char EveningSun[] = N_("Evening sun");
static const char Underwater[] = N_("Underwater");
static const char BlackNWhite[] = N_("Black & white");

const char uf_spot_wb[] = "Spot WB";
const char uf_manual_wb[] = N_("Manual WB");
const char uf_camera_wb[] = N_("Camera WB");
const char uf_auto_wb[] = N_("Auto WB");

const wb_data wb_preset[] = {


  { "Canon", "PowerShot A630", Daylight, 0,	{ 1.831422, 1, 1.245671, 0 } },
  { "Canon", "PowerShot A630", Cloudy, 0,	{ 1.669924, 1, 1.326299, 0 } },
  { "Canon", "PowerShot A630", Tungsten, 0,	{ 1.696768, 1, 1.268658, 0 } },
  { "Canon", "PowerShot A630", Fluorescent, 0,	{ 1.869859, 1, 1.209110, 0 } },
  { "Canon", "PowerShot A630", FluorescentHigh, 0, { 1.855491, 1, 1.206855, 0 } },

  { "Canon", "PowerShot A710 IS", Daylight, 0,	{ 1.683007, 1, 1.893246, 0 } },
  { "Canon", "PowerShot A710 IS", Cloudy, 0,	{ 1.871320, 1, 1.718648, 0 } },
  { "Canon", "PowerShot A710 IS", Tungsten, 0,	{ 1.268692, 1, 2.707944, 0 } },
  { "Canon", "PowerShot A710 IS", Fluorescent, 0, { 1.589857, 1, 2.051819, 0 } },
  { "Canon", "PowerShot A710 IS", DaylightFluorescent, 0, { 1.820287, 1, 1.820287, 0 } },
  { "Canon", "PowerShot A710 IS", Underwater, 0, { 2.926108, 1, 1.376847, 0 } },

  /* These presets were extracted from CHDK-generated DNG files. */
  { "Canon", "PowerShot A720 IS", Daylight, 0,	{ 2.059371, 1, 1.975553, 0 } },
  { "Canon", "PowerShot A720 IS", Cloudy, 0,	{ 2.262722, 1, 1.818935, 0 } },
  { "Canon", "PowerShot A720 IS", Tungsten, 0,	{ 1.632258, 1, 2.974194, 0 } },
  { "Canon", "PowerShot A720 IS", Fluorescent, 0, { 1.902728, 1, 2.365362, 0 } },
  { "Canon", "PowerShot A720 IS", FluorescentHigh, 0, { 2.243961, 1, 1.935990, 0 } },
  { "Canon", "PowerShot A720 IS", Underwater, 0, { 2.303465, 1, 1.882915, 0 } },
  { "Canon", "PowerShot A720 IS", Flash, 0,	{ 2.151586, 1, 1.864865, 0 } },

  { "Canon", "PowerShot G2", Daylight, 0,	{ 2.011483, 1, 1.299522, 0 } },
  { "Canon", "PowerShot G2", Cloudy, 0,		{ 2.032505, 1, 1.285851, 0 } },
  { "Canon", "PowerShot G2", Tungsten, 0,	{ 1.976008, 1, 1.332054, 0 } },
  { "Canon", "PowerShot G2", Fluorescent, 0,	{ 2.022010, 1, 1.295694, 0 } },
  { "Canon", "PowerShot G2", FluorescentHigh, 0, { 2.029637, 1, 1.286807, 0 } },
  { "Canon", "PowerShot G2", Flash, 0,		{ 2.153576, 1, 1.140680, 0 } },

  { "Canon", "PowerShot G3", Daylight, 0,	{ 1.858513, 1, 1.387290, 0 } },
  { "Canon", "PowerShot G3", Cloudy, 0,		{ 1.951132, 1, 1.305125, 0 } },
  { "Canon", "PowerShot G3", Tungsten, 0,	{ 1.128386, 1, 2.313310, 0 } },
  { "Canon", "PowerShot G3", Fluorescent, 0,	{ 1.715573, 1, 2.194337, 0 } },
  { "Canon", "PowerShot G3", FluorescentHigh, 0, { 2.580563, 1, 1.496164, 0 } },
  { "Canon", "PowerShot G3", Flash, 0,		{ 2.293173, 1, 1.187416, 0 } },

  { "Canon", "PowerShot G5", Daylight, 0,	{ 1.639521, 1, 1.528144, 0 } },
  { "Canon", "PowerShot G5", Cloudy, 0,		{ 1.702153, 1, 1.462919, 0 } },
  { "Canon", "PowerShot G5", Tungsten, 0,	{ 1.135071, 1, 2.374408, 0 } },
  { "Canon", "PowerShot G5", Fluorescent, 0,	{ 1.660281, 1, 2.186462, 0 } },
  { "Canon", "PowerShot G5", FluorescentHigh, 0, { 1.463297, 1, 1.764140, 0 } },
  { "Canon", "PowerShot G5", Flash, 0,		{ 1.603593, 1, 1.562874, 0 } },

  { "Canon", "PowerShot G6", Daylight, 0,	{ 1.769704, 1, 1.637931, 0 } },
  { "Canon", "PowerShot G6", Cloudy, 0,		{ 2.062731, 1, 1.442804, 0 } },
  { "Canon", "PowerShot G6", Tungsten, 0,	{ 1.077106, 1, 2.721234, 0 } },
  { "Canon", "PowerShot G6", Fluorescent, 0,	{ 1.914922, 1, 2.142670, 0 } },
  { "Canon", "PowerShot G6", FluorescentHigh, 0, { 2.543677, 1, 1.650587, 0 } },
  { "Canon", "PowerShot G6", Flash, 0,		{ 2.285322, 1, 1.333333, 0 } },

  { "Canon", "PowerShot G9", Daylight, 0,	{ 2.089552, 1, 1.786452, 0 } },
  { "Canon", "PowerShot G9", Cloudy, 0,		{ 2.208716, 1, 1.660550, 0 } },
  { "Canon", "PowerShot G9", Tungsten, 0,	{ 1.533493, 1, 2.586124, 0 } },
  { "Canon", "PowerShot G9", Fluorescent, 0,	{ 2.065668, 1, 1.829493, 0 } },
  { "Canon", "PowerShot G9", FluorescentHigh, 0, { 2.237601, 1, 1.668974, 0 } },
  { "Canon", "PowerShot G9", Flash, 0,		{ 2.461538, 1, 1.498834, 0 } },
  { "Canon", "PowerShot G9", Underwater, 0,	{ 2.237327, 1, 1.661290, 0 } },

  /* Does the Canon PowerShot G10 support native WB presets? Please test. */
  { "Canon", "PowerShot G10", Daylight, 0,	{ 1.598980, 1, 1.830612, 0 } },
  { "Canon", "PowerShot G10", Cloudy, 0,	{ 1.738120, 1, 1.722281, 0 } },
  { "Canon", "PowerShot G10", Tungsten, 0,	{ 1, 1.035550, 3.569954, 0 } },
  { "Canon", "PowerShot G10", Fluorescent, 0,	{ 1.341633, 1, 2.434263, 0 } },
  { "Canon", "PowerShot G10", FluorescentHigh, 0, { 1.749171, 1, 1.907182, 0 } },
  { "Canon", "PowerShot G10", Flash, 0,		{ 1.926829, 1, 1.501591, 0 } },
  { "Canon", "PowerShot G10", Underwater, 0,	{ 1.822314, 1, 1.841942, 0 } },

  { "Canon", "PowerShot G11", Daylight, 0,	{ 1.721591, 1, 2.097727, 0 } },
  { "Canon", "PowerShot G11", Cloudy, 0,	{ 1.910936, 1, 1.856821, 0 } },
  { "Canon", "PowerShot G11", Tungsten, 0,	{ 1.380435, 1, 3.576087, 0 } },
  { "Canon", "PowerShot G11", Fluorescent, 0,	{ 1.649143, 1, 2.693714, 0 } },
  { "Canon", "PowerShot G11", FluorescentHigh, 0, { 2.008168, 1, 1.961494, 0 } },
  { "Canon", "PowerShot G11", Flash, 0,		{ 1.985556, 1, 1.703333, 0 } },
  { "Canon", "PowerShot G11", Underwater, 0,	{ 2.225624, 1, 1.577098, 0 } },

  { "Canon", "PowerShot G12", Daylight, 0,	{ 1.785877, 1, 2.042141, 0 } },
  { "Canon", "PowerShot G12", Cloudy, 0,	{ 1.804323, 1, 2.021615, 0 } },
  { "Canon", "PowerShot G12", Tungsten, 0,	{ 1.310127, 1, 3.170886, 0 } },
  { "Canon", "PowerShot G12", Fluorescent, 0,	{ 1.771139, 1, 2.064262, 0 } },
  { "Canon", "PowerShot G12", FluorescentHigh, 0, { 1.806122, 1, 2.032880, 0 } },
  { "Canon", "PowerShot G12", Flash, 0,		{ 2.102157, 1, 1.706016, 0 } },
  { "Canon", "PowerShot G12", Underwater, 0,	{ 1.807650, 1, 2.112568, 0 } },

  /* Canon PowerShot S3 IS does not support native WB presets. These are made
     as custom WB presets. */
  { "Canon", "PowerShot S3 IS", Daylight, 0,	{ 1.627271, 1, 1.823491, 0 } },
  { "Canon", "PowerShot S3 IS", Cloudy, 0,	{ 1.794382, 1, 1.618412, 0 } },
  { "Canon", "PowerShot S3 IS", Tungsten, 0,	{ 1, 1.192243, 4.546950, 0 } },
  { "Canon", "PowerShot S3 IS", Flash, 0,	{ 1.884691, 1, 1.553869, 0 } },

  { "Canon", "PowerShot S30", Daylight, 0,	{ 1.741088, 1, 1.318949, 0 } },
  { "Canon", "PowerShot S30", Cloudy, 0,	{ 1.766635, 1, 1.298969, 0 } },
  { "Canon", "PowerShot S30", Tungsten, 0,	{ 1.498106, 1, 1.576705, 0 } },
  { "Canon", "PowerShot S30", Fluorescent, 0,	{ 1.660075, 1, 1.394539, 0 } },
  { "Canon", "PowerShot S30", FluorescentHigh, 0, { 1.753515, 1, 1.306467, 0 } },
  { "Canon", "PowerShot S30", Flash, 0,		{ 2.141705, 1, 1.097926, 0 } },

  { "Canon", "PowerShot S45", Daylight, 0,	{ 2.325175, 1, 1.080420, 0 } },
  { "Canon", "PowerShot S45", Cloudy, 0,	{ 2.145047, 1, 1.173349, 0 } },
  { "Canon", "PowerShot S45", Tungsten, 0,	{ 1.213018, 1, 2.087574, 0 } },
  { "Canon", "PowerShot S45", Fluorescent, 0,	{ 1.888183, 1, 1.822109, 0 } },
  { "Canon", "PowerShot S45", FluorescentHigh, 0, { 2.964422, 1, 1.354511, 0 } },
  { "Canon", "PowerShot S45", Flash, 0,		{ 2.534884, 1, 1.065663, 0 } },

  { "Canon", "PowerShot S50", Daylight, 0,	{ 1.772506, 1, 1.536496, 0 } },
  { "Canon", "PowerShot S50", Cloudy, 0,	{ 1.831311, 1, 1.484223, 0 } },
  { "Canon", "PowerShot S50", Tungsten, 0,	{ 1.185542, 1, 2.480723, 0 } },
  { "Canon", "PowerShot S50", Fluorescent, 0,	{ 1.706410, 1, 2.160256, 0 } },
  { "Canon", "PowerShot S50", FluorescentHigh, 0, { 1.562500, 1, 1.817402, 0 } },
  { "Canon", "PowerShot S50", Flash, 0,		{ 1.776156, 1, 1.531630, 0 } },

  { "Canon", "PowerShot S60", Daylight, 0,	{ 1.759169, 1, 1.590465, 0 } },
  { "Canon", "PowerShot S60", Cloudy, 0,	{ 1.903659, 1, 1.467073, 0 } },
  { "Canon", "PowerShot S60", Tungsten, 0,	{ 1.138554, 1, 2.704819, 0 } },
  { "Canon", "PowerShot S60", Fluorescent, 0,	{ 1.720721, 1, 2.185328, 0 } },
  { "Canon", "PowerShot S60", FluorescentHigh, 0, { 2.877095, 1, 2.216480, 0 } },
  { "Canon", "PowerShot S60", Flash, 0,		{ 2.182540, 1, 1.236773, 0 } },
  { "Canon", "PowerShot S60", Underwater, 0,	{ 2.725369, 1, 1.240148, 0 } },

  { "Canon", "PowerShot S70", Daylight, 0,	{ 1.943834, 1, 1.456654, 0 } },
  { "Canon", "PowerShot S70", Cloudy, 0,	{ 2.049939, 1, 1.382460, 0 } },
  { "Canon", "PowerShot S70", Tungsten, 0,	{ 1.169492, 1, 2.654964, 0 } },
  { "Canon", "PowerShot S70", Fluorescent, 0,	{ 1.993456, 1, 2.056283, 0 } },
  { "Canon", "PowerShot S70", FluorescentHigh, 0, { 2.645914, 1, 1.565499, 0 } },
  { "Canon", "PowerShot S70", Flash, 0,		{ 2.389189, 1, 1.147297, 0 } },
  { "Canon", "PowerShot S70", Underwater, 0,	{ 3.110565, 1, 1.162162, 0 } },

  { "Canon", "PowerShot S90", Daylight, 0,	{ 1.955056, 1, 1.797753, 0 } },
  { "Canon", "PowerShot S90", Cloudy, 0,	{ 1.945067, 1, 1.795964, 0 } },
  { "Canon", "PowerShot S90", Tungsten, 0,	{ 2.000000, 1, 1.828018, 0 } },
  { "Canon", "PowerShot S90", Fluorescent, 0,	{ 2.019473, 1, 1.841924, 0 } },
  { "Canon", "PowerShot S90", FluorescentHigh, 0, { 2.009143, 1, 1.840000, 0 } },
  { "Canon", "PowerShot S90", Flash, 0,		{ 2.045784, 1, 1.671692, 0 } },
  { "Canon", "PowerShot S90", Underwater, 0,	{ 2.022297, 1, 1.830546, 0 } },

  { "Canon", "PowerShot S95", Daylight, 0,	{ 1.6182, 1, 2.1430, 0 } },
  { "Canon", "PowerShot S95", Cloudy, 0,	{ 1.6920, 1, 2.0179, 0 } },
  { "Canon", "PowerShot S95", Tungsten, 0,	{ 1.2314, 1, 3.1015, 0 } },
  { "Canon", "PowerShot S95", Fluorescent, 0,	{ 1.6593, 1, 2.0940, 0 } },
  { "Canon", "PowerShot S95", FluorescentHigh, 0, { 1.7272, 1, 1.9811, 0 } },
  { "Canon", "PowerShot S95", Flash, 0,		{ 1.9955, 1, 1.7768, 0 } },
  { "Canon", "PowerShot S95", Underwater, 0,	{ 1.7607, 1, 2.1224, 0 } },

  { "Canon", "PowerShot S100", Daylight, 0,	{ 2.077707, 1, 2.551592, 0 } },
  { "Canon", "PowerShot S100", Cloudy, 0,	{ 2.276402, 1, 2.393742, 0 } },
  { "Canon", "PowerShot S100", Tungsten, 0,	{ 1.267936, 1, 4.224012, 0 } },
  { "Canon", "PowerShot S100", Fluorescent, 0,	{ 1.815115, 1, 3.093117, 0 } },
  { "Canon", "PowerShot S100", FluorescentHigh, 0, { 2.398148, 1, 2.374339, 0 } },
  { "Canon", "PowerShot S100", Flash, 0,	{ 2.615783, 1, 2.001294, 0 } },
  { "Canon", "PowerShot S100", Underwater, 0,	{ 2.248391, 1, 2.338481, 0 } },

  { "Canon", "PowerShot Pro1", Daylight, 0,	{ 1.829238, 1, 1.571253, 0 } },
  { "Canon", "PowerShot Pro1", Cloudy, 0,	{ 1.194139, 1, 2.755800, 0 } },
  { "Canon", "PowerShot Pro1", Tungsten, 0,	{ 1.701416, 1, 2.218790, 0 } },
  { "Canon", "PowerShot Pro1", Fluorescent, 0,	{ 2.014066, 1, 1.776215, 0 } },
  { "Canon", "PowerShot Pro1", FluorescentHigh, 0, { 2.248663, 1, 1.227273, 0 } },
  { "Canon", "PowerShot Pro1", Flash, 0,	{ 2.130081, 1, 1.422764, 0 } },

  { "Canon", "PowerShot SX1 IS", Daylight, 0,	{ 1.574586, 1, 2.114917, 0 } },
  { "Canon", "PowerShot SX1 IS", Cloudy, 0,	{ 1.682628, 1, 2.015590, 0 } },
  { "Canon", "PowerShot SX1 IS", Tungsten, 0,	{ 1.088836, 1, 3.056423, 0 } },
  { "Canon", "PowerShot SX1 IS", Fluorescent, 0, { 1.398259, 1, 2.414581, 0 } },
  { "Canon", "PowerShot SX1 IS", FluorescentHigh, 0, { 1.687500, 1, 2.025670, 0 } },
  { "Canon", "PowerShot SX1 IS", Flash, 0,	{ 1.909699, 1, 1.795987, 0 } },

  { "Canon", "EOS D60", Daylight, 0,		{ 2.472594, 1, 1.225335, 0 } },
  { "Canon", "EOS D60", Cloudy, 0,		{ 2.723926, 1, 1.137423, 0 } },
  { "Canon", "EOS D60", Tungsten, 0,		{ 1.543054, 1, 1.907003, 0 } },
  { "Canon", "EOS D60", Fluorescent, 0,		{ 1.957346, 1, 1.662322, 0 } },
  { "Canon", "EOS D60", Flash, 0,		{ 2.829840, 1, 1.108508, 0 } },

  { "Canon", "EOS 5D", Flash, 0,		{ 2.211914, 1, 1.260742, 0 } }, /*6550K*/
  { "Canon", "EOS 5D", Fluorescent, 0,		{ 1.726054, 1, 2.088123, 0 } }, /*3850K*/
  { "Canon", "EOS 5D", Tungsten, 0,		{ 1.373285, 1, 2.301006, 0 } }, /*3250K*/
  { "Canon", "EOS 5D", Cloudy, 0,		{ 2.151367, 1, 1.321289, 0 } }, /*6100K*/
  { "Canon", "EOS 5D", Shade, 0,		{ 2.300781, 1, 1.208008, 0 } }, /*7200K*/
  { "Canon", "EOS 5D", Daylight, 0,		{ 1.988281, 1, 1.457031, 0 } }, /*5250K*/

  /* Canon EOS 5D Mark II firmware 2.0.7 */
  { "Canon", "EOS 5D Mark II", Daylight, -9,	{ 1.954102, 1, 1.984375, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, -8,	{ 1.976563, 1, 1.954102, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, -7,	{ 2.003906, 1, 1.917969, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, -6,	{ 2.032227, 1, 1.885742, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, -5,	{ 2.060547, 1, 1.851563, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, -4,	{ 2.093750, 1, 1.818359, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, -3,	{ 2.120117, 1, 1.787109, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, -2,	{ 2.151367, 1, 1.756836, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, -1,	{ 2.183594, 1, 1.723633, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, 0,	{ 2.216797, 1, 1.689453, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, 1,	{ 2.250977, 1, 1.659180, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, 2,	{ 2.291016, 1, 1.627930, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, 3,	{ 2.327148, 1, 1.594727, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, 4,	{ 2.359375, 1, 1.563477, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, 5,	{ 2.392578, 1, 1.528320, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, 6,	{ 2.420898, 1, 1.503906, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, 7,	{ 2.450195, 1, 1.477539, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, 8,	{ 2.485352, 1, 1.450195, 0 } },
  { "Canon", "EOS 5D Mark II", Daylight, 9,	{ 2.528320, 1, 1.421875, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, -9,	{ 2.250977, 1, 1.662109, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, -8,	{ 2.286133, 1, 1.630859, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, -7,	{ 2.322266, 1, 1.599609, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, -6,	{ 2.354492, 1, 1.565430, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, -5,	{ 2.386719, 1, 1.533203, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, -4,	{ 2.415039, 1, 1.505859, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, -3,	{ 2.450195, 1, 1.479492, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, -2,	{ 2.485352, 1, 1.452148, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, -1,	{ 2.522461, 1, 1.425781, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, 0,	{ 2.559570, 1, 1.397461, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, 1,	{ 2.585938, 1, 1.383789, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, 2,	{ 2.605469, 1, 1.371094, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, 3,	{ 2.632813, 1, 1.354492, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, 4,	{ 2.660156, 1, 1.338867, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, 5,	{ 2.687500, 1, 1.316406, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, 6,	{ 2.723633, 1, 1.290039, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, 7,	{ 2.767578, 1, 1.260742, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, 8,	{ 2.821289, 1, 1.232422, 0 } },
  { "Canon", "EOS 5D Mark II", Shade, 9,	{ 2.868164, 1, 1.203125, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, -9,	{ 2.098633, 1, 1.812500, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, -8,	{ 2.124023, 1, 1.784180, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, -7,	{ 2.156250, 1, 1.750000, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, -6,	{ 2.188477, 1, 1.717773, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, -5,	{ 2.221680, 1, 1.686523, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, -4,	{ 2.255859, 1, 1.657227, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, -3,	{ 2.295898, 1, 1.623047, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, -2,	{ 2.333008, 1, 1.592773, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, -1,	{ 2.359375, 1, 1.558594, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, 0,	{ 2.392578, 1, 1.523438, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, 1,	{ 2.426758, 1, 1.499023, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, 2,	{ 2.456055, 1, 1.473633, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, 3,	{ 2.491211, 1, 1.446289, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, 4,	{ 2.528320, 1, 1.419922, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, 5,	{ 2.566406, 1, 1.395508, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, 6,	{ 2.585938, 1, 1.379883, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, 7,	{ 2.612305, 1, 1.367188, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, 8,	{ 2.639648, 1, 1.350586, 0 } },
  { "Canon", "EOS 5D Mark II", Cloudy, 9,	{ 2.660156, 1, 1.334961, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, -9,	{ 1.383128, 1, 3.084359, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, -8,	{ 1.399119, 1, 3.026432, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, -7,	{ 1.419098, 1, 2.984085, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, -6,	{ 1.439219, 1, 2.935226, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, -5,	{ 1.460374, 1, 2.886910, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, -4,	{ 1.484361, 1, 2.840036, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, -3,	{ 1.503139, 1, 2.787444, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, -2,	{ 1.527027, 1, 2.740541, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, -1,	{ 1.550226, 1, 2.695023, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, 0,	{ 1.573115, 1, 2.646685, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, 1,	{ 1.594891, 1, 2.596715, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, 2,	{ 1.615949, 1, 2.549038, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, 3,	{ 1.638710, 1, 2.494009, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, 4,	{ 1.661111, 1, 2.443519, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, 5,	{ 1.689013, 1, 2.391993, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, 6,	{ 1.712816, 1, 2.349860, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, 7,	{ 1.736595, 1, 2.305738, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, 8,	{ 1.760417, 1, 2.266098, 0 } },
  { "Canon", "EOS 5D Mark II", Tungsten, 9,	{ 1.787417, 1, 2.222116, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, -9,	{ 1.669691, 1, 2.900181, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, -8,	{ 1.695533, 1, 2.849590, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, -7,	{ 1.717033, 1, 2.806777, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, -6,	{ 1.740809, 1, 2.761029, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, -5,	{ 1.765683, 1, 2.716790, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, -4,	{ 1.794063, 1, 2.667904, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, -3,	{ 1.820298, 1, 2.619181, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, -2,	{ 1.841760, 1, 2.567416, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, -1,	{ 1.870056, 1, 2.516949, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, 0,	{ 1.895833, 1, 2.461174, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, 1,	{ 1.928503, 1, 2.416587, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, 2,	{ 1.956772, 1, 2.365994, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, 3,	{ 1.978744, 1, 2.320773, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, 4,	{ 2.009718, 1, 2.281827, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, 5,	{ 2.036133, 1, 2.240234, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, 6,	{ 2.068359, 1, 2.197266, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, 7,	{ 2.098633, 1, 2.160156, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, 8,	{ 2.128906, 1, 2.120117, 0 } },
  { "Canon", "EOS 5D Mark II", Fluorescent, 9,	{ 2.156250, 1, 2.081055, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, -9,	{ 2.111328, 1, 1.792969, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, -8,	{ 2.142578, 1, 1.762695, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, -7,	{ 2.173828, 1, 1.732422, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, -6,	{ 2.207031, 1, 1.698242, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, -5,	{ 2.240234, 1, 1.667969, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, -4,	{ 2.280273, 1, 1.635742, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, -3,	{ 2.316406, 1, 1.602539, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, -2,	{ 2.348633, 1, 1.570313, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, -1,	{ 2.381836, 1, 1.537109, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, 0,	{ 2.409180, 1, 1.507813, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, 1,	{ 2.444336, 1, 1.484375, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, 2,	{ 2.479492, 1, 1.457031, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, 3,	{ 2.515625, 1, 1.429688, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, 4,	{ 2.553711, 1, 1.402344, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, 5,	{ 2.579102, 1, 1.385742, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, 6,	{ 2.598633, 1, 1.373047, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, 7,	{ 2.625977, 1, 1.356445, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, 8,	{ 2.653320, 1, 1.339844, 0 } },
  { "Canon", "EOS 5D Mark II", Flash, 9,	{ 2.680664, 1, 1.321289, 0 } },

  /* Fine-tuning for the 7D are the camera's Amber-Blue bracketing. */
  { "Canon", "EOS 7D", Daylight, -3,		{ 2.036, 1, 1.595, 0 } },
  { "Canon", "EOS 7D", Daylight, 0,		{ 2.120, 1, 1.506, 0 } },
  { "Canon", "EOS 7D", Daylight, 3,		{ 2.217, 1, 1.437, 0 } },
  { "Canon", "EOS 7D", Shade, -3,		{ 2.349, 1, 1.348, 0 } },
  { "Canon", "EOS 7D", Shade, 0,		{ 2.468, 1, 1.276, 0 } },
  { "Canon", "EOS 7D", Shade, 3,		{ 2.573, 1, 1.228, 0 } },
  { "Canon", "EOS 7D", Cloudy, -3,		{ 2.188, 1, 1.457, 0 } },
  { "Canon", "EOS 7D", Cloudy, 0,		{ 2.286, 1, 1.384, 0 } },
  { "Canon", "EOS 7D", Cloudy, 3,		{ 2.393, 1, 1.319, 0 } },
  { "Canon", "EOS 7D", Tungsten, -3,		{ 1.426, 1, 2.398, 0 } },
  { "Canon", "EOS 7D", Tungsten, 0,		{ 1.490, 1, 2.261, 0 } },
  { "Canon", "EOS 7D", Tungsten, 3,		{ 1.557, 1, 2.156, 0 } },
  { "Canon", "EOS 7D", WhiteFluorescent, -3,	{ 1.771, 1, 2.235, 0 } },
  { "Canon", "EOS 7D", WhiteFluorescent, 0,	{ 1.858, 1, 2.124, 0 } },
  { "Canon", "EOS 7D", WhiteFluorescent, 3,	{ 1.936, 1, 2.023, 0 } },
  { "Canon", "EOS 7D", Flash, -3,		{ 2.240, 1, 1.448, 0 } },
  { "Canon", "EOS 7D", Flash, 0,		{ 2.338, 1, 1.376, 0 } },
  { "Canon", "EOS 7D", Flash, 3,		{ 2.462, 1, 1.312, 0 } },

  { "Canon", "EOS 10D", Daylight, 0,		{ 2.159856, 1, 1.218750, 0 } },
  { "Canon", "EOS 10D", Shade, 0,		{ 2.533654, 1, 1.036058, 0 } },
  { "Canon", "EOS 10D", Cloudy, 0,		{ 2.348558, 1, 1.116587, 0 } },
  { "Canon", "EOS 10D", Tungsten, 0,		{ 1.431544, 1, 1.851040, 0 } },
  { "Canon", "EOS 10D", Fluorescent, 0,		{ 1.891509, 1, 1.647406, 0 } },
  { "Canon", "EOS 10D", Flash, 0,		{ 2.385817, 1, 1.115385, 0 } },

  { "Canon", "EOS 20D", Daylight, 0,		{ 1.954680, 1, 1.478818, 0 } },
  { "Canon", "EOS 20D", Shade, 0,		{ 2.248276, 1, 1.227586, 0 } },
  { "Canon", "EOS 20D", Cloudy, 0,		{ 2.115271, 1, 1.336946, 0 } },
  { "Canon", "EOS 20D", Tungsten, 0,		{ 1.368087, 1, 2.417044, 0 } },
  { "Canon", "EOS 20D", Fluorescent, 0,		{ 1.752709, 1, 2.060098, 0 } },
  { "Canon", "EOS 20D", Flash, 0,		{ 2.145813, 1, 1.293596, 0 } },

  { "Canon", "EOS 30D", Daylight, 0,		{ 2.032227, 1, 1.537109, 0 } },
  { "Canon", "EOS 30D", Shade, 0,		{ 2.354492, 1, 1.264648, 0 } },
  { "Canon", "EOS 30D", Cloudy, 0,		{ 2.197266, 1, 1.389648, 0 } },
  { "Canon", "EOS 30D", Tungsten, 0,		{ 1.411084, 1, 2.447477, 0 } },
  { "Canon", "EOS 30D", Fluorescent, 0,		{ 1.761601, 1, 2.303913, 0 } },
  { "Canon", "EOS 30D", Flash, 0,		{ 2.226562, 1, 1.347656, 0 } },

  { "Canon", "EOS 40D", Daylight, 0,		{ 2.197266, 1, 1.438477, 0 } },
  { "Canon", "EOS 40D", Shade, 0,		{ 2.546875, 1, 1.185547, 0 } },
  { "Canon", "EOS 40D", Cloudy, 0,		{ 2.370117, 1, 1.290039, 0 } },
  { "Canon", "EOS 40D", Tungsten, 0,		{ 1.510563, 1, 2.235915, 0 } },
  { "Canon", "EOS 40D", Fluorescent, 0,		{ 2.019084, 1, 2.129771, 0 } },
  { "Canon", "EOS 40D", Flash, 0,		{ 2.409180, 1, 1.260742, 0 } },

  // Canon EOS 50D (firmware 1.0.3)
  { "Canon", "EOS 50D", Daylight, -9,		{ 1.865234, 1, 1.599609, 0 } },
  { "Canon", "EOS 50D", Daylight, -8,		{ 1.889648, 1, 1.580078, 0 } },
  { "Canon", "EOS 50D", Daylight, -7,		{ 1.910156, 1, 1.556641, 0 } },
  { "Canon", "EOS 50D", Daylight, -6,		{ 1.935547, 1, 1.535156, 0 } },
  { "Canon", "EOS 50D", Daylight, -5,		{ 1.965820, 1, 1.512695, 0 } },
  { "Canon", "EOS 50D", Daylight, -4,		{ 1.992188, 1, 1.490234, 0 } },
  { "Canon", "EOS 50D", Daylight, -3,		{ 2.015625, 1, 1.468750, 0 } },
  { "Canon", "EOS 50D", Daylight, -2,		{ 2.043945, 1, 1.448242, 0 } },
  { "Canon", "EOS 50D", Daylight, -1,		{ 2.068359, 1, 1.425781, 0 } },
  { "Canon", "EOS 50D", Daylight, 0,		{ 2.098633, 1, 1.402344, 0 } },
  { "Canon", "EOS 50D", Daylight, 1,		{ 2.124023, 1, 1.381836, 0 } },
  { "Canon", "EOS 50D", Daylight, 2,		{ 2.156250, 1, 1.358398, 0 } },
  { "Canon", "EOS 50D", Daylight, 3,		{ 2.183594, 1, 1.334961, 0 } },
  { "Canon", "EOS 50D", Daylight, 4,		{ 2.211914, 1, 1.312500, 0 } },
  { "Canon", "EOS 50D", Daylight, 5,		{ 2.240234, 1, 1.288086, 0 } },
  { "Canon", "EOS 50D", Daylight, 6,		{ 2.265625, 1, 1.270508, 0 } },
  { "Canon", "EOS 50D", Daylight, 7,		{ 2.291016, 1, 1.251953, 0 } },
  { "Canon", "EOS 50D", Daylight, 8,		{ 2.322266, 1, 1.233398, 0 } },
  { "Canon", "EOS 50D", Daylight, 9,		{ 2.359375, 1, 1.214844, 0 } },
  { "Canon", "EOS 50D", Shade, -9,		{ 2.124023, 1, 1.383789, 0 } },
  { "Canon", "EOS 50D", Shade, -8,		{ 2.151367, 1, 1.361328, 0 } },
  { "Canon", "EOS 50D", Shade, -7,		{ 2.178711, 1, 1.338867, 0 } },
  { "Canon", "EOS 50D", Shade, -6,		{ 2.207031, 1, 1.314453, 0 } },
  { "Canon", "EOS 50D", Shade, -5,		{ 2.235352, 1, 1.291016, 0 } },
  { "Canon", "EOS 50D", Shade, -4,		{ 2.260742, 1, 1.272461, 0 } },
  { "Canon", "EOS 50D", Shade, -3,		{ 2.291016, 1, 1.254883, 0 } },
  { "Canon", "EOS 50D", Shade, -2,		{ 2.322266, 1, 1.236328, 0 } },
  { "Canon", "EOS 50D", Shade, -1,		{ 2.354492, 1, 1.215820, 0 } },
  { "Canon", "EOS 50D", Shade, 0,		{ 2.386719, 1, 1.196289, 0 } },
  { "Canon", "EOS 50D", Shade, 1,		{ 2.403320, 1, 1.186523, 0 } },
  { "Canon", "EOS 50D", Shade, 2,		{ 2.420898, 1, 1.175781, 0 } },
  { "Canon", "EOS 50D", Shade, 3,		{ 2.438477, 1, 1.165039, 0 } },
  { "Canon", "EOS 50D", Shade, 4,		{ 2.461914, 1, 1.152344, 0 } },
  { "Canon", "EOS 50D", Shade, 5,		{ 2.485352, 1, 1.136719, 0 } },
  { "Canon", "EOS 50D", Shade, 6,		{ 2.522461, 1, 1.115234, 0 } },
  { "Canon", "EOS 50D", Shade, 7,		{ 2.559570, 1, 1.094727, 0 } },
  { "Canon", "EOS 50D", Shade, 8,		{ 2.598633, 1, 1.072266, 0 } },
  { "Canon", "EOS 50D", Shade, 9,		{ 2.645508, 1, 1.051758, 0 } },
  { "Canon", "EOS 50D", Cloudy, -9,		{ 1.996094, 1, 1.486328, 0 } },
  { "Canon", "EOS 50D", Cloudy, -8,		{ 2.019531, 1, 1.466797, 0 } },
  { "Canon", "EOS 50D", Cloudy, -7,		{ 2.043945, 1, 1.444336, 0 } },
  { "Canon", "EOS 50D", Cloudy, -6,		{ 2.073242, 1, 1.421875, 0 } },
  { "Canon", "EOS 50D", Cloudy, -5,		{ 2.102539, 1, 1.400391, 0 } },
  { "Canon", "EOS 50D", Cloudy, -4,		{ 2.128906, 1, 1.377930, 0 } },
  { "Canon", "EOS 50D", Cloudy, -3,		{ 2.156250, 1, 1.356445, 0 } },
  { "Canon", "EOS 50D", Cloudy, -2,		{ 2.188477, 1, 1.333008, 0 } },
  { "Canon", "EOS 50D", Cloudy, -1,		{ 2.211914, 1, 1.309570, 0 } },
  { "Canon", "EOS 50D", Cloudy, 0,		{ 2.240234, 1, 1.285156, 0 } },
  { "Canon", "EOS 50D", Cloudy, 1,		{ 2.270508, 1, 1.268555, 0 } },
  { "Canon", "EOS 50D", Cloudy, 2,		{ 2.295898, 1, 1.250000, 0 } },
  { "Canon", "EOS 50D", Cloudy, 3,		{ 2.327148, 1, 1.232422, 0 } },
  { "Canon", "EOS 50D", Cloudy, 4,		{ 2.359375, 1, 1.211914, 0 } },
  { "Canon", "EOS 50D", Cloudy, 5,		{ 2.392578, 1, 1.195313, 0 } },
  { "Canon", "EOS 50D", Cloudy, 6,		{ 2.409180, 1, 1.183594, 0 } },
  { "Canon", "EOS 50D", Cloudy, 7,		{ 2.426758, 1, 1.172852, 0 } },
  { "Canon", "EOS 50D", Cloudy, 8,		{ 2.444336, 1, 1.161133, 0 } },
  { "Canon", "EOS 50D", Cloudy, 9,		{ 2.467773, 1, 1.149414, 0 } },
  { "Canon", "EOS 50D", Tungsten, -9,		{ 1.379189, 1, 2.206349, 0 } },
  { "Canon", "EOS 50D", Tungsten, -8,		{ 1.394690, 1, 2.176991, 0 } },
  { "Canon", "EOS 50D", Tungsten, -7,		{ 1.412600, 1, 2.155280, 0 } },
  { "Canon", "EOS 50D", Tungsten, -6,		{ 1.428317, 1, 2.127337, 0 } },
  { "Canon", "EOS 50D", Tungsten, -5,		{ 1.448122, 1, 2.101073, 0 } },
  { "Canon", "EOS 50D", Tungsten, -4,		{ 1.467684, 1, 2.078097, 0 } },
  { "Canon", "EOS 50D", Tungsten, -3,		{ 1.484220, 1, 2.054103, 0 } },
  { "Canon", "EOS 50D", Tungsten, -2,		{ 1.501357, 1, 2.027149, 0 } },
  { "Canon", "EOS 50D", Tungsten, -1,		{ 1.521818, 1, 2.003636, 0 } },
  { "Canon", "EOS 50D", Tungsten, 0,		{ 1.542466, 1, 1.976256, 0 } },
  { "Canon", "EOS 50D", Tungsten, 1,		{ 1.561468, 1, 1.949541, 0 } },
  { "Canon", "EOS 50D", Tungsten, 2,		{ 1.581567, 1, 1.923502, 0 } },
  { "Canon", "EOS 50D", Tungsten, 3,		{ 1.602778, 1, 1.894444, 0 } },
  { "Canon", "EOS 50D", Tungsten, 4,		{ 1.624767, 1, 1.867784, 0 } },
  { "Canon", "EOS 50D", Tungsten, 5,		{ 1.647940, 1, 1.841760, 0 } },
  { "Canon", "EOS 50D", Tungsten, 6,		{ 1.669492, 1, 1.815443, 0 } },
  { "Canon", "EOS 50D", Tungsten, 7,		{ 1.686553, 1, 1.789773, 0 } },
  { "Canon", "EOS 50D", Tungsten, 8,		{ 1.708294, 1, 1.766444, 0 } },
  { "Canon", "EOS 50D", Tungsten, 9,		{ 1.729626, 1, 1.738255, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, -9,	{ 1.683196, 1, 2.110193, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, -8,	{ 1.704797, 1, 2.084871, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, -7,	{ 1.727778, 1, 2.061111, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, -6,	{ 1.747907, 1, 2.036279, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, -5,	{ 1.767507, 1, 2.013072, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, -4,	{ 1.791745, 1, 1.988743, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, -3,	{ 1.812264, 1, 1.963208, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, -2,	{ 1.834758, 1, 1.932574, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, -1,	{ 1.863419, 1, 1.907354, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, 0,	{ 1.882805, 1, 1.876081, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, 1,	{ 1.908124, 1, 1.852998, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, 2,	{ 1.931774, 1, 1.822612, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, 3,	{ 1.958008, 1, 1.799805, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, 4,	{ 1.988281, 1, 1.771484, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, 5,	{ 2.011719, 1, 1.747070, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, 6,	{ 2.036133, 1, 1.720703, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, 7,	{ 2.064453, 1, 1.698242, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, 8,	{ 2.093750, 1, 1.678711, 0 } },
  { "Canon", "EOS 50D", WhiteFluorescent, 9,	{ 2.120117, 1, 1.654297, 0 } },
  { "Canon", "EOS 50D", Flash, -9,		{ 2.027344, 1, 1.466797, 0 } },
  { "Canon", "EOS 50D", Flash, -8,		{ 2.056641, 1, 1.446289, 0 } },
  { "Canon", "EOS 50D", Flash, -7,		{ 2.085938, 1, 1.423828, 0 } },
  { "Canon", "EOS 50D", Flash, -6,		{ 2.111328, 1, 1.402344, 0 } },
  { "Canon", "EOS 50D", Flash, -5,		{ 2.137695, 1, 1.379883, 0 } },
  { "Canon", "EOS 50D", Flash, -4,		{ 2.169922, 1, 1.358398, 0 } },
  { "Canon", "EOS 50D", Flash, -3,		{ 2.192383, 1, 1.334961, 0 } },
  { "Canon", "EOS 50D", Flash, -2,		{ 2.221680, 1, 1.311523, 0 } },
  { "Canon", "EOS 50D", Flash, -1,		{ 2.250977, 1, 1.288086, 0 } },
  { "Canon", "EOS 50D", Flash, 0,		{ 2.275391, 1, 1.268555, 0 } },
  { "Canon", "EOS 50D", Flash, 1,		{ 2.306641, 1, 1.251953, 0 } },
  { "Canon", "EOS 50D", Flash, 2,		{ 2.337891, 1, 1.233398, 0 } },
  { "Canon", "EOS 50D", Flash, 3,		{ 2.375977, 1, 1.212891, 0 } },
  { "Canon", "EOS 50D", Flash, 4,		{ 2.398438, 1, 1.195313, 0 } },
  { "Canon", "EOS 50D", Flash, 5,		{ 2.415039, 1, 1.185547, 0 } },
  { "Canon", "EOS 50D", Flash, 6,		{ 2.432617, 1, 1.173828, 0 } },
  { "Canon", "EOS 50D", Flash, 7,		{ 2.450195, 1, 1.162109, 0 } },
  { "Canon", "EOS 50D", Flash, 8,		{ 2.473633, 1, 1.150391, 0 } },
  { "Canon", "EOS 50D", Flash, 9,		{ 2.503906, 1, 1.132813, 0 } },
  { "Canon", "EOS 50D", "5000K", 0,		{ 2.056641, 1, 1.438477, 0 } },
  { "Canon", "EOS 50D", "6500K", 0,		{ 2.311523, 1, 1.239258, 0 } },

  { "Canon", "EOS 60D", Daylight, 0,		{ 2.1514, 1, 1.5420, 0 } },
  { "Canon", "EOS 60D", Shade, 0,		{ 2.5039, 1, 1.3057, 0 } },
  { "Canon", "EOS 60D", Cloudy, 0,		{ 2.3223, 1, 1.4160, 0 } },
  { "Canon", "EOS 60D", Tungsten, 0,		{ 1.5215, 1, 2.3486, 0 } },
  { "Canon", "EOS 60D", Fluorescent, 0,		{ 1.9248, 1, 2.1836, 0 } },
  { "Canon", "EOS 60D", Flash, 0,		{ 2.1514, 1, 1.5420, 0 } },

  { "Canon", "EOS 300D DIGITAL", Daylight, 0,	{ 2.072115, 1, 1.217548, 0 } },
  { "Canon", "EOS 300D DIGITAL", Shade, 0,	{ 2.455529, 1, 1.026442, 0 } },
  { "Canon", "EOS 300D DIGITAL", Cloudy, 0,	{ 2.254808, 1, 1.108173, 0 } },
  { "Canon", "EOS 300D DIGITAL", Tungsten, 0,	{ 1.349057, 1, 1.896226, 0 } },
  { "Canon", "EOS 300D DIGITAL", Fluorescent, 0, { 1.794664, 1, 1.711137, 0 } },
  { "Canon", "EOS 300D DIGITAL", Flash, 0,	{ 2.326923, 1, 1.098558, 0 } },

  { "Canon", "EOS DIGITAL REBEL", Daylight, 0,	{ 2.072115, 1, 1.217548, 0 } },
  { "Canon", "EOS DIGITAL REBEL", Shade, 0,	{ 2.455529, 1, 1.026442, 0 } },
  { "Canon", "EOS DIGITAL REBEL", Cloudy, 0,	{ 2.254808, 1, 1.108173, 0 } },
  { "Canon", "EOS DIGITAL REBEL", Tungsten, 0,	{ 1.349057, 1, 1.896226, 0 } },
  { "Canon", "EOS DIGITAL REBEL", Fluorescent, 0, { 1.794664, 1, 1.711137, 0 } },
  { "Canon", "EOS DIGITAL REBEL", Flash, 0,	{ 2.326923, 1, 1.098558, 0 } },

  { "Canon", "EOS Kiss Digital", Daylight, 0,	{ 2.072115, 1, 1.217548, 0 } },
  { "Canon", "EOS Kiss Digital", Shade, 0,	{ 2.455529, 1, 1.026442, 0 } },
  { "Canon", "EOS Kiss Digital", Cloudy, 0,	{ 2.254808, 1, 1.108173, 0 } },
  { "Canon", "EOS Kiss Digital", Tungsten, 0,	{ 1.349057, 1, 1.896226, 0 } },
  { "Canon", "EOS Kiss Digital", Fluorescent, 0, { 1.794664, 1, 1.711137, 0 } },
  { "Canon", "EOS Kiss Digital", Flash, 0,	{ 2.326923, 1, 1.098558, 0 } },

  { "Canon", "EOS 350D DIGITAL", Tungsten, 0,	{ 1.451524, 1, 2.333333, 0 } },
  { "Canon", "EOS 350D DIGITAL", Daylight, 0,	{ 2.202756, 1, 1.488189, 0 } },
  { "Canon", "EOS 350D DIGITAL", Fluorescent, 0, { 1.846004, 1, 1.987329, 0 } },
  { "Canon", "EOS 350D DIGITAL", Shade, 0,	{ 2.617126, 1, 1.235236, 0 } },
  { "Canon", "EOS 350D DIGITAL", Flash, 0,	{ 2.508858, 1, 1.297244, 0 } },
  { "Canon", "EOS 350D DIGITAL", Cloudy, 0,	{ 2.409449, 1, 1.344488, 0 } },

  { "Canon", "EOS DIGITAL REBEL XT", Tungsten, 0, { 1.451524, 1, 2.333333, 0 } },
  { "Canon", "EOS DIGITAL REBEL XT", Daylight, 0, { 2.202756, 1, 1.488189, 0 } },
  { "Canon", "EOS DIGITAL REBEL XT", Fluorescent, 0, { 1.846004, 1, 1.987329, 0 } },
  { "Canon", "EOS DIGITAL REBEL XT", Shade, 0,	{ 2.617126, 1, 1.235236, 0 } },
  { "Canon", "EOS DIGITAL REBEL XT", Flash, 0,	{ 2.508858, 1, 1.297244, 0 } },
  { "Canon", "EOS DIGITAL REBEL XT", Cloudy, 0,	{ 2.409449, 1, 1.344488, 0 } },

  { "Canon", "EOS Kiss Digital N", Tungsten, 0,	{ 1.451524, 1, 2.333333, 0 } },
  { "Canon", "EOS Kiss Digital N", Daylight, 0,	{ 2.202756, 1, 1.488189, 0 } },
  { "Canon", "EOS Kiss Digital N", Fluorescent, 0, { 1.846004, 1, 1.987329, 0 } },
  { "Canon", "EOS Kiss Digital N", Shade, 0,	{ 2.617126, 1, 1.235236, 0 } },
  { "Canon", "EOS Kiss Digital N", Flash, 0,	{ 2.508858, 1, 1.297244, 0 } },
  { "Canon", "EOS Kiss Digital N", Cloudy, 0,	{ 2.409449, 1, 1.344488, 0 } },

  // Canon EOS 400D (firmware 1.1.1) white balance presets, 5 mireds per step
  { "Canon", "EOS 400D DIGITAL", Daylight, -9,	{ 1.972656, 1, 1.735352, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, -8,	{ 2.003906, 1, 1.707031, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, -7,	{ 2.036133, 1, 1.675781, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, -6,	{ 2.073242, 1, 1.646484, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, -5,	{ 2.111328, 1, 1.615234, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, -4,	{ 2.151367, 1, 1.583008, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, -3,	{ 2.183594, 1, 1.553711, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, -2,	{ 2.221680, 1, 1.523438, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, -1,	{ 2.260742, 1, 1.495117, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, 0,	{ 2.300781, 1, 1.462891, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, 1,	{ 2.337891, 1, 1.436523, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, 2,	{ 2.375977, 1, 1.408203, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, 3,	{ 2.415039, 1, 1.379883, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, 4,	{ 2.461914, 1, 1.354492, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, 5,	{ 2.503906, 1, 1.328125, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, 6,	{ 2.541016, 1, 1.304688, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, 7,	{ 2.579102, 1, 1.280273, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, 8,	{ 2.619141, 1, 1.256836, 0 } },
  { "Canon", "EOS 400D DIGITAL", Daylight, 9,	{ 2.666992, 1, 1.232422, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, -9,	{ 2.333008, 1, 1.440430, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, -8,	{ 2.370117, 1, 1.410156, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, -7,	{ 2.409180, 1, 1.383789, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, -6,	{ 2.456055, 1, 1.356445, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, -5,	{ 2.503906, 1, 1.330078, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, -4,	{ 2.541016, 1, 1.305664, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, -3,	{ 2.579102, 1, 1.283203, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, -2,	{ 2.619141, 1, 1.259766, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, -1,	{ 2.660156, 1, 1.235352, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, 0,	{ 2.708984, 1, 1.208984, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, 1,	{ 2.745117, 1, 1.189453, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, 2,	{ 2.782227, 1, 1.168945, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, 3,	{ 2.829102, 1, 1.148438, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, 4,	{ 2.875977, 1, 1.125000, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, 5,	{ 2.916992, 1, 1.105469, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, 6,	{ 2.951172, 1, 1.087891, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, 7,	{ 2.994141, 1, 1.069336, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, 8,	{ 3.039063, 1, 1.048828, 0 } },
  { "Canon", "EOS 400D DIGITAL", Shade, 9,	{ 3.083984, 1, 1.030273, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, -9,	{ 2.156250, 1, 1.580078, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, -8,	{ 2.188477, 1, 1.551758, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, -7,	{ 2.226563, 1, 1.521484, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, -6,	{ 2.265625, 1, 1.490234, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, -5,	{ 2.306641, 1, 1.460938, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, -4,	{ 2.342773, 1, 1.432617, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, -3,	{ 2.381836, 1, 1.404297, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, -2,	{ 2.420898, 1, 1.375977, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, -1,	{ 2.467773, 1, 1.350586, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, 0,	{ 2.509766, 1, 1.323242, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, 1,	{ 2.546875, 1, 1.300781, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, 2,	{ 2.585938, 1, 1.278320, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, 3,	{ 2.625977, 1, 1.252930, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, 4,	{ 2.673828, 1, 1.229492, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, 5,	{ 2.723633, 1, 1.205078, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, 6,	{ 2.752930, 1, 1.185547, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, 7,	{ 2.790039, 1, 1.165039, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, 8,	{ 2.836914, 1, 1.142578, 0 } },
  { "Canon", "EOS 400D DIGITAL", Cloudy, 9,	{ 2.884766, 1, 1.120117, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, -9,	{ 1.320106, 1, 2.752205, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, -8,	{ 1.340708, 1, 2.703540, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, -7,	{ 1.359680, 1, 2.655417, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, -6,	{ 1.381802, 1, 2.606601, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, -5,	{ 1.406446, 1, 2.555953, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, -4,	{ 1.428957, 1, 2.504496, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, -3,	{ 1.452575, 1, 2.459801, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, -2,	{ 1.475931, 1, 2.419619, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, -1,	{ 1.501825, 1, 2.377737, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, 0,	{ 1.526123, 1, 2.330889, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, 1,	{ 1.548893, 1, 2.286900, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, 2,	{ 1.572753, 1, 2.238184, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, 3,	{ 1.599254, 1, 2.198509, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, 4,	{ 1.624765, 1, 2.149156, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, 5,	{ 1.653774, 1, 2.102830, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, 6,	{ 1.681861, 1, 2.064577, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, 7,	{ 1.709369, 1, 2.022945, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, 8,	{ 1.737247, 1, 1.982676, 0 } },
  { "Canon", "EOS 400D DIGITAL", Tungsten, 9,	{ 1.770349, 1, 1.946705, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, -9, { 1.638122, 1, 2.485267, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, -8, { 1.667900, 1, 2.445883, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, -7, { 1.695814, 1, 2.404651, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, -6, { 1.723364, 1, 2.361682, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, -5, { 1.752820, 1, 2.317669, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, -4, { 1.788079, 1, 2.263009, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, -3, { 1.815414, 1, 2.221694, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, -2, { 1.844828, 1, 2.175287, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, -1, { 1.880309, 1, 2.127413, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, 0, { 1.910506, 1, 2.080739, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, 1, { 1.950195, 1, 2.043945, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, 2, { 1.984375, 1, 2.007813, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, 3, { 2.015625, 1, 1.968750, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, 4, { 2.047852, 1, 1.928711, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, 5, { 2.085938, 1, 1.892578, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, 6, { 2.124023, 1, 1.858398, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, 7, { 2.165039, 1, 1.825195, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, 8, { 2.197266, 1, 1.790039, 0 } },
  { "Canon", "EOS 400D DIGITAL", WhiteFluorescent, 9, { 2.235352, 1, 1.756836, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, -9,	{ 2.398438, 1, 1.432617, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, -8,	{ 2.438477, 1, 1.402344, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, -7,	{ 2.485352, 1, 1.375977, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, -6,	{ 2.528320, 1, 1.349609, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, -5,	{ 2.566406, 1, 1.323242, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, -4,	{ 2.605469, 1, 1.299805, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, -3,	{ 2.645508, 1, 1.276367, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, -2,	{ 2.694336, 1, 1.251953, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, -1,	{ 2.738281, 1, 1.227539, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, 0,	{ 2.767578, 1, 1.203125, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, 1,	{ 2.813477, 1, 1.183594, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, 2,	{ 2.860352, 1, 1.164063, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, 3,	{ 2.900391, 1, 1.141602, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, 4,	{ 2.942383, 1, 1.118164, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, 5,	{ 2.976563, 1, 1.101563, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, 6,	{ 3.020508, 1, 1.082031, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, 7,	{ 3.065430, 1, 1.063477, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, 8,	{ 3.122070, 1, 1.041992, 0 } },
  { "Canon", "EOS 400D DIGITAL", Flash, 9,	{ 3.169922, 1, 1.024414, 0 } },

  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, -9, { 1.972656, 1, 1.735352, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, -8, { 2.003906, 1, 1.707031, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, -7, { 2.036133, 1, 1.675781, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, -6, { 2.073242, 1, 1.646484, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, -5, { 2.111328, 1, 1.615234, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, -4, { 2.151367, 1, 1.583008, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, -3, { 2.183594, 1, 1.553711, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, -2, { 2.221680, 1, 1.523438, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, -1, { 2.260742, 1, 1.495117, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, 0, { 2.300781, 1, 1.462891, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, 1, { 2.337891, 1, 1.436523, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, 2, { 2.375977, 1, 1.408203, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, 3, { 2.415039, 1, 1.379883, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, 4, { 2.461914, 1, 1.354492, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, 5, { 2.503906, 1, 1.328125, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, 6, { 2.541016, 1, 1.304688, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, 7, { 2.579102, 1, 1.280273, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, 8, { 2.619141, 1, 1.256836, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Daylight, 9, { 2.666992, 1, 1.232422, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, -9, { 2.333008, 1, 1.440430, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, -8, { 2.370117, 1, 1.410156, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, -7, { 2.409180, 1, 1.383789, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, -6, { 2.456055, 1, 1.356445, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, -5, { 2.503906, 1, 1.330078, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, -4, { 2.541016, 1, 1.305664, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, -3, { 2.579102, 1, 1.283203, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, -2, { 2.619141, 1, 1.259766, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, -1, { 2.660156, 1, 1.235352, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, 0,	{ 2.708984, 1, 1.208984, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, 1,	{ 2.745117, 1, 1.189453, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, 2,	{ 2.782227, 1, 1.168945, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, 3,	{ 2.829102, 1, 1.148438, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, 4,	{ 2.875977, 1, 1.125000, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, 5,	{ 2.916992, 1, 1.105469, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, 6,	{ 2.951172, 1, 1.087891, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, 7,	{ 2.994141, 1, 1.069336, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, 8,	{ 3.039063, 1, 1.048828, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Shade, 9,	{ 3.083984, 1, 1.030273, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, -9, { 2.156250, 1, 1.580078, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, -8, { 2.188477, 1, 1.551758, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, -7, { 2.226563, 1, 1.521484, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, -6, { 2.265625, 1, 1.490234, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, -5, { 2.306641, 1, 1.460938, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, -4, { 2.342773, 1, 1.432617, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, -3, { 2.381836, 1, 1.404297, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, -2, { 2.420898, 1, 1.375977, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, -1, { 2.467773, 1, 1.350586, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, 0, { 2.509766, 1, 1.323242, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, 1, { 2.546875, 1, 1.300781, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, 2, { 2.585938, 1, 1.278320, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, 3, { 2.625977, 1, 1.252930, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, 4, { 2.673828, 1, 1.229492, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, 5, { 2.723633, 1, 1.205078, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, 6, { 2.752930, 1, 1.185547, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, 7, { 2.790039, 1, 1.165039, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, 8, { 2.836914, 1, 1.142578, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Cloudy, 9, { 2.884766, 1, 1.120117, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, -9, { 1.320106, 1, 2.752205, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, -8, { 1.340708, 1, 2.703540, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, -7, { 1.359680, 1, 2.655417, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, -6, { 1.381802, 1, 2.606601, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, -5, { 1.406446, 1, 2.555953, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, -4, { 1.428957, 1, 2.504496, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, -3, { 1.452575, 1, 2.459801, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, -2, { 1.475931, 1, 2.419619, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, -1, { 1.501825, 1, 2.377737, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, 0, { 1.526123, 1, 2.330889, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, 1, { 1.548893, 1, 2.286900, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, 2, { 1.572753, 1, 2.238184, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, 3, { 1.599254, 1, 2.198509, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, 4, { 1.624765, 1, 2.149156, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, 5, { 1.653774, 1, 2.102830, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, 6, { 1.681861, 1, 2.064577, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, 7, { 1.709369, 1, 2.022945, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, 8, { 1.737247, 1, 1.982676, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Tungsten, 9, { 1.770349, 1, 1.946705, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, -9, { 1.638122, 1, 2.485267, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, -8, { 1.667900, 1, 2.445883, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, -7, { 1.695814, 1, 2.404651, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, -6, { 1.723364, 1, 2.361682, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, -5, { 1.752820, 1, 2.317669, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, -4, { 1.788079, 1, 2.263009, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, -3, { 1.815414, 1, 2.221694, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, -2, { 1.844828, 1, 2.175287, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, -1, { 1.880309, 1, 2.127413, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, 0, { 1.910506, 1, 2.080739, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, 1, { 1.950195, 1, 2.043945, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, 2, { 1.984375, 1, 2.007813, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, 3, { 2.015625, 1, 1.968750, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, 4, { 2.047852, 1, 1.928711, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, 5, { 2.085938, 1, 1.892578, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, 6, { 2.124023, 1, 1.858398, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, 7, { 2.165039, 1, 1.825195, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, 8, { 2.197266, 1, 1.790039, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", WhiteFluorescent, 9, { 2.235352, 1, 1.756836, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, -9, { 2.398438, 1, 1.432617, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, -8, { 2.438477, 1, 1.402344, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, -7, { 2.485352, 1, 1.375977, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, -6, { 2.528320, 1, 1.349609, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, -5, { 2.566406, 1, 1.323242, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, -4, { 2.605469, 1, 1.299805, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, -3, { 2.645508, 1, 1.276367, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, -2, { 2.694336, 1, 1.251953, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, -1, { 2.738281, 1, 1.227539, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, 0,	{ 2.767578, 1, 1.203125, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, 1,	{ 2.813477, 1, 1.183594, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, 2,	{ 2.860352, 1, 1.164063, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, 3,	{ 2.900391, 1, 1.141602, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, 4,	{ 2.942383, 1, 1.118164, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, 5,	{ 2.976563, 1, 1.101563, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, 6,	{ 3.020508, 1, 1.082031, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, 7,	{ 3.065430, 1, 1.063477, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, 8,	{ 3.122070, 1, 1.041992, 0 } },
  { "Canon", "EOS DIGITAL REBEL XTi", Flash, 9,	{ 3.169922, 1, 1.024414, 0 } },

  { "Canon", "EOS Kiss Digital X", Daylight, -9, { 1.972656, 1, 1.735352, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, -8, { 2.003906, 1, 1.707031, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, -7, { 2.036133, 1, 1.675781, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, -6, { 2.073242, 1, 1.646484, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, -5, { 2.111328, 1, 1.615234, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, -4, { 2.151367, 1, 1.583008, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, -3, { 2.183594, 1, 1.553711, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, -2, { 2.221680, 1, 1.523438, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, -1, { 2.260742, 1, 1.495117, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, 0,	{ 2.300781, 1, 1.462891, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, 1,	{ 2.337891, 1, 1.436523, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, 2,	{ 2.375977, 1, 1.408203, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, 3,	{ 2.415039, 1, 1.379883, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, 4,	{ 2.461914, 1, 1.354492, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, 5,	{ 2.503906, 1, 1.328125, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, 6,	{ 2.541016, 1, 1.304688, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, 7,	{ 2.579102, 1, 1.280273, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, 8,	{ 2.619141, 1, 1.256836, 0 } },
  { "Canon", "EOS Kiss Digital X", Daylight, 9,	{ 2.666992, 1, 1.232422, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, -9,	{ 2.333008, 1, 1.440430, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, -8,	{ 2.370117, 1, 1.410156, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, -7,	{ 2.409180, 1, 1.383789, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, -6,	{ 2.456055, 1, 1.356445, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, -5,	{ 2.503906, 1, 1.330078, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, -4,	{ 2.541016, 1, 1.305664, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, -3,	{ 2.579102, 1, 1.283203, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, -2,	{ 2.619141, 1, 1.259766, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, -1,	{ 2.660156, 1, 1.235352, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, 0,	{ 2.708984, 1, 1.208984, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, 1,	{ 2.745117, 1, 1.189453, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, 2,	{ 2.782227, 1, 1.168945, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, 3,	{ 2.829102, 1, 1.148438, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, 4,	{ 2.875977, 1, 1.125000, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, 5,	{ 2.916992, 1, 1.105469, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, 6,	{ 2.951172, 1, 1.087891, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, 7,	{ 2.994141, 1, 1.069336, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, 8,	{ 3.039063, 1, 1.048828, 0 } },
  { "Canon", "EOS Kiss Digital X", Shade, 9,	{ 3.083984, 1, 1.030273, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, -9,	{ 2.156250, 1, 1.580078, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, -8,	{ 2.188477, 1, 1.551758, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, -7,	{ 2.226563, 1, 1.521484, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, -6,	{ 2.265625, 1, 1.490234, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, -5,	{ 2.306641, 1, 1.460938, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, -4,	{ 2.342773, 1, 1.432617, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, -3,	{ 2.381836, 1, 1.404297, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, -2,	{ 2.420898, 1, 1.375977, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, -1,	{ 2.467773, 1, 1.350586, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, 0,	{ 2.509766, 1, 1.323242, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, 1,	{ 2.546875, 1, 1.300781, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, 2,	{ 2.585938, 1, 1.278320, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, 3,	{ 2.625977, 1, 1.252930, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, 4,	{ 2.673828, 1, 1.229492, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, 5,	{ 2.723633, 1, 1.205078, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, 6,	{ 2.752930, 1, 1.185547, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, 7,	{ 2.790039, 1, 1.165039, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, 8,	{ 2.836914, 1, 1.142578, 0 } },
  { "Canon", "EOS Kiss Digital X", Cloudy, 9,	{ 2.884766, 1, 1.120117, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, -9, { 1.320106, 1, 2.752205, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, -8, { 1.340708, 1, 2.703540, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, -7, { 1.359680, 1, 2.655417, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, -6, { 1.381802, 1, 2.606601, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, -5, { 1.406446, 1, 2.555953, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, -4, { 1.428957, 1, 2.504496, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, -3, { 1.452575, 1, 2.459801, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, -2, { 1.475931, 1, 2.419619, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, -1, { 1.501825, 1, 2.377737, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, 0,	{ 1.526123, 1, 2.330889, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, 1,	{ 1.548893, 1, 2.286900, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, 2,	{ 1.572753, 1, 2.238184, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, 3,	{ 1.599254, 1, 2.198509, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, 4,	{ 1.624765, 1, 2.149156, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, 5,	{ 1.653774, 1, 2.102830, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, 6,	{ 1.681861, 1, 2.064577, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, 7,	{ 1.709369, 1, 2.022945, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, 8,	{ 1.737247, 1, 1.982676, 0 } },
  { "Canon", "EOS Kiss Digital X", Tungsten, 9,	{ 1.770349, 1, 1.946705, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, -9, { 1.638122, 1, 2.485267, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, -8, { 1.667900, 1, 2.445883, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, -7, { 1.695814, 1, 2.404651, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, -6, { 1.723364, 1, 2.361682, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, -5, { 1.752820, 1, 2.317669, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, -4, { 1.788079, 1, 2.263009, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, -3, { 1.815414, 1, 2.221694, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, -2, { 1.844828, 1, 2.175287, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, -1, { 1.880309, 1, 2.127413, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, 0, { 1.910506, 1, 2.080739, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, 1, { 1.950195, 1, 2.043945, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, 2, { 1.984375, 1, 2.007813, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, 3, { 2.015625, 1, 1.968750, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, 4, { 2.047852, 1, 1.928711, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, 5, { 2.085938, 1, 1.892578, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, 6, { 2.124023, 1, 1.858398, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, 7, { 2.165039, 1, 1.825195, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, 8, { 2.197266, 1, 1.790039, 0 } },
  { "Canon", "EOS Kiss Digital X", WhiteFluorescent, 9, { 2.235352, 1, 1.756836, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, -9,	{ 2.398438, 1, 1.432617, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, -8,	{ 2.438477, 1, 1.402344, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, -7,	{ 2.485352, 1, 1.375977, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, -6,	{ 2.528320, 1, 1.349609, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, -5,	{ 2.566406, 1, 1.323242, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, -4,	{ 2.605469, 1, 1.299805, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, -3,	{ 2.645508, 1, 1.276367, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, -2,	{ 2.694336, 1, 1.251953, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, -1,	{ 2.738281, 1, 1.227539, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, 0,	{ 2.767578, 1, 1.203125, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, 1,	{ 2.813477, 1, 1.183594, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, 2,	{ 2.860352, 1, 1.164063, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, 3,	{ 2.900391, 1, 1.141602, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, 4,	{ 2.942383, 1, 1.118164, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, 5,	{ 2.976563, 1, 1.101563, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, 6,	{ 3.020508, 1, 1.082031, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, 7,	{ 3.065430, 1, 1.063477, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, 8,	{ 3.122070, 1, 1.041992, 0 } },
  { "Canon", "EOS Kiss Digital X", Flash, 9,	{ 3.169922, 1, 1.024414, 0 } },

  { "Canon", "EOS 450D", Daylight, 0,		{ 2.216797, 1, 1.471680, 0 } },
  { "Canon", "EOS 450D", Shade, 0,		{ 2.566406, 1, 1.241211, 0 } },
  { "Canon", "EOS 450D", Cloudy, 0,		{ 2.386719, 1, 1.345703, 0 } },
  { "Canon", "EOS 450D", Tungsten, 0,		{ 1.559034, 1, 2.170841, 0 } },
  { "Canon", "EOS 450D", Fluorescent, 0,	{ 1.922857, 1, 1.996190, 0 } },
  { "Canon", "EOS 450D", Flash, 0,		{ 2.456055, 1, 1.318359, 0 } },

  { "Canon", "EOS DIGITAL REBEL XSi", Daylight, 0, { 2.216797, 1, 1.471680, 0 } },
  { "Canon", "EOS DIGITAL REBEL XSi", Shade, 0, { 2.566406, 1, 1.241211, 0 } },
  { "Canon", "EOS DIGITAL REBEL XSi", Cloudy, 0, { 2.386719, 1, 1.345703, 0 } },
  { "Canon", "EOS DIGITAL REBEL XSi", Tungsten, 0, { 1.559034, 1, 2.170841, 0 } },
  { "Canon", "EOS DIGITAL REBEL XSi", Fluorescent, 0, { 1.922857, 1, 1.996190, 0 } },
  { "Canon", "EOS DIGITAL REBEL XSi", Flash, 0, { 2.456055, 1, 1.318359, 0 } },

  { "Canon", "EOS Kiss Digital X2", Daylight, 0, { 2.216797, 1, 1.471680, 0 } },
  { "Canon", "EOS Kiss Digital X2", Shade, 0,	{ 2.566406, 1, 1.241211, 0 } },
  { "Canon", "EOS Kiss Digital X2", Cloudy, 0,	{ 2.386719, 1, 1.345703, 0 } },
  { "Canon", "EOS Kiss Digital X2", Tungsten, 0, { 1.559034, 1, 2.170841, 0 } },
  { "Canon", "EOS Kiss Digital X2", Fluorescent, 0, { 1.922857, 1, 1.996190, 0 } },
  { "Canon", "EOS Kiss Digital X2", Flash, 0,	{ 2.456055, 1, 1.318359, 0 } },

  { "Canon", "EOS 500D", Daylight, 0,		{ 2.023438, 1, 1.417969, 0 } },
  { "Canon", "EOS 500D", Shade, 0,		{ 2.291016, 1, 1.217773, 0 } },
  { "Canon", "EOS 500D", Cloudy, 0,		{ 2.156250, 1, 1.304687, 0 } },
  { "Canon", "EOS 500D", Tungsten, 0,		{ 1.481347, 1, 1.976342, 0 } },
  { "Canon", "EOS 500D", Fluorescent, 0,	{ 1.799224, 1, 1.824442, 0 } },
  { "Canon", "EOS 500D", Flash, 0,		{ 2.207031, 1, 1.295898, 0 } },

  { "Canon", "EOS REBEL T1i", Daylight, 0,	{ 2.023438, 1, 1.417969, 0 } },
  { "Canon", "EOS REBEL T1i", Shade, 0,		{ 2.291016, 1, 1.217773, 0 } },
  { "Canon", "EOS REBEL T1i", Cloudy, 0,	{ 2.156250, 1, 1.304687, 0 } },
  { "Canon", "EOS REBEL T1i", Tungsten, 0,	{ 1.481347, 1, 1.976342, 0 } },
  { "Canon", "EOS REBEL T1i", Fluorescent, 0,	{ 1.799224, 1, 1.824442, 0 } },
  { "Canon", "EOS REBEL T1i", Flash, 0,		{ 2.207031, 1, 1.295898, 0 } },

  { "Canon", "EOS Kiss Digital X3", Daylight, 0, { 2.023438, 1, 1.417969, 0 } },
  { "Canon", "EOS Kiss Digital X3", Shade, 0,	{ 2.291016, 1, 1.217773, 0 } },
  { "Canon", "EOS Kiss Digital X3", Cloudy, 0,	{ 2.156250, 1, 1.304687, 0 } },
  { "Canon", "EOS Kiss Digital X3", Tungsten, 0, { 1.481347, 1, 1.976342, 0 } },
  { "Canon", "EOS Kiss Digital X3", Fluorescent, 0, { 1.799224, 1, 1.824442, 0 } },
  { "Canon", "EOS Kiss Digital X3", Flash, 0,	{ 2.207031, 1, 1.295898, 0 } },

  { "Canon", "EOS 550D", Daylight, 0,		{ 2.1426, 1, 1.5488, 0 } },
  { "Canon", "EOS 550D", Shade, 0,		{ 2.4619, 1, 1.3193, 0 } },
  { "Canon", "EOS 550D", Cloudy, 0,		{ 2.3066, 1, 1.4258, 0 } },
  { "Canon", "EOS 550D", Tungsten, 0,		{ 1.5264, 1, 2.3428, 0 } },
  { "Canon", "EOS 550D", WhiteFluorescent, 0,	{ 1.9072, 1, 2.1973, 0 } },
  { "Canon", "EOS 550D", Flash, 0,		{ 2.3701, 1, 1.4141, 0 } },

  { "Canon", "EOS REBEL T2i", Daylight, 0,	{ 2.1426, 1, 1.5488, 0 } },
  { "Canon", "EOS REBEL T2i", Shade, 0,		{ 2.4619, 1, 1.3193, 0 } },
  { "Canon", "EOS REBEL T2i", Cloudy, 0,	{ 2.3066, 1, 1.4258, 0 } },
  { "Canon", "EOS REBEL T2i", Tungsten, 0,	{ 1.5264, 1, 2.3428, 0 } },
  { "Canon", "EOS REBEL T2i", WhiteFluorescent, 0, { 1.9072, 1, 2.1973, 0 } },
  { "Canon", "EOS REBEL T2i", Flash, 0,		{ 2.3701, 1, 1.4141, 0 } },

  { "Canon", "EOS Kiss Digital X4", Daylight, 0, { 2.1426, 1, 1.5488, 0 } },
  { "Canon", "EOS Kiss Digital X4", Shade, 0,	{ 2.4619, 1, 1.3193, 0 } },
  { "Canon", "EOS Kiss Digital X4", Cloudy, 0,	{ 2.3066, 1, 1.4258, 0 } },
  { "Canon", "EOS Kiss Digital X4", Tungsten, 0, { 1.5264, 1, 2.3428, 0 } },
  { "Canon", "EOS Kiss Digital X4", WhiteFluorescent, 0, { 1.9072, 1, 2.1973, 0 } },
  { "Canon", "EOS Kiss Digital X4", Flash, 0,	{ 2.3701, 1, 1.4141, 0 } },

  { "Canon", "EOS 600D", Daylight, 0,		{ 2.1602, 1, 1.5752, 0 } },
  { "Canon", "EOS 600D", Shade, 0,		{ 2.5098, 1, 1.3418, 0 } },
  { "Canon", "EOS 600D", Cloudy, 0,		{ 2.3330, 1, 1.4502, 0 } },
  { "Canon", "EOS 600D", Tungsten, 0,		{ 1.5371, 1, 2.3594, 0 } },
  { "Canon", "EOS 600D", WhiteFluorescent, 0,	{ 1.8789, 1, 2.2402, 0 } },
  { "Canon", "EOS 600D", Flash, 0,		{ 2.4268, 1, 1.4297, 0 } },

  { "Canon", "EOS REBEL T3i", Daylight, 0,	{ 2.1602, 1, 1.5752, 0 } },
  { "Canon", "EOS REBEL T3i", Shade, 0,		{ 2.5098, 1, 1.3418, 0 } },
  { "Canon", "EOS REBEL T3i", Cloudy, 0,	{ 2.3330, 1, 1.4502, 0 } },
  { "Canon", "EOS REBEL T3i", Tungsten, 0,	{ 1.5371, 1, 2.3594, 0 } },
  { "Canon", "EOS REBEL T3i", WhiteFluorescent, 0, { 1.8789, 1, 2.2402, 0 } },
  { "Canon", "EOS REBEL T3i", Flash, 0,		{ 2.4268, 1, 1.4297, 0 } },

  { "Canon", "EOS Kiss Digital X5", Daylight, 0, { 2.1602, 1, 1.5752, 0 } },
  { "Canon", "EOS Kiss Digital X5", Shade, 0,	{ 2.5098, 1, 1.3418, 0 } },
  { "Canon", "EOS Kiss Digital X5", Cloudy, 0,	{ 2.3330, 1, 1.4502, 0 } },
  { "Canon", "EOS Kiss Digital X5", Tungsten, 0, { 1.5371, 1, 2.3594, 0 } },
  { "Canon", "EOS Kiss Digital X5", WhiteFluorescent, 0, { 1.8789, 1, 2.2402, 0 } },
  { "Canon", "EOS Kiss Digital X5", Flash, 0,	{ 2.4268, 1, 1.4297, 0 } },

  { "Canon", "EOS 1000D", Daylight, 0,		{ 2.183594, 1, 1.526367, 0 } },
  { "Canon", "EOS 1000D", Shade, 0,		{ 2.553711, 1, 1.262695, 0 } },
  { "Canon", "EOS 1000D", Cloudy, 0,		{ 2.365234, 1, 1.375977, 0 } },
  { "Canon", "EOS 1000D", Tungsten, 0,		{ 1.470328, 1, 2.402126, 0 } },
  { "Canon", "EOS 1000D", Fluorescent, 0,	{ 1.889648, 1, 2.133789, 0 } },
  { "Canon", "EOS 1000D", Flash, 0,		{ 2.541830, 1, 1.769099, 0 } },

  { "Canon", "EOS DIGITAL REBEL XS", Daylight, 0, { 2.183594, 1, 1.526367, 0 } },
  { "Canon", "EOS DIGITAL REBEL XS", Shade, 0,	{ 2.553711, 1, 1.262695, 0 } },
  { "Canon", "EOS DIGITAL REBEL XS", Cloudy, 0,	{ 2.365234, 1, 1.375977, 0 } },
  { "Canon", "EOS DIGITAL REBEL XS", Tungsten, 0, { 1.470328, 1, 2.402126, 0 } },
  { "Canon", "EOS DIGITAL REBEL XS", Fluorescent, 0, { 1.889648, 1, 2.133789, 0 } },
  { "Canon", "EOS DIGITAL REBEL XS", Flash, 0,	{ 2.541830, 1, 1.769099, 0 } },

  { "Canon", "EOS Kiss Digital F", Daylight, 0,	{ 2.183594, 1, 1.526367, 0 } },
  { "Canon", "EOS Kiss Digital F", Shade, 0,	{ 2.553711, 1, 1.262695, 0 } },
  { "Canon", "EOS Kiss Digital F", Cloudy, 0,	{ 2.365234, 1, 1.375977, 0 } },
  { "Canon", "EOS Kiss Digital F", Tungsten, 0,	{ 1.470328, 1, 2.402126, 0 } },
  { "Canon", "EOS Kiss Digital F", Fluorescent, 0, { 1.889648, 1, 2.133789, 0 } },
  { "Canon", "EOS Kiss Digital F", Flash, 0,	{ 2.541830, 1, 1.769099, 0 } },

  { "Canon", "EOS 1100D", Daylight, 0,		{ 2.2559, 1, 1.4844, 0 } },
  { "Canon", "EOS 1100D", Shade, 0,		{ 2.6455, 1, 1.2725, 0 } },
  { "Canon", "EOS 1100D", Cloudy, 0,		{ 2.4443, 1, 1.3652, 0 } },
  { "Canon", "EOS 1100D", Tungsten, 0,		{ 1.5654, 1, 2.2402, 0 } },
  { "Canon", "EOS 1100D", WhiteFluorescent, 0,	{ 1.9541, 1, 2.0732, 0 } },
  { "Canon", "EOS 1100D", Flash, 0,		{ 2.5283, 1, 1.3584, 0 } },

  { "Canon", "EOS DIGITAL REBEL T3", Daylight, 0, { 2.2559, 1, 1.4844, 0 } },
  { "Canon", "EOS DIGITAL REBEL T3", Shade, 0,	{ 2.6455, 1, 1.2725, 0 } },
  { "Canon", "EOS DIGITAL REBEL T3", Cloudy, 0,	{ 2.4443, 1, 1.3652, 0 } },
  { "Canon", "EOS DIGITAL REBEL T3", Tungsten, 0, { 1.5654, 1, 2.2402, 0 } },
  { "Canon", "EOS DIGITAL REBEL T3", WhiteFluorescent, 0, { 1.9541, 1, 2.0732, 0 } },
  { "Canon", "EOS DIGITAL REBEL T3", Flash, 0,	{ 2.5283, 1, 1.3584, 0 } },

  { "Canon", "EOS Kiss Digital X50", Daylight, 0, { 2.2559, 1, 1.4844, 0 } },
  { "Canon", "EOS Kiss Digital X50", Shade, 0,	{ 2.6455, 1, 1.2725, 0 } },
  { "Canon", "EOS Kiss Digital X50", Cloudy, 0,	{ 2.4443, 1, 1.3652, 0 } },
  { "Canon", "EOS Kiss Digital X50", Tungsten, 0, { 1.5654, 1, 2.2402, 0 } },
  { "Canon", "EOS Kiss Digital X50", WhiteFluorescent, 0, { 1.9541, 1, 2.0732, 0 } },
  { "Canon", "EOS Kiss Digital X50", Flash, 0,	{ 2.5283, 1, 1.3584, 0 } },

  { "Canon", "EOS-1DS", Daylight, 0,		{ 1.6133, 1, 1.2500, 0 } }, /* 5302K */
  { "Canon", "EOS-1DS", Shade, 0,		{ 1.8477, 1, 1.0684, 0 } }, /* 6685K */
  { "Canon", "EOS-1DS", Cloudy, 0,		{ 1.7285, 1, 1.1504, 0 } }, /* 5940K */
  { "Canon", "EOS-1DS", Tungsten, 0,		{ 1.1270, 1, 1.9199, 0 } }, /* 3334K */
  { "Canon", "EOS-1DS", Fluorescent, 0,		{ 1.2012, 1, 1.7168, 0 } }, /* 3643K */
  { "Canon", "EOS-1DS", Flash, 0,		{ 1.7793, 1, 1.1445, 0 } }, /* 6184K */

  // Firmware Version 1.1.6
  { "Canon", "EOS-1Ds Mark II", Daylight, 0,	{ 1.992188, 1, 1.503906, 0 } },
  { "Canon", "EOS-1Ds Mark II", Shade, 0,	{ 2.300781, 1, 1.243164, 0 } },
  { "Canon", "EOS-1Ds Mark II", Cloudy, 0,	{ 2.146484, 1, 1.363281, 0 } },
  { "Canon", "EOS-1Ds Mark II", Tungsten, 0,	{ 1.395018, 1, 2.370107, 0 } },
  { "Canon", "EOS-1Ds Mark II", Fluorescent, 0,	{ 1.854792, 1, 2.067764, 0 } },
  { "Canon", "EOS-1Ds Mark II", Flash, 0,	{ 2.235352, 1, 1.297852, 0 } },

  { "Canon", "EOS-1D Mark II", Cloudy, 0,	{ 2.093750, 1, 1.166016, 0 } },
  { "Canon", "EOS-1D Mark II", Daylight, 0,	{ 1.957031, 1, 1.295898, 0 } },
  { "Canon", "EOS-1D Mark II", Flash, 0,	{ 2.225586, 1, 1.172852, 0 } },
  { "Canon", "EOS-1D Mark II", Fluorescent, 0,	{ 1.785853, 1, 1.785853, 0 } },
  { "Canon", "EOS-1D Mark II", Shade, 0,	{ 2.220703, 1, 1.069336, 0 } },
  { "Canon", "EOS-1D Mark II", Tungsten, 0,	{ 1.415480, 1, 2.160142, 0 } },

  { "Canon", "EOS-1D Mark II N", Cloudy, 0,	{ 2.183594, 1, 1.220703, 0 } },
  { "Canon", "EOS-1D Mark II N", Daylight, 0,	{ 2.019531, 1, 1.349609, 0 } },
  { "Canon", "EOS-1D Mark II N", Flash, 0,	{ 2.291016, 1, 1.149414, 0 } },
  { "Canon", "EOS-1D Mark II N", Fluorescent, 0, { 1.802899, 1, 1.990338, 0 } },
  { "Canon", "EOS-1D Mark II N", Shade, 0,	{ 2.337891, 1, 1.112305, 0 } },
  { "Canon", "EOS-1D Mark II N", Tungsten, 0,	{ 1.408514, 1, 2.147645, 0 } },

  // Firmware Version 1.0.6 (Temporary)
  { "Canon", "EOS-1D Mark IV", Daylight, 0,	{ 2.040039, 1, 1.558594, 0 } },
  { "Canon", "EOS-1D Mark IV", Shade, 0,	{ 2.342773, 1, 1.333008, 0 } },
  { "Canon", "EOS-1D Mark IV", Cloudy, 0,	{ 2.188477, 1, 1.440430, 0 } },
  { "Canon", "EOS-1D Mark IV", Tungsten, 0,	{ 1.458333, 1, 2.305254, 0 } },
  { "Canon", "EOS-1D Mark IV", Fluorescent, 0,	{ 1.767892, 1, 2.205029, 0 } },
  { "Canon", "EOS-1D Mark IV", Flash, 0,	{ 2.230469, 1, 1.423828, 0 } },

  { "FUJIFILM", "FinePix E900", Daylight, 0,	{ 1.571875, 1, 1.128125, 0 } },
  { "FUJIFILM", "FinePix E900", Shade, 0,	{ 1.668750, 1, 1.006250, 0 } },
  { "FUJIFILM", "FinePix E900", DaylightFluorescent, 0, { 1.907609, 1, 1.016304, 0 } },
  { "FUJIFILM", "FinePix E900", WarmWhiteFluorescent, 0, { 1.654891, 1, 1.241848, 0 } },
  { "FUJIFILM", "FinePix E900", CoolWhiteFluorescent, 0, { 1.554348, 1, 1.519022, 0 } },
  { "FUJIFILM", "FinePix E900", Incandescent, 0, { 1.037611, 1, 1.842920, 0 } },

  { "FUJIFILM", "FinePix F700", Daylight, 0,	{ 1.725000, 1, 1.500000, 0 } },
  { "FUJIFILM", "FinePix F700", Shade, 0,	{ 1.950000, 1, 1.325000, 0 } },
  { "FUJIFILM", "FinePix F700", DaylightFluorescent, 0, { 2.032609, 1, 1.336957, 0 } },
  { "FUJIFILM", "FinePix F700", WarmWhiteFluorescent, 0, { 1.706522, 1, 1.663043, 0 } },
  { "FUJIFILM", "FinePix F700", CoolWhiteFluorescent, 0, { 1.684783, 1, 2.152174, 0 } },
  { "FUJIFILM", "FinePix F700", Incandescent, 0, { 1.168142, 1, 2.477876, 0 } },

  { "FUJIFILM", "FinePix HS20EXR", Daylight, 0,	{ 1.4107, 1, 1.9702, 0 } },
  { "FUJIFILM", "FinePix HS20EXR", Shade, 0,	{ 1.5804, 1, 1.7440, 0 } },
  { "FUJIFILM", "FinePix HS20EXR", DaylightFluorescent, 0, { 1.7292, 1, 1.7470, 0 } },
  { "FUJIFILM", "FinePix HS20EXR", WarmWhiteFluorescent, 0, { 1.4821, 1, 2.0476, 0 } },
  { "FUJIFILM", "FinePix HS20EXR", CoolWhiteFluorescent, 0, { 1.5625, 1, 2.5714, 0 } },
  { "FUJIFILM", "FinePix HS20EXR", Incandescent, 0, { 1, 1.0633, 2.9430, 0 } },

  { "FUJIFILM", "FinePix S100FS", Daylight, 0,	{ 1.702381, 1, 1.845238, 0 } },
  { "FUJIFILM", "FinePix S100FS", Shade, 0,	{ 1.830357, 1, 1.601190, 0 } },
  { "FUJIFILM", "FinePix S100FS", DaylightFluorescent, 0, { 1.895833, 1, 1.461309, 0 } },
  { "FUJIFILM", "FinePix S100FS", WarmWhiteFluorescent, 0, { 1.574405, 1, 1.818452, 0 } },
  { "FUJIFILM", "FinePix S100FS", CoolWhiteFluorescent, 0, { 1.663690, 1, 2.309524, 0 } },
  { "FUJIFILM", "FinePix S100FS", Incandescent, 0, { 1.107143, 1, 2.815476, 0 } },

  { "FUJIFILM", "FinePix S20Pro", Daylight, 0,	{ 1.712500, 1, 1.500000, 0 } },
  { "FUJIFILM", "FinePix S20Pro", Cloudy, 0,	{ 1.887500, 1, 1.262500, 0 } },
  { "FUJIFILM", "FinePix S20Pro", DaylightFluorescent, 0, { 2.097826, 1, 1.304348, 0 } },
  { "FUJIFILM", "FinePix S20Pro", WarmWhiteFluorescent, 0, { 1.782609, 1, 1.619565, 0 } },
  { "FUJIFILM", "FinePix S20Pro", CoolWhiteFluorescent, 0, { 1.670213, 1, 2.063830, 0 } },
  { "FUJIFILM", "FinePix S20Pro", Incandescent, 0, { 1.069565, 1, 2.486957, 0 } },

  { "FUJIFILM", "FinePix S2Pro", Daylight, 0,	{ 1.509804, 1, 1.401961, 0 } },
  { "FUJIFILM", "FinePix S2Pro", Cloudy, 0,	{ 1.666667, 1, 1.166667, 0 } },
  { "FUJIFILM", "FinePix S2Pro", Flash, 0,	{ 1, 1.014084, 2.542253, 0 } },
  { "FUJIFILM", "FinePix S2Pro", DaylightFluorescent, 0, { 1.948718, 1, 1.230769, 0 } },
  { "FUJIFILM", "FinePix S2Pro", WarmWhiteFluorescent, 0, { 1.675214, 1, 1.572650, 0 } },
  { "FUJIFILM", "FinePix S2Pro", CoolWhiteFluorescent, 0, { 1.649573, 1, 2.094017, 0 } },

  { "FUJIFILM", "FinePix S5000", Incandescent, 0, { 1.212081, 1, 2.672364, 0 } },
  { "FUJIFILM", "FinePix S5000", Fluorescent, 0, { 1.772316, 1, 2.349902, 0 } },
  { "FUJIFILM", "FinePix S5000", Daylight, 0,	{ 1.860403, 1, 1.515946, 0 } },
  { "FUJIFILM", "FinePix S5000", Flash, 0,	{ 2.202181, 1, 1.423284, 0 } },
  { "FUJIFILM", "FinePix S5000", Cloudy, 0,	{ 2.036578, 1, 1.382513, 0 } },
  { "FUJIFILM", "FinePix S5000", Shade, 0,	{ 2.357215, 1, 1.212016, 0 } },

  { "FUJIFILM", "FinePix S5200", Daylight, 0,	{ 1.587500, 1, 1.381250, 0 } },
  { "FUJIFILM", "FinePix S5200", Shade, 0,	{ 1.946875, 1, 1.175000, 0 } },
  { "FUJIFILM", "FinePix S5200", DaylightFluorescent, 0, { 1.948370, 1, 1.187500, 0 } },
  { "FUJIFILM", "FinePix S5200", WarmWhiteFluorescent, 0, { 1.682065, 1, 1.437500, 0 } },
  { "FUJIFILM", "FinePix S5200", CoolWhiteFluorescent, 0, { 1.595109, 1, 1.839674, 0 } },
  { "FUJIFILM", "FinePix S5200", Incandescent, 0, { 1.077434, 1, 2.170354, 0 } },

  { "FUJIFILM", "FinePix S5500", Daylight, 0,	{ 1.712500, 1, 1.550000, 0 } },
  { "FUJIFILM", "FinePix S5500", Shade, 0,	{ 1.912500, 1, 1.375000, 0 } },
  { "FUJIFILM", "FinePix S5500", DaylightFluorescent, 0, { 1.978261, 1, 1.380435, 0 } },
  { "FUJIFILM", "FinePix S5500", WarmWhiteFluorescent, 0, { 1.673913, 1, 1.673913, 0 } },
  { "FUJIFILM", "FinePix S5500", CoolWhiteFluorescent, 0, { 1.663043, 1, 2.163043, 0 } },
  { "FUJIFILM", "FinePix S5500", Incandescent, 0, { 1.115044, 1, 2.566372, 0 } },

  { "FUJIFILM", "FinePix S5600", Daylight, 0,	{ 1.587500, 1, 1.381250, 0 } },
  { "FUJIFILM", "FinePix S5600", Shade, 0,	{ 1.946875, 1, 1.175000, 0 } },
  { "FUJIFILM", "FinePix S5600", DaylightFluorescent, 0, { 1.948370, 1, 1.187500, 0 } },
  { "FUJIFILM", "FinePix S5600", WarmWhiteFluorescent, 0, { 1.682065, 1, 1.437500, 0 } },
  { "FUJIFILM", "FinePix S5600", CoolWhiteFluorescent, 0, { 1.595109, 1, 1.839674, 0 } },
  { "FUJIFILM", "FinePix S5600", Incandescent, 0, { 1.077434, 1, 2.170354, 0 } },

  { "FUJIFILM", "FinePix S6000fd", Daylight, 0,	{ 1.511905, 1, 1.431548, 0 } },
  { "FUJIFILM", "FinePix S6000fd", Shade, 0,	{ 1.699405, 1, 1.232143, 0 } },
  { "FUJIFILM", "FinePix S6000fd", DaylightFluorescent, 0, { 1.866071, 1, 1.309524, 0 } },
  { "FUJIFILM", "FinePix S6000fd", WarmWhiteFluorescent, 0, { 1.568452, 1, 1.627976, 0 } },
  { "FUJIFILM", "FinePix S6000fd", CoolWhiteFluorescent, 0, { 1.598214, 1, 2.038691, 0 } },
  { "FUJIFILM", "FinePix S6000fd", Incandescent, 0, { 1, 1.024390, 2.466463, 0 } },

  { "FUJIFILM", "FinePix S6500fd", Daylight, 0,	{ 1.398810, 1, 1.470238, 0 } },
  { "FUJIFILM", "FinePix S6500fd", Shade, 0,	{ 1.580357, 1, 1.270833, 0 } },
  { "FUJIFILM", "FinePix S6500fd", DaylightFluorescent, 0, { 1.735119, 1, 1.348214, 0 } },
  { "FUJIFILM", "FinePix S6500fd", WarmWhiteFluorescent, 0, { 1.455357, 1, 1.672619, 0 } },
  { "FUJIFILM", "FinePix S6500fd", CoolWhiteFluorescent, 0, { 1.482143, 1, 2.089286, 0 } },
  { "FUJIFILM", "FinePix S6500fd", Incandescent, 0, { 1, 1.123746, 2.769231, 0 } },

  { "FUJIFILM", "FinePix S7000", Daylight, 0,	{ 1.900000, 1, 1.525000, 0 } },
  { "FUJIFILM", "FinePix S7000", Shade, 0,	{ 2.137500, 1, 1.350000, 0 } },
  { "FUJIFILM", "FinePix S7000", DaylightFluorescent, 0, { 2.315217, 1, 1.347826, 0 } },
  { "FUJIFILM", "FinePix S7000", WarmWhiteFluorescent, 0, { 1.902174, 1, 1.663043, 0 } },
  { "FUJIFILM", "FinePix S7000", CoolWhiteFluorescent, 0, { 1.836957, 1, 2.130435, 0 } },
  { "FUJIFILM", "FinePix S7000", Incandescent, 0, { 1.221239, 1, 2.548673, 0 } },

  /* The S9000 and S9500 are the same camera */
  { "FUJIFILM", "FinePix S9000", Daylight, 0,	{ 1.618750, 1, 1.231250, 0 } },
  { "FUJIFILM", "FinePix S9000", Cloudy, 0,	{ 1.700000, 1, 1.046875, 0 } },
  { "FUJIFILM", "FinePix S9000", DaylightFluorescent, 0, { 1.902174, 1, 1.057065, 0 } },
  { "FUJIFILM", "FinePix S9000", WarmWhiteFluorescent, 0, { 1.633152, 1, 1.293478, 0 } },
  { "FUJIFILM", "FinePix S9000", CoolWhiteFluorescent, 0, { 1.546196, 1, 1.622283, 0 } },
  { "FUJIFILM", "FinePix S9000", Incandescent, 0, { 1.064159, 1, 1.960177, 0 } },

  { "FUJIFILM", "FinePix S9500", Daylight, 0,	{ 1.618750, 1, 1.231250, 0 } },
  { "FUJIFILM", "FinePix S9500", Cloudy, 0,	{ 1.700000, 1, 1.046875, 0 } },
  { "FUJIFILM", "FinePix S9500", DaylightFluorescent, 0, { 1.902174, 1, 1.057065, 0 } },
  { "FUJIFILM", "FinePix S9500", WarmWhiteFluorescent, 0, { 1.633152, 1, 1.293478, 0 } },
  { "FUJIFILM", "FinePix S9500", CoolWhiteFluorescent, 0, { 1.546196, 1, 1.622283, 0 } },
  { "FUJIFILM", "FinePix S9500", Incandescent, 0, { 1.064159, 1, 1.960177, 0 } },

  { "FUJIFILM", "FinePix S9100", Daylight, 0,	{ 1.506250, 1, 1.318750, 0 } },
  { "FUJIFILM", "FinePix S9100", Cloudy, 0,	{ 1.587500, 1, 1.128125, 0 } },
  { "FUJIFILM", "FinePix S9100", DaylightFluorescent, 0, { 1.777174, 1, 1.138587, 0 } },
  { "FUJIFILM", "FinePix S9100", WarmWhiteFluorescent, 0, { 1.521739, 1, 1.380435, 0 } },
  { "FUJIFILM", "FinePix S9100", CoolWhiteFluorescent, 0, { 1.437500, 1, 1.720109, 0 } },
  { "FUJIFILM", "FinePix S9100", Incandescent, 0, { 1, 1.024943, 2.113379, 0 } },

  { "FUJIFILM", "FinePix S9600", Daylight, 0,	{ 1.534375, 1, 1.300000, 0 } },
  { "FUJIFILM", "FinePix S9600", Shade, 0,	{ 1.615625, 1, 1.112500, 0 } },
  { "FUJIFILM", "FinePix S9600", DaylightFluorescent, 0, { 1.809783, 1, 1.122283, 0 } },
  { "FUJIFILM", "FinePix S9600", WarmWhiteFluorescent, 0, { 1.551630, 1, 1.361413, 0 } },
  { "FUJIFILM", "FinePix S9600", CoolWhiteFluorescent, 0, { 1.467391, 1, 1.692935, 0 } },
  { "FUJIFILM", "FinePix S9600", Incandescent, 0, { 1, 1.004444, 2.040000, 0 } },

  { "FUJIFILM", "FinePix X100", Daylight, 0,	{ 1.4503, 1, 1.5033, 0 } },
  { "FUJIFILM", "FinePix X100", Shade, 0,	{ 1.5861, 1, 1.2947, 0 } },
  { "FUJIFILM", "FinePix X100", DaylightFluorescent, 0, { 1.8841, 1, 1.3179, 0 } },
  { "FUJIFILM", "FinePix X100", WarmWhiteFluorescent, 0, { 1.6291, 1, 1.5927, 0 } },
  { "FUJIFILM", "FinePix X100", CoolWhiteFluorescent, 0, { 1.5662, 1, 2.0265, 0 } },
  { "FUJIFILM", "FinePix X100", Incandescent, 0, { 1, 1.0272, 2.4966, 0 } },
  { "FUJIFILM", "FinePix X100", Underwater, 0,	{ 1.4603, 1, 1.5662, 0 } },
  { "FUJIFILM", "FinePix X100", "2700K", 0,	{ 1, 1.3790, 3.8447, 0 } },
  { "FUJIFILM", "FinePix X100", "3000K", 0,	{ 1, 1.1353, 2.7180, 0 } },
  { "FUJIFILM", "FinePix X100", "3300K", 0,	{ 1.0066, 1, 2.1060, 0 } },
  { "FUJIFILM", "FinePix X100", "5000K", 0,	{ 1.4536, 1, 1.3742, 0 } },

  { "Leica Camera AG", "M8 Digital Camera", Cloudy, 0, { 2.136719, 1, 1.168213, 0 } },
  { "Leica Camera AG", "M8 Digital Camera", Daylight, 0, { 2.007996, 1, 1.268982, 0 } },
  { "Leica Camera AG", "M8 Digital Camera", Flash, 0, { 2.164490, 1, 1.177795, 0 } },
  { "Leica Camera AG", "M8 Digital Camera", Fluorescent, 0, { 1.655579, 1, 2.070374, 0 } },
  { "Leica Camera AG", "M8 Digital Camera", Shade, 0, { 2.197754, 1, 1.111084, 0 } },
  { "Leica Camera AG", "M8 Digital Camera", Tungsten, 0, { 1.160034, 1, 2.028381, 0 } },

  { "Leica", "M9 Digital Camera", Tungsten, 0,		{ 1.321288, 1, 2.077024, 0 } },
  { "Leica", "M9 Digital Camera", Fluorescent, 0,	{ 1.673827, 1, 1.855043, 0 } },
  { "Leica", "M9 Digital Camera", DaylightFluorescent, 0, { 2.224852, 1, 1.388000, 0 } },
  { "Leica", "M9 Digital Camera", Daylight, 0,		{ 2.013733, 1, 1.364869, 0 } },
  { "Leica", "M9 Digital Camera", Flash, 0,		{ 1.980652, 1, 1.331111, 0 } },
  { "Leica", "M9 Digital Camera", Cloudy, 0,		{ 2.146728, 1, 1.252197, 0 } },
  { "Leica", "M9 Digital Camera", Shade, 0,		{ 2.249268, 1, 1.179015, 0 } },

  { "Leica Camera AG", "R8 - Digital Back DMR", Incandescent, 0, { 1, 1.109985, 2.430664, 0 } },
  { "Leica Camera AG", "R8 - Digital Back DMR", Fluorescent, 0, { 1.234985, 1, 1.791138, 0 } },
  { "Leica Camera AG", "R8 - Digital Back DMR", Daylight, 0, { 1.459961, 1, 1.184937, 0 } },
  { "Leica Camera AG", "R8 - Digital Back DMR", Flash, 0, { 1.395020, 1, 1.144897, 0 } },
  { "Leica Camera AG", "R8 - Digital Back DMR", Cloudy, 0, { 1.541992, 1, 1.052856, 0 } },
  { "Leica Camera AG", "R8 - Digital Back DMR", Shade, 0, { 1.644897, 1.033936, 1, 0 } },
  { "Leica Camera AG", "R8 - Digital Back DMR", "2700K", 0, { 1, 1.172607, 2.747192, 0 } },
  { "Leica Camera AG", "R8 - Digital Back DMR", "3000K", 0, { 1, 1.049438, 2.156494, 0 } },
  { "Leica Camera AG", "R8 - Digital Back DMR", "3300K", 0, { 1.032349, 1, 1.841187, 0 } },
  { "Leica Camera AG", "R8 - Digital Back DMR", "5000K", 0, { 1.344360, 1, 1.233032, 0 } },

  { "Leica Camera AG", "R9 - Digital Back DMR", Incandescent, 0, { 1, 1.109985, 2.430664, 0 } },
  { "Leica Camera AG", "R9 - Digital Back DMR", Fluorescent, 0, { 1.234985, 1, 1.791138, 0 } },
  { "Leica Camera AG", "R9 - Digital Back DMR", Daylight, 0, { 1.459961, 1, 1.184937, 0 } },
  { "Leica Camera AG", "R9 - Digital Back DMR", Flash, 0, { 1.395020, 1, 1.144897, 0 } },
  { "Leica Camera AG", "R9 - Digital Back DMR", Cloudy, 0, { 1.541992, 1, 1.052856, 0 } },
  { "Leica Camera AG", "R9 - Digital Back DMR", Shade, 0, { 1.644897, 1.033936, 1, 0 } },
  { "Leica Camera AG", "R9 - Digital Back DMR", "2700K", 0, { 1, 1.172607, 2.747192, 0 } },
  { "Leica Camera AG", "R9 - Digital Back DMR", "3000K", 0, { 1, 1.049438, 2.156494, 0 } },
  { "Leica Camera AG", "R9 - Digital Back DMR", "3300K", 0, { 1.032349, 1, 1.841187, 0 } },
  { "Leica Camera AG", "R9 - Digital Back DMR", "5000K", 0, { 1.344360, 1, 1.233032, 0 } },

  { "LEICA", "DIGILUX 2", Daylight, 0,		{ 1.628906, 1, 1.488281, 0 } },
  { "LEICA", "DIGILUX 2", Cloudy, 0,		{ 1.835938, 1, 1.343750, 0 } },
  { "LEICA", "DIGILUX 2", Incandescent, 0,	{ 1.078125, 1, 2.203125, 0 } },
  { "LEICA", "DIGILUX 2", Flash, 0,		{ 2.074219, 1, 1.304688, 0 } },
  { "LEICA", "DIGILUX 2", BlackNWhite, 0,	{ 1.632812, 1, 1.550781, 0 } },

  { "LEICA", "DIGILUX 3", Daylight, 0,		{ 1.942966, 1, 1.399240, 0 } },
  { "LEICA", "DIGILUX 3", Cloudy, 0,		{ 2.083650, 1, 1.247148, 0 } },
  { "LEICA", "DIGILUX 3", Shade, 0,		{ 2.296578, 1, 1.110266, 0 } },
  { "LEICA", "DIGILUX 3", Incandescent, 0,	{ 1.372624, 1, 2.079848, 0 } },
  /* Flash multipliers are variable */
  { "LEICA", "DIGILUX 3", Flash, 0,		{ 2.095057, 1, 1.091255, 0 } },

  /* Digilux 3 Kelvin presets */
  { "LEICA", "DIGILUX 3", "2700K", 0,		{ 1.250951, 1, 2.539924, 0 } },
  { "LEICA", "DIGILUX 3", "3000K", 0,		{ 1.361217, 1, 2.212928, 0 } },
  { "LEICA", "DIGILUX 3", "3300K", 0,		{ 1.471483, 1, 2.167300, 0 } },
  { "LEICA", "DIGILUX 3", "5000K", 0,		{ 1.851711, 1, 1.490494, 0 } },
  { "LEICA", "DIGILUX 3", "5500K", 0,		{ 1.923954, 1, 1.361217, 0 } },
  { "LEICA", "DIGILUX 3", "6500K", 0,		{ 2.102662, 1, 1.193916, 0 } },

  { "Minolta", "DiMAGE 5", Daylight, 0,		{ 2.023438, 1, 1.371094, 0 } },
  { "Minolta", "DiMAGE 5", Incandescent, 0,	{ 1.113281, 1, 2.480469, 0 } },
  { "Minolta", "DiMAGE 5", Fluorescent, 0,	{ 1.957031, 1, 2.058594, 0 } },
  { "Minolta", "DiMAGE 5", Cloudy, 0,		{ 2.199219, 1, 1.300781, 0 } },

  { "Minolta", "DiMAGE 7", Cloudy, 0,		{ 2.082031, 1, 1.226562, 0 } },
  { "Minolta", "DiMAGE 7", Daylight, 0,		{ 1.914062, 1, 1.527344, 0 } },
  { "Minolta", "DiMAGE 7", Fluorescent, 0,	{ 1.917969, 1, 2.007812, 0 } },
  { "Minolta", "DiMAGE 7", Tungsten, 0,		{ 1.050781, 1, 2.437500, 0 } },

  { "Minolta", "DiMAGE 7i", Daylight, 0,	{ 1.441406, 1, 1.457031, 0 } },
  { "Minolta", "DiMAGE 7i", Tungsten, 0,	{ 1, 1.333333, 3.572917, 0 } },
  { "Minolta", "DiMAGE 7i", Fluorescent, 0,	{ 1.554688, 1, 2.230469, 0 } },
  { "Minolta", "DiMAGE 7i", Cloudy, 0,		{ 1.550781, 1, 1.402344, 0 } },

  { "Minolta", "DiMAGE 7Hi", Daylight, 0,	{ 1.609375, 1, 1.328125, 0 } }, /*5500K*/
  { "Minolta", "DiMAGE 7Hi", Tungsten, 0,	{ 1, 1.137778, 2.768889, 0 } }, /*2800K*/
  { "Minolta", "DiMAGE 7Hi", WhiteFluorescent, 0, { 1.664062, 1, 2.105469, 0 } }, /*4060K*/
  { "Minolta", "DiMAGE 7Hi", CoolWhiteFluorescent, 0, { 1.796875, 1, 1.734375, 0 } }, /*4938K*/
  { "Minolta", "DiMAGE 7Hi", Cloudy, 0,		{ 1.730469, 1, 1.269531, 0 } }, /*5823K*/

  { "Minolta", "DiMAGE A1", Daylight, 0,	{ 1.808594, 1, 1.304688, 0 } },
  { "Minolta", "DiMAGE A1", Tungsten, 0,	{ 1.062500, 1, 2.675781, 0 } },
  { "Minolta", "DiMAGE A1", Fluorescent, 0,	{ 1.707031, 1, 2.039063, 0 } },
  { "Minolta", "DiMAGE A1", Cloudy, 0,		{ 1.960938, 1, 1.339844, 0 } },
  { "Minolta", "DiMAGE A1", Shade, 0,		{ 2.253906, 1, 1.199219, 0 } },
  { "Minolta", "DiMAGE A1", Shade, 2,		{ 2.000000, 1, 1.183594, 0 } },
  { "Minolta", "DiMAGE A1", Flash, 0,		{ 1.972656, 1, 1.265625, 0 } },

  { "Minolta", "DiMAGE A2", Cloudy, -3,		{ 2.109375, 1, 1.578125, 0 } },
  { "Minolta", "DiMAGE A2", Cloudy, 0,		{ 2.203125, 1, 1.296875, 0 } },
  { "Minolta", "DiMAGE A2", Cloudy, 3,		{ 2.296875, 1, 1.015625, 0 } },
  { "Minolta", "DiMAGE A2", Daylight, -3,	{ 1.867188, 1, 1.683594, 0 } },
  { "Minolta", "DiMAGE A2", Daylight, 0,	{ 1.960938, 1, 1.402344, 0 } },
  { "Minolta", "DiMAGE A2", Daylight, 3,	{ 2.054688, 1, 1.121094, 0 } },
  { "Minolta", "DiMAGE A2", Flash, -3,		{ 1.945312, 1, 1.613281, 0 } },
  { "Minolta", "DiMAGE A2", Flash, 0,		{ 2.039062, 1, 1.332031, 0 } },
  { "Minolta", "DiMAGE A2", Flash, 3,		{ 2.132812, 1, 1.050781, 0 } },
  { "Minolta", "DiMAGE A2", Fluorescent, -2,	{ 1.136719, 1, 2.746094, 0 } },
  { "Minolta", "DiMAGE A2", Fluorescent, 0,	{ 1.722656, 1, 2.132812, 0 } },
  { "Minolta", "DiMAGE A2", Fluorescent, 4,	{ 2.347656, 1, 1.535156, 0 } },
  { "Minolta", "DiMAGE A2", Shade, -3,		{ 2.273438, 1, 1.546875, 0 } },
  { "Minolta", "DiMAGE A2", Shade, 0,		{ 2.367188, 1, 1.265625, 0 } },
  { "Minolta", "DiMAGE A2", Shade, 3,		{ 2.500000, 1.015873, 1, 0 } },
  { "Minolta", "DiMAGE A2", Tungsten, -3,	{ 1.003906, 1, 3.164062, 0 } },
  { "Minolta", "DiMAGE A2", Tungsten, 0,	{ 1.097656, 1, 2.882812, 0 } },
  { "Minolta", "DiMAGE A2", Tungsten, 3,	{ 1.191406, 1, 2.601562, 0 } },

  { "Minolta", "DiMAGE Z2", Daylight, 0,	{ 1.843749, 1, 1.664062, 0 } },
  { "Minolta", "DiMAGE Z2", Cloudy, 0,		{ 2.195312, 1, 1.449218, 0 } },
  { "Minolta", "DiMAGE Z2", Tungsten, 0,	{ 1.097656, 1, 3.050780, 0 } },
  { "Minolta", "DiMAGE Z2", Fluorescent, 0,	{ 1.796874, 1, 2.257810, 0 } },
  { "Minolta", "DiMAGE Z2", Flash, 0,		{ 2.117186, 1, 1.472656, 0 } },

  { "Minolta", "DiMAGE G500", Daylight, 0,	{ 1.496094, 1, 1.121094, 0 } },
  { "Minolta", "DiMAGE G500", Cloudy, 0,	{ 1.527344, 1, 1.105469, 0 } },
  { "Minolta", "DiMAGE G500", Fluorescent, 0,	{ 1.382813, 1, 1.347656, 0 } },
  { "Minolta", "DiMAGE G500", Tungsten, 0,	{ 1.042969, 1, 1.859375, 0 } },
  { "Minolta", "DiMAGE G500", Flash, 0,		{ 1.647078, 1, 1.218159, 0 } },

  { "MINOLTA", "DYNAX 5D", Daylight, -3,	{ 1.593750, 1, 1.875000, 0 } },
  { "MINOLTA", "DYNAX 5D", Daylight, -2,	{ 1.644531, 1, 1.792969, 0 } },
  { "MINOLTA", "DYNAX 5D", Daylight, -1,	{ 1.699219, 1, 1.718750, 0 } },
  { "MINOLTA", "DYNAX 5D", Daylight, 0,		{ 1.757812, 1, 1.636719, 0 } },
  { "MINOLTA", "DYNAX 5D", Daylight, 1,		{ 1.804688, 1, 1.566406, 0 } },
  { "MINOLTA", "DYNAX 5D", Daylight, 2,		{ 1.863281, 1, 1.500000, 0 } },
  { "MINOLTA", "DYNAX 5D", Daylight, 3,		{ 1.925781, 1, 1.437500, 0 } },
  { "MINOLTA", "DYNAX 5D", Shade, -3,		{ 1.835938, 1, 1.644531, 0 } },
  { "MINOLTA", "DYNAX 5D", Shade, -2,		{ 1.894531, 1, 1.574219, 0 } },
  { "MINOLTA", "DYNAX 5D", Shade, -1,		{ 1.957031, 1, 1.507812, 0 } },
  { "MINOLTA", "DYNAX 5D", Shade, 0,		{ 2.011719, 1, 1.433594, 0 } },
  { "MINOLTA", "DYNAX 5D", Shade, 1,		{ 2.078125, 1, 1.375000, 0 } },
  { "MINOLTA", "DYNAX 5D", Shade, 2,		{ 2.148438, 1, 1.316406, 0 } },
  { "MINOLTA", "DYNAX 5D", Shade, 3,		{ 2.218750, 1, 1.261719, 0 } },
  { "MINOLTA", "DYNAX 5D", Cloudy, -3,		{ 1.718750, 1, 1.738281, 0 } },
  { "MINOLTA", "DYNAX 5D", Cloudy, -2,		{ 1.773438, 1, 1.664062, 0 } },
  { "MINOLTA", "DYNAX 5D", Cloudy, -1,		{ 1.835938, 1, 1.593750, 0 } },
  { "MINOLTA", "DYNAX 5D", Cloudy, 0,		{ 1.886719, 1, 1.500000, 0 } },
  { "MINOLTA", "DYNAX 5D", Cloudy, 1,		{ 1.945312, 1, 1.460938, 0 } },
  { "MINOLTA", "DYNAX 5D", Cloudy, 2,		{ 2.007812, 1, 1.390625, 0 } },
  { "MINOLTA", "DYNAX 5D", Cloudy, 3,		{ 2.078125, 1, 1.332031, 0 } },
  { "MINOLTA", "DYNAX 5D", Tungsten, -3,	{ 1, 1.066667, 4.262500, 0 } },
  { "MINOLTA", "DYNAX 5D", Tungsten, -2,	{ 1, 1.032258, 3.951613, 0 } },
  { "MINOLTA", "DYNAX 5D", Tungsten, -1,	{ 1, 1.000000, 3.671875, 0 } },
  { "MINOLTA", "DYNAX 5D", Tungsten, 0,		{ 1.023438, 1, 3.496094, 0 } },
  { "MINOLTA", "DYNAX 5D", Tungsten, 1,		{ 1.062500, 1, 3.367188, 0 } },
  { "MINOLTA", "DYNAX 5D", Tungsten, 2,		{ 1.097656, 1, 3.203125, 0 } },
  { "MINOLTA", "DYNAX 5D", Tungsten, 3,		{ 1.132812, 1, 3.070312, 0 } },
  { "MINOLTA", "DYNAX 5D", Fluorescent, -2,	{ 1.148438, 1, 3.429688, 0 } },
  { "MINOLTA", "DYNAX 5D", Fluorescent, -1,	{ 1.285156, 1, 3.250000, 0 } },
  { "MINOLTA", "DYNAX 5D", Fluorescent, 0,	{ 1.703125, 1, 2.582031, 0 } },
  { "MINOLTA", "DYNAX 5D", Fluorescent, 1,	{ 1.761719, 1, 2.335938, 0 } },
  { "MINOLTA", "DYNAX 5D", Fluorescent, 2,	{ 1.730469, 1, 1.878906, 0 } },
  { "MINOLTA", "DYNAX 5D", Fluorescent, 3,	{ 1.996094, 1, 1.527344, 0 } },
  { "MINOLTA", "DYNAX 5D", Fluorescent, 4,	{ 2.218750, 1, 1.714844, 0 } },
  { "MINOLTA", "DYNAX 5D", Flash, -3,		{ 1.738281, 1, 1.683594, 0 } },
  { "MINOLTA", "DYNAX 5D", Flash, -2,		{ 1.792969, 1, 1.609375, 0 } },
  { "MINOLTA", "DYNAX 5D", Flash, -1,		{ 1.855469, 1, 1.542969, 0 } },
  { "MINOLTA", "DYNAX 5D", Flash, 0,		{ 1.917969, 1, 1.457031, 0 } },
  { "MINOLTA", "DYNAX 5D", Flash, 1,		{ 1.968750, 1, 1.406250, 0 } },
  { "MINOLTA", "DYNAX 5D", Flash, 2,		{ 2.031250, 1, 1.347656, 0 } },
  { "MINOLTA", "DYNAX 5D", Flash, 3,		{ 2.101562, 1, 1.289062, 0 } },

  { "MINOLTA", "DYNAX 7D", Daylight, -3,	{ 1.476562, 1, 1.824219, 0 } },
  { "MINOLTA", "DYNAX 7D", Daylight, 0,		{ 1.621094, 1, 1.601562, 0 } },
  { "MINOLTA", "DYNAX 7D", Daylight, 3,		{ 1.785156, 1, 1.414062, 0 } },
  { "MINOLTA", "DYNAX 7D", Shade, -3,		{ 1.683594, 1, 1.585938, 0 } },
  { "MINOLTA", "DYNAX 7D", Shade, 0,		{ 1.855469, 1, 1.402344, 0 } },
  { "MINOLTA", "DYNAX 7D", Shade, 3,		{ 2.031250, 1, 1.226562, 0 } },
  { "MINOLTA", "DYNAX 7D", Cloudy, -3,		{ 1.593750, 1, 1.671875, 0 } },
  { "MINOLTA", "DYNAX 7D", Cloudy, 0,		{ 1.738281, 1, 1.464844, 0 } },
  { "MINOLTA", "DYNAX 7D", Cloudy, 3,		{ 1.925781, 1, 1.296875, 0 } },
  { "MINOLTA", "DYNAX 7D", Tungsten, -3,	{ 0.867188, 1, 3.765625, 0 } },
  { "MINOLTA", "DYNAX 7D", Tungsten, 0,		{ 0.945312, 1, 3.292969, 0 } },
  { "MINOLTA", "DYNAX 7D", Tungsten, 3,		{ 1.050781, 1, 2.921875, 0 } },
  { "MINOLTA", "DYNAX 7D", Fluorescent, -2,	{ 1.058594, 1, 3.230469, 0 } },
  { "MINOLTA", "DYNAX 7D", Fluorescent, 0,	{ 1.570312, 1, 2.453125, 0 } },
  { "MINOLTA", "DYNAX 7D", Fluorescent, 1,	{ 1.625000, 1, 2.226562, 0 } },
  { "MINOLTA", "DYNAX 7D", Fluorescent, 4,	{ 2.046875, 1, 1.675781, 0 } },
  { "MINOLTA", "DYNAX 7D", Flash, -3,		{ 1.738281, 1, 1.656250, 0 } },
  { "MINOLTA", "DYNAX 7D", Flash, 0,		{ 1.890625, 1, 1.445312, 0 } },
  { "MINOLTA", "DYNAX 7D", Flash, 3,		{ 2.101562, 1, 1.281250, 0 } },
  { "MINOLTA", "DYNAX 7D", "2700K", 0,		{ 1, 1.089362, 3.829787, 0 } },
  { "MINOLTA", "DYNAX 7D", "3000K", 0,		{ 1.031250, 1, 3.000000, 0 } },
  { "MINOLTA", "DYNAX 7D", "3300K", 0,		{ 1.144531, 1, 2.648438, 0 } },
  { "MINOLTA", "DYNAX 7D", "5000K", 0,		{ 1.609375, 1, 1.726562, 0 } },
  { "MINOLTA", "DYNAX 7D", "5500K", 0,		{ 1.710938, 1, 1.601562, 0 } },
  { "MINOLTA", "DYNAX 7D", "6500K", 0,		{ 1.875000, 1, 1.421875, 0 } },

  { "NIKON", "D1", Incandescent, -3,		{ 1, 1.439891, 2.125769, 0 } },
  { "NIKON", "D1", Incandescent, 0,		{ 1, 1.582583, 2.556096, 0 } },
  { "NIKON", "D1", Incandescent, 3,		{ 1, 1.745033, 3.044175, 0 } },
  { "NIKON", "D1", Fluorescent, -3,		{ 1, 1.013461, 1.489820, 0 } },
  { "NIKON", "D1", Fluorescent, 0,		{ 1, 1.077710, 1.672660, 0 } },
  { "NIKON", "D1", Fluorescent, 3,		{ 1, 1.143167, 1.875227, 0 } },
  { "NIKON", "D1", DirectSunlight, -3,		{ 1.084705, 1.039344, 1, 0 } },
  { "NIKON", "D1", DirectSunlight, 0,		{ 1.000000, 1.000000, 1, 0 } },
  { "NIKON", "D1", DirectSunlight, 3,		{ 1, 1.049801, 1.109411, 0 } },
  { "NIKON", "D1", Flash, -3,			{ 1.317409, 1.116197, 1, 0 } },
  { "NIKON", "D1", Flash, 0,			{ 1.235772, 1.078231, 1, 0 } },
  { "NIKON", "D1", Flash, 3,			{ 1.100855, 1.016026, 1, 0 } },
  { "NIKON", "D1", Cloudy, -3,			{ 1.241160, 1.116197, 1, 0 } },
  { "NIKON", "D1", Cloudy, 0,			{ 1.162116, 1.078231, 1, 0 } },
  { "NIKON", "D1", Cloudy, 3,			{ 1.063923, 1.032573, 1, 0 } },
  { "NIKON", "D1", Shade, -3,			{ 1.361330, 1.191729, 1, 0 } },
  { "NIKON", "D1", Shade, 0,			{ 1.284963, 1.136201, 1, 0 } },
  { "NIKON", "D1", Shade, 3,			{ 1.205117, 1.096886, 1, 0 } },

  { "NIKON", "D1H", Incandescent, -3,		{ 1.503906, 1, 1.832031, 0 } },
  { "NIKON", "D1H", Incandescent, 0,		{ 1.363281, 1, 1.996094, 0 } },
  { "NIKON", "D1H", Incandescent, 3,		{ 1.246094, 1, 2.148438, 0 } },
  { "NIKON", "D1H", Fluorescent, -3,		{ 2.546875, 1, 1.175781, 0 } },
  { "NIKON", "D1H", Fluorescent, 0,		{ 1.925781, 1, 2.054688, 0 } },
  { "NIKON", "D1H", Fluorescent, 3,		{ 1.234375, 1, 2.171875, 0 } },
  { "NIKON", "D1H", DirectSunlight, -3,		{ 2.230469, 1, 1.187500, 0 } },
  { "NIKON", "D1H", DirectSunlight, 0,		{ 2.148438, 1, 1.246094, 0 } },
  { "NIKON", "D1H", DirectSunlight, 3,		{ 2.066406, 1, 1.316406, 0 } },
  { "NIKON", "D1H", Flash, -3,			{ 2.453125, 1, 1.117188, 0 } },
  { "NIKON", "D1H", Flash, 0,			{ 2.347656, 1, 1.140625, 0 } },
  { "NIKON", "D1H", Flash, 3,			{ 2.242188, 1, 1.164062, 0 } },
  { "NIKON", "D1H", Cloudy, -3,			{ 2.441406, 1, 1.046875, 0 } },
  { "NIKON", "D1H", Cloudy, 0,			{ 2.300781, 1, 1.128906, 0 } },
  { "NIKON", "D1H", Cloudy, 3,			{ 2.207031, 1, 1.199219, 0 } },
  { "NIKON", "D1H", Shade, -3,			{ 2.839844, 1, 1.000000, 0 } },
  { "NIKON", "D1H", Shade, 0,			{ 2.628906, 1, 1.011719, 0 } },
  { "NIKON", "D1H", Shade, 3,			{ 2.441406, 1, 1.046875, 0 } },

  { "NIKON", "D1X", Incandescent, -3,		{ 1.503906, 1, 1.832031, 0 } }, /*3250K*/
  { "NIKON", "D1X", Incandescent, -2,		{ 1.445312, 1, 1.890625, 0 } }, /*3150K*/
  { "NIKON", "D1X", Incandescent, -1,		{ 1.410156, 1, 1.937500, 0 } }, /*3100K*/
  { "NIKON", "D1X", Incandescent, 0,		{ 1.363281, 1, 1.996094, 0 } }, /*3000K*/
  { "NIKON", "D1X", Incandescent, 1,		{ 1.316406, 1, 2.042969, 0 } }, /*2900K*/
  { "NIKON", "D1X", Incandescent, 2,		{ 1.281250, 1, 2.101562, 0 } }, /*2800K*/
  { "NIKON", "D1X", Incandescent, 3,		{ 1.246094, 1, 2.148438, 0 } }, /*2700K*/
  { "NIKON", "D1X", Fluorescent, -3,		{ 2.546875, 1, 1.175781, 0 } }, /*7200K*/
  { "NIKON", "D1X", Fluorescent, -2,		{ 2.464844, 1, 1.210938, 0 } }, /*6500K*/
  { "NIKON", "D1X", Fluorescent, -1,		{ 2.160156, 1, 1.386719, 0 } }, /*5000K*/
  { "NIKON", "D1X", Fluorescent, 0,		{ 1.925781, 1, 2.054688, 0 } }, /*4200K*/
  { "NIKON", "D1X", Fluorescent, 1,		{ 1.703125, 1, 2.277344, 0 } }, /*3700K*/
  { "NIKON", "D1X", Fluorescent, 2,		{ 1.328125, 1, 2.394531, 0 } }, /*3000K*/
  { "NIKON", "D1X", Fluorescent, 3,		{ 1.234375, 1, 2.171875, 0 } }, /*2700K*/
  { "NIKON", "D1X", DirectSunlight, -3,		{ 2.230469, 1, 1.187500, 0 } }, /*5600K*/
  { "NIKON", "D1X", DirectSunlight, -2,		{ 2.207031, 1, 1.210938, 0 } }, /*5400K*/
  { "NIKON", "D1X", DirectSunlight, -1,		{ 2.171875, 1, 1.222656, 0 } }, /*5300K*/
  { "NIKON", "D1X", DirectSunlight, 0,		{ 2.148438, 1, 1.246094, 0 } }, /*5200K*/
  { "NIKON", "D1X", DirectSunlight, 1,		{ 2.113281, 1, 1.269531, 0 } }, /*5000K*/
  { "NIKON", "D1X", DirectSunlight, 2,		{ 2.089844, 1, 1.292969, 0 } }, /*4900K*/
  { "NIKON", "D1X", DirectSunlight, 3,		{ 2.066406, 1, 1.316406, 0 } }, /*4800K*/
  { "NIKON", "D1X", Flash, -3,			{ 2.453125, 1, 1.117188, 0 } }, /*6000K*/
  { "NIKON", "D1X", Flash, -2,			{ 2.417969, 1, 1.128906, 0 } }, /*5800K*/
  { "NIKON", "D1X", Flash, -1,			{ 2.382812, 1, 1.128906, 0 } }, /*5600K*/
  { "NIKON", "D1X", Flash, 0,			{ 2.347656, 1, 1.140625, 0 } }, /*5400K*/
  { "NIKON", "D1X", Flash, 1,			{ 2.312500, 1, 1.152344, 0 } }, /*5200K*/
  { "NIKON", "D1X", Flash, 2,			{ 2.277344, 1, 1.164062, 0 } }, /*5000K*/
  { "NIKON", "D1X", Flash, 3,			{ 2.242188, 1, 1.164062, 0 } }, /*4800K*/
  { "NIKON", "D1X", Cloudy, -3,			{ 2.441406, 1, 1.046875, 0 } }, /*6600K*/
  { "NIKON", "D1X", Cloudy, -2,			{ 2.394531, 1, 1.082031, 0 } }, /*6400K*/
  { "NIKON", "D1X", Cloudy, -1,			{ 2.347656, 1, 1.105469, 0 } }, /*6200K*/
  { "NIKON", "D1X", Cloudy, 0,			{ 2.300781, 1, 1.128906, 0 } }, /*6000K*/
  { "NIKON", "D1X", Cloudy, 1,			{ 2.253906, 1, 1.164062, 0 } }, /*5800K*/
  { "NIKON", "D1X", Cloudy, 2,			{ 2.230469, 1, 1.187500, 0 } }, /*5600K*/
  { "NIKON", "D1X", Cloudy, 3,			{ 2.207031, 1, 1.199219, 0 } }, /*5400K*/
  { "NIKON", "D1X", Shade, -3,			{ 2.839844, 1, 1.000000, 0 } }, /*9200K*/
  { "NIKON", "D1X", Shade, -2,			{ 2.769531, 1, 1.000000, 0 } }, /*8800K*/
  { "NIKON", "D1X", Shade, -1,			{ 2.699219, 1, 1.000000, 0 } }, /*8400K*/
  { "NIKON", "D1X", Shade, 0,			{ 2.628906, 1, 1.011719, 0 } }, /*8000K*/
  { "NIKON", "D1X", Shade, 1,			{ 2.558594, 1, 1.023438, 0 } }, /*7500K*/
  { "NIKON", "D1X", Shade, 2,			{ 2.500000, 1, 1.035156, 0 } }, /*7100K*/
  { "NIKON", "D1X", Shade, 3,			{ 2.441406, 1, 1.046875, 0 } }, /*6700K*/

  /*
   * D2X with firmware A 1.01 and B 1.01
   */

  /* D2X basic + fine tune presets */
  { "NIKON", "D2X", Incandescent, -3,		{ 0.98462, 1, 2.61154, 0 } }, /*3300K*/
  { "NIKON", "D2X", Incandescent, -2,		{ 0.95880, 1, 2.71536, 0 } }, /*3200K*/
  { "NIKON", "D2X", Incandescent, -1,		{ 0.94465, 1, 2.77122, 0 } }, /*3100K*/
  { "NIKON", "D2X", Incandescent, 0,		{ 0.92086, 1, 2.89928, 0 } }, /*3000K*/
  { "NIKON", "D2X", Incandescent, 1,		{ 0.89510, 1, 3.03846, 0 } }, /*2900K*/
  { "NIKON", "D2X", Incandescent, 2,		{ 0.86486, 1, 3.17905, 0 } }, /*2800K*/
  { "NIKON", "D2X", Incandescent, 3,		{ 0.83388, 1, 3.34528, 0 } }, /*2700K*/
  { "NIKON", "D2X", Fluorescent, -3,		{ 2.01562, 1, 1.72266, 0 } }, /*7200K*/
  { "NIKON", "D2X", Fluorescent, -2,		{ 1.67969, 1, 1.42578, 0 } }, /*6500K*/
  { "NIKON", "D2X", Fluorescent, -1,		{ 1.42969, 1, 1.80078, 0 } }, /*5000K*/
  { "NIKON", "D2X", Fluorescent, 0,		{ 1.42969, 1, 2.62891, 0 } }, /*4200K*/
  { "NIKON", "D2X", Fluorescent, 1,		{ 1.13672, 1, 3.02734, 0 } }, /*3700K*/
  { "NIKON", "D2X", Fluorescent, 2,		{ 0.94118, 1, 2.68498, 0 } }, /*3000K*/
  { "NIKON", "D2X", Fluorescent, 3,		{ 0.83388, 1, 3.51140, 0 } }, /*2700K*/
  { "NIKON", "D2X", DirectSunlight, -3,		{ 1.61328, 1, 1.61328, 0 } }, /*5600K*/
  { "NIKON", "D2X", DirectSunlight, -2,		{ 1.57031, 1, 1.65234, 0 } }, /*5400K*/
  { "NIKON", "D2X", DirectSunlight, -1,		{ 1.55078, 1, 1.67578, 0 } }, /*5300K*/
  { "NIKON", "D2X", DirectSunlight, 0,		{ 1.52734, 1, 1.69531, 0 } }, /*5200K*/
  { "NIKON", "D2X", DirectSunlight, 1,		{ 1.48438, 1, 1.74609, 0 } }, /*5000K*/
  { "NIKON", "D2X", DirectSunlight, 2,		{ 1.45312, 1, 1.76953, 0 } }, /*4900K*/
  { "NIKON", "D2X", DirectSunlight, 3,		{ 1.42578, 1, 1.78906, 0 } }, /*4800K*/
  { "NIKON", "D2X", Flash, -3,			{ 1.71484, 1, 1.48438, 0 } }, /*6000K*/
  { "NIKON", "D2X", Flash, -2,			{ 1.67578, 1, 1.48438, 0 } }, /*5800K*/
  { "NIKON", "D2X", Flash, -1,			{ 1.66797, 1, 1.50781, 0 } }, /*5600K*/
  { "NIKON", "D2X", Flash, 0,			{ 1.66016, 1, 1.53125, 0 } }, /*5400K*/
  { "NIKON", "D2X", Flash, 1,			{ 1.64453, 1, 1.54297, 0 } }, /*5200K*/
  { "NIKON", "D2X", Flash, 2,			{ 1.62891, 1, 1.54297, 0 } }, /*5000K*/
  { "NIKON", "D2X", Flash, 3,			{ 1.57031, 1, 1.56641, 0 } }, /*4800K*/
  { "NIKON", "D2X", Cloudy, -3,			{ 1.79297, 1, 1.46875, 0 } }, /*6600K*/
  { "NIKON", "D2X", Cloudy, -2,			{ 1.76172, 1, 1.49219, 0 } }, /*6400K*/
  { "NIKON", "D2X", Cloudy, -1,			{ 1.72656, 1, 1.51953, 0 } }, /*6200K*/
  { "NIKON", "D2X", Cloudy, 0,			{ 1.69141, 1, 1.54688, 0 } }, /*6000K*/
  { "NIKON", "D2X", Cloudy, 1,			{ 1.65234, 1, 1.57812, 0 } }, /*5800K*/
  { "NIKON", "D2X", Cloudy, 2,			{ 1.61328, 1, 1.61328, 0 } }, /*5600K*/
  { "NIKON", "D2X", Cloudy, 3,			{ 1.57031, 1, 1.65234, 0 } }, /*5400K*/
  { "NIKON", "D2X", Shade, -3,			{ 2.10938, 1, 1.23828, 0 } }, /*9200K*/
  { "NIKON", "D2X", Shade, -2,			{ 2.07031, 1, 1.26562, 0 } }, /*8800K*/
  { "NIKON", "D2X", Shade, -1,			{ 2.02734, 1, 1.29688, 0 } }, /*8400K*/
  { "NIKON", "D2X", Shade, 0,			{ 1.98047, 1, 1.32812, 0 } }, /*8000K*/
  { "NIKON", "D2X", Shade, 1,			{ 1.92188, 1, 1.37109, 0 } }, /*7500K*/
  { "NIKON", "D2X", Shade, 2,			{ 1.86719, 1, 1.41406, 0 } }, /*7100K*/
  { "NIKON", "D2X", Shade, 3,			{ 1.80859, 1, 1.45703, 0 } }, /*6700K*/

  /* D2X Kelvin presets */
  { "NIKON", "D2X", "2700K", 0,			{ 0.83388, 1, 3.34528, 0 } },
  { "NIKON", "D2X", "3000K", 0,			{ 0.92086, 1, 2.89928, 0 } },
  { "NIKON", "D2X", "3300K", 0,			{ 0.99611, 1, 2.55642, 0 } },
  { "NIKON", "D2X", "5000K", 0,			{ 1.48438, 1, 1.74609, 0 } },

  { "NIKON", "D3", Daylight, 0,			{ 1.81640, 1, 1.35546, 0 } },
  { "NIKON", "D3", Flash, 0,			{ 2.03906, 1, 1.17187, 0 } },
  { "NIKON", "D3", Cloudy, 0,			{ 1.94921, 1, 1.22265, 0 } },
  { "NIKON", "D3", Shade, 0,			{ 2.24609, 1, 1.08593, 0 } },
  { "NIKON", "D3", Incandescent, 0,		{ 1.16796, 1, 2.31640, 0 } },
  { "NIKON", "D3", Fluorescent, 0,		{ 1.68750, 1, 2.10156, 0 } },
  { "NIKON", "D3", "2700K", 0,			{ 1.06640, 1, 2.67968, 0 } },
  { "NIKON", "D3", "3000K", 0,			{ 1.16796, 1, 2.31640, 0 } },
  { "NIKON", "D3", "5000K", 0,			{ 1.77734, 1, 1.40234, 0 } },

  { "NIKON", "D3S", Incandescent, 0,		{ 1.191406, 1, 2.242188, 0 } },
  { "NIKON", "D3S", SodiumVaporFluorescent, 0,	{ 1.132812, 1, 2.511719, 0 } },
  { "NIKON", "D3S", WarmWhiteFluorescent, 0,	{ 1.179688, 1, 1.996094, 0 } },
  { "NIKON", "D3S", WhiteFluorescent, 0,	{ 1.394531, 1, 2.402344, 0 } },
  { "NIKON", "D3S", CoolWhiteFluorescent, 0,	{ 1.703125, 1, 2.066406, 0 } },
  { "NIKON", "D3S", DayWhiteFluorescent, 0,	{ 1.710938, 1, 1.390625, 0 } },
  { "NIKON", "D3S", DaylightFluorescent, 0,	{ 1.941406, 1, 1.113281, 0 } },
  { "NIKON", "D3S", HighTempMercuryVaporFluorescent, 0, { 2.289062, 1, 1.355469, 0 } },
  { "NIKON", "D3S", DirectSunlight, 0,		{ 1.835938, 1, 1.359375, 0 } },
  { "NIKON", "D3S", Flash, 0,			{ 2.035156, 1, 1.183594, 0 } },
  { "NIKON", "D3S", Cloudy, 0,			{ 1.964844, 1, 1.226562, 0 } },
  { "NIKON", "D3S", Shade, 0,			{ 2.253906, 1, 1.089844, 0 } },
  { "NIKON", "D3S", "2700K", 0,			{ 1.093750, 1, 2.558594, 0 } },
  { "NIKON", "D3S", "5000K", 0,			{ 1.796875, 1, 1.402344, 0 } },

  /* D3X with firmware A 1.00 and B 1.01 */
  { "NIKON", "D3X", Incandescent, -4,		{ 1.441406, 1, 2.125000, 0 } },
  { "NIKON", "D3X", Incandescent, -3,		{ 1.421875, 1, 2.167969, 0 } },
  { "NIKON", "D3X", Incandescent, -2,		{ 1.402344, 1, 2.210938, 0 } },
  { "NIKON", "D3X", Incandescent, -1,		{ 1.382813, 1, 2.250000, 0 } },
  { "NIKON", "D3X", Incandescent, 0,		{ 1.367188, 1, 2.292969, 0 } },
  { "NIKON", "D3X", Incandescent, 1,		{ 1.351563, 1, 2.332031, 0 } },
  { "NIKON", "D3X", Incandescent, 2,		{ 1.332031, 1, 2.371093, 0 } },
  { "NIKON", "D3X", Incandescent, 3,		{ 1.316406, 1, 2.414063, 0 } },
  { "NIKON", "D3X", Incandescent, 4,		{ 1.300781, 1, 2.457031, 0 } },
  { "NIKON", "D3X", Fluorescent, -4,		{ 2.183594, 1, 1.980469, 0 } },
  { "NIKON", "D3X", Fluorescent, -3,		{ 2.136719, 1, 2.015625, 0 } },
  { "NIKON", "D3X", Fluorescent, -2,		{ 2.089844, 1, 2.054688, 0 } },
  { "NIKON", "D3X", Fluorescent, -1,		{ 2.039064, 1, 2.089844, 0 } },
  { "NIKON", "D3X", Fluorescent, 0,		{ 1.984375, 1, 2.128906, 0 } },
  { "NIKON", "D3X", Fluorescent, 1,		{ 1.929688, 1, 2.167969, 0 } },
  { "NIKON", "D3X", Fluorescent, 2,		{ 1.875000, 1, 2.207031, 0 } },
  { "NIKON", "D3X", Fluorescent, 3,		{ 1.816406, 1, 2.246094, 0 } },
  { "NIKON", "D3X", Fluorescent, 4,		{ 1.753906, 1, 2.292969, 0 } },
  { "NIKON", "D3X", DirectSunlight, -4,		{ 2.289063, 1, 1.308594, 0 } },
  { "NIKON", "D3X", DirectSunlight, -3,		{ 2.253906, 1, 1.335938, 0 } },
  { "NIKON", "D3X", DirectSunlight, -2,		{ 2.222656, 1, 1.359375, 0 } },
  { "NIKON", "D3X", DirectSunlight, -1,		{ 2.187500, 1, 1.386719, 0 } },
  { "NIKON", "D3X", DirectSunlight, 0,		{ 2.156250, 1, 1.417969, 0 } },
  { "NIKON", "D3X", DirectSunlight, 1,		{ 2.125000, 1, 1.445313, 0 } },
  { "NIKON", "D3X", DirectSunlight, 2,		{ 2.093750, 1, 1.472656, 0 } },
  { "NIKON", "D3X", DirectSunlight, 3,		{ 2.062500, 1, 1.496094, 0 } },
  { "NIKON", "D3X", DirectSunlight, 4,		{ 2.027344, 1, 1.519531, 0 } },
  { "NIKON", "D3X", Flash, -4,			{ 2.566406, 1, 1.183594, 0 } },
  { "NIKON", "D3X", Flash, -3,			{ 2.523438, 1, 1.199219, 0 } },
  { "NIKON", "D3X", Flash, -2,			{ 2.484375, 1, 1.214844, 0 } },
  { "NIKON", "D3X", Flash, -1,			{ 2.445313, 1, 1.226563, 0 } },
  { "NIKON", "D3X", Flash, 0,			{ 2.402344, 1, 1.242187, 0 } },
  { "NIKON", "D3X", Flash, 1,			{ 2.371094, 1, 1.257813, 0 } },
  { "NIKON", "D3X", Flash, 2,			{ 2.343750, 1, 1.273438, 0 } },
  { "NIKON", "D3X", Flash, 3,			{ 2.320313, 1, 1.292969, 0 } },
  { "NIKON", "D3X", Flash, 4,			{ 2.289063, 1, 1.308594, 0 } },
  { "NIKON", "D3X", Cloudy, -4,			{ 2.488281, 1, 1.214844, 0 } },
  { "NIKON", "D3X", Cloudy, -3,			{ 2.445313, 1, 1.230469, 0 } },
  { "NIKON", "D3X", Cloudy, -2,			{ 2.406250, 1, 1.250000, 0 } },
  { "NIKON", "D3X", Cloudy, -1,			{ 2.363281, 1, 1.265625, 0 } },
  { "NIKON", "D3X", Cloudy, 0,			{ 2.328125, 1, 1.289062, 0 } },
  { "NIKON", "D3X", Cloudy, 1,			{ 2.289063, 1, 1.308594, 0 } },
  { "NIKON", "D3X", Cloudy, 2,			{ 2.253906, 1, 1.335938, 0 } },
  { "NIKON", "D3X", Cloudy, 3,			{ 2.222656, 1, 1.359375, 0 } },
  { "NIKON", "D3X", Cloudy, 4,			{ 2.187500, 1, 1.386719, 0 } },
  { "NIKON", "D3X", Shade, -4,			{ 2.937500, 1, 1.089844, 0 } },
  { "NIKON", "D3X", Shade, -3,			{ 2.878906, 1, 1.113281, 0 } },
  { "NIKON", "D3X", Shade, -2,			{ 2.820313, 1, 1.128906, 0 } },
  { "NIKON", "D3X", Shade, -1,			{ 2.761719, 1, 1.144531, 0 } },
  { "NIKON", "D3X", Shade, 0,			{ 2.707031, 1, 1.160156, 0 } },
  { "NIKON", "D3X", Shade, 1,			{ 2.652344, 1, 1.171875, 0 } },
  { "NIKON", "D3X", Shade, 2,			{ 2.601563, 1, 1.183594, 0 } },
  { "NIKON", "D3X", Shade, 3,			{ 2.554688, 1, 1.199219, 0 } },
  { "NIKON", "D3X", Shade, 4,			{ 2.507813, 1, 1.210938, 0 } },

  /* D3X Kelvin presets */
  { "NIKON", "D3X", "2700K", 0,			{ 1.253906, 1, 2.605469, 0 } },
  { "NIKON", "D3X", "5000K", 0,			{ 2.105469, 1, 1.460938, 0 } },

  { "NIKON", "D100", Incandescent, -3,		{ 1.527344, 1, 2.539062, 0 } }, /*3300K*/
  { "NIKON", "D100", Incandescent, -2,		{ 1.476562, 1, 2.656250, 0 } }, /*3200K*/
  { "NIKON", "D100", Incandescent, -1,		{ 1.457031, 1, 2.707031, 0 } }, /*3100K*/
  { "NIKON", "D100", Incandescent, 0,		{ 1.406250, 1, 2.828125, 0 } }, /*3000K*/
  { "NIKON", "D100", Incandescent, 1,		{ 1.367188, 1, 2.937500, 0 } }, /*2900K*/
  { "NIKON", "D100", Incandescent, 2,		{ 1.316406, 1, 3.046875, 0 } }, /*2800K*/
  { "NIKON", "D100", Incandescent, 3,		{ 1.269531, 1, 3.167969, 0 } }, /*2700K*/
  { "NIKON", "D100", Fluorescent, -3,		{ 3.148438, 1, 1.847656, 0 } }, /*7200K*/
  { "NIKON", "D100", Fluorescent, -2,		{ 2.609375, 1, 1.617187, 0 } }, /*6500K*/
  { "NIKON", "D100", Fluorescent, -1,		{ 2.250000, 1, 2.039062, 0 } }, /*5000K*/
  { "NIKON", "D100", Fluorescent, 0,		{ 2.058594, 1, 2.617187, 0 } }, /*4200K*/
  { "NIKON", "D100", Fluorescent, 1,		{ 1.886719, 1, 2.726562, 0 } }, /*3700K*/
  { "NIKON", "D100", Fluorescent, 2,		{ 1.429688, 1, 3.359375, 0 } }, /*3000K*/
  { "NIKON", "D100", Fluorescent, 3,		{ 1.250000, 1, 2.699219, 0 } }, /*2700K*/
  { "NIKON", "D100", DirectSunlight, -3,	{ 2.386719, 1, 1.687500, 0 } }, /*5600K*/
  { "NIKON", "D100", DirectSunlight, -2,	{ 2.316406, 1, 1.726563, 0 } }, /*5400K*/
  { "NIKON", "D100", DirectSunlight, -1,	{ 2.296875, 1, 1.738281, 0 } }, /*5300K*/
  { "NIKON", "D100", DirectSunlight, 0,		{ 2.257812, 1, 1.757812, 0 } }, /*5200K*/
  { "NIKON", "D100", DirectSunlight, 1,		{ 2.187500, 1, 1.796875, 0 } }, /*5000K*/
  { "NIKON", "D100", DirectSunlight, 2,		{ 2.156250, 1, 1.816406, 0 } }, /*4900K*/
  { "NIKON", "D100", DirectSunlight, 3,		{ 2.117187, 1, 1.847656, 0 } }, /*4800K*/
  { "NIKON", "D100", Flash, -3,			{ 2.718750, 1, 1.519531, 0 } }, /*6000K*/
  { "NIKON", "D100", Flash, -2,			{ 2.656250, 1, 1.527344, 0 } }, /*5800K*/
  { "NIKON", "D100", Flash, -1,			{ 2.597656, 1, 1.527344, 0 } }, /*5600K*/
  { "NIKON", "D100", Flash, 0,			{ 2.539062, 1, 1.539062, 0 } }, /*5400K*/
  { "NIKON", "D100", Flash, 1,			{ 2.476562, 1, 1.539062, 0 } }, /*5200K*/
  { "NIKON", "D100", Flash, 2,			{ 2.437500, 1, 1.546875, 0 } }, /*5000K*/
  { "NIKON", "D100", Flash, 3,			{ 2.398438, 1, 1.546875, 0 } }, /*4800K*/
  { "NIKON", "D100", Cloudy, -3,		{ 2.648438, 1, 1.558594, 0 } }, /*6600K*/
  { "NIKON", "D100", Cloudy, -2,		{ 2.609375, 1, 1.578125, 0 } }, /*6400K*/
  { "NIKON", "D100", Cloudy, -1,		{ 2.558594, 1, 1.597656, 0 } }, /*6200K*/
  { "NIKON", "D100", Cloudy, 0,			{ 2.507813, 1, 1.628906, 0 } }, /*6000K*/
  { "NIKON", "D100", Cloudy, 1,			{ 2.449219, 1, 1.656250, 0 } }, /*5800K*/
  { "NIKON", "D100", Cloudy, 2,			{ 2.398438, 1, 1.687500, 0 } }, /*5600K*/
  { "NIKON", "D100", Cloudy, 3,			{ 2.316406, 1, 1.726563, 0 } }, /*5400K*/
  { "NIKON", "D100", Shade, -3,			{ 3.046875, 1, 1.386719, 0 } }, /*9200K*/
  { "NIKON", "D100", Shade, -2,			{ 3.000000, 1, 1.406250, 0 } }, /*8800K*/
  { "NIKON", "D100", Shade, -1,			{ 2.957031, 1, 1.417969, 0 } }, /*8400K*/
  { "NIKON", "D100", Shade, 0,			{ 2.906250, 1, 1.437500, 0 } }, /*8000K*/
  { "NIKON", "D100", Shade, 1,			{ 2.816406, 1, 1.476562, 0 } }, /*7500K*/
  { "NIKON", "D100", Shade, 2,			{ 2.750000, 1, 1.519531, 0 } }, /*7100K*/
  { "NIKON", "D100", Shade, 3,			{ 2.667969, 1, 1.546875, 0 } }, /*6700K*/

  /* D200 basic + fine tune WB presets */
  { "NIKON", "D200", Incandescent, -2,		{ 1.199219, 1, 2.238281, 0 } },
  { "NIKON", "D200", Incandescent, -1,		{ 1.183594, 1, 2.289063, 0 } },
  { "NIKON", "D200", Incandescent, 0,		{ 1.148437, 1, 2.398438, 0 } },
  { "NIKON", "D200", Incandescent, 1,		{ 1.113281, 1, 2.519531, 0 } },
  { "NIKON", "D200", Incandescent, 2,		{ 1.074219, 1, 2.648438, 0 } },
  { "NIKON", "D200", Incandescent, 3,		{ 1.031250, 1, 2.804688, 0 } },
  { "NIKON", "D200", Fluorescent, -3,		{ 2.273438, 1, 1.410156, 0 } },
  { "NIKON", "D200", Fluorescent, -2,		{ 1.933594, 1, 1.152344, 0 } },
  { "NIKON", "D200", Fluorescent, -1,		{ 1.675781, 1, 1.453125, 0 } },
  { "NIKON", "D200", Fluorescent, 0,		{ 1.664062, 1, 2.148437, 0 } },
  { "NIKON", "D200", Fluorescent, 1,		{ 1.335937, 1, 2.453125, 0 } },
  { "NIKON", "D200", Fluorescent, 2,		{ 1.140625, 1, 2.214844, 0 } },
  { "NIKON", "D200", Fluorescent, 3,		{ 1.035156, 1, 2.410156, 0 } },
  { "NIKON", "D200", DirectSunlight, -3,	{ 1.863281, 1, 1.320312, 0 } },
  { "NIKON", "D200", DirectSunlight, -2,	{ 1.835938, 1, 1.355469, 0 } },
  { "NIKON", "D200", DirectSunlight, -1,	{ 1.820313, 1, 1.375000, 0 } },
  { "NIKON", "D200", DirectSunlight, 0,		{ 1.804688, 1, 1.398437, 0 } },
  { "NIKON", "D200", DirectSunlight, 1,		{ 1.746094, 1, 1.425781, 0 } },
  { "NIKON", "D200", DirectSunlight, 2,		{ 1.714844, 1, 1.437500, 0 } },
  { "NIKON", "D200", DirectSunlight, 3,		{ 1.687500, 1, 1.449219, 0 } },
  { "NIKON", "D200", Flash, -3,			{ 2.066406, 1, 1.183594, 0 } },
  { "NIKON", "D200", Flash, -2,			{ 2.046875, 1, 1.191406, 0 } },
  { "NIKON", "D200", Flash, -1,			{ 2.027344, 1, 1.199219, 0 } },
  { "NIKON", "D200", Flash, 0,			{ 2.007813, 1, 1.171875, 0 } },
  { "NIKON", "D200", Flash, 1,			{ 1.984375, 1, 1.207031, 0 } },
  { "NIKON", "D200", Flash, 2,			{ 1.964844, 1, 1.214844, 0 } },
  { "NIKON", "D200", Flash, 3,			{ 1.945312, 1, 1.222656, 0 } },
  { "NIKON", "D200", Cloudy, -3,		{ 2.027344, 1, 1.210937, 0 } },
  { "NIKON", "D200", Cloudy, -2,		{ 1.992187, 1, 1.226562, 0 } },
  { "NIKON", "D200", Cloudy, -1,		{ 1.953125, 1, 1.242187, 0 } },
  { "NIKON", "D200", Cloudy, 0,			{ 1.917969, 1, 1.261719, 0 } },
  { "NIKON", "D200", Cloudy, 1,			{ 1.890625, 1, 1.285156, 0 } },
  { "NIKON", "D200", Cloudy, 2,			{ 1.863281, 1, 1.320312, 0 } },
  { "NIKON", "D200", Cloudy, 3,			{ 1.835938, 1, 1.355469, 0 } },
  { "NIKON", "D200", Shade, -3,			{ 2.378906, 1, 1.066406, 0 } },
  { "NIKON", "D200", Shade, -2,			{ 2.332031, 1, 1.085938, 0 } },
  { "NIKON", "D200", Shade, -1,			{ 2.289063, 1, 1.105469, 0 } },
  { "NIKON", "D200", Shade, 0,			{ 2.234375, 1, 1.125000, 0 } },
  { "NIKON", "D200", Shade, 1,			{ 2.167969, 1, 1.152344, 0 } },
  { "NIKON", "D200", Shade, 2,			{ 2.105469, 1, 1.175781, 0 } },
  { "NIKON", "D200", Shade, 3,			{ 2.046875, 1, 1.199219, 0 } },

  /* D200 Kelvin presets */
  { "NIKON", "D200", "2700K", 0,		{ 1.031250, 1, 2.804688, 0 } },
  { "NIKON", "D200", "3000K", 0,		{ 1.148438, 1, 2.398438, 0 } },
  { "NIKON", "D200", "3300K", 0,		{ 1.250000, 1, 2.097656, 0 } },
  { "NIKON", "D200", "5000K", 0,		{ 1.746094, 1, 1.425781, 0 } },

  { "NIKON", "D300", Incandescent, -6,		{ 1.097656, 1, 1.898438, 0 } },
  { "NIKON", "D300", Incandescent, -5,		{ 1.085938, 1, 1.929688, 0 } },
  { "NIKON", "D300", Incandescent, -4,		{ 1.070313, 1, 1.964844, 0 } },
  { "NIKON", "D300", Incandescent, -3,		{ 1.058594, 1, 2.000000, 0 } },
  { "NIKON", "D300", Incandescent, -2,		{ 1.042969, 1, 2.035156, 0 } },
  { "NIKON", "D300", Incandescent, -1,		{ 1.031250, 1, 2.074219, 0 } },
  { "NIKON", "D300", Incandescent, 0,		{ 1.019531, 1, 2.109375, 0 } },
  { "NIKON", "D300", Incandescent, 1,		{ 1.007813, 1, 2.144531, 0 } },
  { "NIKON", "D300", Incandescent, 2,		{ 0.996094, 1, 2.183594, 0 } },
  { "NIKON", "D300", Incandescent, 3,		{ 0.984375, 1, 2.218750, 0 } },
  { "NIKON", "D300", Incandescent, 4,		{ 0.972656, 1, 2.257813, 0 } },
  { "NIKON", "D300", Incandescent, 5,		{ 0.964844, 1, 2.296875, 0 } },
  { "NIKON", "D300", Incandescent, 6,		{ 0.953125, 1, 2.335938, 0 } },
  { "NIKON", "D300", SodiumVaporFluorescent, -6, { 1.031250, 1, 2.101563, 0 } },
  { "NIKON", "D300", SodiumVaporFluorescent, -5, { 1.015625, 1, 2.136719, 0 } },
  { "NIKON", "D300", SodiumVaporFluorescent, -4, { 1.003906, 1, 2.167969, 0 } },
  { "NIKON", "D300", SodiumVaporFluorescent, -3, { 0.988281, 1, 2.207031, 0 } },
  { "NIKON", "D300", SodiumVaporFluorescent, -2, { 0.976563, 1, 2.242188, 0 } },
  { "NIKON", "D300", SodiumVaporFluorescent, -1, { 0.960938, 1, 2.281250, 0 } },
  { "NIKON", "D300", SodiumVaporFluorescent, 0,	{ 0.949219, 1, 2.320313, 0 } },
  { "NIKON", "D300", SodiumVaporFluorescent, 1,	{ 0.937500, 1, 2.363281, 0 } },
  { "NIKON", "D300", SodiumVaporFluorescent, 2,	{ 0.925781, 1, 2.410156, 0 } },
  { "NIKON", "D300", SodiumVaporFluorescent, 3,	{ 0.914063, 1, 2.457031, 0 } },
  { "NIKON", "D300", SodiumVaporFluorescent, 4,	{ 0.902344, 1, 2.503906, 0 } },
  { "NIKON", "D300", SodiumVaporFluorescent, 5,	{ 0.890625, 1, 2.558594, 0 } },
  { "NIKON", "D300", SodiumVaporFluorescent, 6,	{ 0.878906, 1, 2.613281, 0 } },
  { "NIKON", "D300", WarmWhiteFluorescent, -6,	{ 1.128906, 1, 1.847656, 0 } },
  { "NIKON", "D300", WarmWhiteFluorescent, -5,	{ 1.113281, 1, 1.867188, 0 } },
  { "NIKON", "D300", WarmWhiteFluorescent, -4,	{ 1.097656, 1, 1.886719, 0 } },
  { "NIKON", "D300", WarmWhiteFluorescent, -3,	{ 1.085938, 1, 1.906250, 0 } },
  { "NIKON", "D300", WarmWhiteFluorescent, -2,	{ 1.070313, 1, 1.925781, 0 } },
  { "NIKON", "D300", WarmWhiteFluorescent, -1,	{ 1.058594, 1, 1.945313, 0 } },
  { "NIKON", "D300", WarmWhiteFluorescent, 0,	{ 1.046875, 1, 1.960938, 0 } },
  { "NIKON", "D300", WarmWhiteFluorescent, 1,	{ 1.035156, 1, 1.980469, 0 } },
  { "NIKON", "D300", WarmWhiteFluorescent, 2,	{ 1.023438, 1, 1.996094, 0 } },
  { "NIKON", "D300", WarmWhiteFluorescent, 3,	{ 1.007813, 1, 2.015625, 0 } },
  { "NIKON", "D300", WarmWhiteFluorescent, 4,	{ 1.000000, 1, 2.031250, 0 } },
  { "NIKON", "D300", WarmWhiteFluorescent, 5,	{ 0.988281, 1, 2.046875, 0 } },
  { "NIKON", "D300", WarmWhiteFluorescent, 6,	{ 0.976563, 1, 2.062500, 0 } },
  { "NIKON", "D300", WhiteFluorescent, -6,	{ 1.453125, 1, 2.050781, 0 } },
  { "NIKON", "D300", WhiteFluorescent, -5,	{ 1.414063, 1, 2.093750, 0 } },
  { "NIKON", "D300", WhiteFluorescent, -4,	{ 1.371094, 1, 2.132813, 0 } },
  { "NIKON", "D300", WhiteFluorescent, -3,	{ 1.328125, 1, 2.175781, 0 } },
  { "NIKON", "D300", WhiteFluorescent, -2,	{ 1.285156, 1, 2.218750, 0 } },
  { "NIKON", "D300", WhiteFluorescent, -1,	{ 1.238281, 1, 2.261719, 0 } },
  { "NIKON", "D300", WhiteFluorescent, 0,	{ 1.191406, 1, 2.304688, 0 } },
  { "NIKON", "D300", WhiteFluorescent, 1,	{ 1.140625, 1, 2.351563, 0 } },
  { "NIKON", "D300", WhiteFluorescent, 2,	{ 1.089844, 1, 2.394531, 0 } },
  { "NIKON", "D300", WhiteFluorescent, 3,	{ 1.039063, 1, 2.441406, 0 } },
  { "NIKON", "D300", WhiteFluorescent, 4,	{ 0.984375, 1, 2.488281, 0 } },
  { "NIKON", "D300", WhiteFluorescent, 5,	{ 0.925781, 1, 2.535156, 0 } },
  { "NIKON", "D300", WhiteFluorescent, 6,	{ 0.867188, 1, 2.582031, 0 } },
  { "NIKON", "D300", CoolWhiteFluorescent, -6,	{ 1.667969, 1, 1.800781, 0 } },
  { "NIKON", "D300", CoolWhiteFluorescent, -5,	{ 1.636719, 1, 1.835938, 0 } },
  { "NIKON", "D300", CoolWhiteFluorescent, -4,	{ 1.605469, 1, 1.875000, 0 } },
  { "NIKON", "D300", CoolWhiteFluorescent, -3,	{ 1.574219, 1, 1.914063, 0 } },
  { "NIKON", "D300", CoolWhiteFluorescent, -2,	{ 1.539063, 1, 1.953125, 0 } },
  { "NIKON", "D300", CoolWhiteFluorescent, -1,	{ 1.503906, 1, 1.996094, 0 } },
  { "NIKON", "D300", CoolWhiteFluorescent, 0,	{ 1.468750, 1, 2.035156, 0 } },
  { "NIKON", "D300", CoolWhiteFluorescent, 1,	{ 1.429688, 1, 2.074219, 0 } },
  { "NIKON", "D300", CoolWhiteFluorescent, 2,	{ 1.386719, 1, 2.117188, 0 } },
  { "NIKON", "D300", CoolWhiteFluorescent, 3,	{ 1.347656, 1, 2.160156, 0 } },
  { "NIKON", "D300", CoolWhiteFluorescent, 4,	{ 1.304688, 1, 2.203125, 0 } },
  { "NIKON", "D300", CoolWhiteFluorescent, 5,	{ 1.257813, 1, 2.246094, 0 } },
  { "NIKON", "D300", CoolWhiteFluorescent, 6,	{ 1.210938, 1, 2.289063, 0 } },
  { "NIKON", "D300", DayWhiteFluorescent, -6,	{ 1.625000, 1, 1.195313, 0 } },
  { "NIKON", "D300", DayWhiteFluorescent, -5,	{ 1.601563, 1, 1.222656, 0 } },
  { "NIKON", "D300", DayWhiteFluorescent, -4,	{ 1.582031, 1, 1.253906, 0 } },
  { "NIKON", "D300", DayWhiteFluorescent, -3,	{ 1.558594, 1, 1.281250, 0 } },
  { "NIKON", "D300", DayWhiteFluorescent, -2,	{ 1.535156, 1, 1.308594, 0 } },
  { "NIKON", "D300", DayWhiteFluorescent, -1,	{ 1.515625, 1, 1.335938, 0 } },
  { "NIKON", "D300", DayWhiteFluorescent, 0,	{ 1.492188, 1, 1.363281, 0 } },
  { "NIKON", "D300", DayWhiteFluorescent, 1,	{ 1.472656, 1, 1.390625, 0 } },
  { "NIKON", "D300", DayWhiteFluorescent, 2,	{ 1.453125, 1, 1.417969, 0 } },
  { "NIKON", "D300", DayWhiteFluorescent, 3,	{ 1.433594, 1, 1.441406, 0 } },
  { "NIKON", "D300", DayWhiteFluorescent, 4,	{ 1.410156, 1, 1.468750, 0 } },
  { "NIKON", "D300", DayWhiteFluorescent, 5,	{ 1.390625, 1, 1.492188, 0 } },
  { "NIKON", "D300", DayWhiteFluorescent, 6,	{ 1.375000, 1, 1.519531, 0 } },
  { "NIKON", "D300", DaylightFluorescent, -6,	{ 1.851563, 1, 1.000000, 0 } },
  { "NIKON", "D300", DaylightFluorescent, -5,	{ 1.824219, 1, 1.000000, 0 } },
  { "NIKON", "D300", DaylightFluorescent, -4,	{ 1.796875, 1, 1.000000, 0 } },
  { "NIKON", "D300", DaylightFluorescent, -3,	{ 1.773438, 1, 1.007813, 0 } },
  { "NIKON", "D300", DaylightFluorescent, -2,	{ 1.750000, 1, 1.039063, 0 } },
  { "NIKON", "D300", DaylightFluorescent, -1,	{ 1.722656, 1, 1.070313, 0 } },
  { "NIKON", "D300", DaylightFluorescent, 0,	{ 1.699219, 1, 1.101563, 0 } },
  { "NIKON", "D300", DaylightFluorescent, 1,	{ 1.675781, 1, 1.128906, 0 } },
  { "NIKON", "D300", DaylightFluorescent, 2,	{ 1.652344, 1, 1.160156, 0 } },
  { "NIKON", "D300", DaylightFluorescent, 3,	{ 1.628906, 1, 1.187500, 0 } },
  { "NIKON", "D300", DaylightFluorescent, 4,	{ 1.605469, 1, 1.218750, 0 } },
  { "NIKON", "D300", DaylightFluorescent, 5,	{ 1.585938, 1, 1.246094, 0 } },
  { "NIKON", "D300", DaylightFluorescent, 6,	{ 1.562500, 1, 1.273438, 0 } },
  { "NIKON", "D300", HighTempMercuryVaporFluorescent, -6, { 2.039063, 1, 1.156250, 0 } },
  { "NIKON", "D300", HighTempMercuryVaporFluorescent, -5, { 2.027344, 1, 1.183594, 0 } },
  { "NIKON", "D300", HighTempMercuryVaporFluorescent, -4, { 2.015625, 1, 1.210938, 0 } },
  { "NIKON", "D300", HighTempMercuryVaporFluorescent, -3, { 2.003906, 1, 1.238281, 0 } },
  { "NIKON", "D300", HighTempMercuryVaporFluorescent, -2, { 1.992188, 1, 1.269531, 0 } },
  { "NIKON", "D300", HighTempMercuryVaporFluorescent, -1, { 1.976563, 1, 1.300781, 0 } },
  { "NIKON", "D300", HighTempMercuryVaporFluorescent, 0, { 1.960938, 1, 1.328125, 0 } },
  { "NIKON", "D300", HighTempMercuryVaporFluorescent, 1, { 1.945313, 1, 1.359375, 0 } },
  { "NIKON", "D300", HighTempMercuryVaporFluorescent, 2, { 1.929688, 1, 1.390625, 0 } },
  { "NIKON", "D300", HighTempMercuryVaporFluorescent, 3, { 1.914063, 1, 1.421875, 0 } },
  { "NIKON", "D300", HighTempMercuryVaporFluorescent, 4, { 1.894531, 1, 1.457031, 0 } },
  { "NIKON", "D300", HighTempMercuryVaporFluorescent, 5, { 1.875000, 1, 1.488281, 0 } },
  { "NIKON", "D300", HighTempMercuryVaporFluorescent, 6, { 1.855469, 1, 1.523438, 0 } },
  { "NIKON", "D300", DirectSunlight, -6,	{ 1.687500, 1, 1.167969, 0 } },
  { "NIKON", "D300", DirectSunlight, -5,	{ 1.664063, 1, 1.187500, 0 } },
  { "NIKON", "D300", DirectSunlight, -4,	{ 1.644531, 1, 1.207031, 0 } },
  { "NIKON", "D300", DirectSunlight, -3,	{ 1.625000, 1, 1.230469, 0 } },
  { "NIKON", "D300", DirectSunlight, -2,	{ 1.601563, 1, 1.253906, 0 } },
  { "NIKON", "D300", DirectSunlight, -1,	{ 1.582031, 1, 1.281250, 0 } },
  { "NIKON", "D300", DirectSunlight, 0,		{ 1.562500, 1, 1.308594, 0 } },
  { "NIKON", "D300", DirectSunlight, 1,		{ 1.542969, 1, 1.335938, 0 } },
  { "NIKON", "D300", DirectSunlight, 2,		{ 1.523438, 1, 1.359375, 0 } },
  { "NIKON", "D300", DirectSunlight, 3,		{ 1.503906, 1, 1.386719, 0 } },
  { "NIKON", "D300", DirectSunlight, 4,		{ 1.480469, 1, 1.414063, 0 } },
  { "NIKON", "D300", DirectSunlight, 5,		{ 1.457031, 1, 1.437500, 0 } },
  { "NIKON", "D300", DirectSunlight, 6,		{ 1.429688, 1, 1.457031, 0 } },
  { "NIKON", "D300", Flash, -6,			{ 1.910156, 1, 1.058594, 0 } },
  { "NIKON", "D300", Flash, -5,			{ 1.863281, 1, 1.078125, 0 } },
  { "NIKON", "D300", Flash, -4,			{ 1.820313, 1, 1.093750, 0 } },
  { "NIKON", "D300", Flash, -3,			{ 1.781250, 1, 1.105469, 0 } },
  { "NIKON", "D300", Flash, -2,			{ 1.746094, 1, 1.121094, 0 } },
  { "NIKON", "D300", Flash, -1,			{ 1.714844, 1, 1.136719, 0 } },
  { "NIKON", "D300", Flash, 0,			{ 1.687500, 1, 1.152344, 0 } },
  { "NIKON", "D300", Flash, 1,			{ 1.660156, 1, 1.164063, 0 } },
  { "NIKON", "D300", Flash, 2,			{ 1.636719, 1, 1.179688, 0 } },
  { "NIKON", "D300", Flash, 3,			{ 1.613281, 1, 1.195313, 0 } },
  { "NIKON", "D300", Flash, 4,			{ 1.593750, 1, 1.210938, 0 } },
  { "NIKON", "D300", Flash, 5,			{ 1.574219, 1, 1.230469, 0 } },
  { "NIKON", "D300", Flash, 6,			{ 1.554688, 1, 1.246094, 0 } },
  { "NIKON", "D300", Cloudy, -6,		{ 1.820313, 1, 1.093750, 0 } },
  { "NIKON", "D300", Cloudy, -5,		{ 1.789063, 1, 1.105469, 0 } },
  { "NIKON", "D300", Cloudy, -4,		{ 1.761719, 1, 1.117188, 0 } },
  { "NIKON", "D300", Cloudy, -3,		{ 1.734375, 1, 1.132813, 0 } },
  { "NIKON", "D300", Cloudy, -2,		{ 1.710938, 1, 1.148438, 0 } },
  { "NIKON", "D300", Cloudy, -1,		{ 1.687500, 1, 1.167969, 0 } },
  { "NIKON", "D300", Cloudy, 0,			{ 1.664063, 1, 1.187500, 0 } },
  { "NIKON", "D300", Cloudy, 1,			{ 1.644531, 1, 1.207031, 0 } },
  { "NIKON", "D300", Cloudy, 2,			{ 1.625000, 1, 1.230469, 0 } },
  { "NIKON", "D300", Cloudy, 3,			{ 1.601563, 1, 1.253906, 0 } },
  { "NIKON", "D300", Cloudy, 4,			{ 1.582031, 1, 1.281250, 0 } },
  { "NIKON", "D300", Cloudy, 5,			{ 1.562500, 1, 1.308594, 0 } },
  { "NIKON", "D300", Cloudy, 6,			{ 1.542969, 1, 1.335938, 0 } },
  { "NIKON", "D300", Shade, -6,			{ 2.156250, 1, 1.000000, 0 } },
  { "NIKON", "D300", Shade, -5,			{ 2.109375, 1, 1.000000, 0 } },
  { "NIKON", "D300", Shade, -4,			{ 2.062500, 1, 1.011719, 0 } },
  { "NIKON", "D300", Shade, -3,			{ 2.019531, 1, 1.027344, 0 } },
  { "NIKON", "D300", Shade, -2,			{ 1.976563, 1, 1.042969, 0 } },
  { "NIKON", "D300", Shade, -1,			{ 1.937500, 1, 1.054688, 0 } },
  { "NIKON", "D300", Shade, 0,			{ 1.902344, 1, 1.066406, 0 } },
  { "NIKON", "D300", Shade, 1,			{ 1.867188, 1, 1.074219, 0 } },
  { "NIKON", "D300", Shade, 2,			{ 1.832031, 1, 1.085938, 0 } },
  { "NIKON", "D300", Shade, 3,			{ 1.804688, 1, 1.097656, 0 } },
  { "NIKON", "D300", Shade, 4,			{ 1.773438, 1, 1.113281, 0 } },
  { "NIKON", "D300", Shade, 5,			{ 1.746094, 1, 1.125000, 0 } },
  { "NIKON", "D300", Shade, 6,			{ 1.722656, 1, 1.140625, 0 } },
  { "NIKON", "D300", "2700K", 0,		{ 0.941406, 1, 2.390625, 0 } },
  { "NIKON", "D300", "5000K", 0,		{ 1.531250, 1, 1.351563, 0 } },

  { "NIKON", "D300S", DirectSunlight, -6,	{ 1.687, 1, 1.168, 0 } },
  { "NIKON", "D300S", DirectSunlight, 0,	{ 1.563, 1, 1.309, 0 } },
  { "NIKON", "D300S", DirectSunlight, 6,	{ 1.430, 1, 1.457, 0 } },
  { "NIKON", "D300S", Flash, -6,		{ 1.910, 1, 1.059, 0 } },
  { "NIKON", "D300S", Flash, 0,			{ 1.687, 1, 1.152, 0 } },
  { "NIKON", "D300S", Flash, 6,			{ 1.555, 1, 1.246, 0 } },
  { "NIKON", "D300S", Cloudy, -6,		{ 1.820, 1, 1.094, 0 } },
  { "NIKON", "D300S", Cloudy, 0,		{ 1.664, 1, 1.187, 0 } },
  { "NIKON", "D300S", Cloudy, 6,		{ 1.543, 1, 1.336, 0 } },
  { "NIKON", "D300S", Shade, -6,		{ 2.156, 1, 1.000, 0 } },
  { "NIKON", "D300S", Shade, 0,			{ 1.902, 1, 1.066, 0 } },
  { "NIKON", "D300S", Shade, 6,			{ 1.723, 1, 1.141, 0 } },
  { "NIKON", "D300S", Incandescent, -6,		{ 1.098, 1, 1.898, 0 } },
  { "NIKON", "D300S", Incandescent, 0,		{ 1.020, 1, 2.109, 0 } },
  { "NIKON", "D300S", Incandescent, 6,		{ 1, 1.049, 2.451, 0 } },
  { "NIKON", "D300S", SodiumVaporFluorescent, -6, { 1.031, 1, 2.102, 0 } },
  { "NIKON", "D300S", SodiumVaporFluorescent, 0, { 1, 1.053, 2.444, 0 } },
  { "NIKON", "D300S", SodiumVaporFluorescent, 6, { 1, 1.138, 2.973, 0 } },
  { "NIKON", "D300S", WarmWhiteFluorescent, -6,	{ 1.129, 1, 1.848, 0 } },
  { "NIKON", "D300S", WarmWhiteFluorescent, 0,	{ 1.047, 1, 1.961, 0 } },
  { "NIKON", "D300S", WarmWhiteFluorescent, 6,	{ 1, 1.024, 2.112, 0 } },
  { "NIKON", "D300S", WhiteFluorescent, -6,	{ 1.453, 1, 2.051, 0 } },
  { "NIKON", "D300S", WhiteFluorescent, 0,	{ 1.191, 1, 2.305, 0 } },
  { "NIKON", "D300S", WhiteFluorescent, 6,	{ 1, 1.153, 2.977, 0 } },
  { "NIKON", "D300S", CoolWhiteFluorescent, -6,	{ 1.668, 1, 1.801, 0 } },
  { "NIKON", "D300S", CoolWhiteFluorescent, 0,	{ 1.469, 1, 2.035, 0 } },
  { "NIKON", "D300S", CoolWhiteFluorescent, 6,	{ 1.211, 1, 2.289, 0 } },
  { "NIKON", "D300S", DayWhiteFluorescent, -6,	{ 1.625, 1, 1.195, 0 } },
  { "NIKON", "D300S", DayWhiteFluorescent, 0,	{ 1.492, 1, 1.363, 0 } },
  { "NIKON", "D300S", DayWhiteFluorescent, 6,	{ 1.375, 1, 1.520, 0 } },
  { "NIKON", "D300S", DaylightFluorescent, -6,	{ 1.852, 1, 1.000, 0 } },
  { "NIKON", "D300S", DaylightFluorescent, 0,	{ 1.699, 1, 1.102, 0 } },
  { "NIKON", "D300S", DaylightFluorescent, 6,	{ 1.563, 1, 1.273, 0 } },
  { "NIKON", "D300S", HighTempMercuryVaporFluorescent, -6, { 2.039, 1, 1.156, 0 } },
  { "NIKON", "D300S", HighTempMercuryVaporFluorescent, 0, { 1.961, 1, 1.328, 0 } },
  { "NIKON", "D300S", HighTempMercuryVaporFluorescent, 6, { 1.855, 1, 1.523, 0 } },

  { "NIKON", "D700", DirectSunlight, -6,	{ 1.980469, 1, 1.199219, 0 } },
  { "NIKON", "D700", DirectSunlight, 0,		{ 1.816406, 1, 1.355469, 0 } },
  { "NIKON", "D700", DirectSunlight, 6,		{ 1.652344, 1, 1.523437, 0 } },
  { "NIKON", "D700", Flash, -6,			{ 2.261719, 1, 1.082031, 0 } },
  { "NIKON", "D700", Flash, 0,			{ 2.039063, 1, 1.171875, 0 } },
  { "NIKON", "D700", Flash, 6,			{ 1.871094, 1, 1.281250, 0 } },
  { "NIKON", "D700", Cloudy, -6,		{ 2.148437, 1, 1.117187, 0 } },
  { "NIKON", "D700", Cloudy, 0,			{ 1.949219, 1, 1.222656, 0 } },
  { "NIKON", "D700", Cloudy, 6,			{ 1.792969, 1, 1.386719, 0 } },
  { "NIKON", "D700", Shade, -6,			{ 2.535156, 1, 1.000000, 0 } },
  { "NIKON", "D700", Shade, 0,			{ 2.246094, 1, 1.085937, 0 } },
  { "NIKON", "D700", Shade, 6,			{ 2.023438, 1, 1.171875, 0 } },
  { "NIKON", "D700", Incandescent , -6,		{ 1.265625, 1, 2.050781, 0 } },
  { "NIKON", "D700", Incandescent , 0,		{ 1.167969, 1, 2.316406, 0 } },
  { "NIKON", "D700", Incandescent , 6,		{ 1.085938, 1, 2.605469, 0 } },
  { "NIKON", "D700", SodiumVaporFluorescent, -6, { 1.175781, 1, 2.191406, 0 } },
  { "NIKON", "D700", SodiumVaporFluorescent, 0, { 1.062500, 1, 2.464844, 0 } },
  { "NIKON", "D700", SodiumVaporFluorescent, 6, { 1.000000, 1, 2.789062, 0 } },
  { "NIKON", "D700", WarmWhiteFluorescent, -6,	{ 1.269531, 1, 1.968750, 0 } },
  { "NIKON", "D700", WarmWhiteFluorescent, 0,	{ 1.167969, 1, 2.109375, 0 } },
  { "NIKON", "D700", WarmWhiteFluorescent, 6,	{ 1.078125, 1, 2.230469, 0 } },
  { "NIKON", "D700", WhiteFluorescent, -6,	{ 1.671875, 1, 2.121094, 0 } },
  { "NIKON", "D700", WhiteFluorescent, 0,	{ 1.363281, 1, 2.425781, 0 } },
  { "NIKON", "D700", WhiteFluorescent, 6,	{ 1, 1.015873, 2.813492, 0 } },
  { "NIKON", "D700", CoolWhiteFluorescent, -6,	{ 1.929687, 1, 1.835938, 0 } },
  { "NIKON", "D700", CoolWhiteFluorescent, 0,	{ 1.687500, 1, 2.101563, 0 } },
  { "NIKON", "D700", CoolWhiteFluorescent, 6,	{ 1.386719, 1, 2.406250, 0 } },
  { "NIKON", "D700", DayWhiteFluorescent, -6,	{ 1.867188, 1, 1.218750, 0 } },
  { "NIKON", "D700", DayWhiteFluorescent, 0,	{ 1.710938, 1, 1.410156, 0 } },
  { "NIKON", "D700", DayWhiteFluorescent, 6,	{ 1.570313, 1, 1.585938, 0 } },
  { "NIKON", "D700", DaylightFluorescent, -6,	{ 2.128906, 1, 1.000000, 0 } },
  { "NIKON", "D700", DaylightFluorescent, 0,	{ 1.953125, 1, 1.113281, 0 } },
  { "NIKON", "D700", DaylightFluorescent, 6,	{ 1.792969, 1, 1.308594, 0 } },
  { "NIKON", "D700", HighTempMercuryVaporFluorescent, -6, { 2.378906, 1, 1.218750, 0 } },
  { "NIKON", "D700", HighTempMercuryVaporFluorescent, 0, { 2.289063, 1, 1.363281, 0 } },
  { "NIKON", "D700", HighTempMercuryVaporFluorescent, 6, { 2.164063, 1, 1.542969, 0 } },
  { "NIKON", "D700", "2700K", 0,		{ 1.066406, 1, 2.679687, 0 } },
  { "NIKON", "D700", "5000K", 0,		{ 1.777344, 1, 1.402344, 0 } },

  { "NIKON", "D40", Incandescent, -3,		{ 1.492188, 1, 2.164063, 0 } },
  { "NIKON", "D40", Incandescent, -2,		{ 1.437500, 1, 2.367188, 0 } },
  { "NIKON", "D40", Incandescent, -1,		{ 1.417969, 1, 2.414062, 0 } },
  { "NIKON", "D40", Incandescent, 0,		{ 1.375000, 1, 2.511719, 0 } },
  { "NIKON", "D40", Incandescent, 1,		{ 1.324219, 1, 2.628906, 0 } },
  { "NIKON", "D40", Incandescent, 2,		{ 1.277344, 1, 2.753906, 0 } },
  { "NIKON", "D40", Incandescent, 3,		{ 1.222656, 1, 2.914063, 0 } },
  { "NIKON", "D40", Fluorescent, -3,		{ 2.738281, 1, 1.492188, 0 } },
  { "NIKON", "D40", Fluorescent, -2,		{ 2.417969, 1, 1.246094, 0 } },
  { "NIKON", "D40", Fluorescent, -1,		{ 2.093750, 1, 1.570312, 0 } },
  { "NIKON", "D40", Fluorescent, 0,		{ 2.007813, 1, 2.269531, 0 } },
  { "NIKON", "D40", Fluorescent, 1,		{ 1.613281, 1, 2.593750, 0 } },
  { "NIKON", "D40", Fluorescent, 2,		{ 1.394531, 1, 2.343750, 0 } },
  { "NIKON", "D40", Fluorescent, 3,		{ 1.210938, 1, 2.621094, 0 } },
  { "NIKON", "D40", DirectSunlight, -3,		{ 2.328125, 1, 1.371094, 0 } },
  { "NIKON", "D40", DirectSunlight, -2,		{ 2.269531, 1, 1.394531, 0 } },
  { "NIKON", "D40", DirectSunlight, -1,		{ 2.230469, 1, 1.410156, 0 } },
  { "NIKON", "D40", DirectSunlight, 0,		{ 2.195313, 1, 1.421875, 0 } },
  { "NIKON", "D40", DirectSunlight, 1,		{ 2.113281, 1, 1.445312, 0 } },
  { "NIKON", "D40", DirectSunlight, 2,		{ 2.070312, 1, 1.453125, 0 } },
  { "NIKON", "D40", DirectSunlight, 3,		{ 2.039063, 1, 1.457031, 0 } },
  { "NIKON", "D40", Flash, -3,			{ 2.667969, 1, 1.214844, 0 } },
  { "NIKON", "D40", Flash, -2,			{ 2.605469, 1, 1.234375, 0 } },
  { "NIKON", "D40", Flash, -1,			{ 2.539062, 1, 1.257812, 0 } },
  { "NIKON", "D40", Flash, 0,			{ 2.464844, 1, 1.281250, 0 } },
  { "NIKON", "D40", Flash, 1,			{ 2.390625, 1, 1.312500, 0 } },
  { "NIKON", "D40", Flash, 2,			{ 2.308594, 1, 1.343750, 0 } },
  { "NIKON", "D40", Flash, 3,			{ 2.222656, 1, 1.386719, 0 } },
  { "NIKON", "D40", Cloudy, -3,			{ 2.570313, 1, 1.246094, 0 } },
  { "NIKON", "D40", Cloudy, -2,			{ 2.523438, 1, 1.269531, 0 } },
  { "NIKON", "D40", Cloudy, -1,			{ 2.476562, 1, 1.296875, 0 } },
  { "NIKON", "D40", Cloudy, 0,			{ 2.429688, 1, 1.320313, 0 } },
  { "NIKON", "D40", Cloudy, 1,			{ 2.382812, 1, 1.343750, 0 } },
  { "NIKON", "D40", Cloudy, 2,			{ 2.328125, 1, 1.371094, 0 } },
  { "NIKON", "D40", Cloudy, 3,			{ 2.269531, 1, 1.394531, 0 } },
  { "NIKON", "D40", Shade, -3,			{ 2.957031, 1, 1.054688, 0 } },
  { "NIKON", "D40", Shade, -2,			{ 2.921875, 1, 1.074219, 0 } },
  { "NIKON", "D40", Shade, -1,			{ 2.878906, 1, 1.097656, 0 } },
  { "NIKON", "D40", Shade, 0,			{ 2.820313, 1, 1.125000, 0 } },
  { "NIKON", "D40", Shade, 1,			{ 2.746094, 1, 1.160156, 0 } },
  { "NIKON", "D40", Shade, 2,			{ 2.671875, 1, 1.195312, 0 } },
  { "NIKON", "D40", Shade, 3,			{ 2.597656, 1, 1.234375, 0 } },

  { "NIKON", "D40X", Incandescent, -3,		{ 1.234375, 1, 2.140625, 0 } },
  { "NIKON", "D40X", Incandescent, 0,		{ 1.148438, 1, 2.386719, 0 } },
  { "NIKON", "D40X", Incandescent, 3,		{ 1.039062, 1, 2.734375, 0 } },
  { "NIKON", "D40X", Fluorescent, -3,		{ 2.296875, 1, 1.398438, 0 } },
  { "NIKON", "D40X", Fluorescent, 0,		{ 1.683594, 1, 2.117188, 0 } },
  { "NIKON", "D40X", Fluorescent, 3,		{ 1.000000, 1, 2.527344, 0 } },
  { "NIKON", "D40X", DirectSunlight, -3,	{ 1.882812, 1, 1.300781, 0 } },
  { "NIKON", "D40X", DirectSunlight, 0,		{ 1.792969, 1, 1.371094, 0 } },
  { "NIKON", "D40X", DirectSunlight, 3,		{ 1.695312, 1, 1.437500, 0 } },
  { "NIKON", "D40X", Flash, -3,			{ 2.089844, 1, 1.132812, 0 } },
  { "NIKON", "D40X", Flash, 0,			{ 1.949219, 1, 1.187500, 0 } },
  { "NIKON", "D40X", Flash, 3,			{ 1.769531, 1, 1.269531, 0 } },
  { "NIKON", "D40X", Cloudy, -3,		{ 2.070312, 1, 1.191406, 0 } },
  { "NIKON", "D40X", Cloudy, 0,			{ 1.960938, 1, 1.253906, 0 } },
  { "NIKON", "D40X", Cloudy, 3,			{ 1.835938, 1, 1.332031, 0 } },
  { "NIKON", "D40X", Shade, -3,			{ 2.414062, 1, 1.042969, 0 } },
  { "NIKON", "D40X", Shade, 0,			{ 2.277344, 1, 1.089844, 0 } },
  { "NIKON", "D40X", Shade, 3,			{ 2.085938, 1, 1.183594, 0 } },

  { "NIKON", "D50", Incandescent, 0,		{ 1.328125, 1, 2.500000, 0 } },
  { "NIKON", "D50", Fluorescent, 0,		{ 1.945312, 1, 2.191406, 0 } },
  { "NIKON", "D50", DirectSunlight, 0,		{ 2.140625, 1, 1.398438, 0 } },
  { "NIKON", "D50", Flash, 0,			{ 2.398438, 1, 1.339844, 0 } },
  { "NIKON", "D50", Cloudy, 0,			{ 2.360269, 1, 1.282828, 0 } },
  { "NIKON", "D50", Shade, 0,			{ 2.746094, 1, 1.156250, 0 } },

  { "NIKON", "D60", DirectSunlight, 0,		{ 1.792969, 1, 1.371094, 0 } },
  { "NIKON", "D60", Flash, 0,			{ 2.007813, 1, 1.187500, 0 } },
  { "NIKON", "D60", Cloudy, 0,			{ 1.960937, 1, 1.253906, 0 } },
  { "NIKON", "D60", Shade, 0,			{ 2.277344, 1, 1.089844, 0 } },
  { "NIKON", "D60", Incandescent, 0,		{ 1.148437, 1, 2.382812, 0 } },
  { "NIKON", "D60", SodiumVaporFluorescent, 0,	{ 1.035156, 1, 2.468750, 0 } },
  { "NIKON", "D60", WarmWhiteFluorescent, 0,	{ 1.136719, 1, 2.167969, 0 } },
  { "NIKON", "D60", WhiteFluorescent, 0,	{ 1.343750, 1, 2.480469, 0 } },
  { "NIKON", "D60", CoolWhiteFluorescent, 0,	{ 1.683594, 1, 2.117187, 0 } },
  { "NIKON", "D60", DayWhiteFluorescent, 0,	{ 1.679688, 1, 1.414063, 0 } },
  { "NIKON", "D60", DaylightFluorescent, 0,	{ 1.953125, 1, 1.121094, 0 } },
  { "NIKON", "D60", HighTempMercuryVaporFluorescent, 0, { 2.296875, 1, 1.398438, 0 } },

  { "NIKON", "D70", Incandescent, -3,		{ 1.429688, 1, 2.539063, 0 } }, /*3300K*/
  { "NIKON", "D70", Incandescent, -2,		{ 1.398438, 1, 2.632813, 0 } }, /*3200K*/
  { "NIKON", "D70", Incandescent, -1,		{ 1.378906, 1, 2.687500, 0 } }, /*3100K*/
  { "NIKON", "D70", Incandescent, 0,		{ 1.343750, 1, 2.816406, 0 } }, /*3000K*/
  { "NIKON", "D70", Incandescent, 1,		{ 1.312500, 1, 2.937500, 0 } }, /*2900K*/
  { "NIKON", "D70", Incandescent, 2,		{ 1.281250, 1, 3.089844, 0 } }, /*2800K*/
  { "NIKON", "D70", Incandescent, 3,		{ 1.253906, 1, 3.250000, 0 } }, /*2700K*/
  { "NIKON", "D70", Fluorescent, -3,		{ 2.734375, 1, 1.621094, 0 } }, /*7200K*/
  { "NIKON", "D70", Fluorescent, -2,		{ 2.417969, 1, 1.343750, 0 } }, /*6500K*/
  { "NIKON", "D70", Fluorescent, -1,		{ 2.078125, 1, 1.691406, 0 } }, /*5000K*/
  { "NIKON", "D70", Fluorescent, 0,		{ 1.964844, 1, 2.476563, 0 } }, /*4200K*/
  { "NIKON", "D70", Fluorescent, 1,		{ 1.566406, 1, 2.753906, 0 } }, /*3700K*/
  { "NIKON", "D70", Fluorescent, 2,		{ 1.406250, 1, 2.550781, 0 } }, /*3000K*/
  { "NIKON", "D70", Fluorescent, 3,		{ 1.312500, 1, 2.562500, 0 } }, /*2700K*/
  { "NIKON", "D70", DirectSunlight, -3,		{ 2.156250, 1, 1.523438, 0 } }, /*5600K*/
  { "NIKON", "D70", DirectSunlight, -2,		{ 2.109375, 1, 1.562500, 0 } }, /*5400K*/
  { "NIKON", "D70", DirectSunlight, -1,		{ 2.089844, 1, 1.574219, 0 } }, /*5300K*/
  { "NIKON", "D70", DirectSunlight, 0,		{ 2.062500, 1, 1.597656, 0 } }, /*5200K*/
  { "NIKON", "D70", DirectSunlight, 1,		{ 2.007813, 1, 1.648438, 0 } }, /*5000K*/
  { "NIKON", "D70", DirectSunlight, 2,		{ 1.980469, 1, 1.671875, 0 } }, /*4900K*/
  { "NIKON", "D70", DirectSunlight, 3,		{ 1.953125, 1, 1.695313, 0 } }, /*4800K*/
  { "NIKON", "D70", Flash, -3,			{ 2.578125, 1, 1.476563, 0 } }, /*6000K*/
  { "NIKON", "D70", Flash, -2,			{ 2.535156, 1, 1.484375, 0 } }, /*5800K*/
  { "NIKON", "D70", Flash, -1,			{ 2.488281, 1, 1.492188, 0 } }, /*5600K*/
  { "NIKON", "D70", Flash, 0,			{ 2.441406, 1, 1.500000, 0 } }, /*5400K*/
  { "NIKON", "D70", Flash, 1,			{ 2.421875, 1, 1.507813, 0 } }, /*5200K*/
  { "NIKON", "D70", Flash, 2,			{ 2.398438, 1, 1.515625, 0 } }, /*5000K*/
  { "NIKON", "D70", Flash, 3,			{ 2.378906, 1, 1.523438, 0 } }, /*4800K*/
  { "NIKON", "D70", Cloudy, -3,			{ 2.375000, 1, 1.386719, 0 } }, /*6600K*/
  { "NIKON", "D70", Cloudy, -2,			{ 2.343750, 1, 1.406250, 0 } }, /*6400K*/
  { "NIKON", "D70", Cloudy, -1,			{ 2.300781, 1, 1.429688, 0 } }, /*6200K*/
  { "NIKON", "D70", Cloudy, 0,			{ 2.257813, 1, 1.457031, 0 } }, /*6000K*/
  { "NIKON", "D70", Cloudy, 1,			{ 2.207031, 1, 1.488281, 0 } }, /*5800K*/
  { "NIKON", "D70", Cloudy, 2,			{ 2.156250, 1, 1.523438, 0 } }, /*5600K*/
  { "NIKON", "D70", Cloudy, 3,			{ 2.109375, 1, 1.562500, 0 } }, /*5400K*/
  { "NIKON", "D70", Shade, -3,			{ 2.757813, 1, 1.226563, 0 } }, /*9200K*/
  { "NIKON", "D70", Shade, -2,			{ 2.710938, 1, 1.242188, 0 } }, /*8800K*/
  { "NIKON", "D70", Shade, -1,			{ 2.660156, 1, 1.257813, 0 } }, /*8400K*/
  { "NIKON", "D70", Shade, 0,			{ 2.613281, 1, 1.277344, 0 } }, /*8000K*/
  { "NIKON", "D70", Shade, 1,			{ 2.531250, 1, 1.308594, 0 } }, /*7500K*/
  { "NIKON", "D70", Shade, 2,			{ 2.472656, 1, 1.335938, 0 } }, /*7100K*/
  { "NIKON", "D70", Shade, 3,			{ 2.394531, 1, 1.375000, 0 } }, /*6700K*/

  { "NIKON", "D70s", Incandescent, -3,		{ 1.429688, 1, 2.539063, 0 } }, /*3300K*/
  { "NIKON", "D70s", Incandescent, -2,		{ 1.398438, 1, 2.632813, 0 } }, /*3200K*/
  { "NIKON", "D70s", Incandescent, -1,		{ 1.378906, 1, 2.687500, 0 } }, /*3100K*/
  { "NIKON", "D70s", Incandescent, 0,		{ 1.343750, 1, 2.816406, 0 } }, /*3000K*/
  { "NIKON", "D70s", Incandescent, 1,		{ 1.312500, 1, 2.937500, 0 } }, /*2900K*/
  { "NIKON", "D70s", Incandescent, 2,		{ 1.281250, 1, 3.089844, 0 } }, /*2800K*/
  { "NIKON", "D70s", Incandescent, 3,		{ 1.253906, 1, 3.250000, 0 } }, /*2700K*/
  { "NIKON", "D70s", Fluorescent, -3,		{ 2.734375, 1, 1.621094, 0 } }, /*7200K*/
  { "NIKON", "D70s", Fluorescent, -2,		{ 2.417969, 1, 1.343750, 0 } }, /*6500K*/
  { "NIKON", "D70s", Fluorescent, -1,		{ 2.078125, 1, 1.691406, 0 } }, /*5000K*/
  { "NIKON", "D70s", Fluorescent, 0,		{ 1.964844, 1, 2.476563, 0 } }, /*4200K*/
  { "NIKON", "D70s", Fluorescent, 1,		{ 1.566406, 1, 2.753906, 0 } }, /*3700K*/
  { "NIKON", "D70s", Fluorescent, 2,		{ 1.406250, 1, 2.550781, 0 } }, /*3000K*/
  { "NIKON", "D70s", Fluorescent, 3,		{ 1.312500, 1, 2.562500, 0 } }, /*2700K*/
  { "NIKON", "D70s", DirectSunlight, -3,	{ 2.156250, 1, 1.523438, 0 } }, /*5600K*/
  { "NIKON", "D70s", DirectSunlight, -2,	{ 2.109375, 1, 1.562500, 0 } }, /*5400K*/
  { "NIKON", "D70s", DirectSunlight, -1,	{ 2.089844, 1, 1.574219, 0 } }, /*5300K*/
  { "NIKON", "D70s", DirectSunlight, 0,		{ 2.062500, 1, 1.597656, 0 } }, /*5200K*/
  { "NIKON", "D70s", DirectSunlight, 1,		{ 2.007813, 1, 1.648438, 0 } }, /*5000K*/
  { "NIKON", "D70s", DirectSunlight, 2,		{ 1.980469, 1, 1.671875, 0 } }, /*4900K*/
  { "NIKON", "D70s", DirectSunlight, 3,		{ 1.953125, 1, 1.695313, 0 } }, /*4800K*/
  { "NIKON", "D70s", Flash, -3,			{ 2.578125, 1, 1.476563, 0 } }, /*6000K*/
  { "NIKON", "D70s", Flash, -2,			{ 2.535156, 1, 1.484375, 0 } }, /*5800K*/
  { "NIKON", "D70s", Flash, -1,			{ 2.488281, 1, 1.492188, 0 } }, /*5600K*/
  { "NIKON", "D70s", Flash, 0,			{ 2.441406, 1, 1.500000, 0 } }, /*5400K*/
  { "NIKON", "D70s", Flash, 1,			{ 2.421875, 1, 1.507813, 0 } }, /*5200K*/
  { "NIKON", "D70s", Flash, 2,			{ 2.398438, 1, 1.515625, 0 } }, /*5000K*/
  { "NIKON", "D70s", Flash, 3,			{ 2.378906, 1, 1.523438, 0 } }, /*4800K*/
  { "NIKON", "D70s", Cloudy, -3,		{ 2.375000, 1, 1.386719, 0 } }, /*6600K*/
  { "NIKON", "D70s", Cloudy, -2,		{ 2.343750, 1, 1.406250, 0 } }, /*6400K*/
  { "NIKON", "D70s", Cloudy, -1,		{ 2.300781, 1, 1.429688, 0 } }, /*6200K*/
  { "NIKON", "D70s", Cloudy, 0,			{ 2.257813, 1, 1.457031, 0 } }, /*6000K*/
  { "NIKON", "D70s", Cloudy, 1,			{ 2.207031, 1, 1.488281, 0 } }, /*5800K*/
  { "NIKON", "D70s", Cloudy, 2,			{ 2.156250, 1, 1.523438, 0 } }, /*5600K*/
  { "NIKON", "D70s", Cloudy, 3,			{ 2.109375, 1, 1.562500, 0 } }, /*5400K*/
  { "NIKON", "D70s", Shade, -3,			{ 2.757813, 1, 1.226563, 0 } }, /*9200K*/
  { "NIKON", "D70s", Shade, -2,			{ 2.710938, 1, 1.242188, 0 } }, /*8800K*/
  { "NIKON", "D70s", Shade, -1,			{ 2.660156, 1, 1.257813, 0 } }, /*8400K*/
  { "NIKON", "D70s", Shade, 0,			{ 2.613281, 1, 1.277344, 0 } }, /*8000K*/
  { "NIKON", "D70s", Shade, 1,			{ 2.531250, 1, 1.308594, 0 } }, /*7500K*/
  { "NIKON", "D70s", Shade, 2,			{ 2.472656, 1, 1.335938, 0 } }, /*7100K*/
  { "NIKON", "D70s", Shade, 3,			{ 2.394531, 1, 1.375000, 0 } }, /*6700K*/

  { "NIKON", "D80", Incandescent, -3,		{ 1.234375, 1, 2.140625, 0 } },
  { "NIKON", "D80", Incandescent, 0,		{ 1.148438, 1, 2.386719, 0 } },
  { "NIKON", "D80", Incandescent, 3,		{ 1.039062, 1, 2.734375, 0 } },
  { "NIKON", "D80", Fluorescent, -3,		{ 2.296875, 1, 1.398438, 0 } },
  { "NIKON", "D80", Fluorescent, 0,		{ 1.683594, 1, 2.117188, 0 } },
  { "NIKON", "D80", Fluorescent, 3,		{ 1.000000, 1, 2.527344, 0 } },
  { "NIKON", "D80", Daylight, -3,		{ 1.882812, 1, 1.300781, 0 } },
  { "NIKON", "D80", Daylight, 0,		{ 1.792969, 1, 1.371094, 0 } },
  { "NIKON", "D80", Daylight, 3,		{ 1.695312, 1, 1.437500, 0 } },
  { "NIKON", "D80", Flash, -3,			{ 2.070312, 1, 1.144531, 0 } },
  { "NIKON", "D80", Flash, 0,			{ 2.007812, 1, 1.242188, 0 } },
  { "NIKON", "D80", Flash, 3,			{ 1.972656, 1, 1.156250, 0 } },
  { "NIKON", "D80", Cloudy, -3,			{ 2.070312, 1, 1.191406, 0 } },
  { "NIKON", "D80", Cloudy, 0,			{ 1.960938, 1, 1.253906, 0 } },
  { "NIKON", "D80", Cloudy, 3,			{ 1.835938, 1, 1.332031, 0 } },
  { "NIKON", "D80", Shade, -3,			{ 2.414062, 1, 1.042969, 0 } },
  { "NIKON", "D80", Shade, 0,			{ 2.277344, 1, 1.089844, 0 } },
  { "NIKON", "D80", Shade, 3,			{ 2.085938, 1, 1.183594, 0 } },
  { "NIKON", "D80", "5000K", 0,			{ 1.746094, 1, 1.410156, 0 } },

  { "NIKON", "D90", Incandescent, -6,		{ 1.273438, 1, 1.906250, 0 } },
  { "NIKON", "D90", Incandescent, 0,		{ 1.179688, 1, 2.097656, 0 } },
  { "NIKON", "D90", Incandescent, 6,		{ 1.113281, 1, 2.320313, 0 } },
  { "NIKON", "D90", SodiumVaporFluorescent, -6,	{ 1.164063, 1, 2.058594, 0 } },
  { "NIKON", "D90", SodiumVaporFluorescent, 0,	{ 1.062500, 1, 2.289063, 0 } },
  { "NIKON", "D90", SodiumVaporFluorescent, 6,	{ 1.000000, 1, 2.554688, 0 } },
  { "NIKON", "D90", WarmWhiteFluorescent, -6,	{ 1.285156, 1, 1.761719, 0 } },
  { "NIKON", "D90", WarmWhiteFluorescent, 0,	{ 1.191406, 1, 1.871094, 0 } },
  { "NIKON", "D90", WarmWhiteFluorescent, 6,	{ 1.105469, 1, 1.968750, 0 } },
  { "NIKON", "D90", WhiteFluorescent, -6,	{ 1.628906, 1, 1.953125, 0 } },
  { "NIKON", "D90", WhiteFluorescent, 0,	{ 1.343750, 1, 2.183594, 0 } },
  { "NIKON", "D90", WhiteFluorescent, 6,	{ 1.000000, 1, 2.429688, 0 } },
  { "NIKON", "D90", CoolWhiteFluorescent, -6,	{ 1.867188, 1, 1.722656, 0 } },
  { "NIKON", "D90", CoolWhiteFluorescent, 0,	{ 1.644531, 1, 1.937500, 0 } },
  { "NIKON", "D90", CoolWhiteFluorescent, 6,	{ 1.363281, 1, 2.167969, 0 } },
  { "NIKON", "D90", DayWhiteFluorescent, -6,	{ 1.843750, 1, 1.160156, 0 } },
  { "NIKON", "D90", DayWhiteFluorescent, 0,	{ 1.695313, 1, 1.312500, 0 } },
  { "NIKON", "D90", DayWhiteFluorescent, 6,	{ 1.562500, 1, 1.457031, 0 } },
  { "NIKON", "D90", DaylightFluorescent, -6,	{ 2.089844, 1, 1.000000, 0 } },
  { "NIKON", "D90", DaylightFluorescent, 0,	{ 1.925781, 1, 1.074219, 0 } },
  { "NIKON", "D90", DaylightFluorescent, 6,	{ 1.773438, 1, 1.234375, 0 } },
  { "NIKON", "D90", HighTempMercuryVaporFluorescent, -6, { 2.308594, 1, 1.132813, 0 } },
  { "NIKON", "D90", HighTempMercuryVaporFluorescent, 0, { 2.207031, 1, 1.292969, 0 } },
  { "NIKON", "D90", HighTempMercuryVaporFluorescent, 6, { 2.085938, 1, 1.468750, 0 } },
  { "NIKON", "D90", DirectSunlight, -6,		{ 1.949219, 1, 1.171875, 0 } },
  { "NIKON", "D90", DirectSunlight, 0,		{ 1.800781, 1, 1.308594, 0 } },
  { "NIKON", "D90", DirectSunlight, 6,		{ 1.640625, 1, 1.457031, 0 } },
  { "NIKON", "D90", Flash, -6,			{ 2.218750, 1, 1.062500, 0 } },
  { "NIKON", "D90", Flash, 0,			{ 1.976563, 1, 1.152344, 0 } },
  { "NIKON", "D90", Flash, 6,			{ 1.789063, 1, 1.253906, 0 } },
  { "NIKON", "D90", Cloudy, -6,			{ 2.093750, 1, 1.093750, 0 } },
  { "NIKON", "D90", Cloudy, 0,			{ 1.917969, 1, 1.187500, 0 } },
  { "NIKON", "D90", Cloudy, 6,			{ 1.765625, 1, 1.332031, 0 } },
  { "NIKON", "D90", Shade, -6,			{ 2.453125, 1, 1.000000, 0 } },
  { "NIKON", "D90", Shade, 0,			{ 2.183594, 1, 1.062500, 0 } },
  { "NIKON", "D90", Shade, 6,			{ 1.984375, 1, 1.140625, 0 } },
  { "NIKON", "D90", "2700K", 0,			{ 1.093750, 1, 2.378906, 0 } },
  { "NIKON", "D90", "5000K", 0,			{ 1.761719, 1, 1.347656, 0 } },

  { "NIKON", "D3000", DirectSunlight, 0,	{ 1.851563, 1, 1.347656, 0 } },
  { "NIKON", "D3000", Flash, 0,			{ 2.113281, 1, 1.164062, 0 } },
  { "NIKON", "D3000", Cloudy, 0,		{ 2.019531, 1, 1.214844, 0 } },
  { "NIKON", "D3000", Shade, 0,			{ 2.355469, 1, 1.082031, 0 } },
  { "NIKON", "D3000", Incandescent, 0,		{ 1.171875, 1, 2.316406, 0 } },
  { "NIKON", "D3000", SodiumVaporFluorescent, 0, { 1.023438, 1, 2.371094, 0 } },
  { "NIKON", "D3000", WarmWhiteFluorescent, 0,	{ 1.179688, 1, 2.074219, 0 } },
  { "NIKON", "D3000", WhiteFluorescent, 0,	{ 1.355469, 1, 2.328125, 0 } },
  { "NIKON", "D3000", CoolWhiteFluorescent, 0,	{ 1.703125, 1, 2.019531, 0 } },
  { "NIKON", "D3000", DayWhiteFluorescent, 0,	{ 1.750000, 1, 1.386719, 0 } },
  { "NIKON", "D3000", DaylightFluorescent, 0,	{ 1.960937, 1, 1.105469, 0 } },
  { "NIKON", "D3000", HighTempMercuryVaporFluorescent, 0, { 2.351563, 1, 1.328125, 0 } },

  { "NIKON", "D3100", DirectSunlight, 0,	{ 2.109375, 1, 1.257813, 0 } },
  { "NIKON", "D3100", Flash, 0,			{ 2.386719, 1, 1.097656, 0 } },
  { "NIKON", "D3100", Cloudy, 0,		{ 2.257812, 1, 1.140625, 0 } },
  { "NIKON", "D3100", Shade, 0,			{ 2.609375, 1, 1.015625, 0 } },
  { "NIKON", "D3100", Incandescent, 0,		{ 1.320313, 1, 2.039063, 0 } },
  { "NIKON", "D3100", SodiumVaporFluorescent, 0, { 1.222656, 1, 2.238281, 0 } },
  { "NIKON", "D3100", WarmWhiteFluorescent, 0,	{ 1.312500, 1, 1.847656, 0 } },
  { "NIKON", "D3100", WhiteFluorescent, 0,	{ 1.531250, 1, 2.152344, 0 } },
  { "NIKON", "D3100", CoolWhiteFluorescent, 0,	{ 1.894531, 1, 1.875000, 0 } },
  { "NIKON", "D3100", DayWhiteFluorescent, 0,	{ 1.941406, 1, 1.281250, 0 } },
  { "NIKON", "D3100", DaylightFluorescent, 0,	{ 2.214844, 1, 1.015625, 0 } },
  { "NIKON", "D3100", HighTempMercuryVaporFluorescent, 0, { 2.589844, 1, 1.250000, 0 } },

  { "NIKON", "D5000", DirectSunlight, 0,	{ 1.800781, 1, 1.308594, 0 } },
  { "NIKON", "D5000", Flash, 0,			{ 1.976562, 1, 1.152344, 0 } },
  { "NIKON", "D5000", Cloudy, 0,		{ 1.917969, 1, 1.187500, 0 } },
  { "NIKON", "D5000", Shade, 0,			{ 2.183594, 1, 1.062500, 0 } },
  { "NIKON", "D5000", Incandescent, 0,		{ 1.179687, 1, 2.097656, 0 } },
  { "NIKON", "D5000", SodiumVaporFluorescent, 0, { 1.062500, 1, 2.289063, 0 } },
  { "NIKON", "D5000", WarmWhiteFluorescent, 0,	{ 1.191406, 1, 1.871094, 0 } },
  { "NIKON", "D5000", WhiteFluorescent, 0,	{ 1.343750, 1, 2.183594, 0 } },
  { "NIKON", "D5000", CoolWhiteFluorescent, 0,	{ 1.644531, 1, 1.937500, 0 } },
  { "NIKON", "D5000", DayWhiteFluorescent, 0,	{ 1.695313, 1, 1.312500, 0 } },
  { "NIKON", "D5000", DaylightFluorescent, 0,	{ 1.925781, 1, 1.074219, 0 } },
  { "NIKON", "D5000", HighTempMercuryVaporFluorescent, 0, { 2.207031, 1, 1.292969, 0 } },

  { "NIKON", "D5100", DirectSunlight, 0,	{ 2.0273, 1, 1.3906, 0 } },
  { "NIKON", "D5100", Flash, 0,			{ 2.2813, 1, 1.2109, 0 } },
  { "NIKON", "D5100", Cloudy, 0,		{ 2.1680, 1, 1.2656, 0 } },
  { "NIKON", "D5100", Shade, 0,			{ 2.4922, 1, 1.1367, 0 } },
  { "NIKON", "D5100", Incandescent, 0,		{ 1.3047, 1, 2.2148, 0 } },
  { "NIKON", "D5100", SodiumVaporFluorescent, 0, { 1.2031, 1, 2.4375, 0 } },
  { "NIKON", "D5100", WarmWhiteFluorescent, 0,	{ 1.3008, 1, 1.9648, 0 } },
  { "NIKON", "D5100", WhiteFluorescent, 0,	{ 1.4961, 1, 2.3047, 0 } },
  { "NIKON", "D5100", CoolWhiteFluorescent, 0,	{ 1.8320, 1, 2.0273, 0 } },
  { "NIKON", "D5100", DayWhiteFluorescent, 0,	{ 1.8984, 1, 1.3906, 0 } },
  { "NIKON", "D5100", DaylightFluorescent, 0,	{ 2.1680, 1, 1.1172, 0 } },
  { "NIKON", "D5100", HighTempMercuryVaporFluorescent, 0, { 2.4844, 1, 1.3672, 0 } },

  { "NIKON", "D7000", DirectSunlight, -6,	{ 2.199219, 1, 1.242187, 0 } },
  { "NIKON", "D7000", DirectSunlight, 0,	{ 2.027344, 1, 1.390625, 0 } },
  { "NIKON", "D7000", DirectSunlight, 6,	{ 1.847656, 1, 1.542969, 0 } },
  { "NIKON", "D7000", Flash, -6,		{ 2.511719, 1, 1.132812, 0 } },
  { "NIKON", "D7000", Flash, 0,			{ 2.281250, 1, 1.210938, 0 } },
  { "NIKON", "D7000", Flash, 6,			{ 2.089844, 1, 1.308594, 0 } },
  { "NIKON", "D7000", Cloudy, -6,		{ 2.386719, 1, 1.164062, 0 } },
  { "NIKON", "D7000", Cloudy, 0,		{ 2.167969, 1, 1.265625, 0 } },
  { "NIKON", "D7000", Cloudy, 6,		{ 2.000000, 1, 1.421875, 0 } },
  { "NIKON", "D7000", Shade, -6,		{ 2.828125, 1, 1.023437, 0 } },
  { "NIKON", "D7000", Shade, 0,			{ 2.492188, 1, 1.136719, 0 } },
  { "NIKON", "D7000", Shade, 6,			{ 2.246094, 1, 1.218750, 0 } },
  { "NIKON", "D7000", Incandescent, -6,		{ 1.417969, 1, 2.003906, 0 } },
  { "NIKON", "D7000", Incandescent, 0,		{ 1.304688, 1, 2.214844, 0 } },
  { "NIKON", "D7000", Incandescent, 6,		{ 1.218750, 1, 2.464844, 0 } },
  { "NIKON", "D7000", SodiumVaporFluorescent, -6, { 1.324219, 1, 2.187500, 0 } },
  { "NIKON", "D7000", SodiumVaporFluorescent, 0, { 1.203125, 1, 2.437500, 0 } },
  { "NIKON", "D7000", SodiumVaporFluorescent, 6, { 1.128906, 1, 2.718750, 0 } },
  { "NIKON", "D7000", WarmWhiteFluorescent, -6,	{ 1.414062, 1, 1.859375, 0 } },
  { "NIKON", "D7000", WarmWhiteFluorescent, 0,	{ 1.300781, 1, 1.964844, 0 } },
  { "NIKON", "D7000", WarmWhiteFluorescent, 6,	{ 1.199219, 1, 2.082031, 0 } },
  { "NIKON", "D7000", WhiteFluorescent, -6,	{ 1.812500, 1, 2.042969, 0 } },
  { "NIKON", "D7000", WhiteFluorescent, 0,	{ 1.496094, 1, 2.304688, 0 } },
  { "NIKON", "D7000", WhiteFluorescent, 6,	{ 1.078125, 1, 2.628906, 0 } },
  { "NIKON", "D7000", CoolWhiteFluorescent, -6,	{ 2.105469, 1, 1.808594, 0 } },
  { "NIKON", "D7000", CoolWhiteFluorescent, 0,	{ 1.832031, 1, 2.027344, 0 } },
  { "NIKON", "D7000", CoolWhiteFluorescent, 6,	{ 1.519531, 1, 2.281250, 0 } },
  { "NIKON", "D7000", DayWhiteFluorescent, -6,	{ 2.070312, 1, 1.214844, 0 } },
  { "NIKON", "D7000", DayWhiteFluorescent, 0,	{ 1.898438, 1, 1.390625, 0 } },
  { "NIKON", "D7000", DayWhiteFluorescent, 6,	{ 1.742187, 1, 1.546875, 0 } },
  { "NIKON", "D7000", DaylightFluorescent, -6,	{ 2.359375, 1, 1.000000, 0 } },
  { "NIKON", "D7000", DaylightFluorescent, 0,	{ 2.167969, 1, 1.117187, 0 } },
  { "NIKON", "D7000", DaylightFluorescent, 6,	{ 1.988281, 1, 1.300781, 0 } },
  { "NIKON", "D7000", HighTempMercuryVaporFluorescent, -6, { 2.578125, 1, 1.222656, 0 } },
  { "NIKON", "D7000", HighTempMercuryVaporFluorescent, 0, { 2.484375, 1, 1.367188, 0 } },
  { "NIKON", "D7000", HighTempMercuryVaporFluorescent, 6, { 2.351563, 1, 1.539063, 0 } },

  { "NIKON", "E5400", Daylight, -3,		{ 2.046875, 1, 1.449219, 0 } },
  { "NIKON", "E5400", Daylight, 0,		{ 1.800781, 1, 1.636719, 0 } },
  { "NIKON", "E5400", Daylight, 3,		{ 1.539062, 1, 1.820312, 0 } },
  { "NIKON", "E5400", Incandescent, -3,		{ 1.218750, 1, 2.656250, 0 } },
  { "NIKON", "E5400", Incandescent, 0,		{ 1.218750, 1, 2.656250, 0 } },
  { "NIKON", "E5400", Incandescent, 3,		{ 1.382812, 1, 2.351562, 0 } },
  { "NIKON", "E5400", Fluorescent, -3,		{ 1.703125, 1, 2.460938, 0 } },
  { "NIKON", "E5400", Fluorescent, 0,		{ 1.218750, 1, 2.656250, 0 } },
  { "NIKON", "E5400", Fluorescent, 3,		{ 1.953125, 1, 1.906250, 0 } },
  { "NIKON", "E5400", Cloudy, -3,		{ 1.703125, 1, 2.460938, 0 } },
  { "NIKON", "E5400", Cloudy, 0,		{ 1.996094, 1, 1.421875, 0 } },
  { "NIKON", "E5400", Cloudy, 3,		{ 2.265625, 1, 1.261719, 0 } },
  { "NIKON", "E5400", Flash, -3,		{ 2.792969, 1, 1.152344, 0 } },
  { "NIKON", "E5400", Flash, 0,			{ 2.328125, 1, 1.386719, 0 } },
  { "NIKON", "E5400", Flash, 3,			{ 2.328125, 1, 1.386719, 0 } },
  { "NIKON", "E5400", Shade, -3,		{ 2.722656, 1, 1.011719, 0 } },
  { "NIKON", "E5400", Shade, 0,			{ 2.269531, 1, 1.218750, 0 } },
  { "NIKON", "E5400", Shade, 3,			{ 2.269531, 1, 1.218750, 0 } },

  { "NIKON", "E8700", Daylight, 0,		{ 1.968750, 1, 1.582031, 0 } },
  { "NIKON", "E8700", Incandescent, 0,		{ 1.265625, 1, 2.765625, 0 } },
  { "NIKON", "E8700", Fluorescent, 0,		{ 1.863281, 1, 2.304688, 0 } },
  { "NIKON", "E8700", Cloudy, 0,		{ 2.218750, 1, 1.359375, 0 } },
  { "NIKON", "E8700", Flash, 0,			{ 2.535156, 1, 1.273438, 0 } },
  { "NIKON", "E8700", Shade, 0,			{ 2.527344, 1, 1.175781, 0 } },

  { "OLYMPUS", "C5050Z", Shade, -7,		{ 3.887324, 1.201878, 1, 0 } },
  { "OLYMPUS", "C5050Z", Shade, 0,		{ 1.757812, 1, 1.437500, 0 } },
  { "OLYMPUS", "C5050Z", Shade, 7,		{ 1.019531, 1, 2.140625, 0 } },
  { "OLYMPUS", "C5050Z", Cloudy, -7,		{ 3.255507, 1.127753, 1, 0 } },
  { "OLYMPUS", "C5050Z", Cloudy, 0,		{ 1.570312, 1, 1.531250, 0 } },
  { "OLYMPUS", "C5050Z", Cloudy, 7,		{ 1, 1.098712, 2.506438, 0 } },
  { "OLYMPUS", "C5050Z", Daylight, -7,		{ 2.892116, 1.062241, 1, 0 } },
  { "OLYMPUS", "C5050Z", Daylight, 0,		{ 1.480469, 1, 1.628906, 0 } },
  { "OLYMPUS", "C5050Z", Daylight, 7,		{ 1, 1.168950, 2.835616, 0 } },
  { "OLYMPUS", "C5050Z", EveningSun, -7,	{ 3.072649, 1.094017, 1, 0 } },
  { "OLYMPUS", "C5050Z", EveningSun, 0,		{ 1.527344, 1, 1.578125, 0 } },
  { "OLYMPUS", "C5050Z", EveningSun, 7,		{ 1, 1.132743, 2.659292, 0 } },
  { "OLYMPUS", "C5050Z", DaylightFluorescent, -7, { 3.321267, 1.158371, 1, 0 } }, /*6700K*/
  { "OLYMPUS", "C5050Z", DaylightFluorescent, 0, { 1.558594, 1, 1.492188, 0 } }, /*6700K*/
  { "OLYMPUS", "C5050Z", DaylightFluorescent, 7, { 1, 1.108225, 2.463203, 0 } }, /*6700K*/
  { "OLYMPUS", "C5050Z", NeutralFluorescent, -7, { 2.606426, 1.028112, 1, 0 } }, /*5000K*/
  { "OLYMPUS", "C5050Z", NeutralFluorescent, 0,	{ 1.378906, 1, 1.679688, 0 } }, /*5000K*/
  { "OLYMPUS", "C5050Z", NeutralFluorescent, 7,	{ 1, 1.254902, 3.137255, 0 } }, /*5000K*/
  { "OLYMPUS", "C5050Z", CoolWhiteFluorescent, -7, { 2.519531, 1, 1.281250, 0 } }, /*4200K*/
  { "OLYMPUS", "C5050Z", CoolWhiteFluorescent, 0, { 1.371094, 1, 2.210938, 0 } }, /*4200K*/
  { "OLYMPUS", "C5050Z", CoolWhiteFluorescent, 7, { 1, 1.261084, 4.152709, 0 } }, /*4200K*/
  { "OLYMPUS", "C5050Z", WhiteFluorescent, -7,	{ 1.707031, 1, 1.699219, 0 } }, /*3500K*/
  { "OLYMPUS", "C5050Z", WhiteFluorescent, 0,	{ 1, 1.075630, 3.151261, 0 } }, /*3500K*/
  { "OLYMPUS", "C5050Z", WhiteFluorescent, 7,	{ 1, 1.855072, 8.094203, 0 } }, /*3500K*/
  { "OLYMPUS", "C5050Z", Incandescent, -7,	{ 1.679688, 1, 1.652344, 0 } }, /*3000K*/
  { "OLYMPUS", "C5050Z", Incandescent, 0,	{ 1, 1.094017, 3.123932, 0 } }, /*3000K*/
  { "OLYMPUS", "C5050Z", Incandescent, 7,	{ 1, 1.896296, 8.066667, 0 } }, /*3000K*/

  { "OLYMPUS", "C5060WZ", Shade, 0,		{ 1.949219, 1, 1.195312, 0 } },
  { "OLYMPUS", "C5060WZ", Cloudy, 0,		{ 1.621094, 1, 1.410156, 0 } },
  { "OLYMPUS", "C5060WZ", DirectSunlight, 0,	{ 1.511719, 1, 1.500000, 0 } },
  { "OLYMPUS", "C5060WZ", EveningSun, 0,	{ 1.636719, 1, 1.496094, 0 } },
  { "OLYMPUS", "C5060WZ", DaylightFluorescent, 0, { 1.734375, 1, 1.343750, 0 } },
  { "OLYMPUS", "C5060WZ", NeutralFluorescent, 0, { 1.457031, 1, 1.691406, 0 } },
  { "OLYMPUS", "C5060WZ", CoolWhiteFluorescent, 0, { 1.417969, 1, 2.230469, 0 } },
  { "OLYMPUS", "C5060WZ", WhiteFluorescent, 0,	{ 1, 1.103448, 3.422414, 0 } },
  { "OLYMPUS", "C5060WZ", Incandescent, 0,	{ 1, 1.153153, 3.662162, 0 } },
  { "OLYMPUS", "C5060WZ", FlashAuto, 0,		{ 1.850000, 1, 1.308044, 0 } },

  // Olympus C8080WZ - firmware 757-78
  { "OLYMPUS", "C8080WZ", Shade, -7,		{ 1.515625, 1, 1.773438, 0 } },
  { "OLYMPUS", "C8080WZ", Shade, -6,		{ 1.671875, 1, 1.691406, 0 } },
  { "OLYMPUS", "C8080WZ", Shade, -5,		{ 1.832031, 1, 1.605469, 0 } },
  { "OLYMPUS", "C8080WZ", Shade, -4,		{ 1.988281, 1, 1.523438, 0 } },
  { "OLYMPUS", "C8080WZ", Shade, -3,		{ 2.144531, 1, 1.441406, 0 } },
  { "OLYMPUS", "C8080WZ", Shade, -2,		{ 2.300781, 1, 1.355469, 0 } },
  { "OLYMPUS", "C8080WZ", Shade, -1,		{ 2.457031, 1, 1.273438, 0 } },
  { "OLYMPUS", "C8080WZ", Shade, 0,		{ 2.617188, 1, 1.191406, 0 } },
  { "OLYMPUS", "C8080WZ", Shade, 1,		{ 2.929688, 1, 1.117188, 0 } },
  { "OLYMPUS", "C8080WZ", Shade, 2,		{ 3.242188, 1, 1.046875, 0 } },
  { "OLYMPUS", "C8080WZ", Shade, 3,		{ 3.644000, 1.024000, 1, 0 } },
  { "OLYMPUS", "C8080WZ", Shade, 4,		{ 4.290043, 1.108225, 1, 0 } },
  { "OLYMPUS", "C8080WZ", Shade, 5,		{ 5.032864, 1.201878, 1, 0 } },
  { "OLYMPUS", "C8080WZ", Shade, 6,		{ 5.907692, 1.312821, 1, 0 } },
  { "OLYMPUS", "C8080WZ", Shade, 7,		{ 7.000000, 1.454545, 1, 0 } },
  { "OLYMPUS", "C8080WZ", Cloudy, -7,		{ 1.277344, 1, 2.164062, 0 } },
  { "OLYMPUS", "C8080WZ", Cloudy, -6,		{ 1.406250, 1, 2.062500, 0 } },
  { "OLYMPUS", "C8080WZ", Cloudy, -5,		{ 1.539062, 1, 1.960938, 0 } },
  { "OLYMPUS", "C8080WZ", Cloudy, -4,		{ 1.671875, 1, 1.859375, 0 } },
  { "OLYMPUS", "C8080WZ", Cloudy, -3,		{ 1.804688, 1, 1.757812, 0 } },
  { "OLYMPUS", "C8080WZ", Cloudy, -2,		{ 1.937500, 1, 1.656250, 0 } },
  { "OLYMPUS", "C8080WZ", Cloudy, -1,		{ 2.070312, 1, 1.554688, 0 } },
  { "OLYMPUS", "C8080WZ", Cloudy, 0,		{ 2.203125, 1, 1.453125, 0 } },
  { "OLYMPUS", "C8080WZ", Cloudy, 1,		{ 2.464844, 1, 1.363281, 0 } },
  { "OLYMPUS", "C8080WZ", Cloudy, 2,		{ 2.730469, 1, 1.277344, 0 } },
  { "OLYMPUS", "C8080WZ", Cloudy, 3,		{ 2.996094, 1, 1.191406, 0 } },
  { "OLYMPUS", "C8080WZ", Cloudy, 4,		{ 3.257812, 1, 1.101562, 0 } },
  { "OLYMPUS", "C8080WZ", Cloudy, 5,		{ 3.523438, 1, 1.015625, 0 } },
  { "OLYMPUS", "C8080WZ", Cloudy, 6,		{ 4.075630, 1.075630, 1, 0 } },
  { "OLYMPUS", "C8080WZ", Cloudy, 7,		{ 4.823256, 1.190698, 1, 0 } },
  { "OLYMPUS", "C8080WZ", Daylight, -7,		{ 1.234375, 1, 2.343750, 0 } },
  { "OLYMPUS", "C8080WZ", Daylight, -6,		{ 1.359375, 1, 2.234375, 0 } },
  { "OLYMPUS", "C8080WZ", Daylight, -5,		{ 1.488281, 1, 2.125000, 0 } },
  { "OLYMPUS", "C8080WZ", Daylight, -4,		{ 1.617188, 1, 2.011719, 0 } },
  { "OLYMPUS", "C8080WZ", Daylight, -3,		{ 1.742188, 1, 1.902344, 0 } },
  { "OLYMPUS", "C8080WZ", Daylight, -2,		{ 1.871094, 1, 1.792969, 0 } },
  { "OLYMPUS", "C8080WZ", Daylight, -1,		{ 2.000000, 1, 1.683594, 0 } },
  { "OLYMPUS", "C8080WZ", Daylight, 0,		{ 2.128906, 1, 1.574219, 0 } },
  { "OLYMPUS", "C8080WZ", Daylight, 1,		{ 2.382812, 1, 1.476562, 0 } },
  { "OLYMPUS", "C8080WZ", Daylight, 2,		{ 2.636719, 1, 1.382812, 0 } },
  { "OLYMPUS", "C8080WZ", Daylight, 3,		{ 2.894531, 1, 1.289062, 0 } },
  { "OLYMPUS", "C8080WZ", Daylight, 4,		{ 3.148438, 1, 1.195312, 0 } },
  { "OLYMPUS", "C8080WZ", Daylight, 5,		{ 3.406250, 1, 1.101562, 0 } },
  { "OLYMPUS", "C8080WZ", Daylight, 6,		{ 3.660156, 1, 1.003906, 0 } },
  { "OLYMPUS", "C8080WZ", Daylight, 7,		{ 4.300429, 1.098712, 1, 0 } },
  { "OLYMPUS", "C8080WZ", EveningSun, -7,	{ 1.308594, 1, 2.199219, 0 } },
  { "OLYMPUS", "C8080WZ", EveningSun, -6,	{ 1.445312, 1, 2.093750, 0 } },
  { "OLYMPUS", "C8080WZ", EveningSun, -5,	{ 1.582031, 1, 1.992188, 0 } },
  { "OLYMPUS", "C8080WZ", EveningSun, -4,	{ 1.718750, 1, 1.886719, 0 } },
  { "OLYMPUS", "C8080WZ", EveningSun, -3,	{ 1.851562, 1, 1.785156, 0 } },
  { "OLYMPUS", "C8080WZ", EveningSun, -2,	{ 1.988281, 1, 1.679688, 0 } },
  { "OLYMPUS", "C8080WZ", EveningSun, -1,	{ 2.125000, 1, 1.578125, 0 } },
  { "OLYMPUS", "C8080WZ", EveningSun, 0,	{ 2.261719, 1, 1.476562, 0 } },
  { "OLYMPUS", "C8080WZ", EveningSun, 1,	{ 2.531250, 1, 1.386719, 0 } },
  { "OLYMPUS", "C8080WZ", EveningSun, 2,	{ 2.800781, 1, 1.296875, 0 } },
  { "OLYMPUS", "C8080WZ", EveningSun, 3,	{ 3.074219, 1, 1.207031, 0 } },
  { "OLYMPUS", "C8080WZ", EveningSun, 4,	{ 3.343750, 1, 1.121094, 0 } },
  { "OLYMPUS", "C8080WZ", EveningSun, 5,	{ 3.617188, 1, 1.031250, 0 } },
  { "OLYMPUS", "C8080WZ", EveningSun, 6,	{ 4.128631, 1.062241, 1, 0 } },
  { "OLYMPUS", "C8080WZ", EveningSun, 7,	{ 4.863014, 1.168950, 1, 0 } },
  { "OLYMPUS", "C8080WZ", FlashAuto, -7,	{ 1.488281, 1, 2.214844, 0 } },
  { "OLYMPUS", "C8080WZ", FlashAuto, -6,	{ 1.652344, 1, 2.105469, 0 } },
  { "OLYMPUS", "C8080WZ", FlashAuto, -5,	{ 1.812500, 1, 1.992188, 0 } },
  { "OLYMPUS", "C8080WZ", FlashAuto, -4,	{ 1.976562, 1, 1.882812, 0 } },
  { "OLYMPUS", "C8080WZ", FlashAuto, -3,	{ 2.117188, 1, 1.773438, 0 } },
  { "OLYMPUS", "C8080WZ", FlashAuto, -2,	{ 2.253906, 1, 1.675781, 0 } },
  { "OLYMPUS", "C8080WZ", FlashAuto, -1,	{ 2.425781, 1, 1.585938, 0 } },
  { "OLYMPUS", "C8080WZ", FlashAuto, 0,		{ 2.570312, 1, 1.468750, 0 } },
  { "OLYMPUS", "C8080WZ", FlashAuto, 1,		{ 2.890625, 1, 1.386719, 0 } },
  { "OLYMPUS", "C8080WZ", FlashAuto, 2,		{ 3.199219, 1, 1.308594, 0 } },
  { "OLYMPUS", "C8080WZ", FlashAuto, 3,		{ 3.500000, 1, 1.214844, 0 } },
  { "OLYMPUS", "C8080WZ", FlashAuto, 4,		{ 3.820312, 1, 1.125000, 0 } },
  { "OLYMPUS", "C8080WZ", FlashAuto, 5,		{ 4.128906, 1, 1.039062, 0 } },
  { "OLYMPUS", "C8080WZ", FlashAuto, 6,		{ 4.711934, 1.053498, 1, 0 } },
  { "OLYMPUS", "C8080WZ", FlashAuto, 7,		{ 5.450450, 1.153153, 1, 0 } },
  { "OLYMPUS", "C8080WZ", DaylightFluorescent, -7, { 1.425781, 1, 2.097656, 0 } }, /*6700K*/
  { "OLYMPUS", "C8080WZ", DaylightFluorescent, -6, { 1.574219, 1, 2.000000, 0 } }, /*6700K*/
  { "OLYMPUS", "C8080WZ", DaylightFluorescent, -5, { 1.722656, 1, 1.902344, 0 } }, /*6700K*/
  { "OLYMPUS", "C8080WZ", DaylightFluorescent, -4, { 1.867188, 1, 1.804688, 0 } }, /*6700K*/
  { "OLYMPUS", "C8080WZ", DaylightFluorescent, -3, { 2.015625, 1, 1.703125, 0 } }, /*6700K*/
  { "OLYMPUS", "C8080WZ", DaylightFluorescent, -2, { 2.164062, 1, 1.605469, 0 } }, /*6700K*/
  { "OLYMPUS", "C8080WZ", DaylightFluorescent, -1, { 2.312500, 1, 1.507812, 0 } }, /*6700K*/
  { "OLYMPUS", "C8080WZ", DaylightFluorescent, 0, { 2.460938, 1, 1.410156, 0 } }, /*6700K*/
  { "OLYMPUS", "C8080WZ", DaylightFluorescent, 1, { 2.753906, 1, 1.324219, 0 } }, /*6700K*/
  { "OLYMPUS", "C8080WZ", DaylightFluorescent, 2, { 3.050781, 1, 1.238281, 0 } }, /*6700K*/
  { "OLYMPUS", "C8080WZ", DaylightFluorescent, 3, { 3.343750, 1, 1.156250, 0 } }, /*6700K*/
  { "OLYMPUS", "C8080WZ", DaylightFluorescent, 4, { 3.640625, 1, 1.070312, 0 } }, /*6700K*/
  { "OLYMPUS", "C8080WZ", DaylightFluorescent, 5, { 4.000000, 1.015873, 1, 0 } }, /*6700K*/
  { "OLYMPUS", "C8080WZ", DaylightFluorescent, 6, { 4.688312, 1.108225, 1, 0 } }, /*6700K*/
  { "OLYMPUS", "C8080WZ", DaylightFluorescent, 7, { 5.545455, 1.224880, 1, 0 } }, /*6700K*/
  { "OLYMPUS", "C8080WZ", NeutralFluorescent, -7, { 1.195312, 1, 2.589844, 0 } }, /*5000K*/
  { "OLYMPUS", "C8080WZ", NeutralFluorescent, -6, { 1.316406, 1, 2.464844, 0 } }, /*5000K*/
  { "OLYMPUS", "C8080WZ", NeutralFluorescent, -5, { 1.441406, 1, 2.343750, 0 } }, /*5000K*/
  { "OLYMPUS", "C8080WZ", NeutralFluorescent, -4, { 1.566406, 1, 2.222656, 0 } }, /*5000K*/
  { "OLYMPUS", "C8080WZ", NeutralFluorescent, -3, { 1.687500, 1, 2.101562, 0 } }, /*5000K*/
  { "OLYMPUS", "C8080WZ", NeutralFluorescent, -2, { 1.812500, 1, 1.980469, 0 } }, /*5000K*/
  { "OLYMPUS", "C8080WZ", NeutralFluorescent, -1, { 1.937500, 1, 1.859375, 0 } }, /*5000K*/
  { "OLYMPUS", "C8080WZ", NeutralFluorescent, 0, { 2.062500, 1, 1.738281, 0 } }, /*5000K*/
  { "OLYMPUS", "C8080WZ", NeutralFluorescent, 1, { 2.308594, 1, 1.632812, 0 } }, /*5000K*/
  { "OLYMPUS", "C8080WZ", NeutralFluorescent, 2, { 2.554688, 1, 1.527344, 0 } }, /*5000K*/
  { "OLYMPUS", "C8080WZ", NeutralFluorescent, 3, { 2.804688, 1, 1.421875, 0 } }, /*5000K*/
  { "OLYMPUS", "C8080WZ", NeutralFluorescent, 4, { 3.050781, 1, 1.320312, 0 } }, /*5000K*/
  { "OLYMPUS", "C8080WZ", NeutralFluorescent, 5, { 3.296875, 1, 1.214844, 0 } }, /*5000K*/
  { "OLYMPUS", "C8080WZ", NeutralFluorescent, 6, { 3.546875, 1, 1.109375, 0 } }, /*5000K*/
  { "OLYMPUS", "C8080WZ", NeutralFluorescent, 7, { 3.792969, 1, 1.007812, 0 } }, /*5000K*/
  { "OLYMPUS", "C8080WZ", CoolWhiteFluorescent, -7, { 1.109375, 1, 3.257812, 0 } }, /*4200K*/
  { "OLYMPUS", "C8080WZ", CoolWhiteFluorescent, -6, { 1.226562, 1, 3.105469, 0 } }, /*4200K*/
  { "OLYMPUS", "C8080WZ", CoolWhiteFluorescent, -5, { 1.339844, 1, 2.953125, 0 } }, /*4200K*/
  { "OLYMPUS", "C8080WZ", CoolWhiteFluorescent, -4, { 1.457031, 1, 2.796875, 0 } }, /*4200K*/
  { "OLYMPUS", "C8080WZ", CoolWhiteFluorescent, -3, { 1.570312, 1, 2.644531, 0 } }, /*4200K*/
  { "OLYMPUS", "C8080WZ", CoolWhiteFluorescent, -2, { 1.687500, 1, 2.492188, 0 } }, /*4200K*/
  { "OLYMPUS", "C8080WZ", CoolWhiteFluorescent, -1, { 1.800781, 1, 2.339844, 0 } }, /*4200K*/
  { "OLYMPUS", "C8080WZ", CoolWhiteFluorescent, 0, { 1.917969, 1, 2.187500, 0 } }, /*4200K*/
  { "OLYMPUS", "C8080WZ", CoolWhiteFluorescent, 1, { 2.144531, 1, 2.054688, 0 } }, /*4200K*/
  { "OLYMPUS", "C8080WZ", CoolWhiteFluorescent, 2, { 2.375000, 1, 1.921875, 0 } }, /*4200K*/
  { "OLYMPUS", "C8080WZ", CoolWhiteFluorescent, 3, { 2.605469, 1, 1.792969, 0 } }, /*4200K*/
  { "OLYMPUS", "C8080WZ", CoolWhiteFluorescent, 4, { 2.835938, 1, 1.660156, 0 } }, /*4200K*/
  { "OLYMPUS", "C8080WZ", CoolWhiteFluorescent, 5, { 3.066406, 1, 1.531250, 0 } }, /*4200K*/
  { "OLYMPUS", "C8080WZ", CoolWhiteFluorescent, 6, { 3.296875, 1, 1.398438, 0 } }, /*4200K*/
  { "OLYMPUS", "C8080WZ", CoolWhiteFluorescent, 7, { 3.527344, 1, 1.265625, 0 } }, /*4200K*/
  { "OLYMPUS", "C8080WZ", WhiteFluorescent, -7,	{ 1, 1.347368, 5.963158, 0 } }, /*3500K*/
  { "OLYMPUS", "C8080WZ", WhiteFluorescent, -6,	{ 1, 1.224880, 5.167464, 0 } }, /*3500K*/
  { "OLYMPUS", "C8080WZ", WhiteFluorescent, -5,	{ 1, 1.117904, 4.484716, 0 } }, /*3500K*/
  { "OLYMPUS", "C8080WZ", WhiteFluorescent, -4,	{ 1, 1.028112, 3.911647, 0 } }, /*3500K*/
  { "OLYMPUS", "C8080WZ", WhiteFluorescent, -3,	{ 1.046875, 1, 3.593750, 0 } }, /*3500K*/
  { "OLYMPUS", "C8080WZ", WhiteFluorescent, -2,	{ 1.125000, 1, 3.386719, 0 } }, /*3500K*/
  { "OLYMPUS", "C8080WZ", WhiteFluorescent, -1,	{ 1.203125, 1, 3.179688, 0 } }, /*3500K*/
  { "OLYMPUS", "C8080WZ", WhiteFluorescent, 0,	{ 1.281250, 1, 2.972656, 0 } }, /*3500K*/
  { "OLYMPUS", "C8080WZ", WhiteFluorescent, 1,	{ 1.433594, 1, 2.792969, 0 } }, /*3500K*/
  { "OLYMPUS", "C8080WZ", WhiteFluorescent, 2,	{ 1.585938, 1, 2.613281, 0 } }, /*3500K*/
  { "OLYMPUS", "C8080WZ", WhiteFluorescent, 3,	{ 1.742188, 1, 2.437500, 0 } }, /*3500K*/
  { "OLYMPUS", "C8080WZ", WhiteFluorescent, 4,	{ 1.894531, 1, 2.257812, 0 } }, /*3500K*/
  { "OLYMPUS", "C8080WZ", WhiteFluorescent, 5,	{ 2.046875, 1, 2.078125, 0 } }, /*3500K*/
  { "OLYMPUS", "C8080WZ", WhiteFluorescent, 6,	{ 2.203125, 1, 1.902344, 0 } }, /*3500K*/
  { "OLYMPUS", "C8080WZ", WhiteFluorescent, 7,	{ 2.355469, 1, 1.722656, 0 } }, /*3500K*/
  { "OLYMPUS", "C8080WZ", Tungsten, -7,		{ 1, 1.488372, 6.988372, 0 } }, /*3000K*/
  { "OLYMPUS", "C8080WZ", Tungsten, -6,		{ 1, 1.347368, 6.026316, 0 } }, /*3000K*/
  { "OLYMPUS", "C8080WZ", Tungsten, -5,		{ 1, 1.230769, 5.235577, 0 } }, /*3000K*/
  { "OLYMPUS", "C8080WZ", Tungsten, -4,		{ 1, 1.132743, 4.566372, 0 } }, /*3000K*/
  { "OLYMPUS", "C8080WZ", Tungsten, -3,		{ 1, 1.049180, 4.000000, 0 } }, /*3000K*/
  { "OLYMPUS", "C8080WZ", Tungsten, -2,		{ 1.023438, 1, 3.589844, 0 } }, /*3000K*/
  { "OLYMPUS", "C8080WZ", Tungsten, -1,		{ 1.093750, 1, 3.371094, 0 } }, /*3000K*/
  { "OLYMPUS", "C8080WZ", Tungsten, 0,		{ 1.164062, 1, 3.152344, 0 } }, /*3000K*/
  { "OLYMPUS", "C8080WZ", Tungsten, 1,		{ 1.300781, 1, 2.960938, 0 } }, /*3000K*/
  { "OLYMPUS", "C8080WZ", Tungsten, 2,		{ 1.441406, 1, 2.773438, 0 } }, /*3000K*/
  { "OLYMPUS", "C8080WZ", Tungsten, 3,		{ 1.582031, 1, 2.582031, 0 } }, /*3000K*/
  { "OLYMPUS", "C8080WZ", Tungsten, 4,		{ 1.722656, 1, 2.394531, 0 } }, /*3000K*/
  { "OLYMPUS", "C8080WZ", Tungsten, 5,		{ 1.722656, 1, 2.394531, 0 } }, /*3000K*/
  { "OLYMPUS", "C8080WZ", Tungsten, 6,		{ 2.000000, 1, 2.015625, 0 } }, /*3000K*/
  { "OLYMPUS", "C8080WZ", Tungsten, 7,		{ 2.140625, 1, 1.828125, 0 } }, /*3000K*/
// Fin ajout

  { "OLYMPUS", "E-1", Incandescent, -7,		{ 1.195312, 1, 1.562500, 0 } }, /*3600K*/
  { "OLYMPUS", "E-1", Incandescent, -6,		{ 1.187500, 1, 1.578125, 0 } }, /*3600K*/
  { "OLYMPUS", "E-1", Incandescent, -5,		{ 1.187500, 1, 1.585938, 0 } }, /*3600K*/
  { "OLYMPUS", "E-1", Incandescent, -4,		{ 1.179688, 1, 1.601562, 0 } }, /*3600K*/
  { "OLYMPUS", "E-1", Incandescent, -3,		{ 1.171875, 1, 1.609375, 0 } }, /*3600K*/
  { "OLYMPUS", "E-1", Incandescent, -2,		{ 1.164062, 1, 1.617188, 0 } }, /*3600K*/
  { "OLYMPUS", "E-1", Incandescent, -1,		{ 1.156250, 1, 1.632812, 0 } }, /*3600K*/
  { "OLYMPUS", "E-1", Incandescent, 0,		{ 1.156250, 1, 1.640625, 0 } }, /*3600K*/
  { "OLYMPUS", "E-1", Incandescent, 1,		{ 1.140625, 1, 1.648438, 0 } }, /*3600K*/
  { "OLYMPUS", "E-1", Incandescent, 2,		{ 1.132812, 1, 1.664062, 0 } }, /*3600K*/
  { "OLYMPUS", "E-1", Incandescent, 3,		{ 1.125000, 1, 1.671875, 0 } }, /*3600K*/
  { "OLYMPUS", "E-1", Incandescent, 4,		{ 1.117188, 1, 1.679688, 0 } }, /*3600K*/
  { "OLYMPUS", "E-1", Incandescent, 5,		{ 1.117188, 1, 1.695312, 0 } }, /*3600K*/
  { "OLYMPUS", "E-1", Incandescent, 6,		{ 1.109375, 1, 1.703125, 0 } }, /*3600K*/
  { "OLYMPUS", "E-1", Incandescent, 7,		{ 1.101562, 1, 1.718750, 0 } }, /*3600K*/
  { "OLYMPUS", "E-1", IncandescentWarm, -7,	{ 1.015625, 1, 1.867188, 0 } }, /*3000K*/
  { "OLYMPUS", "E-1", IncandescentWarm, -6,	{ 1.007812, 1, 1.875000, 0 } }, /*3000K*/
  { "OLYMPUS", "E-1", IncandescentWarm, -5,	{ 1.000000, 1, 1.890625, 0 } }, /*3000K*/
  { "OLYMPUS", "E-1", IncandescentWarm, -4,	{ 1, 1.007874, 1.913386, 0 } }, /*3000K*/
  { "OLYMPUS", "E-1", IncandescentWarm, -3,	{ 1, 1.015873, 1.944444, 0 } }, /*3000K*/
  { "OLYMPUS", "E-1", IncandescentWarm, -2,	{ 1, 1.015873, 1.952381, 0 } }, /*3000K*/
  { "OLYMPUS", "E-1", IncandescentWarm, -1,	{ 1, 1.024000, 1.984000, 0 } }, /*3000K*/
  { "OLYMPUS", "E-1", IncandescentWarm, 0,	{ 1, 1.024000, 1.992000, 0 } }, /*3000K*/
  { "OLYMPUS", "E-1", IncandescentWarm, 1,	{ 1, 1.032258, 2.008065, 0 } }, /*3000K*/
  { "OLYMPUS", "E-1", IncandescentWarm, 2,	{ 1, 1.040650, 2.040650, 0 } }, /*3000K*/
  { "OLYMPUS", "E-1", IncandescentWarm, 3,	{ 1, 1.040650, 2.048780, 0 } }, /*3000K*/
  { "OLYMPUS", "E-1", IncandescentWarm, 4,	{ 1, 1.049180, 2.081967, 0 } }, /*3000K*/
  { "OLYMPUS", "E-1", IncandescentWarm, 5,	{ 1, 1.057851, 2.107438, 0 } }, /*3000K*/
  { "OLYMPUS", "E-1", IncandescentWarm, 6,	{ 1, 1.066667, 2.141667, 0 } }, /*3000K*/
  { "OLYMPUS", "E-1", IncandescentWarm, 7,	{ 1, 1.075630, 2.168067, 0 } }, /*3000K*/
  { "OLYMPUS", "E-1", WhiteFluorescent, -7,	{ 2.296875, 1, 1.445312, 0 } }, /*4000K*/
  { "OLYMPUS", "E-1", WhiteFluorescent, -6,	{ 2.273438, 1, 1.468750, 0 } }, /*4000K*/
  { "OLYMPUS", "E-1", WhiteFluorescent, -5,	{ 2.242188, 1, 1.492188, 0 } }, /*4000K*/
  { "OLYMPUS", "E-1", WhiteFluorescent, -4,	{ 2.210938, 1, 1.523438, 0 } }, /*4000K*/
  { "OLYMPUS", "E-1", WhiteFluorescent, -3,	{ 2.171875, 1, 1.562500, 0 } }, /*4000K*/
  { "OLYMPUS", "E-1", WhiteFluorescent, -2,	{ 2.132812, 1, 1.601562, 0 } }, /*4000K*/
  { "OLYMPUS", "E-1", WhiteFluorescent, -1,	{ 2.093750, 1, 1.640625, 0 } }, /*4000K*/
  { "OLYMPUS", "E-1", WhiteFluorescent, 0,	{ 2.062500, 1, 1.679688, 0 } }, /*4000K*/
  { "OLYMPUS", "E-1", WhiteFluorescent, 1,	{ 2.039062, 1, 1.703125, 0 } }, /*4000K*/
  { "OLYMPUS", "E-1", WhiteFluorescent, 2,	{ 2.015625, 1, 1.734375, 0 } }, /*4000K*/
  { "OLYMPUS", "E-1", WhiteFluorescent, 3,	{ 2.000000, 1, 1.757812, 0 } }, /*4000K*/
  { "OLYMPUS", "E-1", WhiteFluorescent, 4,	{ 1.984375, 1, 1.789062, 0 } }, /*4000K*/
  { "OLYMPUS", "E-1", WhiteFluorescent, 5,	{ 1.968750, 1, 1.812500, 0 } }, /*4000K*/
  { "OLYMPUS", "E-1", WhiteFluorescent, 6,	{ 1.945312, 1, 1.835938, 0 } }, /*4000K*/
  { "OLYMPUS", "E-1", WhiteFluorescent, 7,	{ 1.929688, 1, 1.867188, 0 } }, /*4000K*/
  { "OLYMPUS", "E-1", NeutralFluorescent, -7,	{ 1.984375, 1, 1.203125, 0 } }, /*4500K*/
  { "OLYMPUS", "E-1", NeutralFluorescent, -6,	{ 1.960938, 1, 1.218750, 0 } }, /*4500K*/
  { "OLYMPUS", "E-1", NeutralFluorescent, -5,	{ 1.937500, 1, 1.234375, 0 } }, /*4500K*/
  { "OLYMPUS", "E-1", NeutralFluorescent, -4,	{ 1.921875, 1, 1.257812, 0 } }, /*4500K*/
  { "OLYMPUS", "E-1", NeutralFluorescent, -3,	{ 1.898438, 1, 1.273438, 0 } }, /*4500K*/
  { "OLYMPUS", "E-1", NeutralFluorescent, -2,	{ 1.875000, 1, 1.289062, 0 } }, /*4500K*/
  { "OLYMPUS", "E-1", NeutralFluorescent, -1,	{ 1.851562, 1, 1.304688, 0 } }, /*4500K*/
  { "OLYMPUS", "E-1", NeutralFluorescent, 0,	{ 1.835938, 1, 1.320312, 0 } }, /*4500K*/
  { "OLYMPUS", "E-1", NeutralFluorescent, 1,	{ 1.804688, 1, 1.343750, 0 } }, /*4500K*/
  { "OLYMPUS", "E-1", NeutralFluorescent, 2,	{ 1.773438, 1, 1.367188, 0 } }, /*4500K*/
  { "OLYMPUS", "E-1", NeutralFluorescent, 3,	{ 1.750000, 1, 1.390625, 0 } }, /*4500K*/
  { "OLYMPUS", "E-1", NeutralFluorescent, 4,	{ 1.718750, 1, 1.414062, 0 } }, /*4500K*/
  { "OLYMPUS", "E-1", NeutralFluorescent, 5,	{ 1.695312, 1, 1.437500, 0 } }, /*4500K*/
  { "OLYMPUS", "E-1", NeutralFluorescent, 6,	{ 1.656250, 1, 1.476562, 0 } }, /*4500K*/
  { "OLYMPUS", "E-1", NeutralFluorescent, 7,	{ 1.617188, 1, 1.515625, 0 } }, /*4500K*/
  { "OLYMPUS", "E-1", DaylightFluorescent, -7,	{ 2.819820, 1.153153, 1, 0 } }, /*6600K*/
  { "OLYMPUS", "E-1", DaylightFluorescent, -6,	{ 2.669565, 1.113043, 1, 0 } }, /*6600K*/
  { "OLYMPUS", "E-1", DaylightFluorescent, -5,	{ 2.521008, 1.075630, 1, 0 } }, /*6600K*/
  { "OLYMPUS", "E-1", DaylightFluorescent, -4,	{ 2.390244, 1.040650, 1, 0 } }, /*6600K*/
  { "OLYMPUS", "E-1", DaylightFluorescent, -3,	{ 2.259843, 1.007874, 1, 0 } }, /*6600K*/
  { "OLYMPUS", "E-1", DaylightFluorescent, -2,	{ 2.195312, 1, 1.023438, 0 } }, /*6600K*/
  { "OLYMPUS", "E-1", DaylightFluorescent, -1,	{ 2.140625, 1, 1.054688, 0 } }, /*6600K*/
  { "OLYMPUS", "E-1", DaylightFluorescent, 0,	{ 2.101562, 1, 1.085938, 0 } }, /*6600K*/
  { "OLYMPUS", "E-1", DaylightFluorescent, 1,	{ 2.070312, 1, 1.101562, 0 } }, /*6600K*/
  { "OLYMPUS", "E-1", DaylightFluorescent, 2,	{ 2.046875, 1, 1.117188, 0 } }, /*6600K*/
  { "OLYMPUS", "E-1", DaylightFluorescent, 3,	{ 2.023438, 1, 1.132812, 0 } }, /*6600K*/
  { "OLYMPUS", "E-1", DaylightFluorescent, 4,	{ 2.000000, 1, 1.156250, 0 } }, /*6600K*/
  { "OLYMPUS", "E-1", DaylightFluorescent, 5,	{ 1.976562, 1, 1.171875, 0 } }, /*6600K*/
  { "OLYMPUS", "E-1", DaylightFluorescent, 6,	{ 1.953125, 1, 1.187500, 0 } }, /*6600K*/
  { "OLYMPUS", "E-1", DaylightFluorescent, 7,	{ 1.929688, 1, 1.203125, 0 } }, /*6600K*/
  { "OLYMPUS", "E-1", Daylight, -7,		{ 1.726562, 1, 1.093750, 0 } }, /*5300K*/
  { "OLYMPUS", "E-1", Daylight, -6,		{ 1.710938, 1, 1.101562, 0 } }, /*5300K*/
  { "OLYMPUS", "E-1", Daylight, -5,		{ 1.703125, 1, 1.109375, 0 } }, /*5300K*/
  { "OLYMPUS", "E-1", Daylight, -4,		{ 1.695312, 1, 1.117188, 0 } }, /*5300K*/
  { "OLYMPUS", "E-1", Daylight, -3,		{ 1.687500, 1, 1.117188, 0 } }, /*5300K*/
  { "OLYMPUS", "E-1", Daylight, -2,		{ 1.671875, 1, 1.125000, 0 } }, /*5300K*/
  { "OLYMPUS", "E-1", Daylight, -1,		{ 1.664062, 1, 1.132812, 0 } }, /*5300K*/
  { "OLYMPUS", "E-1", Daylight, 0,		{ 1.664062, 1, 1.140625, 0 } }, /*5300K*/
  { "OLYMPUS", "E-1", Daylight, 1,		{ 1.648438, 1, 1.148438, 0 } }, /*5300K*/
  { "OLYMPUS", "E-1", Daylight, 2,		{ 1.640625, 1, 1.156250, 0 } }, /*5300K*/
  { "OLYMPUS", "E-1", Daylight, 3,		{ 1.632812, 1, 1.164062, 0 } }, /*5300K*/
  { "OLYMPUS", "E-1", Daylight, 4,		{ 1.617188, 1, 1.164062, 0 } }, /*5300K*/
  { "OLYMPUS", "E-1", Daylight, 5,		{ 1.609375, 1, 1.171875, 0 } }, /*5300K*/
  { "OLYMPUS", "E-1", Daylight, 6,		{ 1.601562, 1, 1.179688, 0 } }, /*5300K*/
  { "OLYMPUS", "E-1", Daylight, 7,		{ 1.593750, 1, 1.187500, 0 } }, /*5300K*/
  { "OLYMPUS", "E-1", Cloudy, -7,		{ 2.008130, 1.040650, 1, 0 } }, /*6000K*/
  { "OLYMPUS", "E-1", Cloudy, -6,		{ 1.967742, 1.032258, 1, 0 } }, /*6000K*/
  { "OLYMPUS", "E-1", Cloudy, -5,		{ 1.920635, 1.015873, 1, 0 } }, /*6000K*/
  { "OLYMPUS", "E-1", Cloudy, -4,		{ 1.867188, 1, 1.000000, 0 } }, /*6000K*/
  { "OLYMPUS", "E-1", Cloudy, -3,		{ 1.851562, 1, 1.007812, 0 } }, /*6000K*/
  { "OLYMPUS", "E-1", Cloudy, -2,		{ 1.828125, 1, 1.023438, 0 } }, /*6000K*/
  { "OLYMPUS", "E-1", Cloudy, -1,		{ 1.812500, 1, 1.031250, 0 } }, /*6000K*/
  { "OLYMPUS", "E-1", Cloudy, 0,		{ 1.796875, 1, 1.046875, 0 } }, /*6000K*/
  { "OLYMPUS", "E-1", Cloudy, 1,		{ 1.781250, 1, 1.054688, 0 } }, /*6000K*/
  { "OLYMPUS", "E-1", Cloudy, 2,		{ 1.773438, 1, 1.062500, 0 } }, /*6000K*/
  { "OLYMPUS", "E-1", Cloudy, 3,		{ 1.757812, 1, 1.070312, 0 } }, /*6000K*/
  { "OLYMPUS", "E-1", Cloudy, 4,		{ 1.750000, 1, 1.070312, 0 } }, /*6000K*/
  { "OLYMPUS", "E-1", Cloudy, 5,		{ 1.742188, 1, 1.078125, 0 } }, /*6000K*/
  { "OLYMPUS", "E-1", Cloudy, 6,		{ 1.734375, 1, 1.085938, 0 } }, /*6000K*/
  { "OLYMPUS", "E-1", Cloudy, 7,		{ 1.726562, 1, 1.093750, 0 } }, /*6000K*/
  { "OLYMPUS", "E-1", Shade, -7,		{ 2.584906, 1.207547, 1, 0 } }, /*7500K*/
  { "OLYMPUS", "E-1", Shade, -6,		{ 2.532710, 1.196262, 1, 0 } }, /*7500K*/
  { "OLYMPUS", "E-1", Shade, -5,		{ 2.467890, 1.174312, 1, 0 } }, /*7500K*/
  { "OLYMPUS", "E-1", Shade, -4,		{ 2.396396, 1.153153, 1, 0 } }, /*7500K*/
  { "OLYMPUS", "E-1", Shade, -3,		{ 2.357143, 1.142857, 1, 0 } }, /*7500K*/
  { "OLYMPUS", "E-1", Shade, -2,		{ 2.289474, 1.122807, 1, 0 } }, /*7500K*/
  { "OLYMPUS", "E-1", Shade, -1,		{ 2.252174, 1.113043, 1, 0 } }, /*7500K*/
  { "OLYMPUS", "E-1", Shade, 0,			{ 2.196581, 1.094017, 1, 0 } }, /*7500K*/
  { "OLYMPUS", "E-1", Shade, 1,			{ 2.126050, 1.075630, 1, 0 } }, /*7500K*/
  { "OLYMPUS", "E-1", Shade, 2,			{ 2.091667, 1.066667, 1, 0 } }, /*7500K*/
  { "OLYMPUS", "E-1", Shade, 3,			{ 2.032787, 1.049180, 1, 0 } }, /*7500K*/
  { "OLYMPUS", "E-1", Shade, 4,			{ 2.000000, 1.040650, 1, 0 } }, /*7500K*/
  { "OLYMPUS", "E-1", Shade, 5,			{ 1.944000, 1.024000, 1, 0 } }, /*7500K*/
  { "OLYMPUS", "E-1", Shade, 6,			{ 1.897638, 1.007874, 1, 0 } }, /*7500K*/
  { "OLYMPUS", "E-1", Shade, 7,			{ 1.859375, 1, 1.000000, 0 } }, /*7500K*/
  { "OLYMPUS", "E-1", "3300K", -7,		{ 1.109375, 1, 1.695312, 0 } },
  { "OLYMPUS", "E-1", "3300K", -6,		{ 1.101562, 1, 1.710938, 0 } },
  { "OLYMPUS", "E-1", "3300K", -5,		{ 1.093750, 1, 1.718750, 0 } },
  { "OLYMPUS", "E-1", "3300K", -4,		{ 1.093750, 1, 1.734375, 0 } },
  { "OLYMPUS", "E-1", "3300K", -3,		{ 1.085938, 1, 1.742188, 0 } },
  { "OLYMPUS", "E-1", "3300K", -2,		{ 1.078125, 1, 1.750000, 0 } },
  { "OLYMPUS", "E-1", "3300K", -1,		{ 1.070312, 1, 1.765625, 0 } },
  { "OLYMPUS", "E-1", "3300K", 0,		{ 1.070312, 1, 1.773438, 0 } },
  { "OLYMPUS", "E-1", "3300K", 1,		{ 1.054688, 1, 1.781250, 0 } },
  { "OLYMPUS", "E-1", "3300K", 2,		{ 1.046875, 1, 1.796875, 0 } },
  { "OLYMPUS", "E-1", "3300K", 3,		{ 1.046875, 1, 1.804688, 0 } },
  { "OLYMPUS", "E-1", "3300K", 4,		{ 1.039062, 1, 1.820312, 0 } },
  { "OLYMPUS", "E-1", "3300K", 5,		{ 1.031250, 1, 1.828125, 0 } },
  { "OLYMPUS", "E-1", "3300K", 6,		{ 1.023438, 1, 1.843750, 0 } },
  { "OLYMPUS", "E-1", "3300K", 7,		{ 1.015625, 1, 1.851562, 0 } },

  { "OLYMPUS", "E-10", Incandescent, 0,		{ 1, 1.153153, 3.441442, 0 } }, /*3000K*/
  { "OLYMPUS", "E-10", IncandescentWarm, 0,	{ 1.101562, 1, 2.351562, 0 } }, /*3700K*/
  { "OLYMPUS", "E-10", WhiteFluorescent, 0,	{ 1.460938, 1, 2.546875, 0 } }, /*4000K*/
  { "OLYMPUS", "E-10", DaylightFluorescent, 0,	{ 1.460938, 1, 1.843750, 0 } }, /*4500K*/
  { "OLYMPUS", "E-10", Daylight, 0,		{ 1.523438, 1, 1.617188, 0 } }, /*5500K*/
  { "OLYMPUS", "E-10", Cloudy, 0,		{ 1.687500, 1, 1.437500, 0 } }, /*6500K*/
  { "OLYMPUS", "E-10", Shade, 0,		{ 1.812500, 1, 1.312500, 0 } }, /*7500K*/

  /* Firmware version 1.4 */
  { "OLYMPUS", "E-3", Daylight, 0,		{ 1.7812, 1, 1.4688, 0 } },
  { "OLYMPUS", "E-3", Shade, 0,			{ 2.1406, 1, 1.1484, 0 } },
  { "OLYMPUS", "E-3", Cloudy, 0,		{ 1.9531, 1, 1.3359, 0 } },
  { "OLYMPUS", "E-3", Incandescent, 0,		{ 1.0312, 1, 2.8125, 0 } },
  { "OLYMPUS", "E-3", WhiteFluorescent, 0,	{ 1.6328, 1, 2.1953, 0 } },
  { "OLYMPUS", "E-3", NeutralFluorescent, 0,	{ 1.6641, 1, 1.7734, 0 } },
  { "OLYMPUS", "E-3", DaylightFluorescent, 0,	{ 1.9531, 1, 1.4844, 0 } },
  { "OLYMPUS", "E-3", Flash, 0,			{ 1.9609, 1, 1.3359, 0 } },
  { "OLYMPUS", "E-3", "2700K", 0,		{ 1, 1.0240, 3.0080, 0 } },
  { "OLYMPUS", "E-3", "3000K", 0,		{ 1.1328, 1, 2.5156, 0 } },
  { "OLYMPUS", "E-3", "3300K", 0,		{ 1.2734, 1, 2.1641, 0 } },
  { "OLYMPUS", "E-3", "5000K", 0,		{ 1.7266, 1, 1.5156, 0 } },

  { "OLYMPUS", "E-5", Daylight, 0,		{ 1.7031, 1, 1.3672, 0 } },
  { "OLYMPUS", "E-5", Shade, 0,			{ 2.0547, 1, 1.0625, 0 } },
  { "OLYMPUS", "E-5", Cloudy, 0,		{ 1.8750, 1, 1.2344, 0 } },
  { "OLYMPUS", "E-5", Incandescent, 0,		{ 1, 1.0079, 2.5118, 0 } },
  { "OLYMPUS", "E-5", Fluorescent, 0,		{ 1.6172, 1, 1.9687, 0 } },
  { "OLYMPUS", "E-5", Underwater, 0,		{ 1.6406, 1, 1.4375, 0 } },
  { "OLYMPUS", "E-5", Flash, 0,			{ 1.8750, 1, 1.2266, 0 } },

  { "OLYMPUS", "E-30", Daylight, -7,		{ 1.554688, 1, 1.515625, 0 } },
  { "OLYMPUS", "E-30", Daylight, 0,		{ 1.812500, 1, 1.335937, 0 } },
  { "OLYMPUS", "E-30", Daylight, 7,		{ 2.062500, 1, 1.148437, 0 } },
  { "OLYMPUS", "E-30", Shade, -7,		{ 1.867188, 1, 1.171875, 0 } },
  { "OLYMPUS", "E-30", Shade, 0,		{ 2.179688, 1, 1.031250, 0 } },
  { "OLYMPUS", "E-30", Shade, 7,		{ 2.814159, 1.132743, 1, 0 } },
  { "OLYMPUS", "E-30", Cloudy, -7,		{ 1.710938, 1, 1.359375, 0 } },
  { "OLYMPUS", "E-30", Cloudy, 0,		{ 1.992187, 1, 1.195312, 0 } },
  { "OLYMPUS", "E-30", Cloudy, 7,		{ 2.265625, 1, 1.023438, 0 } },
  { "OLYMPUS", "E-30", Incandescent, -7,	{ 1, 1.103448, 3.137931, 0 } },
  { "OLYMPUS", "E-30", Incandescent, 0,		{ 1.054687, 1, 2.500000, 0 } },
  { "OLYMPUS", "E-30", Incandescent, 7,		{ 1.195313, 1, 2.148437, 0 } },
  { "OLYMPUS", "E-30", WhiteFluorescent, -7,	{ 1.453125, 1, 2.187500, 0 } },
  { "OLYMPUS", "E-30", WhiteFluorescent, 0,	{ 1.695313, 1, 1.921875, 0 } },
  { "OLYMPUS", "E-30", WhiteFluorescent, 7,	{ 1.929687, 1, 1.648437, 0 } },
  { "OLYMPUS", "E-30", NeutralFluorescent, -7,	{ 1.437500, 1, 1.929687, 0 } },
  { "OLYMPUS", "E-30", NeutralFluorescent, 0,	{ 1.679687, 1, 1.695313, 0 } },
  { "OLYMPUS", "E-30", NeutralFluorescent, 7,	{ 1.914063, 1, 1.453125, 0 } },
  { "OLYMPUS", "E-30", DaylightFluorescent, -7,	{ 1.765625, 1, 1.500000, 0 } },
  { "OLYMPUS", "E-30", DaylightFluorescent, 0,	{ 2.054688, 1, 1.320313, 0 } },
  { "OLYMPUS", "E-30", DaylightFluorescent, 7,	{ 2.335938, 1, 1.132812, 0 } },
  { "OLYMPUS", "E-30", Flash, -7,		{ 1.710938, 1, 1.359375, 0 } },
  { "OLYMPUS", "E-30", Flash, 0,		{ 1.992187, 1, 1.195312, 0 } },
  { "OLYMPUS", "E-30", Flash, 7,		{ 2.265625, 1, 1.023438, 0 } },
  { "OLYMPUS", "E-30", "2700K",  0,		{ 1.000000, 1, 2.632813, 0 } },
  { "OLYMPUS", "E-30", "3000K",  0,		{ 1.164062, 1, 2.250000, 0 } },
  { "OLYMPUS", "E-30", "3300K",  0,		{ 1.304687, 1, 1.929687, 0 } },
  { "OLYMPUS", "E-30", "5000K",  0,		{ 1.742187, 1, 1.406250, 0 } },

  { "OLYMPUS", "E-300", Incandescent, -7,	{ 1.179688, 1, 2.125000, 0 } },
  { "OLYMPUS", "E-300", Incandescent, 0,	{ 1.140625, 1, 2.203125, 0 } },
  { "OLYMPUS", "E-300", Incandescent, 7,	{ 1.093750, 1, 2.273438, 0 } },
  { "OLYMPUS", "E-300", IncandescentWarm, -7,	{ 1.382812, 1, 1.859375, 0 } },
  { "OLYMPUS", "E-300", IncandescentWarm, 0,	{ 1.312500, 1, 1.906250, 0 } },
  { "OLYMPUS", "E-300", IncandescentWarm, 7,	{ 1.257812, 1, 1.984375, 0 } },
  { "OLYMPUS", "E-300", WhiteFluorescent, -7,	{ 2.109375, 1, 1.710938, 0 } },
  { "OLYMPUS", "E-300", WhiteFluorescent, 0,	{ 1.976562, 1, 1.921875, 0 } },
  { "OLYMPUS", "E-300", WhiteFluorescent, 7,	{ 1.804688, 1, 2.062500, 0 } },
  { "OLYMPUS", "E-300", NeutralFluorescent, -7,	{ 1.945312, 1, 1.445312, 0 } },
  { "OLYMPUS", "E-300", NeutralFluorescent, 0,	{ 1.820312, 1, 1.562500, 0 } },
  { "OLYMPUS", "E-300", NeutralFluorescent, 7,	{ 1.585938, 1, 1.945312, 0 } },
  { "OLYMPUS", "E-300", DaylightFluorescent, -7, { 2.203125, 1, 1.000000, 0 } },
  { "OLYMPUS", "E-300", DaylightFluorescent, 0,	{ 2.031250, 1, 1.328125, 0 } },
  { "OLYMPUS", "E-300", DaylightFluorescent, 7,	{ 1.765625, 1, 1.367188, 0 } },
  { "OLYMPUS", "E-300", Daylight, -7,		{ 1.835938, 1, 1.304688, 0 } },
  { "OLYMPUS", "E-300", Daylight, 0,		{ 1.789062, 1, 1.351562, 0 } },
  { "OLYMPUS", "E-300", Daylight, 7,		{ 1.726562, 1, 1.398438, 0 } },
  { "OLYMPUS", "E-300", Cloudy, -7,		{ 2.000000, 1, 1.156250, 0 } },
  { "OLYMPUS", "E-300", Cloudy, 0,		{ 1.890625, 1, 1.257812, 0 } },
  { "OLYMPUS", "E-300", Cloudy, 7,		{ 1.835938, 1, 1.304688, 0 } },
  { "OLYMPUS", "E-300", Shade, -7,		{ 2.179688, 1, 1.007812, 0 } },
  { "OLYMPUS", "E-300", Shade, 0,		{ 2.070312, 1, 1.109375, 0 } },
  { "OLYMPUS", "E-300", Shade, 7,		{ 1.945312, 1, 1.210938, 0 } },

  { "OLYMPUS", "E-330", Daylight, 0,		{ 1.812500, 1, 1.296875, 0 } }, /*5300K*/
  { "OLYMPUS", "E-330", Cloudy, 0,		{ 1.953125, 1, 1.195312, 0 } }, /*6000K*/
  { "OLYMPUS", "E-330", Shade, 0,		{ 2.187500, 1, 1.054688, 0 } }, /*7500K*/
  { "OLYMPUS", "E-330", Incandescent, 0,	{ 1.039062, 1, 2.437500, 0 } }, /*3000K*/
  { "OLYMPUS", "E-330", WhiteFluorescent, 0,	{ 1.710938, 1, 1.906250, 0 } }, /*4000K*/
  { "OLYMPUS", "E-330", NeutralFluorescent, 0,	{ 1.750000, 1, 1.531250, 0 } }, /*4500K*/
  { "OLYMPUS", "E-330", DaylightFluorescent, 0,	{ 2.062500, 1, 1.289062, 0 } }, /*6600K*/

  { "OLYMPUS", "E-400", Daylight, -7,		{ 2.554687, 1, 1.390625, 0 } },
  { "OLYMPUS", "E-400", Daylight, 0,		{ 2.312500, 1, 1.179687, 0 } },
  { "OLYMPUS", "E-400", Daylight, 7,		{ 2.096774, 1.032258, 1, 0 } },
  { "OLYMPUS", "E-400", Cloudy, -7,		{ 2.695312, 1, 1.289062, 0 } },
  { "OLYMPUS", "E-400", Cloudy, 0,		{ 2.437500, 1, 1.093750, 0 } },
  { "OLYMPUS", "E-400", Cloudy, 7,		{ 2.554545, 1.163636, 1, 0 } },
  { "OLYMPUS", "E-400", Shade, -7,		{ 2.835937, 1, 1.187500, 0 } },
  { "OLYMPUS", "E-400", Shade, 0,		{ 2.754098, 1.049180, 1, 0 } },
  { "OLYMPUS", "E-400", Shade, 7,		{ 3.202128, 1.361702, 1, 0 } },
  { "OLYMPUS", "E-400", Incandescent, -7,	{ 1.500000, 1, 2.710938, 0 } },
  { "OLYMPUS", "E-400", Incandescent, 0,	{ 1.460937, 1, 2.171875, 0 } },
  { "OLYMPUS", "E-400", Incandescent, 7,	{ 1.367187, 1, 1.679688, 0 } },
  { "OLYMPUS", "E-400", WhiteFluorescent, -7,	{ 2.523438, 1, 2.250000, 0 } },
  { "OLYMPUS", "E-400", WhiteFluorescent, 0,	{ 2.390625, 1, 1.796875, 0 } },
  { "OLYMPUS", "E-400", WhiteFluorescent, 7,	{ 2.164063, 1, 1.429688, 0 } },
  { "OLYMPUS", "E-400", NeutralFluorescent, -7,	{ 2.226562, 1, 1.828125, 0 } },
  { "OLYMPUS", "E-400", NeutralFluorescent, 0,	{ 2.132812, 1, 1.468750, 0 } },
  { "OLYMPUS", "E-400", NeutralFluorescent, 7,	{ 1.953125, 1, 1.156250, 0 } },
  { "OLYMPUS", "E-400", DaylightFluorescent, -7, { 2.593750, 1, 1.359375, 0 } },
  { "OLYMPUS", "E-400", DaylightFluorescent, 0,	{ 2.445313, 1, 1.195313, 0 } },
  { "OLYMPUS", "E-400", DaylightFluorescent, 7,	{ 3.293478, 1.391304, 1, 0 } },

  { "OLYMPUS", "E-410", Daylight, 0,		{ 1.914063, 1, 1.367188, 0 } }, /*5300K*/
  { "OLYMPUS", "E-410", Cloudy, 0,		{ 2.054688, 1, 1.250000, 0 } }, /*6000K*/
  { "OLYMPUS", "E-410", Shade, 0,		{ 2.304688, 1, 1.031250, 0 } }, /*7500K*/
  { "OLYMPUS", "E-410", Incandescent, 0,	{ 1.062500, 1, 2.781250, 0 } }, /*3000K*/
  { "OLYMPUS", "E-410", WhiteFluorescent, 0,	{ 1.726562, 1, 2.226562, 0 } }, /*4000K*/
  { "OLYMPUS", "E-410", NeutralFluorescent, 0,	{ 1.703125, 1, 1.796875, 0 } }, /*4500K*/
  { "OLYMPUS", "E-410", DaylightFluorescent, 0,	{ 2.039063, 1, 1.476562, 0 } }, /*6600K*/

  { "OLYMPUS", "E-420", Daylight, 0,		{ 1.820313, 1, 1.437500, 0 } },
  { "OLYMPUS", "E-420", Shade, 0,		{ 2.179688, 1, 1.140625, 0 } },
  { "OLYMPUS", "E-420", Cloudy, 0,		{ 2.000000, 1, 1.289062, 0 } },
  { "OLYMPUS", "E-420", Incandescent, 0,	{ 1.039062, 1, 2.726562, 0 } },
  { "OLYMPUS", "E-420", WhiteFluorescent, 0,	{ 1.703125, 1, 2.109375, 0 } },
  { "OLYMPUS", "E-420", NeutralFluorescent, 0,	{ 1.703125, 1, 1.757812, 0 } },
  { "OLYMPUS", "E-420", Flash, 0,		{ 2.078125, 1, 1.375000, 0 } },

  { "OLYMPUS", "E-500", Daylight, 0,		{ 1.898438, 1, 1.359375, 0 } }, /*5300K*/
  { "OLYMPUS", "E-500", Cloudy, 0,		{ 1.992188, 1, 1.265625, 0 } }, /*6000K*/
  { "OLYMPUS", "E-500", Shade, 0,		{ 2.148438, 1, 1.125000, 0 } }, /*7500K*/
  { "OLYMPUS", "E-500", Incandescent, 0,	{ 1.265625, 1, 2.195312, 0 } }, /*3000K*/
  { "OLYMPUS", "E-500", WhiteFluorescent, 0,	{ 1.976562, 1, 1.914062, 0 } }, /*4000K*/
  { "OLYMPUS", "E-500", NeutralFluorescent, 0,	{ 1.828125, 1, 1.562500, 0 } }, /*4500K*/
  { "OLYMPUS", "E-500", DaylightFluorescent, 0,	{ 2.046875, 1, 1.359375, 0 } }, /*6600K*/

  { "OLYMPUS", "E-510", Daylight, -7,		{ 2.164063, 1, 1.546875, 0 } },
  { "OLYMPUS", "E-510", Daylight, 0,		{ 1.968750, 1, 1.296875, 0 } },
  { "OLYMPUS", "E-510", Daylight, 7,		{ 1.742187, 1, 1.062500, 0 } },
  { "OLYMPUS", "E-510", Shade, -7,		{ 2.492188, 1, 1.273438, 0 } },
  { "OLYMPUS", "E-510", Shade, 0,		{ 2.439024, 1.040650, 1, 0 } },
  { "OLYMPUS", "E-510", Shade, 7,		{ 3.055556, 1.422222, 1, 0 } },
  { "OLYMPUS", "E-510", Cloudy, -7,		{ 2.312500, 1, 1.414062, 0 } },
  { "OLYMPUS", "E-510", Cloudy, 0,		{ 2.109375, 1, 1.187500, 0 } },
  { "OLYMPUS", "E-510", Cloudy, 7,		{ 2.192982, 1.122807, 1, 0 } },
  { "OLYMPUS", "E-510", Incandescent, -7,	{ 1.109375, 1, 3.351562, 0 } },
  { "OLYMPUS", "E-510", Incandescent, 0,	{ 1.093750, 1, 2.671875, 0 } },
  { "OLYMPUS", "E-510", Incandescent, 7,	{ 1.031250, 1, 2.054688, 0 } },
  { "OLYMPUS", "E-510", WhiteFluorescent, -7,	{ 1.578125, 1, 2.250000, 0 } },
  { "OLYMPUS", "E-510", WhiteFluorescent, 0,	{ 1.718750, 1, 2.109375, 0 } },
  { "OLYMPUS", "E-510", WhiteFluorescent, 7,	{ 1.523437, 1, 1.265625, 0 } },
  { "OLYMPUS", "E-510", NeutralFluorescent, -7,	{ 1.835938, 1, 1.828125, 0 } },
  { "OLYMPUS", "E-510", NeutralFluorescent, 0,	{ 1.687500, 1, 1.710938, 0 } },
  { "OLYMPUS", "E-510", NeutralFluorescent, 7,	{ 1.726562, 1, 1.078125, 0 } },
  { "OLYMPUS", "E-510", DaylightFluorescent, -7, { 2.203125, 1, 1.500000, 0 } },
  { "OLYMPUS", "E-510", DaylightFluorescent, 0,	{ 2.023438, 1, 1.398437, 0 } },
  { "OLYMPUS", "E-510", DaylightFluorescent, 7,	{ 3.193182, 1.454545, 1, 0 } },

  { "OLYMPUS", "E-520", Daylight, 0,		{ 1.859375, 1, 1.445312, 0 } },
  { "OLYMPUS", "E-520", Shade, 0,		{ 2.234375, 1, 1.140625, 0 } },
  { "OLYMPUS", "E-520", Cloudy, 0,		{ 2.046875, 1, 1.296875, 0 } },
  { "OLYMPUS", "E-520", Tungsten, 0,		{ 1.062500, 1, 2.687500, 0 } },
  { "OLYMPUS", "E-520", WhiteFluorescent, 0,	{ 1.703125, 1, 2.109375, 0 } },
  { "OLYMPUS", "E-520", NeutralFluorescent, 0,	{ 1.718750, 1, 1.765625, 0 } },
  { "OLYMPUS", "E-520", DaylightFluorescent, 0,	{ 2.101563, 1, 1.375000, 0 } },
  { "OLYMPUS", "E-520", Flash, 0,		{ 2.039063, 1, 1.296875, 0 } },
  { "OLYMPUS", "E-520", "2700K", 0,		{ 1.007812, 1, 2.828125, 0 } },
  { "OLYMPUS", "E-520", "3000K", 0,		{ 1.171875, 1, 2.421875, 0 } },
  { "OLYMPUS", "E-520", "3300K", 0,		{ 1.312500, 1, 2.078125, 0 } },
  { "OLYMPUS", "E-520", "5000K", 0,		{ 1.789062, 1, 1.515625, 0 } },

  /* -7/+7 fine tuning is -7/+7 in both amber-blue and green-magenta */
  { "OLYMPUS", "E-600", Daylight, -7,		{ 1.804688, 1, 1.671875, 0 } },
  { "OLYMPUS", "E-600", Daylight, 0,		{ 1.851563, 1, 1.289063, 0 } },
  { "OLYMPUS", "E-600", Daylight, 7,		{ 1.917355, 1.057851, 1, 0 } },
  { "OLYMPUS", "E-600", Shade, -7,		{ 2.179688, 1, 1.281250, 0 } },
  { "OLYMPUS", "E-600", Shade, 0,		{ 2.244094, 1.007874, 1, 0 } },
  { "OLYMPUS", "E-600", Shade, 7,		{ 2.989247, 1.376344, 1, 0 } },
  { "OLYMPUS", "E-600", Cloudy, -7,		{ 2.000000, 1, 1.500000, 0 } },
  { "OLYMPUS", "E-600", Cloudy, 0,		{ 2.046875, 1, 1.164062, 0 } },
  { "OLYMPUS", "E-600", Cloudy, 7,		{ 2.327273, 1.163636, 1, 0 } },
  { "OLYMPUS", "E-600", Incandescent, -7,	{ 1.062500, 1, 3.156250, 0 } },
  { "OLYMPUS", "E-600", Incandescent, 0,	{ 1.093750, 1, 2.437500, 0 } },
  { "OLYMPUS", "E-600", Incandescent, 7,	{ 1.062500, 1, 1.796875, 0 } },
  { "OLYMPUS", "E-600", WhiteFluorescent, -7,	{ 1.703125, 1, 2.398438, 0 } },
  { "OLYMPUS", "E-600", WhiteFluorescent, 0,	{ 1.750000, 1, 1.851563, 0 } },
  { "OLYMPUS", "E-600", WhiteFluorescent, 7,	{ 1.710938, 1, 1.359375, 0 } },
  { "OLYMPUS", "E-600", NeutralFluorescent, -7,	{ 1.671875, 1, 2.109375, 0 } },
  { "OLYMPUS", "E-600", NeutralFluorescent, 0,	{ 1.710938, 1, 1.625000, 0 } },
  { "OLYMPUS", "E-600", NeutralFluorescent, 7,	{ 1.671875, 1, 1.195312, 0 } },
  { "OLYMPUS", "E-600", DaylightFluorescent, -7, { 2.039063, 1, 1.632813, 0 } },
  { "OLYMPUS", "E-600", DaylightFluorescent, 0,	{ 2.085937, 1, 1.265625, 0 } },
  { "OLYMPUS", "E-600", DaylightFluorescent, 7,	{ 2.193277, 1.075630, 1, 0 } },
  { "OLYMPUS", "E-600", Flash, -7,		{ 1.992187, 1, 1.492187, 0 } },
  { "OLYMPUS", "E-600", Flash, 0,		{ 2.039063, 1, 1.156250, 0 } },
  { "OLYMPUS", "E-600", Flash, 7,		{ 2.339450, 1.174312, 1, 0 } },

  /* -7/+7 fine tuning is -7/+7 in both amber-blue and green-magenta */
  { "OLYMPUS", "E-620", Daylight, -7,		{ 1.804688, 1, 1.726563, 0 } },
  { "OLYMPUS", "E-620", Daylight, 0,		{ 1.851563, 1, 1.335938, 0 } },
  { "OLYMPUS", "E-620", Daylight, 7,		{ 1.841270, 1.015873, 1, 0 } },
  { "OLYMPUS", "E-620", Shade, -7,		{ 2.171875, 1, 1.320312, 0 } },
  { "OLYMPUS", "E-620", Shade, 0,		{ 2.218750, 1, 1.023438, 0 } },
  { "OLYMPUS", "E-620", Shade, 7,		{ 2.885417, 1.333333, 1, 0 } },
  { "OLYMPUS", "E-620", Cloudy, -7,		{ 1.992187, 1, 1.539062, 0 } },
  { "OLYMPUS", "E-620", Cloudy, 0,		{ 2.039063, 1, 1.187500, 0 } },
  { "OLYMPUS", "E-620", Cloudy, 7,		{ 2.297297, 1.153153, 1, 0 } },
  { "OLYMPUS", "E-620", Incandescent, -7,	{ 1.070312, 1, 3.281250, 0 } },
  { "OLYMPUS", "E-620", Incandescent, 0,	{ 1.101563, 1, 2.531250, 0 } },
  { "OLYMPUS", "E-620", Incandescent, 7,	{ 1.070313, 1, 1.867188, 0 } },
  { "OLYMPUS", "E-620", WhiteFluorescent, -7,	{ 1.679687, 1, 2.500000, 0 } },
  { "OLYMPUS", "E-620", WhiteFluorescent, 0,	{ 1.718750, 1, 1.929687, 0 } },
  { "OLYMPUS", "E-620", WhiteFluorescent, 7,	{ 1.679688, 1, 1.421875, 0 } },
  { "OLYMPUS", "E-620", NeutralFluorescent, -7,	{ 1.632813, 1, 2.179688, 0 } },
  { "OLYMPUS", "E-620", NeutralFluorescent, 0,	{ 1.671875, 1, 1.679688, 0 } },
  { "OLYMPUS", "E-620", NeutralFluorescent, 7,	{ 1.625000, 1, 1.234375, 0 } },
  { "OLYMPUS", "E-620", DaylightFluorescent, -7, { 2.000000, 1, 1.687500, 0 } },
  { "OLYMPUS", "E-620", DaylightFluorescent, 0,	{ 2.046875, 1, 1.304687, 0 } },
  { "OLYMPUS", "E-620", DaylightFluorescent, 7,	{ 2.098361, 1.049180, 1, 0 } },
  { "OLYMPUS", "E-620", Flash, -7,		{ 1.992187, 1, 1.546875, 0 } },
  { "OLYMPUS", "E-620", Flash, 0,		{ 2.039063, 1, 1.195313, 0 } },
  { "OLYMPUS", "E-620", Flash, 7,		{ 2.276786, 1.142857, 1, 0 } },

  { "OLYMPUS", "E-P1", Daylight, 0,		{ 1.835938, 1, 1.351563, 0 } },
  { "OLYMPUS", "E-P1", Shade, 0,		{ 2.195313, 1, 1.046875, 0 } },
  { "OLYMPUS", "E-P1", Cloudy, 0,		{ 2.031250, 1, 1.203125, 0 } },
  { "OLYMPUS", "E-P1", Incandescent, 0,		{ 1.078125, 1, 2.570312, 0 } },
  { "OLYMPUS", "E-P1", WhiteFluorescent, 0,	{ 1.695313, 1, 1.937500, 0 } },
  { "OLYMPUS", "E-P1", NeutralFluorescent, 0,	{ 1.687500, 1, 1.703125, 0 } },
  { "OLYMPUS", "E-P1", DaylightFluorescent, 0,	{ 2.070312, 1, 1.312500, 0 } },

  { "OLYMPUS", "E-P2", Daylight, -7,		{ 1.789063, 1, 1.789063, 0 } },
  { "OLYMPUS", "E-P2", Daylight, -6,		{ 1.789063, 1, 1.726563, 0 } },
  { "OLYMPUS", "E-P2", Daylight, -5,		{ 1.804688, 1, 1.664063, 0 } },
  { "OLYMPUS", "E-P2", Daylight, -4,		{ 1.812500, 1, 1.609375, 0 } },
  { "OLYMPUS", "E-P2", Daylight, -3,		{ 1.812500, 1, 1.546875, 0 } },
  { "OLYMPUS", "E-P2", Daylight, -2,		{ 1.812500, 1, 1.492188, 0 } },
  { "OLYMPUS", "E-P2", Daylight, -1,		{ 1.820313, 1, 1.429688, 0 } },
  { "OLYMPUS", "E-P2", Daylight, 0,		{ 1.828125, 1, 1.382813, 0 } },
  { "OLYMPUS", "E-P2", Daylight, 1,		{ 1.820313, 1, 1.320313, 0 } },
  { "OLYMPUS", "E-P2", Daylight, 2,		{ 1.820313, 1, 1.265625, 0 } },
  { "OLYMPUS", "E-P2", Daylight, 3,		{ 1.820313, 1, 1.218750, 0 } },
  { "OLYMPUS", "E-P2", Daylight, 4,		{ 1.804688, 1, 1.164063, 0 } },
  { "OLYMPUS", "E-P2", Daylight, 5,		{ 1.804688, 1, 1.117188, 0 } },
  { "OLYMPUS", "E-P2", Daylight, 6,		{ 1.796875, 1, 1.062500, 0 } },
  { "OLYMPUS", "E-P2", Daylight, 7,		{ 1.781250, 1, 1.015625, 0 } },
  { "OLYMPUS", "E-P2", Shade, -7,		{ 2.125000, 1, 1.382813, 0 } },
  { "OLYMPUS", "E-P2", Shade, -6,		{ 2.132813, 1, 1.335938, 0 } },
  { "OLYMPUS", "E-P2", Shade, -5,		{ 2.148438, 1, 1.289063, 0 } },
  { "OLYMPUS", "E-P2", Shade, -4,		{ 2.148438, 1, 1.234375, 0 } },
  { "OLYMPUS", "E-P2", Shade, -3,		{ 2.156250, 1, 1.195313, 0 } },
  { "OLYMPUS", "E-P2", Shade, -2,		{ 2.156250, 1, 1.148438, 0 } },
  { "OLYMPUS", "E-P2", Shade, -1,		{ 2.164063, 1, 1.101563, 0 } },
  { "OLYMPUS", "E-P2", Shade, 0,		{ 2.171875, 1, 1.070313, 0 } },
  { "OLYMPUS", "E-P2", Shade, 1,		{ 2.164063, 1, 1.023438, 0 } },
  { "OLYMPUS", "E-P2", Shade, 2,		{ 2.164063, 1, 0.976563, 0 } },
  { "OLYMPUS", "E-P2", Shade, 3,		{ 2.156250, 1, 0.937500, 0 } },
  { "OLYMPUS", "E-P2", Shade, 4,		{ 2.156250, 1, 0.898438, 0 } },
  { "OLYMPUS", "E-P2", Shade, 5,		{ 2.140625, 1, 0.859375, 0 } },
  { "OLYMPUS", "E-P2", Shade, 6,		{ 2.132813, 1, 0.820313, 0 } },
  { "OLYMPUS", "E-P2", Shade, 7,		{ 2.117188, 1, 0.781250, 0 } },
  { "OLYMPUS", "E-P2", Cloudy, -7,		{ 1.953125, 1, 1.617188, 0 } },
  { "OLYMPUS", "E-P2", Cloudy, -6,		{ 1.968750, 1, 1.562500, 0 } },
  { "OLYMPUS", "E-P2", Cloudy, -5,		{ 1.976563, 1, 1.507813, 0 } },
  { "OLYMPUS", "E-P2", Cloudy, -4,		{ 1.976563, 1, 1.445313, 0 } },
  { "OLYMPUS", "E-P2", Cloudy, -3,		{ 1.984375, 1, 1.398438, 0 } },
  { "OLYMPUS", "E-P2", Cloudy, -2,		{ 1.984375, 1, 1.343750, 0 } },
  { "OLYMPUS", "E-P2", Cloudy, -1,		{ 1.992188, 1, 1.296875, 0 } },
  { "OLYMPUS", "E-P2", Cloudy, 0,		{ 2.000000, 1, 1.250000, 0 } },
  { "OLYMPUS", "E-P2", Cloudy, 1,		{ 1.992188, 1, 1.187500, 0 } },
  { "OLYMPUS", "E-P2", Cloudy, 2,		{ 1.992188, 1, 1.140625, 0 } },
  { "OLYMPUS", "E-P2", Cloudy, 3,		{ 1.984375, 1, 1.101563, 0 } },
  { "OLYMPUS", "E-P2", Cloudy, 4,		{ 1.976563, 1, 1.054688, 0 } },
  { "OLYMPUS", "E-P2", Cloudy, 5,		{ 1.968750, 1, 1.007813, 0 } },
  { "OLYMPUS", "E-P2", Cloudy, 6,		{ 1.960938, 1, 0.960938, 0 } },
  { "OLYMPUS", "E-P2", Cloudy, 7,		{ 1.953125, 1, 0.914063, 0 } },
  { "OLYMPUS", "E-P2", Incandescent, -7,	{ 1.039063, 1, 3.445313, 0 } },
  { "OLYMPUS", "E-P2", Incandescent, -6,	{ 1.046875, 1, 3.320313, 0 } },
  { "OLYMPUS", "E-P2", Incandescent, -5,	{ 1.054688, 1, 3.210938, 0 } },
  { "OLYMPUS", "E-P2", Incandescent, -4,	{ 1.062500, 1, 3.093750, 0 } },
  { "OLYMPUS", "E-P2", Incandescent, -3,	{ 1.054688, 1, 2.976563, 0 } },
  { "OLYMPUS", "E-P2", Incandescent, -2,	{ 1.062500, 1, 2.867188, 0 } },
  { "OLYMPUS", "E-P2", Incandescent, -1,	{ 1.062500, 1, 2.750000, 0 } },
  { "OLYMPUS", "E-P2", Incandescent, 0,		{ 1.070313, 1, 2.656250, 0 } },
  { "OLYMPUS", "E-P2", Incandescent, 1,		{ 1.062500, 1, 2.546875, 0 } },
  { "OLYMPUS", "E-P2", Incandescent, 2,		{ 1.062500, 1, 2.437500, 0 } },
  { "OLYMPUS", "E-P2", Incandescent, 3,		{ 1.062500, 1, 2.335938, 0 } },
  { "OLYMPUS", "E-P2", Incandescent, 4,		{ 1.054688, 1, 2.242188, 0 } },
  { "OLYMPUS", "E-P2", Incandescent, 5,		{ 1.054688, 1, 2.148438, 0 } },
  { "OLYMPUS", "E-P2", Incandescent, 6,		{ 1.046875, 1, 2.054688, 0 } },
  { "OLYMPUS", "E-P2", Incandescent, 7,		{ 1.046875, 1, 1.960938, 0 } },
  { "OLYMPUS", "E-P2", WhiteFluorescent, -7,	{ 1.742188, 1, 2.617188, 0 } },
  { "OLYMPUS", "E-P2", WhiteFluorescent, -6,	{ 1.750000, 1, 2.515625, 0 } },
  { "OLYMPUS", "E-P2", WhiteFluorescent, -5,	{ 1.757813, 1, 2.429688, 0 } },
  { "OLYMPUS", "E-P2", WhiteFluorescent, -4,	{ 1.757813, 1, 2.343750, 0 } },
  { "OLYMPUS", "E-P2", WhiteFluorescent, -3,	{ 1.765625, 1, 2.257813, 0 } },
  { "OLYMPUS", "E-P2", WhiteFluorescent, -2,	{ 1.765625, 1, 2.171875, 0 } },
  { "OLYMPUS", "E-P2", WhiteFluorescent, -1,	{ 1.773438, 1, 2.093750, 0 } },
  { "OLYMPUS", "E-P2", WhiteFluorescent, 0,	{ 1.781250, 1, 2.015625, 0 } },
  { "OLYMPUS", "E-P2", WhiteFluorescent, 1,	{ 1.773438, 1, 1.921875, 0 } },
  { "OLYMPUS", "E-P2", WhiteFluorescent, 2,	{ 1.773438, 1, 1.851563, 0 } },
  { "OLYMPUS", "E-P2", WhiteFluorescent, 3,	{ 1.765625, 1, 1.773438, 0 } },
  { "OLYMPUS", "E-P2", WhiteFluorescent, 4,	{ 1.765625, 1, 1.703125, 0 } },
  { "OLYMPUS", "E-P2", WhiteFluorescent, 5,	{ 1.757813, 1, 1.625000, 0 } },
  { "OLYMPUS", "E-P2", WhiteFluorescent, 6,	{ 1.750000, 1, 1.554688, 0 } },
  { "OLYMPUS", "E-P2", WhiteFluorescent, 7,	{ 1.734375, 1, 1.484375, 0 } },
  { "OLYMPUS", "E-P2", NeutralFluorescent, -7,	{ 1.695313, 1, 2.328125, 0 } },
  { "OLYMPUS", "E-P2", NeutralFluorescent, -6,	{ 1.710938, 1, 2.242188, 0 } },
  { "OLYMPUS", "E-P2", NeutralFluorescent, -5,	{ 1.718750, 1, 2.171875, 0 } },
  { "OLYMPUS", "E-P2", NeutralFluorescent, -4,	{ 1.726563, 1, 2.085938, 0 } },
  { "OLYMPUS", "E-P2", NeutralFluorescent, -3,	{ 1.726563, 1, 2.007813, 0 } },
  { "OLYMPUS", "E-P2", NeutralFluorescent, -2,	{ 1.734375, 1, 1.937500, 0 } },
  { "OLYMPUS", "E-P2", NeutralFluorescent, -1,	{ 1.734375, 1, 1.859375, 0 } },
  { "OLYMPUS", "E-P2", NeutralFluorescent, 0,	{ 1.742188, 1, 1.796875, 0 } },
  { "OLYMPUS", "E-P2", NeutralFluorescent, 1,	{ 1.734375, 1, 1.718750, 0 } },
  { "OLYMPUS", "E-P2", NeutralFluorescent, 2,	{ 1.726563, 1, 1.648438, 0 } },
  { "OLYMPUS", "E-P2", NeutralFluorescent, 3,	{ 1.726563, 1, 1.585938, 0 } },
  { "OLYMPUS", "E-P2", NeutralFluorescent, 4,	{ 1.718750, 1, 1.515625, 0 } },
  { "OLYMPUS", "E-P2", NeutralFluorescent, 5,	{ 1.718750, 1, 1.453125, 0 } },
  { "OLYMPUS", "E-P2", NeutralFluorescent, 6,	{ 1.710938, 1, 1.382813, 0 } },
  { "OLYMPUS", "E-P2", NeutralFluorescent, 7,	{ 1.703125, 1, 1.320313, 0 } },
  { "OLYMPUS", "E-P2", DaylightFluorescent, -7,	{ 2.078125, 1, 1.765625, 0 } },
  { "OLYMPUS", "E-P2", DaylightFluorescent, -6,	{ 2.093750, 1, 1.710938, 0 } },
  { "OLYMPUS", "E-P2", DaylightFluorescent, -5,	{ 2.101563, 1, 1.648438, 0 } },
  { "OLYMPUS", "E-P2", DaylightFluorescent, -4,	{ 2.117188, 1, 1.593750, 0 } },
  { "OLYMPUS", "E-P2", DaylightFluorescent, -3,	{ 2.117188, 1, 1.531250, 0 } },
  { "OLYMPUS", "E-P2", DaylightFluorescent, -2,	{ 2.125000, 1, 1.476563, 0 } },
  { "OLYMPUS", "E-P2", DaylightFluorescent, -1,	{ 2.125000, 1, 1.414063, 0 } },
  { "OLYMPUS", "E-P2", DaylightFluorescent, 0,	{ 2.132813, 1, 1.367188, 0 } },
  { "OLYMPUS", "E-P2", DaylightFluorescent, 1,	{ 2.125000, 1, 1.304688, 0 } },
  { "OLYMPUS", "E-P2", DaylightFluorescent, 2,	{ 2.117188, 1, 1.257813, 0 } },
  { "OLYMPUS", "E-P2", DaylightFluorescent, 3,	{ 2.117188, 1, 1.203125, 0 } },
  { "OLYMPUS", "E-P2", DaylightFluorescent, 4,	{ 2.109375, 1, 1.156250, 0 } },
  { "OLYMPUS", "E-P2", DaylightFluorescent, 5,	{ 2.109375, 1, 1.101563, 0 } },
  { "OLYMPUS", "E-P2", DaylightFluorescent, 6,	{ 2.093750, 1, 1.054688, 0 } },
  { "OLYMPUS", "E-P2", DaylightFluorescent, 7,	{ 2.085938, 1, 1.007813, 0 } },
  { "OLYMPUS", "E-P2", Flash, -7,		{ 1.960938, 1, 1.609375, 0 } },
  { "OLYMPUS", "E-P2", Flash, -6,		{ 1.976563, 1, 1.554688, 0 } },
  { "OLYMPUS", "E-P2", Flash, -5,		{ 1.984375, 1, 1.492188, 0 } },
  { "OLYMPUS", "E-P2", Flash, -4,		{ 1.984375, 1, 1.437500, 0 } },
  { "OLYMPUS", "E-P2", Flash, -3,		{ 1.992188, 1, 1.390625, 0 } },
  { "OLYMPUS", "E-P2", Flash, -2,		{ 1.992188, 1, 1.335938, 0 } },
  { "OLYMPUS", "E-P2", Flash, -1,		{ 2.000000, 1, 1.289063, 0 } },
  { "OLYMPUS", "E-P2", Flash, 0,		{ 2.007813, 1, 1.242188, 0 } },
  { "OLYMPUS", "E-P2", Flash, 1,		{ 2.000000, 1, 1.179688, 0 } },
  { "OLYMPUS", "E-P2", Flash, 2,		{ 2.000000, 1, 1.132813, 0 } },
  { "OLYMPUS", "E-P2", Flash, 3,		{ 1.992188, 1, 1.093750, 0 } },
  { "OLYMPUS", "E-P2", Flash, 4,		{ 1.984375, 1, 1.046875, 0 } },
  { "OLYMPUS", "E-P2", Flash, 5,		{ 1.976563, 1, 1.000000, 0 } },
  { "OLYMPUS", "E-P2", Flash, 6,		{ 1.968750, 1, 0.953125, 0 } },
  { "OLYMPUS", "E-P2", Flash, 7,		{ 1.960938, 1, 0.906250, 0 } },

  { "OLYMPUS", "E-P3", Daylight, 0,		{ 2.0469, 1, 1.4922, 0 } },
  { "OLYMPUS", "E-P3", Shade, 0,		{ 2.4375, 1, 1.1875, 0 } },
  { "OLYMPUS", "E-P3", Cloudy, 0,		{ 2.2188, 1, 1.3750, 0 } },
  { "OLYMPUS", "E-P3", Incandescent, 0,		{ 1.2266, 1, 2.5312, 0 } },
  { "OLYMPUS", "E-P3", Fluorescent, 0,		{ 1.9766, 1, 1.9766, 0 } },
  { "OLYMPUS", "E-P3", Flash, 0,		{ 2.2109, 1, 1.3672, 0 } },

  /* Firmware version 1.2 */
  { "OLYMPUS", "E-PL1", Daylight, 0,		{ 1.726562, 1, 1.343750, 0 } },
  { "OLYMPUS", "E-PL1", Shade, 0,		{ 2.085937, 1, 1.039062, 0 } },
  { "OLYMPUS", "E-PL1", Cloudy, 0,		{ 1.906250, 1, 1.210938, 0 } },
  { "OLYMPUS", "E-PL1", Incandescent, 0,	{ 1, 1.015873, 2.619048, 0 } },
  { "OLYMPUS", "E-PL1", WhiteFluorescent, 0,	{ 1.648437, 1, 1.960937, 0 } },
  { "OLYMPUS", "E-PL1", NeutralFluorescent, 0,	{ 1.609375, 1, 1.710938, 0 } },
  { "OLYMPUS", "E-PL1", DaylightFluorescent, 0,	{ 2.015625, 1, 1.328125, 0 } },
  { "OLYMPUS", "E-PL1", Flash, 0,		{ 1.898438, 1, 1.203125, 0 } },
  { "OLYMPUS", "E-PL1", "2700K", 0,		{ 1, 1.075630, 2.924370, 0 } },
  { "OLYMPUS", "E-PL1", "3000K", 0,		{ 1.085937, 1, 2.312500, 0 } },
  { "OLYMPUS", "E-PL1", "3300K", 0,		{ 1.218750, 1, 1.976562, 0 } },
  { "OLYMPUS", "E-PL1", "5000K", 0,		{ 1.656250, 1, 1.414062, 0 } },

  { "OLYMPUS", "E-PL2", Daylight, 0,		{ 1.4609, 1, 1.4219, 0 } },
  { "OLYMPUS", "E-PL2", Shade, 0,		{ 1.7422, 1, 1.1094, 0 } },
  { "OLYMPUS", "E-PL2", Cloudy, 0,		{ 1.6172, 1, 1.2891, 0 } },
  { "OLYMPUS", "E-PL2", Tungsten, 0,		{ 1, 1.1327, 2.9115, 0 } },
  { "OLYMPUS", "E-PL2", CoolWhiteFluorescent, 0, { 1.3828, 1, 2.0859, 0 } },
  { "OLYMPUS", "E-PL2", Flash, 0,		{ 1.6016, 1, 1.2891, 0 } },
  { "OLYMPUS", "E-PL2", "2700K", 0,		{ 1, 1.1852, 3.1852, 0 } },
  { "OLYMPUS", "E-PL2", "3000K", 0,		{ 1, 1.0492, 2.4672, 0 } },
  { "OLYMPUS", "E-PL2", "3300K", 0,		{ 1.0547, 1, 2.0625, 0 } },
  { "OLYMPUS", "E-PL2", "5000K", 0,		{ 1.4062, 1, 1.4922, 0 } },

  { "OLYMPUS", "SP500UZ", Daylight, -7,		{ 1.136719, 1, 2.359375, 0 } },
  { "OLYMPUS", "SP500UZ", Daylight, 0,		{ 1.960937, 1, 1.585937, 0 } },
  { "OLYMPUS", "SP500UZ", Daylight, 7,		{ 3.927660, 1.089362, 1, 0 } },
  { "OLYMPUS", "SP500UZ", Cloudy, -7,		{ 1.191406, 1, 2.210937, 0 } },
  { "OLYMPUS", "SP500UZ", Cloudy, 0,		{ 2.058594, 1, 1.484375, 0 } },
  { "OLYMPUS", "SP500UZ", Cloudy, 7,		{ 4.404545, 1.163636, 1, 0 } },
  { "OLYMPUS", "SP500UZ", EveningSun, -7,	{ 1.199219, 1, 2.214844, 0 } },
  { "OLYMPUS", "SP500UZ", EveningSun, 0,	{ 2.074219, 1, 1.488281, 0 } },
  { "OLYMPUS", "SP500UZ", EveningSun, 7,	{ 4.440909, 1.163636, 1, 0 } },
  { "OLYMPUS", "SP500UZ", Tungsten, -7,		{ 1, 1.590062, 6.490683, 0 } },
  { "OLYMPUS", "SP500UZ", Tungsten, 0,		{ 1.085937, 1, 2.742188, 0 } },
  { "OLYMPUS", "SP500UZ", Tungsten, 7,		{ 1.996094, 1, 1.589844, 0 } },
  { "OLYMPUS", "SP500UZ", Fluorescent, -7,	{ 1.324219, 1, 2.214844, 0 } },
  { "OLYMPUS", "SP500UZ", Fluorescent, 0,	{ 2.285156, 1, 1.488281, 0 } },
  { "OLYMPUS", "SP500UZ", Fluorescent, 7,	{ 4.890909, 1.163636, 1, 0 } },

  { "OLYMPUS", "SP510UZ", Daylight, 0,		{ 1.656250, 1, 1.621094, 0 } },
  { "OLYMPUS", "SP510UZ", Cloudy, 0,		{ 1.789063, 1, 1.546875, 0 } },
  { "OLYMPUS", "SP510UZ", Incandescent, 0,	{ 1, 1.066667, 2.891667, 0 } },
  { "OLYMPUS", "SP510UZ", WhiteFluorescent, 0,	{ 1.929688, 1, 1.562500, 0 } },
  { "OLYMPUS", "SP510UZ", NeutralFluorescent, 0, { 1.644531, 1, 1.843750, 0 } },
  { "OLYMPUS", "SP510UZ", DaylightFluorescent, 0, { 1.628906, 1, 2.210938, 0 } },

  { "OLYMPUS", "XZ-1", Daylight, 0,		{ 1.8437, 1, 1.8750, 0 } },
  { "OLYMPUS", "XZ-1", Shade, 0,		{ 2.1875, 1, 1.5703, 0 } },
  { "OLYMPUS", "XZ-1", Cloudy, 0,		{ 1.9766, 1, 1.7422, 0 } },
  { "OLYMPUS", "XZ-1", Incandescent, 0,		{ 1.1172, 1, 3.1875, 0 } },
  { "OLYMPUS", "XZ-1", Fluorescent, 0,		{ 1.8125, 1, 2.5938, 0 } },
  { "OLYMPUS", "XZ-1", Underwater, 0,		{ 1.4844, 1, 2.2188, 0 } },

  { "Panasonic", "DMC-FZ8", Daylight, 0,	{ 1.904943, 1, 1.596958, 0 } },
  { "Panasonic", "DMC-FZ8", Cloudy, 0,		{ 2.060836, 1, 1.498099, 0 } },
  { "Panasonic", "DMC-FZ8", Shade, 0,		{ 2.258555, 1, 1.391635, 0 } },
  { "Panasonic", "DMC-FZ8", Incandescent, 0,	{ 1.247148, 1, 2.288973, 0 } },
  { "Panasonic", "DMC-FZ8", Flash, 0,		{ 2.072243, 1, 1.456274, 0 } },

  { "Panasonic", "DMC-FZ18", Daylight, 0,	{ 1.783270, 1, 1.889734, 0 } },
  { "Panasonic", "DMC-FZ18", Cloudy, 0,		{ 1.946768, 1, 1.680608, 0 } },
  { "Panasonic", "DMC-FZ18", Shade, 0,		{ 2.117871, 1, 1.558935, 0 } },
  { "Panasonic", "DMC-FZ18", Incandescent, 0,	{ 1.140684, 1, 2.627376, 0 } },
  { "Panasonic", "DMC-FZ18", Flash, 0,		{ 1.882129, 1, 1.703422, 0 } },

  { "Panasonic", "DMC-FZ28", Daylight, 0,	{ 1.684411, 1, 1.802281, 0 } },
  { "Panasonic", "DMC-FZ28", Cloudy, 0,		{ 1.825095, 1, 1.676806, 0 } },
  { "Panasonic", "DMC-FZ28", Shade, 0,		{ 1.996198, 1, 1.566540, 0 } },
  { "Panasonic", "DMC-FZ28", Incandescent, 0,	{ 1.117871, 1, 2.558935, 0 } },
  { "Panasonic", "DMC-FZ28", Flash, 0,		{ 1.939164, 1, 1.596958, 0 } },
  { "Panasonic", "DMC-FZ28", "3000K", 0,	{ 1.015209, 1, 2.771863, 0 } },
  { "Panasonic", "DMC-FZ28", "5000K", 0,	{ 1.585551, 1, 1.889734, 0 } },

  { "Panasonic", "DMC-FZ30", Daylight, 0,	{ 1.757576, 1, 1.446970, 0 } },
  { "Panasonic", "DMC-FZ30", Cloudy, 0,		{ 1.943182, 1, 1.276515, 0 } },
  { "Panasonic", "DMC-FZ30", Incandescent, 0,	{ 1.098485, 1, 2.106061, 0 } },
  { "Panasonic", "DMC-FZ30", Flash, 0,		{ 1.965909, 1, 1.303030, 0 } },

  { "Panasonic", "DMC-FZ50", Daylight, 0,	{ 2.095057, 1, 1.642586, 0 } },
  { "Panasonic", "DMC-FZ50", Cloudy, 0,		{ 2.319392, 1, 1.482890, 0 } },
  { "Panasonic", "DMC-FZ50", Shade, 0,		{ 2.463878, 1, 1.414449, 0 } },
  { "Panasonic", "DMC-FZ50", Incandescent, 0,	{ 1.365019, 1, 2.311787, 0 } },
  { "Panasonic", "DMC-FZ50", Flash, 0,		{ 2.338403, 1, 1.338403, 0 } },

  { "Panasonic", "DMC-G1", Daylight, 0,		{ 1.942966, 1, 1.448669, 0 } },
  { "Panasonic", "DMC-G1", Cloudy, 0,		{ 2.106464, 1, 1.326996, 0 } },
  { "Panasonic", "DMC-G1", Shade, 0,		{ 2.323194, 1, 1.224335, 0 } },
  { "Panasonic", "DMC-G1", Incandescent, 0,	{ 1.319392, 1, 2.148289, 0 } },
  { "Panasonic", "DMC-G1", Flash, 0,		{ 1.528517, 1, 1.277567, 0 } },

  { "Panasonic", "DMC-G2", Daylight, 0,		{ 1.931559, 1, 1.414449, 0 } },
  { "Panasonic", "DMC-G2", Cloudy, 0,		{ 2.292776, 1, 1.231939, 0 } },
  { "Panasonic", "DMC-G2", Shade, 0,		{ 2.243346, 1, 1.231939, 0 } },
  { "Panasonic", "DMC-G2", Incandescent, 0,	{ 2.190114, 1, 1.250951, 0 } },
  { "Panasonic", "DMC-G2", Flash, 0,		{ 2.296578, 1, 1.190114, 0 } },

  { "Panasonic", "DMC-G3", Daylight, 0,		{ 2.7925, 1, 1.5472, 0 } },
  { "Panasonic", "DMC-G3", Cloudy, 0,		{ 2.9660, 1, 1.4528, 0 } },
  { "Panasonic", "DMC-G3", Shade, 0,		{ 3.2642, 1, 1.3698, 0 } },
  { "Panasonic", "DMC-G3", Incandescent, 0,	{ 1.8491, 1, 2.2566, 0 } },
  { "Panasonic", "DMC-G3", Flash, 0,		{ 3.2868, 1, 1.3547, 0 } },
  { "Panasonic", "DMC-G3", "2700K", 0,		{ 1.5283, 1, 2.6717, 0 } },
  { "Panasonic", "DMC-G3", "3000K", 0,		{ 1.7094, 1, 2.4189, 0 } },
  { "Panasonic", "DMC-G3", "3300K", 0,		{ 1.8792, 1, 2.1849, 0 } },
  { "Panasonic", "DMC-G3", "5000K", 0,		{ 2.6679, 1, 1.6264, 0 } },
  { "Panasonic", "DMC-G3", "5500K", 0,		{ 2.7925, 1, 1.5472, 0 } },
  { "Panasonic", "DMC-G3", "6500K", 0,		{ 3.0679, 1, 1.4189, 0 } },

  { "Panasonic", "DMC-GF1", Daylight, 0,	{ 2.007605, 1, 1.418251, 0 } },
  { "Panasonic", "DMC-GF1", Cloudy, 0,		{ 2.174905, 1, 1.296578, 0 } },
  { "Panasonic", "DMC-GF1", Shade, 0,		{ 2.395437, 1, 1.193916, 0 } },
  { "Panasonic", "DMC-GF1", Incandescent, 0,	{ 1.365019, 1, 2.117871, 0 } },
  { "Panasonic", "DMC-GF1", Flash, 0,		{ 2.353612, 1, 1.231939, 0 } },
  
  { "Panasonic", "DMC-GX1", Daylight, 0,	{ 2.7925, 1, 1.5472, 0 } },
  { "Panasonic", "DMC-GX1", Cloudy, 0,		{ 2.9660, 1, 1.4528, 0 } },
  { "Panasonic", "DMC-GX1", Shade, 0,		{ 3.2642, 1, 1.3698, 0 } },
  { "Panasonic", "DMC-GX1", Incandescent, 0,	{ 1.8491, 1, 2.2566, 0 } },
  { "Panasonic", "DMC-GX1", Flash, 0,		{ 3.2868, 1, 1.3547, 0 } },
  { "Panasonic", "DMC-GX1", "2700K", 0,		{ 1.5283, 1, 2.6717, 0 } },
  { "Panasonic", "DMC-GX1", "3000K", 0,		{ 1.7094, 1, 2.4189, 0 } },
  { "Panasonic", "DMC-GX1", "3300K", 0,		{ 1.8792, 1, 2.1849, 0 } },
  { "Panasonic", "DMC-GX1", "5000K", 0,		{ 2.6679, 1, 1.6264, 0 } },
  { "Panasonic", "DMC-GX1", "5500K", 0,		{ 2.7925, 1, 1.5472, 0 } },
  { "Panasonic", "DMC-GX1", "6500K", 0,		{ 3.0679, 1, 1.4189, 0 } },
  
  { "Panasonic", "DMC-L1", Daylight, 0,		{ 1.980989, 1, 1.444867, 0 } },
  { "Panasonic", "DMC-L1", Cloudy, 0,		{ 2.129278, 1, 1.300380, 0 } },
  { "Panasonic", "DMC-L1", Shade, 0,		{ 2.361217, 1, 1.167300, 0 } },
  { "Panasonic", "DMC-L1", Incandescent, 0,	{ 1.368821, 1, 2.091255, 0 } },
  /* Flash multipliers are variable */
  { "Panasonic", "DMC-L1", Flash, 0,		{ 2.319392, 1, 1.053232, 0 } },

  /* DMC-L1 Kelvin presets */
  { "Panasonic", "DMC-L1", "2700K", 0,		{ 1.285171, 1, 2.520913, 0 } },
  { "Panasonic", "DMC-L1", "3000K", 0,		{ 1.406844, 1, 2.209126, 0 } },
  { "Panasonic", "DMC-L1", "3300K", 0,		{ 1.524715, 1, 2.163498, 0 } },
  { "Panasonic", "DMC-L1", "5000K", 0,		{ 1.942966, 1, 1.528517, 0 } },
  { "Panasonic", "DMC-L1", "5500K", 0,		{ 2.019011, 1, 1.403042, 0 } },
  { "Panasonic", "DMC-L1", "6500K", 0,		{ 2.216730, 1, 1.243346, 0 } },

  { "Panasonic", "DMC-LX1", Daylight, 0,	{ 1.837121, 1, 1.484848, 0 } },
  { "Panasonic", "DMC-LX1", Cloudy, 0,		{ 2.003788, 1, 1.310606, 0 } },
  { "Panasonic", "DMC-LX1", Incandescent, 0,	{ 1.098485, 1, 2.272727, 0 } },

  { "Panasonic", "DMC-LX2", Daylight, -3,	{ 2.456274, 1, 1.806084, 0 } },
  { "Panasonic", "DMC-LX2", Daylight, 0,	{ 2.114068, 1, 1.726236, 0 } },
  { "Panasonic", "DMC-LX2", Daylight, 3,	{ 1.916350, 1, 1.585551, 0 } },
  { "Panasonic", "DMC-LX2", Cloudy, -3,		{ 2.714829, 1, 1.650190, 0 } },
  { "Panasonic", "DMC-LX2", Cloudy, 0,		{ 2.338403, 1, 1.577947, 0 } },
  { "Panasonic", "DMC-LX2", Cloudy, 3,		{ 2.121673, 1, 1.448669, 0 } },
  { "Panasonic", "DMC-LX2", Shade, -3,		{ 2.939163, 1, 1.577947, 0 } },
  { "Panasonic", "DMC-LX2", Shade, 0,		{ 2.532319, 1, 1.509506, 0 } },
  { "Panasonic", "DMC-LX2", Shade, 3,		{ 2.292776, 1, 1.384030, 0 } },
  { "Panasonic", "DMC-LX2", Incandescent, -3,	{ 1.581749, 1, 2.524715, 0 } },
  { "Panasonic", "DMC-LX2", Incandescent, 0,	{ 1.365019, 1, 2.410646, 0 } },
  { "Panasonic", "DMC-LX2", Incandescent, 3,	{ 1.235741, 1, 2.212928, 0 } },

  { "Panasonic", "DMC-LX3", Daylight, 0,	{ 2.022814, 1, 1.623574, 0 } },
  { "Panasonic", "DMC-LX3", Cloudy, 0,		{ 2.224335, 1, 1.520913, 0 } },
  { "Panasonic", "DMC-LX3", Shade, 0,		{ 2.475285, 1, 1.399240, 0 } },
  { "Panasonic", "DMC-LX3", Flash, 0,		{ 2.296578, 1, 1.482890, 0 } },
  { "Panasonic", "DMC-LX3", Incandescent, 0,	{ 1.346008, 1, 2.269962, 0 } },

  { "Panasonic", "DMC-LX5", Daylight, 0,	{ 1.9202, 1, 1.7567, 0 } },
  { "Panasonic", "DMC-LX5", Cloudy, 0,		{ 2.0760, 1, 1.6388, 0 } },
  { "Panasonic", "DMC-LX5", Shade, 0,		{ 2.1635, 1, 1.5817, 0 } },
  { "Panasonic", "DMC-LX5", Flash, 0,		{ 2.0760, 1, 1.6388, 0 } },
  { "Panasonic", "DMC-LX5", Incandescent, 0,	{ 1.2243, 1, 2.5741, 0 } },

  /* It seems that the *ist D WB settings are not really presets. */
  { "PENTAX", "*ist D", Daylight, 0,		{ 1.460938, 1, 1.019531, 0 } },
  { "PENTAX", "*ist D", Shade, 0,		{ 1.734375, 1, 1.000000, 0 } },
  { "PENTAX", "*ist D", Cloudy, 0,		{ 1.634921, 1.015873, 1, 0 } },
  { "PENTAX", "*ist D", DaylightFluorescent, 0,	{ 1.657025, 1.057851, 1, 0 } },
  { "PENTAX", "*ist D", NeutralFluorescent, 0,	{ 1.425781, 1, 1.117188, 0 } },
  { "PENTAX", "*ist D", WhiteFluorescent, 0,	{ 1.328125, 1, 1.210938, 0 } },
  { "PENTAX", "*ist D", Tungsten, 0,		{ 1.000000, 1, 2.226563, 0 } },
  { "PENTAX", "*ist D", Flash, 0,		{ 1.750000, 1, 1.000000, 0 } },

  /* It seems that the *ist DL WB settings are not really presets. */
  { "PENTAX", "*ist DL", Daylight, 0,		{ 1.546875, 1, 1.007812, 0 } },
  { "PENTAX", "*ist DL", Shade, 0,		{ 1.933594, 1, 1.027344, 0 } },
  { "PENTAX", "*ist DL", Cloudy, 0,		{ 1.703125, 1, 1.003906, 0 } },
  { "PENTAX", "*ist DL", DaylightFluorescent, 0, { 2.593909, 1.299492, 1, 0 } },
  { "PENTAX", "*ist DL", NeutralFluorescent, 0,	{ 1.539062, 1, 1.003906, 0 } },
  { "PENTAX", "*ist DL", WhiteFluorescent, 0,	{ 1.390625, 1, 1.117188, 0 } },
  { "PENTAX", "*ist DL", Tungsten, 0,		{ 1.000000, 1, 2.074219, 0 } },
  { "PENTAX", "*ist DL", Flash, 0,		{ 1.621094, 1, 1.027344, 0 } },

  /* It seems that the *ist DS WB settings are not really presets. */
  { "PENTAX", "*ist DS", Daylight, 0,		{ 1.632812, 1, 1.000000, 0 } },
  { "PENTAX", "*ist DS", Shade, 0,		{ 1.964844, 1, 1.000000, 0 } },
  { "PENTAX", "*ist DS", Cloudy, 0,		{ 1.761719, 1, 1.000000, 0 } },
  { "PENTAX", "*ist DS", DaylightFluorescent, 0, { 1.910156, 1, 1.000000, 0 } },
  { "PENTAX", "*ist DS", NeutralFluorescent, 0,	{ 1.521569, 1.003922, 1, 0 } },
  { "PENTAX", "*ist DS", WhiteFluorescent, 0,	{ 1.496094, 1, 1.023438, 0 } },
  { "PENTAX", "*ist DS", Tungsten, 0,		{ 1.000000, 1, 2.027344, 0 } },
  { "PENTAX", "*ist DS", Flash, 0,		{ 1.695312, 1, 1.000000, 0 } },

  { "PENTAX", "K10D", Daylight, 0,		{ 1.660156, 1, 1.066406, 0 } },
  { "PENTAX", "K10D", Shade, 0,			{ 2.434783, 1.236715, 1, 0 } },
  { "PENTAX", "K10D", Cloudy, 0,		{ 1.872428, 1.053498, 1, 0 } },
  { "PENTAX", "K10D", DaylightFluorescent, 0,	{ 2.121094, 1, 1.078125, 0 } },
  { "PENTAX", "K10D", NeutralFluorescent, 0,	{ 1.773438, 1, 1.226562, 0 } },
  { "PENTAX", "K10D", WhiteFluorescent, 0,	{ 1.597656, 1, 1.488281, 0 } },
  { "PENTAX", "K10D", Tungsten, 0,		{ 1.000000, 1, 2.558594, 0 } },
  { "PENTAX", "K10D", Flash, 0,			{ 1.664062, 1, 1.046875, 0 } },

  { "PENTAX", "K20D", Daylight, 0,		{ 1.691406, 1, 1.257812, 0 } },
  { "PENTAX", "K20D", Shade, 0,			{ 2.012245, 1.299492, 1, 0 } },
  { "PENTAX", "K20D", Cloudy, 0,		{ 1.792969, 1, 1.109375, 0 } },
  { "PENTAX", "K20D", DaylightFluorescent, 0,	{ 2.234375, 1, 1.183594, 0 } },
  { "PENTAX", "K20D", NeutralFluorescent, 0,	{ 1.898438, 1, 1.347656, 0 } },
  { "PENTAX", "K20D", WhiteFluorescent, 0,	{ 1.769531, 1, 1.675781, 0 } },
  { "PENTAX", "K20D", Tungsten, 0,		{ 1, 1.089362, 2.961702, 0 } },
  { "PENTAX", "K20D", Flash, 0,			{ 1.792969, 1, 1.183594, 0 } },

  { "PENTAX", "K100D", Daylight, 0,		{ 1.468750, 1, 1.023438, 0 } },
  { "PENTAX", "K100D", Shade, 0,		{ 1.769531, 1, 1.000000, 0 } },
  { "PENTAX", "K100D", Cloudy, 0,		{ 1.589844, 1, 1.000000, 0 } },
  { "PENTAX", "K100D", DaylightFluorescent, 0,	{ 1.722656, 1, 1.039063, 0 } },
  { "PENTAX", "K100D", NeutralFluorescent, 0,	{ 1.425781, 1, 1.160156, 0 } },
  { "PENTAX", "K100D", WhiteFluorescent, 0,	{ 1.265625, 1, 1.414063, 0 } },
  { "PENTAX", "K100D", Tungsten, 0,		{ 1, 1.015873, 2.055556, 0 } },
  { "PENTAX", "K100D", Flash, 0,		{ 1.527344, 1, 1.000000, 0 } },

  { "PENTAX", "K100D Super", Daylight, 0,	{ 1.593750, 1, 1.011719, 0 } },
  { "PENTAX", "K100D Super", Shade, 0,		{ 1.917969, 1, 1.000000, 0 } },
  { "PENTAX", "K100D Super", Cloudy, 0,		{ 1.703125, 1, 1.015625, 0 } },
  { "PENTAX", "K100D Super", DaylightFluorescent, 0, { 1.708502, 1.036437, 1, 0 } },
  { "PENTAX", "K100D Super", NeutralFluorescent, 0, { 1.634538, 1.028112, 1, 0 } },
  { "PENTAX", "K100D Super", WhiteFluorescent, 0, { 1.425781, 1, 1.136719, 0 } },
  { "PENTAX", "K100D Super", Tungsten, 0,	{ 1.015625, 1, 2.046875, 0 } },
  { "PENTAX", "K100D Super", Flash, 0,		{ 1.670588, 1.003922, 1, 0 } },

  { "PENTAX", "K110D", Daylight, 0,		{ 1.468750, 1, 1.023438, 0 } },
  { "PENTAX", "K110D", Shade, 0,		{ 1.769531, 1, 1.000000, 0 } },
  { "PENTAX", "K110D", Cloudy, 0,		{ 1.589844, 1, 1.000000, 0 } },
  { "PENTAX", "K110D", DaylightFluorescent, 0,	{ 1.722656, 1, 1.039063, 0 } },
  { "PENTAX", "K110D", NeutralFluorescent, 0,	{ 1.425781, 1, 1.160156, 0 } },
  { "PENTAX", "K110D", WhiteFluorescent, 0,	{ 1.265625, 1, 1.414063, 0 } },
  { "PENTAX", "K110D", Tungsten, 0,		{ 1, 1.015873, 2.055556, 0 } },
  { "PENTAX", "K110D", Flash, 0,		{ 1.527344, 1, 1.000000, 0 } },

  { "PENTAX", "K200D", Daylight, 0,		{ 1.804688, 1, 1.304688, 0 } },
  { "PENTAX", "K200D", Shade, 0,		{ 2.140625, 1, 1.085937, 0 } },
  { "PENTAX", "K200D", Cloudy, 0,		{ 1.957031, 1, 1.179687, 0 } },
  { "PENTAX", "K200D", DaylightFluorescent, 0,	{ 2.121094, 1, 1.195313, 0 } },
  { "PENTAX", "K200D", NeutralFluorescent, 0,	{ 1.773438, 1, 1.359375, 0 } },
  { "PENTAX", "K200D", WhiteFluorescent, 0,	{ 1.597656, 1, 1.648437, 0 } },
  { "PENTAX", "K200D", Tungsten, 0,		{ 1.000000, 1, 2.835937, 0 } },
  { "PENTAX", "K200D", Flash, 0,		{ 1.917969, 1, 1.214844, 0 } },

  { "PENTAX", "K-5", Daylight, 0,		{ 2.1211, 1, 1.5781, 0 } },
  { "PENTAX", "K-5", Shade, 0,			{ 2.5312, 1, 1.1758, 0 } },
  { "PENTAX", "K-5", Cloudy, 0,			{ 2.2852, 1, 1.3477, 0 } },
  { "PENTAX", "K-5", DaylightFluorescent, 0,	{ 2.6172, 1, 1.5195, 0 } },
  { "PENTAX", "K-5", WhiteFluorescent, 0,	{ 2.2422, 1, 1.6914, 0 } },
  { "PENTAX", "K-5", CoolWhiteFluorescent, 0,	{ 2.0391, 1, 2.0898, 0 } },
  { "PENTAX", "K-5", WarmWhiteFluorescent, 0,	{ 1.7070, 1, 2.6172, 0 } },
  { "PENTAX", "K-5", Tungsten, 0,		{ 1.2734, 1, 2.9258, 0 } },
  { "PENTAX", "K-5", Flash, 0,			{ 2.4023, 1, 1.4492, 0 } },
  { "PENTAX", "K-5", "3000K", 0,		{ 1.3867, 1, 2.2578, 0 } },
  { "PENTAX", "K-5", "5000K", 0,		{ 2.0508, 1, 1.4883, 0 } },

  { "PENTAX", "K-7", Daylight, 0,		{ 1.808594, 1, 1.285156, 0 } },
  { "PENTAX", "K-7", Shade, 0,			{ 2.207171, 1.019920, 1, 0 } },
  { "PENTAX", "K-7", Cloudy, 0,			{ 1.960937, 1, 1.136719, 0 } },
  { "PENTAX", "K-7", DaylightFluorescent, 0,	{ 2.281250, 1, 1.191406, 0 } },
  { "PENTAX", "K-7", NeutralFluorescent, 0,	{ 1.937500, 1, 1.355469, 0 } },
  { "PENTAX", "K-7", CoolWhiteFluorescent, 0,	{ 1.808594, 1, 1.687500, 0 } },
  { "PENTAX", "K-7", WarmWhiteFluorescent, 0,	{ 1.589844, 1, 2.164063, 0 } },
  { "PENTAX", "K-7", Tungsten, 0,		{ 1.105469, 1, 2.347656, 0 } },
  { "PENTAX", "K-7", Flash, 0,			{ 2.093750, 1, 1.082031, 0 } },

  { "PENTAX", "K-m", Daylight, 0,		{ 1.738281, 1, 1.363281, 0 } },
  { "PENTAX", "K-m", Shade, 0,			{ 2.027344, 1, 1.027344, 0 } },
  { "PENTAX", "K-m", Cloudy, 0,			{ 1.832031, 1, 1.183594, 0 } },
  { "PENTAX", "K-m", DaylightFluorescent, 0,	{ 2.183594, 1, 1.250000, 0 } },
  { "PENTAX", "K-m", NeutralFluorescent, 0,	{ 1.824219, 1, 1.417969, 0 } },
  { "PENTAX", "K-m", WhiteFluorescent, 0,	{ 1.644531, 1, 1.714844, 0 } },
  { "PENTAX", "K-m", Tungsten, 0,		{ 1.429687, 1, 1.980469, 0 } },
  { "PENTAX", "K-m", Flash, 0,			{ 1.738281, 1, 1.363281, 0 } },

  /* Firmware version 1.11 */
  { "PENTAX", "K-r", Daylight, 0,		{ 1.8477, 1, 1.3906, 0 } },
  { "PENTAX", "K-r", Shade, 0,			{ 2.1133, 1, 1.0586, 0 } },
  { "PENTAX", "K-r", Cloudy, 0,			{ 1.9766, 1, 1.1445, 0 } },
  { "PENTAX", "K-r", DaylightFluorescent, 0,	{ 2.2617, 1, 1.3203, 0 } },
  { "PENTAX", "K-r", WhiteFluorescent, 0,	{ 1.9414, 1, 1.4688, 0 } },
  { "PENTAX", "K-r", CoolWhiteFluorescent, 0,	{ 1.7656, 1, 1.8164, 0 } },
  { "PENTAX", "K-r", WarmWhiteFluorescent, 0,	{ 1.4766, 1, 2.2734, 0 } },
  { "PENTAX", "K-r", Tungsten, 0,		{ 1.1016, 1, 2.5391, 0 } },
  { "PENTAX", "K-r", Flash, 0,			{ 2.0117, 1, 1.1172, 0 } },

  { "PENTAX", "K-x", Daylight, 0,		{ 1.8803, 1, 1.4054, 0 } },
  { "PENTAX", "K-x", Shade, 0,			{ 2.2278, 1, 1.0309, 0 } },
  { "PENTAX", "K-x", Cloudy, 0,			{ 2.0077, 1, 1.1853, 0 } },
  { "PENTAX", "K-x", DaylightFluorescent, 0,	{ 2.3012, 1, 1.3359, 0 } },
  { "PENTAX", "K-x", WhiteFluorescent, 0,	{ 1.9730, 1, 1.4826, 0 } },
  { "PENTAX", "K-x", CoolWhiteFluorescent, 0,	{ 1.7915, 1, 1.8378, 0 } },
  { "PENTAX", "K-x", WarmWhiteFluorescent, 0,	{ 1.5019, 1, 2.2973, 0 } },
  { "PENTAX", "K-x", Tungsten, 0,		{ 1.0463, 1, 3.4015, 0 } },
  { "PENTAX", "K-x", Flash, 0,			{ 2.1120, 1, 1.2741, 0 } },

  { "RICOH", "Caplio GX100", Daylight, 0,	{ 1.910001, 1, 1.820002, 0 } },
  { "RICOH", "Caplio GX100", Cloudy, 0,		{ 2.240003, 1, 1.530002, 0 } },
  { "RICOH", "Caplio GX100", Incandescent, 0,	{ 1.520002, 1, 2.520003, 0 } },
  { "RICOH", "Caplio GX100", Fluorescent, 0,	{ 1.840001, 1, 1.970001, 0 } },

  { "SAMSUNG", "GX-1S", Daylight, 0,		{ 1.574219, 1, 1.109375, 0 } },
  { "SAMSUNG", "GX-1S", Shade, 0,		{ 1.855469, 1, 1.000000, 0 } },
  { "SAMSUNG", "GX-1S", Cloudy, 0,		{ 1.664062, 1, 1.000000, 0 } },
  { "SAMSUNG", "GX-1S", DaylightFluorescent, 0,	{ 1.854251, 1.036437, 1, 0 } },
  { "SAMSUNG", "GX-1S", NeutralFluorescent, 0,	{ 1.574219, 1, 1.171875, 0 } },
  { "SAMSUNG", "GX-1S", WhiteFluorescent, 0,	{ 1.363281, 1, 1.335938, 0 } },
  { "SAMSUNG", "GX-1S", Tungsten, 0,		{ 1.000000, 1, 2.226562, 0 } },
  { "SAMSUNG", "GX-1S", Flash, 0,		{ 1.609375, 1, 1.031250, 0 } },

  { "SAMSUNG", "GX10", Daylight, 0,		{ 1.660156, 1, 1.066406, 0 } },
  { "SAMSUNG", "GX10", Shade, 0,		{ 2.434783, 1.236715, 1, 0 } },
  { "SAMSUNG", "GX10", Cloudy, 0,		{ 1.872428, 1.053498, 1, 0 } },
  { "SAMSUNG", "GX10", DaylightFluorescent, 0,	{ 2.121094, 1, 1.078125, 0 } },
  { "SAMSUNG", "GX10", NeutralFluorescent, 0,	{ 1.773438, 1, 1.226562, 0 } },
  { "SAMSUNG", "GX10", WhiteFluorescent, 0,	{ 1.597656, 1, 1.488281, 0 } },
  { "SAMSUNG", "GX10", Tungsten, 0,		{ 1.000000, 1, 2.558594, 0 } },
  { "SAMSUNG", "GX10", Flash, 0,		{ 1.664062, 1, 1.046875, 0 } },

  // SAMSUNG NX100 (firmware 1.10) white balance presets with finetuning steps
  { "SAMSUNG", "NX100", Daylight , -7,		{ 1.559003, 1, 1.529685, 0 } },
  { "SAMSUNG", "NX100", Daylight, 0,		{ 1.751771, 1, 1.167115, 0 } },
  { "SAMSUNG", "NX100", Daylight, 7,		{ 1.961884, 1, 0.968987, 0 } },
  { "SAMSUNG", "NX100", Cloudy, -7,		{ 1.630833, 1, 1.449059, 0 } },
  { "SAMSUNG", "NX100", Cloudy, 0,		{ 1.833333, 1, 1.106305, 0 } },
  { "SAMSUNG", "NX100", Cloudy, 7,		{ 2.053387, 1, 0.918125, 0 } },
  { "SAMSUNG", "NX100", WhiteFluorescent, -7,	{ 1.519306, 1, 2.321359, 0 } },
  { "SAMSUNG", "NX100", WhiteFluorescent, 0,	{ 1.707234, 1, 1.771750, 0 } },
  { "SAMSUNG", "NX100", WhiteFluorescent, 7,	{ 1.912268, 1, 1.470430, 0 } },
  { "SAMSUNG", "NX100", NeutralFluorescent, -7,	{ 1.683362, 1, 1.888102, 0 } },
  { "SAMSUNG", "NX100", NeutralFluorescent, 0,	{ 1.891984, 1, 1.441349, 0 } },
  { "SAMSUNG", "NX100", NeutralFluorescent, 7,	{ 2.119013, 1, 1.196237, 0 } },
  { "SAMSUNG", "NX100", DaylightFluorescent, -7, { 1.986559, 1, 1.617058, 0 } },
  { "SAMSUNG", "NX100", DaylightFluorescent, 0,	{ 2.232160, 1, 1.234115, 0 } },
  { "SAMSUNG", "NX100", DaylightFluorescent, 7,	{ 2.500000, 1, 1.024194, 0 } },
  { "SAMSUNG", "NX100", Tungsten, -7,		{ 0.947015, 1, 3.091393, 0 } },
  { "SAMSUNG", "NX100", Tungsten, 0,		{ 1.063767, 1, 2.359394, 0 } },
  { "SAMSUNG", "NX100", Tungsten, 7,		{ 1.191838, 1, 1.958456, 0 } },
  { "SAMSUNG", "NX100", Flash, -7,		{ 1.655265, 1, 1.315172, 0 } },
  { "SAMSUNG", "NX100", Flash, 0,		{ 1.860459, 1, 1.004154, 0 } },
  { "SAMSUNG", "NX100", Flash, 7,		{ 2.083469, 1, 0.833265, 0 } },
  { "SAMSUNG", "NX100", "5000K", 0,		{ 1.676277, 1, 1.097728, 0 } },
  { "SAMSUNG", "NX100", "5500K", 0,		{ 1.683993, 1, 0.847587, 0 } },
  { "SAMSUNG", "NX100", "6500K", 0,		{ 2.102913, 1, 0.883064, 0 } },

  { "SONY", "DSLR-A100", Daylight, -3,		{ 1.601562, 1, 2.101562, 0 } },
  { "SONY", "DSLR-A100", Daylight, 0,		{ 1.746094, 1, 1.843750, 0 } },
  { "SONY", "DSLR-A100", Daylight, 3,		{ 1.914062, 1, 1.628906, 0 } },
  { "SONY", "DSLR-A100", Shade, -3,		{ 1.906250, 1, 1.843750, 0 } },
  { "SONY", "DSLR-A100", Shade, 0,		{ 2.070312, 1, 1.609375, 0 } },
  { "SONY", "DSLR-A100", Shade, 3,		{ 2.281250, 1, 1.429688, 0 } },
  { "SONY", "DSLR-A100", Cloudy, -3,		{ 1.691406, 1, 1.863281, 0 } },
  { "SONY", "DSLR-A100", Cloudy, 0,		{ 1.855469, 1, 1.628906, 0 } },
  { "SONY", "DSLR-A100", Cloudy, 3,		{ 2.023438, 1, 1.445312, 0 } },
  { "SONY", "DSLR-A100", Tungsten, -3,		{ 1, 1.028112, 4.610442, 0 } },
  { "SONY", "DSLR-A100", Tungsten, 0,		{ 1.054688, 1, 3.917969, 0 } },
  { "SONY", "DSLR-A100", Tungsten, 3,		{ 1.164062, 1, 3.476562, 0 } },
  { "SONY", "DSLR-A100", Fluorescent, -2,	{ 1.058594, 1, 4.453125, 0 } },
  { "SONY", "DSLR-A100", Fluorescent, 0,	{ 1.718750, 1, 3.058594, 0 } },
  { "SONY", "DSLR-A100", Fluorescent, 3,	{ 2.238281, 1, 1.949219, 0 } },
  { "SONY", "DSLR-A100", Fluorescent, 4,	{ 1.992188, 1, 1.757812, 0 } },
  { "SONY", "DSLR-A100", Flash, -3,		{ 1.710938, 1, 1.988281, 0 } },
  { "SONY", "DSLR-A100", Flash, 0,		{ 1.859375, 1, 1.746094, 0 } },
  { "SONY", "DSLR-A100", Flash, 3,		{ 2.046875, 1, 1.542969, 0 } },

  { "SONY", "DSLR-A200", Daylight, -3 ,		{ 1.507812, 1, 1.996094, 0 } },
  { "SONY", "DSLR-A200", Daylight, 0 ,		{ 1.664062, 1, 1.757812, 0 } },
  { "SONY", "DSLR-A200", Daylight, 3 ,		{ 1.820313, 1, 1.546875, 0 } },
  { "SONY", "DSLR-A200", Shade, -3 ,		{ 1.800781, 1, 1.578125, 0 } },
  { "SONY", "DSLR-A200", Shade, 0 ,		{ 1.972656, 1, 1.390625, 0 } },
  { "SONY", "DSLR-A200", Shade, 3 ,		{ 2.164063, 1, 1.218750, 0 } },
  { "SONY", "DSLR-A200", Cloudy, -3 ,		{ 1.636719, 1, 1.800781, 0 } },
  { "SONY", "DSLR-A200", Cloudy, 0 ,		{ 1.800781, 1, 1.585937, 0 } },
  { "SONY", "DSLR-A200", Cloudy, 3 ,		{ 1.972656, 1, 1.390625, 0 } },
  { "SONY", "DSLR-A200", Tungsten, -3 ,		{ 1, 1.136719, 4.355469, 0 } },
  { "SONY", "DSLR-A200", Tungsten, 0 ,		{ 1, 1.027344, 3.492187, 0 } },
  { "SONY", "DSLR-A200", Tungsten, 3 ,		{ 1.082031, 1, 3.019531, 0 } },
  { "SONY", "DSLR-A200", Fluorescent, -2 ,	{ 1, 1.066406, 4.453125, 0 } },
  { "SONY", "DSLR-A200", Fluorescent, 0 ,	{ 1.554687, 1, 2.601562, 0 } },
  { "SONY", "DSLR-A200", Fluorescent, 3 ,	{ 2.109375, 1, 1.828125, 0 } },
  { "SONY", "DSLR-A200", Flash, -3 ,		{ 1.746094, 1, 1.660156, 0 } },
  { "SONY", "DSLR-A200", Flash, 0 ,		{ 1.917969, 1, 1.460937, 0 } },
  { "SONY", "DSLR-A200", Flash, 3 ,		{ 2.109375, 1, 1.285156, 0 } },

  { "SONY", "DSLR-A300", Daylight, -3,		{ 1.480469, 1, 1.960937, 0 } },
  { "SONY", "DSLR-A300", Daylight, 0,		{ 1.632813, 1, 1.730469, 0 } },
  { "SONY", "DSLR-A300", Daylight, 3,		{ 1.789062, 1, 1.527344, 0 } },
  { "SONY", "DSLR-A300", Shade, -3,		{ 1.769531, 1, 1.554687, 0 } },
  { "SONY", "DSLR-A300", Shade, 0,		{ 1.937500, 1, 1.371094, 0 } },
  { "SONY", "DSLR-A300", Shade, 3,		{ 2.121094, 1, 1.207031, 0 } },
  { "SONY", "DSLR-A300", Cloudy, -3,		{ 1.609375, 1, 1.769531, 0 } },
  { "SONY", "DSLR-A300", Cloudy, 0,		{ 1.769531, 1, 1.566406, 0 } },
  { "SONY", "DSLR-A300", Cloudy, 3,		{ 1.937500, 1, 1.371094, 0 } },
  { "SONY", "DSLR-A300", Tungsten, -3,		{ 1, 1.152344, 4.308594, 0 } },
  { "SONY", "DSLR-A300", Tungsten, 0,		{ 1, 1.039063, 3.449219, 0 } },
  { "SONY", "DSLR-A300", Tungsten, 3,		{ 1.066406, 1, 2.953125, 0 } },
  { "SONY", "DSLR-A300", Fluorescent, -2,	{ 1, 1.082031, 4.410156, 0 } },
  { "SONY", "DSLR-A300", Fluorescent, -1,	{ 1.117187, 1, 3.343750, 0 } },
  { "SONY", "DSLR-A300", Fluorescent, 0,	{ 1.527344, 1, 2.546875, 0 } },
  { "SONY", "DSLR-A300", Fluorescent, 1,	{ 1.714844, 1, 2.109375, 0 } },
  { "SONY", "DSLR-A300", Fluorescent, 2,	{ 1.546875, 1, 1.769531, 0 } },
  { "SONY", "DSLR-A300", Fluorescent, 3,	{ 2.070312, 1, 1.796875, 0 } },
  { "SONY", "DSLR-A300", Fluorescent, 4,	{ 1.761719, 1, 1.527344, 0 } },
  { "SONY", "DSLR-A300", Flash, -3,		{ 1.714844, 1, 1.632812, 0 } },
  { "SONY", "DSLR-A300", Flash, 0,		{ 1.882812, 1, 1.441406, 0 } },
  { "SONY", "DSLR-A300", Flash, 3,		{ 2.070312, 1, 1.273438, 0 } },

  { "SONY", "DSLR-A330", Daylight, -3,		{ 1.5898, 1, 1.9687, 0 } },
  { "SONY", "DSLR-A330", Daylight, -2,		{ 1.6406, 1, 1.8789, 0 } },
  { "SONY", "DSLR-A330", Daylight, -1,		{ 1.6914, 1, 1.7969, 0 } },
  { "SONY", "DSLR-A330", Daylight, 0,		{ 1.7500, 1, 1.7227, 0 } },
  { "SONY", "DSLR-A330", Daylight, 1,		{ 1.7969, 1, 1.6523, 0 } },
  { "SONY", "DSLR-A330", Daylight, 2,		{ 1.8555, 1, 1.5742, 0 } },
  { "SONY", "DSLR-A330", Daylight, 3,		{ 1.9141, 1, 1.5000, 0 } },
  { "SONY", "DSLR-A330", Shade, -3,		{ 1.8906, 1, 1.5352, 0 } },
  { "SONY", "DSLR-A330", Shade, -2,		{ 1.9492, 1, 1.4648, 0 } },
  { "SONY", "DSLR-A330", Shade, -1,		{ 2.0078, 1, 1.4023, 0 } },
  { "SONY", "DSLR-A330", Shade, 0,		{ 2.0703, 1, 1.3359, 0 } },
  { "SONY", "DSLR-A330", Shade, 1,		{ 2.1328, 1, 1.2734, 0 } },
  { "SONY", "DSLR-A330", Shade, 2,		{ 2.2031, 1, 1.2109, 0 } },
  { "SONY", "DSLR-A330", Shade, 3,		{ 2.2656, 1, 1.1602, 0 } },
  { "SONY", "DSLR-A330", Cloudy, -3,		{ 1.7227, 1, 1.7617, 0 } },
  { "SONY", "DSLR-A330", Cloudy, -2,		{ 1.7773, 1, 1.6836, 0 } },
  { "SONY", "DSLR-A330", Cloudy, -1,		{ 1.8359, 1, 1.6094, 0 } },
  { "SONY", "DSLR-A330", Cloudy, 0,		{ 1.8906, 1, 1.5430, 0 } },
  { "SONY", "DSLR-A330", Cloudy, 1,		{ 1.9492, 1, 1.4727, 0 } },
  { "SONY", "DSLR-A330", Cloudy, 2,		{ 2.0078, 1, 1.4023, 0 } },
  { "SONY", "DSLR-A330", Cloudy, 3,		{ 2.0703, 1, 1.3359, 0 } },
  { "SONY", "DSLR-A330", Tungsten, -3,		{ 1.0664, 1, 1.0664, 0 } },
  { "SONY", "DSLR-A330", Tungsten, -2,		{ 1.0313, 1, 1.0313, 0 } },
  { "SONY", "DSLR-A330", Tungsten, -1,		{ 1.0039, 1, 3.5625, 0 } },
  { "SONY", "DSLR-A330", Tungsten, 0,		{ 1.0352, 1, 3.4219, 0 } },
  { "SONY", "DSLR-A330", Tungsten, 1,		{ 1.0703, 1, 3.2812, 0 } },
  { "SONY", "DSLR-A330", Tungsten, 2,		{ 1.1055, 1, 3.1602, 0 } },
  { "SONY", "DSLR-A330", Tungsten, 3,		{ 1.1484, 1, 3.0313, 0 } },
  { "SONY", "DSLR-A330", Fluorescent, -2,	{ 1.0312, 1, 1.0312, 0 } },
  { "SONY", "DSLR-A330", Fluorescent, -1,	{ 1.2305, 1, 3.2383, 0 } },
  { "SONY", "DSLR-A330", Fluorescent, 0,	{ 1.6172, 1, 2.4648, 0 } },
  { "SONY", "DSLR-A330", Fluorescent, 1,	{ 1.8242, 1, 2.0508, 0 } },
  { "SONY", "DSLR-A330", Fluorescent, 2,	{ 1.7305, 1, 1.7695, 0 } },
  { "SONY", "DSLR-A330", Fluorescent, 3,	{ 2.2227, 1, 1.7695, 0 } },
  { "SONY", "DSLR-A330", Fluorescent, 4,	{ 1.9805, 1, 1.5586, 0 } },
  { "SONY", "DSLR-A330", Flash, -3,		{ 1.7656, 1, 1.7500, 0 } },
  { "SONY", "DSLR-A330", Flash, -2,		{ 1.8203, 1, 1.6836, 0 } },
  { "SONY", "DSLR-A330", Flash, -1,		{ 1.8789, 1, 1.6094, 0 } },
  { "SONY", "DSLR-A330", Flash, 0,		{ 1.9375, 1, 1.5313, 0 } },
  { "SONY", "DSLR-A330", Flash, 1,		{ 2.0000, 1, 1.4648, 0 } },
  { "SONY", "DSLR-A330", Flash, 2,		{ 2.0625, 1, 1.3945, 0 } },
  { "SONY", "DSLR-A330", Flash, 3,		{ 2.1250, 1, 1.3359, 0 } },

  { "SONY", "DSLR-A350", Daylight, -3,		{ 2.316406, 1, 1.886719, 0 } },
  { "SONY", "DSLR-A350", Daylight, 0,		{ 2.531250, 1, 1.648437, 0 } },
  { "SONY", "DSLR-A350", Daylight, 3,		{ 2.750000, 1, 1.437500, 0 } },
  { "SONY", "DSLR-A350", Shade, -3,		{ 2.722656, 1, 1.468750, 0 } },
  { "SONY", "DSLR-A350", Shade, 0,		{ 2.960937, 1, 1.281250, 0 } },
  { "SONY", "DSLR-A350", Shade, 3,		{ 3.222656, 1, 1.109375, 0 } },
  { "SONY", "DSLR-A350", Cloudy, -3,		{ 2.496094, 1, 1.691406, 0 } },
  { "SONY", "DSLR-A350", Cloudy, 0,		{ 2.722656, 1, 1.476562, 0 } },
  { "SONY", "DSLR-A350", Cloudy, 3,		{ 2.960937, 1, 1.281250, 0 } },
  { "SONY", "DSLR-A350", Tungsten, -3,		{ 1.445313, 1, 3.722656, 0 } },
  { "SONY", "DSLR-A350", Tungsten, 0,		{ 1.578125, 1, 3.289062, 0 } },
  { "SONY", "DSLR-A350", Tungsten, 3,		{ 1.726562, 1, 2.910156, 0 } },
  { "SONY", "DSLR-A350", Flash, -3,		{ 2.644531, 1, 1.558594, 0 } },
  { "SONY", "DSLR-A350", Flash, 0,		{ 2.875000, 1, 1.355469, 0 } },
  { "SONY", "DSLR-A350", Flash, 3,		{ 3.136719, 1, 1.175781, 0 } },
  { "SONY", "DSLR-A350", CoolWhiteFluorescent, 0, { 2.226563, 1, 2.355469, 0 } },
  { "SONY", "DSLR-A350", Fluorescent, 0,	{ 1.554687, 1, 3.984375, 0 } },
  { "SONY", "DSLR-A350", WarmWhiteFluorescent, 0, { 1.816406, 1, 3.207031, 0 } },
  { "SONY", "DSLR-A350", DayWhiteFluorescent, 0, { 2.511719, 1, 1.957031, 0 } },
//  { "SONY", "DSLR-A350", DayWhiteFluorescent, 0, { 2.484375, 1, 1.683594, 0 } },
  { "SONY", "DSLR-A350", DaylightFluorescent, 0, { 3.023437, 1, 1.671875, 0 } },
//  { "SONY", "DSLR-A350", DaylightFluorescent, 0, { 2.773438, 1, 1.441406, 0 } },

  { "SONY", "DSLR-A380", Daylight, -3,		{ 2.335938, 1, 1.875000, 0 } },
  { "SONY", "DSLR-A380", Daylight, 0,		{ 2.562500, 1, 1.648438, 0 } },
  { "SONY", "DSLR-A380", Daylight, 3,		{ 2.796875, 1, 1.445312, 0 } },
  { "SONY", "DSLR-A380", Shade, -3,		{ 2.765625, 1, 1.472656, 0 } },
  { "SONY", "DSLR-A380", Shade, 0,		{ 3.019531, 1, 1.292969, 0 } },
  { "SONY", "DSLR-A380", Shade, 3,		{ 3.296875, 1, 1.128906, 0 } },
  { "SONY", "DSLR-A380", Cloudy, -3,		{ 2.527344, 1, 1.687500, 0 } },
  { "SONY", "DSLR-A380", Cloudy, 0,		{ 2.765625, 1, 1.480469, 0 } },
  { "SONY", "DSLR-A380", Cloudy, 3,		{ 3.019531, 1, 1.292969, 0 } },
  { "SONY", "DSLR-A380", Tungsten, -3,		{ 1.410156, 1, 3.636719, 0 } },
  { "SONY", "DSLR-A380", Tungsten, 0,		{ 1.550781, 1, 3.222656, 0 } },
  { "SONY", "DSLR-A380", Tungsten, 3,		{ 1.710938, 1, 2.859375, 0 } },
  { "SONY", "DSLR-A380", Fluorescent, -2,	{ 1.429687, 1, 3.906250, 0 } },
  { "SONY", "DSLR-A380", Fluorescent, 0,	{ 2.234375, 1, 2.335938, 0 } },
  { "SONY", "DSLR-A380", Fluorescent, 4,	{ 2.792969, 1, 1.445312, 0 } },
  { "SONY", "DSLR-A380", Flash, -3,		{ 2.574219, 1, 1.664063, 0 } },
  { "SONY", "DSLR-A380", Flash, 0,		{ 2.816406, 1, 1.453125, 0 } },
  { "SONY", "DSLR-A380", Flash, 3,		{ 3.070312, 1, 1.273437, 0 } },

  { "SONY", "DSLR-A390", Daylight, -3,		{ 2.0273, 1, 1.5820, 0 } },
  { "SONY", "DSLR-A390", Daylight, 0,		{ 2.2188, 1, 1.3711, 0 } },
  { "SONY", "DSLR-A390", Daylight, 3,		{ 2.4180, 1, 1.1836, 0 } },
  { "SONY", "DSLR-A390", Shade, -3,		{ 2.3906, 1, 1.2148, 0 } },
  { "SONY", "DSLR-A390", Shade, 0,		{ 2.6055, 1, 1.0469, 0 } },
  { "SONY", "DSLR-A390", Shade, 3,		{ 2.8392, 1, 1.0000, 0 } },
  { "SONY", "DSLR-A390", Cloudy, -3,		{ 2.1875, 1, 1.4062, 0 } },
  { "SONY", "DSLR-A390", Cloudy, 0,		{ 2.3906, 1, 1.2227, 0 } },
  { "SONY", "DSLR-A390", Cloudy, 3,		{ 2.6055, 1, 1.0469, 0 } },
  { "SONY", "DSLR-A390", Tungsten, -3,		{ 1.2461, 1, 3.1992, 0 } },
  { "SONY", "DSLR-A390", Tungsten, 0,		{ 1.3633, 1, 2.8164, 0 } },
  { "SONY", "DSLR-A390", Tungsten, 3,		{ 1.4961, 1, 2.4844, 0 } },
  { "SONY", "DSLR-A390", Fluorescent, -2,	{ 1.2617, 1, 3.4453, 0 } },
  { "SONY", "DSLR-A390", Fluorescent, 0,	{ 1.9414, 1, 2.0039, 0 } },
  { "SONY", "DSLR-A390", Fluorescent, 4,	{ 2.4102, 1, 1.1836, 0 } },
  { "SONY", "DSLR-A390", Flash, -3,		{ 2.2305, 1, 1.3867, 0 } },
  { "SONY", "DSLR-A390", Flash, 0,		{ 2.4336, 1, 1.1953, 0 } },
  { "SONY", "DSLR-A390", Flash, 3,		{ 2.6484, 1, 1.0313, 0 } },

  /* Sony A450 presets */
  { "SONY", "DSLR-A450", Daylight, -3,		{ 2.109375, 1, 1.593750, 0 } },
  { "SONY", "DSLR-A450", Daylight, 0,		{ 2.296875, 1, 1.445312, 0 } },
  { "SONY", "DSLR-A450", Daylight, 3,		{ 2.503906, 1, 1.312500, 0 } },
  { "SONY", "DSLR-A450", Shade, -3,		{ 2.468750, 1, 1.332031, 0 } },
  { "SONY", "DSLR-A450", Shade, 0,		{ 2.691406, 1, 1.214844, 0 } },
  { "SONY", "DSLR-A450", Shade, 3,		{ 2.925781, 1, 1.105469, 0 } },
  { "SONY", "DSLR-A450", Cloudy, -3,		{ 2.261719, 1, 1.468750, 0 } },
  { "SONY", "DSLR-A450", Cloudy, 0,		{ 2.464844, 1, 1.335938, 0 } },
  { "SONY", "DSLR-A450", Cloudy, 3,		{ 2.683594, 1, 1.214844, 0 } },
  { "SONY", "DSLR-A450", Tungsten, -3,		{ 1.312500, 1, 2.734375, 0 } },
  { "SONY", "DSLR-A450", Tungsten, 0,		{ 1.437500, 1, 2.468750, 0 } },
  { "SONY", "DSLR-A450", Tungsten, 3,		{ 1.566406, 1, 2.234375, 0 } },
  { "SONY", "DSLR-A450", Fluorescent, -1,	{ 1.636719, 1, 2.507812, 0 } },
  { "SONY", "DSLR-A450", Fluorescent, 0,	{ 2.019531, 1, 2.003906, 0 } },
  { "SONY", "DSLR-A450", Fluorescent, 3,	{ 2.507812, 1, 1.355469, 0 } },
  { "SONY", "DSLR-A450", Flash, -3,		{ 2.339844, 1, 1.433594, 0 } },
  { "SONY", "DSLR-A450", Flash, 0,		{ 2.550781, 1, 1.304688, 0 } },
  { "SONY", "DSLR-A450", Flash, 3,		{ 2.777344, 1, 1.183594, 0 } },

  { "SONY", "DSLR-A550", Daylight, 0,		{ 2.160156, 1, 1.496094, 0 } },
  { "SONY", "DSLR-A550", Shade, 0,		{ 2.519531, 1, 1.234375, 0 } },
  { "SONY", "DSLR-A550", Cloudy, 0,		{ 2.312500, 1, 1.375000, 0 } },
  { "SONY", "DSLR-A550", Tungsten, 0,		{ 1.367188, 1, 2.632813, 0 } },
  { "SONY", "DSLR-A550", Fluorescent, 0,	{ 1.902344, 1, 2.117188, 0 } },
  { "SONY", "DSLR-A550", Flash, 0,		{ 2.390625, 1, 1.335938, 0 } },

  /* Sony A700 presets - firmware v2 */
  { "SONY", "DSLR-A700", Daylight, -3,		{ 1.937500, 1, 1.640625, 0 } },
  { "SONY", "DSLR-A700", Daylight, 0,		{ 2.101563, 1, 1.484375, 0 } },
  { "SONY", "DSLR-A700", Daylight, 3,		{ 2.273437, 1, 1.343750, 0 } },
  { "SONY", "DSLR-A700", Shade, -3,		{ 2.257812, 1, 1.359375, 0 } },
  { "SONY", "DSLR-A700", Shade, 0,		{ 2.445313, 1, 1.226563, 0 } },
  { "SONY", "DSLR-A700", Shade, 3,		{ 2.652344, 1, 1.113281, 0 } },
  { "SONY", "DSLR-A700", Cloudy, -3,		{ 2.070312, 1, 1.507812, 0 } },
  { "SONY", "DSLR-A700", Cloudy, 0,		{ 2.250000, 1, 1.367188, 0 } },
  { "SONY", "DSLR-A700", Cloudy, 3,		{ 2.429688, 1, 1.234375, 0 } },
  { "SONY", "DSLR-A700", Tungsten, -3,		{ 1.230469, 1, 2.859375, 0 } },
  { "SONY", "DSLR-A700", Tungsten, 0,		{ 1.335938, 1, 2.597656, 0 } },
  { "SONY", "DSLR-A700", Tungsten, 3,		{ 1.449219, 1, 2.343750, 0 } },
  { "SONY", "DSLR-A700", Fluorescent, -2,	{ 1.292969, 1, 3.199219, 0 } },
  { "SONY", "DSLR-A700", Fluorescent, 0,	{ 1.878906, 1, 2.152344, 0 } },
  { "SONY", "DSLR-A700", Fluorescent, 3,	{ 2.433594, 1, 1.539063, 0 } },
  { "SONY", "DSLR-A700", Fluorescent, 4,	{ 2.273437, 1, 1.347656, 0 } },
  { "SONY", "DSLR-A700", Flash, -3,		{ 2.128906, 1, 1.460937, 0 } },
  { "SONY", "DSLR-A700", Flash, 0,		{ 2.277344, 1, 1.312500, 0 } },
  { "SONY", "DSLR-A700", Flash, 3,		{ 2.496094, 1, 1.199219, 0 } },

  /* Sony A850 presets - firmware v1.00 */
  { "SONY", "DSLR-A850", Daylight, -3,		{ 2.445313, 1, 1.515625, 0 } },
  { "SONY", "DSLR-A850", Daylight, 0,		{ 2.683594, 1, 1.359375, 0 } },
  { "SONY", "DSLR-A850", Daylight, 3,		{ 2.929688, 1, 1.222656, 0 } },
  { "SONY", "DSLR-A850", Shade, -3,		{ 2.898438, 1, 1.242188, 0 } },
  { "SONY", "DSLR-A850", Shade, 0,		{ 3.164062, 1, 1.121094, 0 } },
  { "SONY", "DSLR-A850", Shade, 3,		{ 3.457031, 1, 1.011719, 0 } },
  { "SONY", "DSLR-A850", Cloudy, -3,		{ 2.644531, 1, 1.386719, 0 } },
  { "SONY", "DSLR-A850", Cloudy, 0,		{ 2.898438, 1, 1.250000, 0 } },
  { "SONY", "DSLR-A850", Cloudy, 3,		{ 3.164062, 1, 1.121094, 0 } },
  { "SONY", "DSLR-A850", Tungsten, -3,		{ 1.468750, 1, 2.703125, 0 } },
  { "SONY", "DSLR-A850", Tungsten, 0,		{ 1.617188, 1, 2.421875, 0 } },
  { "SONY", "DSLR-A850", Tungsten, 3,		{ 1.785156, 1, 2.175781, 0 } },
  { "SONY", "DSLR-A850", Fluorescent, -2,	{ 1.531250, 1, 2.925781, 0 } },
  { "SONY", "DSLR-A850", Fluorescent, 0,	{ 2.324219, 1, 1.882813, 0 } },
  { "SONY", "DSLR-A850", Fluorescent, 4,	{ 2.945313, 1, 1.238281, 0 } },
  { "SONY", "DSLR-A850", Flash, -3,		{ 2.757813, 1, 1.324219, 0 } },
  { "SONY", "DSLR-A850", Flash, 0,		{ 3.015625, 1, 1.191406, 0 } },
  { "SONY", "DSLR-A850", Flash, 3,		{ 3.296875, 1, 1.070312, 0 } },

  { "SONY", "DSLR-A900", Daylight, -3,		{ 2.351563, 1, 1.511719, 0 } },
  { "SONY", "DSLR-A900", Daylight, 0,		{ 2.585938, 1, 1.355469, 0 } },
  { "SONY", "DSLR-A900", Daylight, 3,		{ 2.824219, 1, 1.218750, 0 } },
  { "SONY", "DSLR-A900", Shade, -3,		{ 2.792969, 1, 1.238281, 0 } },
  { "SONY", "DSLR-A900", Shade, 0,		{ 3.054688, 1, 1.113281, 0 } },
  { "SONY", "DSLR-A900", Shade, 3,		{ 3.339844, 1, 1.003906, 0 } },
  { "SONY", "DSLR-A900", Cloudy, -3,		{ 2.546875, 1, 1.382813, 0 } },
  { "SONY", "DSLR-A900", Cloudy, 0,		{ 2.792969, 1, 1.242187, 0 } },
  { "SONY", "DSLR-A900", Cloudy, 3,		{ 3.054688, 1, 1.113281, 0 } },
  { "SONY", "DSLR-A900", Tungsten, -3,		{ 1.402344, 1, 2.707031, 0 } },
  { "SONY", "DSLR-A900", Tungsten, 0,		{ 1.546875, 1, 2.425781, 0 } },
  { "SONY", "DSLR-A900", Tungsten, 3,		{ 1.710938, 1, 2.179688, 0 } },
  { "SONY", "DSLR-A900", Fluorescent, -2,	{ 1.460938, 1, 2.933594, 0 } },
  { "SONY", "DSLR-A900", Fluorescent, 0,	{ 2.234375, 1, 1.882812, 0 } },
  { "SONY", "DSLR-A900", Fluorescent, 4,	{ 2.839844, 1, 1.230469, 0 } },
  { "SONY", "DSLR-A900", Flash, -3,		{ 2.656250, 1, 1.316406, 0 } },
  { "SONY", "DSLR-A900", Flash, 0,		{ 2.910156, 1, 1.183594, 0 } },
  { "SONY", "DSLR-A900", Flash, 3,		{ 3.183594, 1, 1.062500, 0 } },

  { "SONY", "NEX-3", Daylight, -3,		{ 2.0742, 1, 1.6289, 0 } },
  { "SONY", "NEX-3", Daylight, -2,		{ 2.1328, 1, 1.5742, 0 } },
  { "SONY", "NEX-3", Daylight, -1,		{ 2.1914, 1, 1.5195, 0 } },
  { "SONY", "NEX-3", Daylight, 0,		{ 2.2539, 1, 1.4727, 0 } },
  { "SONY", "NEX-3", Daylight, 1,		{ 2.3164, 1, 1.4219, 0 } },
  { "SONY", "NEX-3", Daylight, 2,		{ 2.3828, 1, 1.3750, 0 } },
  { "SONY", "NEX-3", Daylight, 3,		{ 2.4492, 1, 1.3281, 0 } },
  { "SONY", "NEX-3", Shade, -3,			{ 2.4180, 1, 1.3516, 0 } },
  { "SONY", "NEX-3", Shade, -2,			{ 2.4844, 1, 1.3047, 0 } },
  { "SONY", "NEX-3", Shade, -1,			{ 2.5508, 1, 1.2617, 0 } },
  { "SONY", "NEX-3", Shade, 0,			{ 2.6289, 1, 1.2188, 0 } },
  { "SONY", "NEX-3", Shade, 1,			{ 2.6992, 1, 1.1797, 0 } },
  { "SONY", "NEX-3", Shade, 2,			{ 2.7773, 1, 1.1445, 0 } },
  { "SONY", "NEX-3", Shade, 3,			{ 2.8555, 1, 1.1055, 0 } },
  { "SONY", "NEX-3", Cloudy, -3,		{ 2.2188, 1, 1.4961, 0 } },
  { "SONY", "NEX-3", Cloudy, -2,		{ 2.2813, 1, 1.4453, 0 } },
  { "SONY", "NEX-3", Cloudy, -1,		{ 2.3477, 1, 1.3984, 0 } },
  { "SONY", "NEX-3", Cloudy, 0,			{ 2.4141, 1, 1.3516, 0 } },
  { "SONY", "NEX-3", Cloudy, 1,			{ 2.4805, 1, 1.3086, 0 } },
  { "SONY", "NEX-3", Cloudy, 2,			{ 2.5508, 1, 1.2656, 0 } },
  { "SONY", "NEX-3", Cloudy, 3,			{ 2.6250, 1, 1.2227, 0 } },
  { "SONY", "NEX-3", Incandescent, -3,		{ 1.3164, 1, 2.8594, 0 } },
  { "SONY", "NEX-3", Incandescent, -2,		{ 1.3516, 1, 2.7617, 0 } },
  { "SONY", "NEX-3", Incandescent, -1,		{ 1.3906, 1, 2.6641, 0 } },
  { "SONY", "NEX-3", Incandescent, 0,		{ 1.4336, 1, 2.5742, 0 } },
  { "SONY", "NEX-3", Incandescent, 1,		{ 1.4766, 1, 2.4883, 0 } },
  { "SONY", "NEX-3", Incandescent, 2,		{ 1.5156, 1, 2.4023, 0 } },
  { "SONY", "NEX-3", Incandescent, 3,		{ 1.5586, 1, 2.3203, 0 } },
  { "SONY", "NEX-3", Fluorescent, -1,		{ 1.6250, 1, 2.6172, 0 } },
  { "SONY", "NEX-3", Fluorescent, 0,		{ 1.9883, 1, 2.0742, 0 } },
  { "SONY", "NEX-3", Fluorescent, 1,		{ 2.1875, 1, 1.5273, 0 } },
  { "SONY", "NEX-3", Fluorescent, 2,		{ 2.4531, 1, 1.3711, 0 } },
  { "SONY", "NEX-3", Flash, -3,			{ 2.5352, 1, 1.3633, 0 } },
  { "SONY", "NEX-3", Flash, -2,			{ 2.6016, 1, 1.3242, 0 } },
  { "SONY", "NEX-3", Flash, -1,			{ 2.6758, 1, 1.2773, 0 } },
  { "SONY", "NEX-3", Flash, 0,			{ 2.7500, 1, 1.2383, 0 } },
  { "SONY", "NEX-3", Flash, 1,			{ 2.8281, 1, 1.1953, 0 } },
  { "SONY", "NEX-3", Flash, 2,			{ 2.9063, 1, 1.1602, 0 } },
  { "SONY", "NEX-3", Flash, 3,			{ 2.9883, 1, 1.1211, 0 } },

  { "SONY", "NEX-5", Daylight, -3,		{ 2.1250, 1, 1.6055, 0 } },
  { "SONY", "NEX-5", Daylight, -2,		{ 2.1836, 1, 1.5508, 0 } },
  { "SONY", "NEX-5", Daylight, -1,		{ 2.2500, 1, 1.4961, 0 } },
  { "SONY", "NEX-5", Daylight, 0,		{ 2.3125, 1, 1.4453, 0 } },
  { "SONY", "NEX-5", Daylight, 1,		{ 2.3750, 1, 1.3984, 0 } },
  { "SONY", "NEX-5", Daylight, 2,		{ 2.4453, 1, 1.3477, 0 } },
  { "SONY", "NEX-5", Daylight, 3,		{ 2.5156, 1, 1.3008, 0 } },
  { "SONY", "NEX-5", Shade, -3,			{ 2.4805, 1, 1.3242, 0 } },
  { "SONY", "NEX-5", Shade, -2,			{ 2.5508, 1, 1.2773, 0 } },
  { "SONY", "NEX-5", Shade, -1,			{ 2.6211, 1, 1.2344, 0 } },
  { "SONY", "NEX-5", Shade, 0,			{ 2.6992, 1, 1.1914, 0 } },
  { "SONY", "NEX-5", Shade, 1,			{ 2.7734, 1, 1.1523, 0 } },
  { "SONY", "NEX-5", Shade, 2,			{ 2.8555, 1, 1.1133, 0 } },
  { "SONY", "NEX-5", Shade, 3,			{ 2.9336, 1, 1.0742, 0 } },
  { "SONY", "NEX-5", Cloudy, -3,		{ 2.2734, 1, 1.4727, 0 } },
  { "SONY", "NEX-5", Cloudy, -2,		{ 2.3398, 1, 1.4219, 0 } },
  { "SONY", "NEX-5", Cloudy, -1,		{ 2.4063, 1, 1.3711, 0 } },
  { "SONY", "NEX-5", Cloudy, 0,			{ 2.4766, 1, 1.3281, 0 } },
  { "SONY", "NEX-5", Cloudy, 1,			{ 2.5469, 1, 1.2813, 0 } },
  { "SONY", "NEX-5", Cloudy, 2,			{ 2.6211, 1, 1.2383, 0 } },
  { "SONY", "NEX-5", Cloudy, 3,			{ 2.6953, 1, 1.1953, 0 } },
  { "SONY", "NEX-5", Incandescent, -3,		{ 1.3398, 1, 2.8594, 0 } },
  { "SONY", "NEX-5", Incandescent, -2,		{ 1.3750, 1, 2.7617, 0 } },
  { "SONY", "NEX-5", Incandescent, -1,		{ 1.4180, 1, 2.6641, 0 } },
  { "SONY", "NEX-5", Incandescent, 0,		{ 1.4609, 1, 2.5703, 0 } },
  { "SONY", "NEX-5", Incandescent, 1,		{ 1.5039, 1, 2.4805, 0 } },
  { "SONY", "NEX-5", Incandescent, 2,		{ 1.5469, 1, 2.3945, 0 } },
  { "SONY", "NEX-5", Incandescent, 3,		{ 1.5898, 1, 2.3125, 0 } },
  { "SONY", "NEX-5", Fluorescent, -1,		{ 1.6563, 1, 2.6133, 0 } },
  { "SONY", "NEX-5", Fluorescent, 0,		{ 2.0352, 1, 2.0625, 0 } },
  { "SONY", "NEX-5", Fluorescent, 1,		{ 2.2422, 1, 1.5039, 0 } },
  { "SONY", "NEX-5", Fluorescent, 2,		{ 2.5195, 1, 1.3438, 0 } },
  { "SONY", "NEX-5", Flash, -3,			{ 2.6055, 1, 1.3398, 0 } },
  { "SONY", "NEX-5", Flash, -2,			{ 2.6719, 1, 1.2969, 0 } },
  { "SONY", "NEX-5", Flash, -1,			{ 2.7500, 1, 1.2500, 0 } },
  { "SONY", "NEX-5", Flash, 0,			{ 2.8281, 1, 1.2109, 0 } },
  { "SONY", "NEX-5", Flash, 1,			{ 2.9063, 1, 1.1641, 0 } },
  { "SONY", "NEX-5", Flash, 2,			{ 2.9883, 1, 1.1289, 0 } },
  { "SONY", "NEX-5", Flash, 3,			{ 3.0742, 1, 1.0938, 0 } },

  { "SONY", "SLT-A55V", Daylight, -3,		{ 2.3320, 1, 1.6758, 0 } },
  { "SONY", "SLT-A55V", Daylight, -2,		{ 2.3984, 1, 1.6172, 0 } },
  { "SONY", "SLT-A55V", Daylight, -1,		{ 2.4727, 1, 1.5625, 0 } },
  { "SONY", "SLT-A55V", Daylight, 0,		{ 2.5469, 1, 1.5117, 0 } },
  { "SONY", "SLT-A55V", Daylight, 1,		{ 2.6172, 1, 1.4609, 0 } },
  { "SONY", "SLT-A55V", Daylight, 2,		{ 2.6992, 1, 1.4102, 0 } },
  { "SONY", "SLT-A55V", Daylight, 3,		{ 2.7773, 1, 1.3633, 0 } },
  { "SONY", "SLT-A55V", Shade, -3,		{ 2.7422, 1, 1.3867, 0 } },
  { "SONY", "SLT-A55V", Shade, -2,		{ 2.8203, 1, 1.3398, 0 } },
  { "SONY", "SLT-A55V", Shade, -1,		{ 2.8984, 1, 1.2969, 0 } },
  { "SONY", "SLT-A55V", Shade, 0,		{ 2.9922, 1, 1.2500, 0 } },
  { "SONY", "SLT-A55V", Shade, 1,		{ 3.0781, 1, 1.2109, 0 } },
  { "SONY", "SLT-A55V", Shade, 2,		{ 3.1680, 1, 1.1719, 0 } },
  { "SONY", "SLT-A55V", Shade, 3,		{ 3.2578, 1, 1.1328, 0 } },
  { "SONY", "SLT-A55V", Cloudy, -3,		{ 2.5039, 1, 1.5352, 0 } },
  { "SONY", "SLT-A55V", Cloudy, -2,		{ 2.5781, 1, 1.4844, 0 } },
  { "SONY", "SLT-A55V", Cloudy, -1,		{ 2.6562, 1, 1.4375, 0 } },
  { "SONY", "SLT-A55V", Cloudy, 0,		{ 2.7344, 1, 1.3906, 0 } },
  { "SONY", "SLT-A55V", Cloudy, 1,		{ 2.8125, 1, 1.3437, 0 } },
  { "SONY", "SLT-A55V", Cloudy, 2,		{ 2.8984, 1, 1.2969, 0 } },
  { "SONY", "SLT-A55V", Cloudy, 3,		{ 2.9844, 1, 1.2539, 0 } },
  { "SONY", "SLT-A55V", Incandescent, -3,	{ 1.4297, 1, 2.9453, 0 } },
  { "SONY", "SLT-A55V", Incandescent, -2,	{ 1.4727, 1, 2.8477, 0 } },
  { "SONY", "SLT-A55V", Incandescent, -1,	{ 1.5234, 1, 2.7461, 0 } },
  { "SONY", "SLT-A55V", Incandescent, 0,	{ 1.5703, 1, 2.6523, 0 } },
  { "SONY", "SLT-A55V", Incandescent, 1,	{ 1.6172, 1, 2.5625, 0 } },
  { "SONY", "SLT-A55V", Incandescent, 2,	{ 1.6680, 1, 2.4727, 0 } },
  { "SONY", "SLT-A55V", Incandescent, 3,	{ 1.7148, 1, 2.3906, 0 } },
  { "SONY", "SLT-A55V", Fluorescent, -1,	{ 1.8008, 1, 2.6562, 0 } },
  { "SONY", "SLT-A55V", Fluorescent, 0,		{ 2.2305, 1, 2.1094, 0 } },
  { "SONY", "SLT-A55V", Fluorescent, 1,		{ 2.4648, 1, 1.5508, 0 } },
  { "SONY", "SLT-A55V", Fluorescent, 2,		{ 2.7969, 1, 1.3828, 0 } },
  { "SONY", "SLT-A55V", Flash, -3,		{ 2.6055, 1, 1.4805, 0 } },
  { "SONY", "SLT-A55V", Flash, -2,		{ 2.6797, 1, 1.4297, 0 } },
  { "SONY", "SLT-A55V", Flash, -1,		{ 2.7578, 1, 1.3828, 0 } },
  { "SONY", "SLT-A55V", Flash, 0,		{ 2.8437, 1, 1.3359, 0 } },
  { "SONY", "SLT-A55V", Flash, 1,		{ 2.9258, 1, 1.2930, 0 } },
  { "SONY", "SLT-A55V", Flash, 2,		{ 3.0156, 1, 1.2461, 0 } },
  { "SONY", "SLT-A55V", Flash, 3,		{ 3.1016, 1, 1.2070, 0 } },

  { "SONY", "SLT-A65V", Daylight, 0,		{ 2.628906, 1, 1.433594, 0 } },
  { "SONY", "SLT-A65V", Shade, 0,		{ 3.132813, 1, 1.191406, 0 } },
  { "SONY", "SLT-A65V", Cloudy, 0,		{ 2.839844, 1, 1.316406, 0 } },
  { "SONY", "SLT-A65V", Tungsten, 0,		{ 1.617188, 1, 2.488281, 0 } },
  { "SONY", "SLT-A65V", Fluorescent, 0,		{ 2.363281, 1, 1.980469, 0 } },
  { "SONY", "SLT-A65V", Flash, 0,		{ 3.070313, 1, 1.238281, 0 } },

};

const int wb_preset_count = sizeof(wb_preset) / sizeof(wb_data);
