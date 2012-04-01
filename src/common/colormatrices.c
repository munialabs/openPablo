/*
    This file is part of darktable,
    copyright (c) 2009--2010 johannes hanika.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    darktable is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with darktable.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * this is a collection of custom measured color matrices, profiled
 * for darktable (http://www.darktable.org), so far all calculated by Pascal de Bruijn.
 */
typedef struct dt_profiled_colormatrix_t
{
  const char *makermodel;
  int rXYZ[3], gXYZ[3], bXYZ[3], white[3];
}
dt_profiled_colormatrix_t;

// image submitter, chart type, illuminant, comments
static dt_profiled_colormatrix_t dt_profiled_colormatrices[] =
{

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "PENTAX K-x",                   { 821548, 337357,  42923}, { 247818, 1042969, -218735}, { -4105, -293045, 1085129}, {792206, 821823, 668640}},

  // Denis Cheremisov, CMP Digital Target 4, strobe, well lit
  { "PENTAX K-5",                   { 795456, 343674,  70389}, { 137650,  907654, -299805}, { 31097, -251328, 1054321}, {663452, 689972, 517853}},

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "PENTAX K-7",                   { 738541, 294037,  28061}, { 316025,  984482, -189682}, { 12543, -185852, 1075027}, {812683, 843994, 682587}},

  // Pascal de Bruijn, Homebrew ColorChecker, strobe, well lit (this is not a joke)
  { "PENTAX 645D",                  { 814209, 295822,  76019}, { 194641, 1101898, -541473}, { 83664, -313370, 1450531}, {740036, 767288, 629959}},

  // Sven Lindahl, Wolf Faust IT8, direct sunlight, well lit
  { "Canon EOS-1Ds Mark II",        {1078033, 378601, -31113}, { -15396, 1112045, -245743}, {166794, -252411, 1284531}, {681213, 705048, 590790}},

  // Xavier Besse, CMP Digital Target 3, direct sunlight, well lit
  { "Canon EOS 5D Mark II",         { 967590, 399139,  36026}, { -52094,  819046, -232071}, {144455, -143158, 1069305}, {864227, 899139, 741547}},

  // Deacon MacMillan, Kodak Q60 (IT8), strobe, well lit
  { "Canon EOS 5D",                 { 971420, 386429,   5753}, { 176849, 1141586, -137955}, { 81909, -284790, 1198090}, {753662, 783997, 645142}},

  // Alberto Ferrante, Wolf Faust IT8, direct sunlight, well lit
  { "Canon EOS 7D",                 { 977829, 294815, -44205}, { 154175, 1238007, -325684}, {103363, -297791, 1397461}, {707291, 741760, 626251}},

  // Wim Koorenneef, Wolf Faust IT8, direct sunlight, well lit
  { "Canon EOS 20D",                { 885468, 342117,  20798}, { 278702, 1194733, -164246}, { 42389, -302963, 1147125}, {741379, 771881, 664261}},

  // Martin Fahrendorf, Wolf Faust IT8, direct sunlight, well lit
  { "Canon EOS 30D",                { 955612, 353485, -33371}, { 220200, 1250488, -146393}, { 51956, -361450, 1201355}, {680405, 707977, 597366}},

  // Roy Niswanger, ColorChecker DC, direct sunlight, experimental
  // { "Canon EOS 30D",             { 840195, 148773, -67017}, { 112915, 1104553, -369720}, {240005,  -19562, 1468338}, {827255, 873337, 715317}},

  // Pascal de Bruijn, CMP Digital Target 3, strobe, well lit
  { "Canon EOS 40D",                { 845901, 325760, -13077}, { 110809,  960724, -213577}, { 82230, -218063, 1110229}, {837906, 868393, 705704}},

  // Pascal de Bruijn, CMP Digital Target 3, strobe, well lit
  { "Canon EOS 50D",                {1035110, 365005,  -8057}, {-192184,  930511, -477417}, {189545, -233353, 1360870}, {863983, 888763, 730026}},

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "Canon EOS 60D",                { 811844, 271149,  -2258}, { 233673, 1232880, -165558}, {  9354, -396515, 1055908}, {820908, 814270, 703735}},

  // Pascal de Bruijn, CMP Digital Target 3, strobe, well lit
  { "Canon EOS 350D DIGITAL",       { 784348, 329681, -18875}, { 227249, 1001602, -115692}, { 23834, -270844, 1011185}, {861252, 886368, 721420}},
  { "Canon EOS DIGITAL REBEL XT",   { 784348, 329681, -18875}, { 227249, 1001602, -115692}, { 23834, -270844, 1011185}, {861252, 886368, 721420}},
  { "Canon EOS Kiss Digital N",     { 784348, 329681, -18875}, { 227249, 1001602, -115692}, { 23834, -270844, 1011185}, {861252, 886368, 721420}},

  // Pascal de Bruijn, CMP Digital Target 3, strobe, well lit
  { "Canon EOS 400D DIGITAL",       { 743546, 283783, -16647}, { 256531, 1035355, -117432}, { 36560, -256836, 1013535}, {855698, 880066, 726181}},
  { "Canon EOS DIGITAL REBEL XTi",  { 743546, 283783, -16647}, { 256531, 1035355, -117432}, { 36560, -256836, 1013535}, {855698, 880066, 726181}},
  { "Canon EOS Kiss Digital X",     { 743546, 283783, -16647}, { 256531, 1035355, -117432}, { 36560, -256836, 1013535}, {855698, 880066, 726181}},

  // Pascal de Bruijn, CMP Digital Target 3, strobe, well lit
  { "Canon EOS 450D",               { 960098, 404968,  22842}, { -85114,  855072, -310928}, {159851, -194611, 1164276}, {851379, 871506, 711823}},
  { "Canon EOS DIGITAL REBEL XSi",  { 960098, 404968,  22842}, { -85114,  855072, -310928}, {159851, -194611, 1164276}, {851379, 871506, 711823}},
  { "Canon EOS Kiss Digital X2",    { 960098, 404968,  22842}, { -85114,  855072, -310928}, {159851, -194611, 1164276}, {851379, 871506, 711823}},

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "Canon EOS REBEL T1i",          { 956711, 314590,   1236}, {  27405, 1158569, -346283}, { 95444, -376572, 1260895}, {870087, 898087, 734146}},
  { "Canon EOS 500D",               { 956711, 314590,   1236}, {  27405, 1158569, -346283}, { 95444, -376572, 1260895}, {870087, 898087, 734146}},
  { "Canon EOS Kiss Digital X3",    { 956711, 314590,   1236}, {  27405, 1158569, -346283}, { 95444, -376572, 1260895}, {870087, 898087, 734146}},

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "Canon EOS REBEL T2i",          { 864960, 319305,  36880}, { 160904, 1113586, -251587}, { 68832, -334290, 1143463}, {848404, 883118, 718628}},
  { "Canon EOS 550D",               { 864960, 319305,  36880}, { 160904, 1113586, -251587}, { 68832, -334290, 1143463}, {848404, 883118, 718628}},
  { "Canon EOS Kiss Digital X4",    { 864960, 319305,  36880}, { 160904, 1113586, -251587}, { 68832, -334290, 1143463}, {848404, 883118, 718628}},

  // M. Emre Meydan, Wolf Faust IT8, direct sunlight, well lit
  { "Canon EOS REBEL T3i",          { 998352, 349960,  -2716}, {  48340, 1270676, -315140}, {114716, -360596, 1265518}, {671249, 670547, 606339}},
  { "Canon EOS 600D",               { 998352, 349960,  -2716}, {  48340, 1270676, -315140}, {114716, -360596, 1265518}, {671249, 670547, 606339}},
  { "Canon EOS Kiss Digital X5",    { 998352, 349960,  -2716}, {  48340, 1270676, -315140}, {114716, -360596, 1265518}, {671249, 670547, 606339}},

  // M. Emre Meydan, Wolf Faust IT8, direct sunlight, well lit
  { "Canon EOS DIGITAL REBEL XS",   { 875580, 325546,   -912}, { 298859, 1301361, -153580}, { 26108, -378876, 1150177}, {675369, 697647, 606659}},
  { "Canon EOS 1000D",              { 875580, 325546,   -912}, { 298859, 1301361, -153580}, { 26108, -378876, 1150177}, {675369, 697647, 606659}},
  { "Canon EOS Kiss F",             { 875580, 325546,   -912}, { 298859, 1301361, -153580}, { 26108, -378876, 1150177}, {675369, 697647, 606659}},

  // Artis Rozentals, Wolf Faust IT8, direct sunlight, well lit
  { "Canon PowerShot S60",          { 879990, 321808,  23041}, { 272324, 1104752, -410950}, { 75500, -184097, 1373230}, {702026, 740524, 622131}},

  // Pascal de Bruijn, CMP Digital Target 3, camera strobe, well lit
  { "Canon PowerShot S90",          { 866531, 231995,  55756}, {  76965, 1067474, -461502}, {106369, -243286, 1314529}, {807449, 855270, 690750}},

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "Canon PowerShot G12",          { 738434, 188904,  71182}, { 318008, 1222260, -338455}, { 13290, -324036, 1207855}, {803146, 841522, 676529}},

  // Henrik Andersson, Homebrew ColorChecker, strobe, well lit
  { "NIKON D40X",                   { 801178, 365555,  13702}, { 276398,  988342,  -84167}, { 21378, -264755, 1052521}, {859116, 893936, 739807}},

  // Henrik Andersson, Homebrew ColorChecker, strobe, well lit
  { "NIKON D60",                    { 746475, 318924,   9277}, { 254776,  946991, -130447}, { 63171, -166458, 1029190}, {753220, 787949, 652695}},

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "NIKON D3000",                  { 778854, 333221,  21927}, { 292007, 1031448,  -88516}, { 27664, -245956,  997391}, {714828, 740387, 601334}},

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "NIKON D3100",                  { 856476, 350891,  48691}, { 221741, 1049164, -218933}, { 12115, -297424, 1083755}, {807373, 841156, 682846}},

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "NIKON D5000",                  { 852386, 356232,  42389}, { 205353, 1026688, -220184}, {  6348, -292526, 1083313}, {822647, 849106, 688538}},

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "NIKON D7000",                  { 744919, 228027, -46982}, { 454605, 1326797,  -33585}, {-132294, -467194, 985611}, {609375, 629852, 515625}},

  // Henrik Andersson, Homebrew ColorChecker, strobe, well lit
  { "NIKON D90",                    { 855072, 361176,  22751}, { 177414,  963577, -241501}, { 28931, -229019, 1123062}, {751816, 781677, 650024}},

  // Rolf Steinort, Wolf Faust IT8, direct sunlight, well lit
  { "NIKON D200",                   { 878922, 352966,   2914}, { 273575, 1048141, -116302}, { 61661, -171021, 1126297}, {691483, 727142, 615204}},

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "NIKON D300s",                  { 813202, 327667,  31067}, { 248810, 1047043, -203049}, { -1160, -284607, 1075790}, {774872, 800415, 648727}},

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "NIKON D700",                   { 789261, 332016,  34149}, { 270386,  985748, -129135}, {  4074, -230209,  999008}, {798172, 826721, 673126}},

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "NIKON COOLPIX P7000",          { 804947, 229630,  97717}, { 178146, 1138763, -395233}, { 88699, -282013, 1234650}, {809998, 842819, 682144}},

  // Karl Mikaelsson, Homebrew ColorChecker, strobe, well lit
  { "SONY DSLR-A100",               { 823853, 374588,  28259}, { 220200,  934509, -108643}, { 48141, -226440, 1062881}, {689651, 715225, 602127}},

  // Alexander Rabtchevich, Wolf Faust IT8, direct sunlight, well lit
  { "SONY DSLR-A200",               { 846786, 366302, -22858}, { 311584, 1046249, -107056}, { 54596, -192993, 1191406}, {708405, 744507, 596771}},

  // Stephane Chauveau, Wolf Faust IT8, direct sunlight, well lit
  { "SONY DSLR-A550",               {1031235, 405899,   1572}, { 185623, 1122162, -272659}, {-25528, -329514, 1249969}, {729797, 753586, 633530}},

  // Karl Mikaelsson, Homebrew ColorChecker, strobe, well lit
  { "SONY DSLR-A700",               { 895737, 374771, -10330}, { 251389, 1076294, -176910}, {-33203, -356445, 1182465}, {742783, 773407, 637604}},

  // Alexander Rabtchevich, Wolf Faust IT8, direct sunlight, well lit
  { "SONY DSLR-A850",               { 968216, 463638,  -4883}, { 279083, 1156906, -230194}, {-21851, -379623, 1297455}, {749298, 799271, 638580}},

  // Copied from A850
  { "SONY DSLR-A900",               { 968216, 463638,  -4883}, { 279083, 1156906, -230194}, {-21851, -379623, 1297455}, {749298, 799271, 638580}},

  // David Meier, Wolf Faust IT8, direct sunlight, well lit
  { "SONY SLT-A55V",                { 969696, 407043,  40268}, { 218201, 1182556, -285400}, { 21042, -342819, 1260223}, {762085, 793961, 670151}},

  // Denis Cheremisov, CMP Digital Target 4, strobe, well lit
  { "SONY NEX-5N",                  { 913406, 394043,   3237}, { 206253, 1085022,  -19917}, {-69138, -377472, 1038483}, {800079, 824112, 674850}},

  // Mark Haun, Wolf Faust IT8, direct sunlight, well lit
  { "OLYMPUS E-PL1",                { 824387, 288086,  -7355}, { 299500, 1148865, -308929}, { 91858, -198425, 1346603}, {720139, 750717, 619751}},

  // Eugene Kraf, Wolf Faust IT8, direct sunlight, well lit
  { "OLYMPUS E-PL2",                { 785522, 280624,  28503}, { 322266, 1211975, -305984}, { 82550, -246841, 1278198}, {731506, 752808, 645309}},

  // Karl Mikaelsson, Homebrew ColorChecker, strobe, well lit
  { "OLYMPUS E-500",                { 925171, 247681,  26367}, { 257187, 1270187, -455826}, {-87784, -426529, 1383041}, {790421, 812775, 708054}},

  // Henrik Andersson, Homebrew ColorChecker, camera strobe, well lit
  { "OLYMPUS SP570UZ",              { 780991, 262283,  27969}, { 147522, 1135239, -422974}, {142731, -293610, 1316803}, {769669, 804474, 676895}},

  // Robert Park, ColorChecker Passport, camera strobe, well lit
  { "Panasonic DMC-FZ40",           { 833542, 259720,  35721}, { 129517, 1239594, -525848}, {117340, -405273, 1440384}, {825226, 863846, 688431}},

  // Robert Park, ColorChecker Passport, camera strobe, well lit
  { "Panasonic DMC-FZ100",          { 700119, 181885, -50354}, { 355804, 1326492, -441132}, {   244, -424149, 1415451}, {734222, 767410, 619049}},

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "Panasonic DMC-G1",             { 747467, 300064,  74265}, { 225922, 1028946, -310913}, { 91782, -229019, 1153793}, {846222, 864502, 694458}},

  // Deacon MacMillan, Kodak Q60 (IT8), strobe, well lit
  { "Panasonic DMC-GF1",            { 802048, 330963,   7477}, { 194519, 968170,  -270004}, { 47211, -246552, 1177536}, {719223, 750900, 614120}},

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "Panasonic DMC-G2",             { 753250, 303024,  75287}, { 225540, 1036041, -320923}, { 90927, -233749, 1170151}, {837860, 857056, 687210}},

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "Panasonic DMC-LX3",            { 779907, 298859,  94101}, { 239655, 1167938, -489197}, { 53589, -371368, 1317261}, {796707, 825119, 668030}},

  // Robert Park, ColorChecker Passport, strobe, well lit
  { "Panasonic DMC-LX5",            { 845215, 228226,  59219}, { 190109, 1297211, -543121}, { 42511, -433456, 1414032}, {761322, 790985, 642044}},

  // Pascal de Bruijn, CMP Digital Target 3, strobe (PSEF15A), well lit
  { "SAMSUNG NX100",                { 859955, 369919,  17136}, { 127045,  869888, -258362}, { 69351, -149155, 1121475}, {854538, 897888, 691147}},

  // Copied from NX100
  { "SAMSUNG NX5",                  { 859955, 369919,  17136}, { 127045,  869888, -258362}, { 69351, -149155, 1121475}, {854538, 897888, 691147}},
  { "SAMSUNG NX10",                 { 859955, 369919,  17136}, { 127045,  869888, -258362}, { 69351, -149155, 1121475}, {854538, 897888, 691147}},
  { "SAMSUNG NX11",                 { 859955, 369919,  17136}, { 127045,  869888, -258362}, { 69351, -149155, 1121475}, {854538, 897888, 691147}},

  // Pieter de Boer, CMP Digital Target 3, camera strobe, well lit
  { "KODAK EASYSHARE Z1015 IS",     { 716446, 157928, -39536}, { 288498, 1234573, -412460}, { 43045, -337677, 1385773}, {774048, 823563, 644012}},

  // Rolf Steinort, Wolf Faust IT8, direct sunlight, well lit
  { "FUJIFILM FinePix X100",        { 734619, 274628,  -6302}, { 325272, 1076035, -198608}, {-15366, -280670, 1061050}, {637207, 668228, 578690}},

  // Oleg Dzhimiev, ColorChecker Classic, office lighting, well lit
  { "Elphel 353E",                  {782623, 147903, -272369}, { 110016, 1115250, -729172}, {175949, -157227, 1930222}, {821899, 860794, 671768}}
};

static const int dt_profiled_colormatrix_cnt = sizeof(dt_profiled_colormatrices)/sizeof(dt_profiled_colormatrix_t);

