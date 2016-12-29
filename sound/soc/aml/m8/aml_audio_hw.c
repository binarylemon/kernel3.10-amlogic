#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <mach/am_regs.h>
#include <linux/clk.h>
#include <linux/module.h>
#if MESON_CPU_TYPE >= MESON_CPU_TYPE_MESON6
//#include <mach/utils.h>
#endif

#include "aml_audio_hw.h"

#ifndef MREG_AIU_958_chstat0
#define AIU_958_chstat0	AIU_958_CHSTAT_L0
#endif

#ifndef MREG_AIU_958_chstat1
#define AIU_958_chstat1	AIU_958_CHSTAT_L1
#endif


unsigned ENABLE_IEC958 = 1;
unsigned IEC958_MODE   = AIU_958_MODE_PCM16;
unsigned I2S_MODE      = AIU_I2S_MODE_PCM16;


int  audio_in_buf_ready = 0;
int audio_out_buf_ready = 0;

unsigned int IEC958_bpf = 0x7dd;
unsigned int IEC958_brst = 0xc;
unsigned int IEC958_length = 0x7dd*8;
unsigned int IEC958_padsize = 0x8000;
unsigned int IEC958_mode = 1;
unsigned int IEC958_syncword1 = 0x7ffe;
unsigned int IEC958_syncword2 = 0x8001;
unsigned int IEC958_syncword3 = 0;
unsigned int IEC958_syncword1_mask = 0;
unsigned int IEC958_syncword2_mask = 0;
unsigned int IEC958_syncword3_mask = 0xffff;
unsigned int IEC958_chstat0_l = 0x1902 ;
unsigned int IEC958_chstat0_r = 0x1902 ;
unsigned int IEC958_chstat1_l = 0x200;
unsigned int IEC958_chstat1_r = 0x200;
unsigned int IEC958_mode_raw = 0;
/*
 0 --  other formats except(DD,DD+,DTS)
 1 --  DTS
 2 --  DD
 3 -- DTS with 958 PCM RAW package mode
 4 -- DD+
*/
unsigned int IEC958_mode_codec;
/*
bit 0:soc in slave mode for adc;
bit 1:audio in data source from spdif in;
bit 2:adc & spdif in work at the same time;
*/
unsigned audioin_mode = I2SIN_MASTER_MODE;

EXPORT_SYMBOL(IEC958_bpf);
EXPORT_SYMBOL(IEC958_brst);
EXPORT_SYMBOL(IEC958_length);
EXPORT_SYMBOL(IEC958_padsize);
EXPORT_SYMBOL(IEC958_mode);
EXPORT_SYMBOL(IEC958_syncword1);
EXPORT_SYMBOL(IEC958_syncword2);
EXPORT_SYMBOL(IEC958_syncword3);
EXPORT_SYMBOL(IEC958_syncword1_mask);
EXPORT_SYMBOL(IEC958_syncword2_mask);
EXPORT_SYMBOL(IEC958_syncword3_mask);
EXPORT_SYMBOL(IEC958_chstat0_l);
EXPORT_SYMBOL(IEC958_chstat0_r);
EXPORT_SYMBOL(IEC958_chstat1_l);
EXPORT_SYMBOL(IEC958_chstat1_r);
EXPORT_SYMBOL(IEC958_mode_raw);
EXPORT_SYMBOL(IEC958_mode_codec);

// Bit 3:  mute constant
//         0 => 'h0000000
//         1 => 'h800000
unsigned int dac_mute_const = 0x800000;

static int i2s_mpll_reg;
static int i2s_clk_src;
static int i2s_freq_index;
static int i2s_fs_config;
/*
                                fIn * (M)
            Fout   =  -----------------------------
                      		(N) * (OD+1) * (XD)
*/
#define MCLK_BASE_INDEX 50
static unsigned int mclk_tune_table[13][101] = {
	{//AUDIO_CLK_FREQ_192
		49180930,
		49180351,
		49179772,
		49179193,
		49178614,
		49178035,
		49177457,
		49176878,
		49176299,
		49175720,
		49175141,
		49174562,
		49173984,
		49173405,
		49172826,
		49172247,
		49171669,
		49171090,
		49170511,
		49169933,
		49169354,
		49168775,
		49168197,
		49167618,
		49167039,
		49166461,
		49165882,
		49165304,
		49164725,
		49164146,
		49163568,
		49162989,
		49162411,
		49161832,
		49161254,
		49160675,
		49160097,
		49159519,
		49158940,
		49158362,
		49157783,
		49157205,
		49156627,
		49156048,
		49155470,
		49154891,
		49154313,
		49153735,
		49153157,
		49152578,
		49152000,
		49151422,
		49150844,
		49150265,
		49149687,
		49149109,
		49148531,
		49147953,
		49147374,
		49146796,
		49146218,
		49145640,
		49145062,
		49144484,
		49143906,
		49143328,
		49142750,
		49142172,
		49141594,
		49141016,
		49140438,
		49139860,
		49139282,
		49138704,
		49138126,
		49137548,
		49136970,
		49136392,
		49135814,
		49135236,
		49134658,
		49134081,
		49133503,
		49132925,
		49132347,
		49131769,
		49131191,
		49130614,
		49130036,
		49129458,
		49128881,
		49128303,
		49127725,
		49127147,
		49126570,
		49125992,
		49125414,
		49124837,
		49124259,
		49123682,
		49123104,
	},
	{//AUDIO_CLK_FREQ_1764
	},
	{//AUDIO_CLK_FREQ_96
		24590465, //0
		24590176,
		24589886,
		24589597,
		24589307,
		24589018,
		24588728,
		24588439,
		24588149,
		24587860,
		24587571,//10
		24587281,
		24586992,
		24586702,
		24586413,
		24586124,
		24585834,
		24585545,
		24585256,
		24584966,
		24584677,//20
		24584388,
		24584098,
		24583809,
		24583520,
		24583230,
		24582941,
		24582652,
		24582362,
		24582073,
		24581784,//30
		24581495,
		24581205,
		24580916,
		24580627,
		24580338,
		24580048,
		24579759,
		24579470,
		24579181,
		24578892,//40
		24578602,
		24578313,
		24578024,
		24577735,
		24577446,
		24577157,
		24576867,
		24576578,
		24576289,
		24576000, //50 base
		24575711,
		24575422,
		24575133,
		24574844,
		24574554,
		24574265,
		24573976,
		24573687,
		24573398,
		24573109,//60
		24572820,
		24572531,
		24572242,
		24571953,
		24571664,
		24571375,
		24571086,
		24570797,
		24570508,
		24570219,//70
		24569930,
		24569641,
		24569352,
		24569063,
		24568774,
		24568485,
		24568196,
		24567907,
		24567618,
		24567329,//80
		24567040,
		24566751,
		24566462,
		24566174,
		24565885,
		24565596,
		24565307,
		24565018,
		24564729,
		24564440,//90
		24564151,
		24563863,
		24563574,
		24563285,
		24562996,
		24562707,
		24562418,
		24562130,
		24561841,
		24561552,//100
	},
	{//AUDIO_CLK_FREQ_882
		22592462,
		22592193,
		22591924,
		22591655,
		22591387,
		22591118,
		22590849,
		22590580,
		22590312,
		22590043,
		22589774,
		22589505,
		22589237,
		22588968,
		22588699,
		22588431,
		22588162,
		22587893,
		22587625,
		22587356,
		22587087,
		22586819,
		22586550,
		22586281,
		22586013,
		22585744,
		22585476,
		22585207,
		22584938,
		22584670,
		22584401,
		22584133,
		22583864,
		22583595,
		22583327,
		22583058,
		22582790,
		22582521,
		22582253,
		22581984,
		22581716,
		22581447,
		22581179,
		22580910,
		22580642,
		22580373,
		22580105,
		22579836,
		22579568,
		22579299,
		22579031,
		22578762,
		22578494,
		22578225,
		22577957,
		22577689,
		22577420,
		22577152,
		22576883,
		22576615,
		22576347,
		22576078,
		22575810,
		22575541,
		22575273,
		22575005,
		22574736,
		22574468,
		22574200,
		22573931,
		22573663,
		22573395,
		22573126,
		22572858,
		22572590,
		22572321,
		22572053,
		22571785,
		22571517,
		22571248,
		22570980,
		22570712,
		22570443,
		22570175,
		22569907,
		22569639,
		22569371,
		22569102,
		22568834,
		22568566,
		22568298,
		22568029,
		22567761,
		22567493,
		22567225,
		22566957,
		22566689,
		22566420,
		22566152,
		22565884,
		22565616,
	},	
	{//AUDIO_CLK_FREQ_48
		12297042,
		12296861,
		12296680,
		12296499,
		12296318,
		12296137,
		12295956,
		12295775,
		12295594,
		12295413,
		12295232,
		12295052,
		12294871,
		12294690,
		12294509,
		12294328,
		12294147,
		12293966,
		12293785,
		12293604,
		12293424,
		12293243,
		12293062,
		12292881,
		12292700,
		12292519,
		12292338,
		12292158,
		12291977,
		12291796,
		12291615,
		12291434,
		12291254,
		12291073,
		12290892,
		12290711,
		12290530,
		12290350,
		12290169,
		12289988,
		12289807,
		12289627,
		12289446,
		12289265,
		12289084,
		12288904,
		12288723,
		12288542,
		12288361,
		12288181,
		12288000,
		12287819,
		12287639,
		12287458,
		12287277,
		12287097,
		12286916,
		12286735,
		12286555,
		12286374,
		12286193,
		12286013,
		12285832,
		12285651,
		12285471,
		12285290,
		12285109,
		12284929,
		12284748,
		12284568,
		12284387,
		12284206,
		12284026,
		12283845,
		12283665,
		12283484,
		12283303,
		12283123,
		12282942,
		12282762,
		12282581,
		12282401,
		12282220,
		12282040,
		12281859,
		12281679,
		12281498,
		12281318,
		12281137,
		12280957,
		12280776,
		12280596,
		12280415,
		12280235,
		12280054,
		12279874,
		12279693,
		12279513,
		12279332,
		12279152,
		12278971,
	},
	{//AUDIO_CLK_FREQ_441
		11295705,
		11295583,
		11295461,
		11295339,
		11295217,
		11295095,
		11294973,
		11294850,
		11294728,
		11294606,
		11294484,
		11294362,
		11294240,
		11294118,
		11293996,
		11293873,
		11293751,
		11293629,
		11293507,
		11293385,
		11293263,
		11293141,
		11293019,
		11292897,
		11292774,
		11292652,
		11292530,
		11292408,
		11292286,
		11292164,
		11292042,
		11291920,
		11291798,
		11291676,
		11291554,
		11291432,
		11291309,
		11291187,
		11291065,
		11290943,
		11290821,
		11290699,
		11290577,
		11290455,
		11290333,
		11290211,
		11290089,
		11289967,
		11289845,
		11289723,
		11289601,
		11289479,
		11289357,
		11289235,
		11289113,
		11288991,
		11288869,
		11288747,
		11288625,
		11288503,
		11288381,
		11288259,
		11288137,
		11288015,
		11287893,
		11287771,
		11287649,
		11287527,
		11287405,
		11287283,
		11287161,
		11287039,
		11286917,
		11286795,
		11286673,
		11286551,
		11286429,
		11286307,
		11286185,
		11286063,
		11285941,
		11285819,
		11285697,
		11285575,
		11285453,
		11285331,
		11285210,
		11285088,
		11284966,
		11284844,
		11284722,
		11284600,
		11284478,
		11284356,
		11284234,
		11284112,
		11283990,
		11283868,
		11283747,
		11283625,
		11283503,
	},
	{//AUDIO_CLK_FREQ_32
		8196822,
		8196725,
		8196629,
		8196532,
		8196436,
		8196339,
		8196243,
		8196146,
		8196050,
		8195953,
		8195857,
		8195760,
		8195664,
		8195567,
		8195471,
		8195375,
		8195278,
		8195182,
		8195085,
		8194989,
		8194892,
		8194796,
		8194699,
		8194603,
		8194507,
		8194410,
		8194314,
		8194217,
		8194121,
		8194024,
		8193928,
		8193832,
		8193735,
		8193639,
		8193542,
		8193446,
		8193349,
		8193253,
		8193157,
		8193060,
		8192964,
		8192867,
		8192771,
		8192675,
		8192578,
		8192482,
		8192386,
		8192289,
		8192193,
		8192096,
		8192000,
		8191904,
		8191807,
		8191711,
		8191615,
		8191518,
		8191422,
		8191325,
		8191229,
		8191133,
		8191036,
		8190940,
		8190844,
		8190747,
		8190651,
		8190555,
		8190458,
		8190362,
		8190266,
		8190169,
		8190073,
		8189977,
		8189880,
		8189784,
		8189688,
		8189591,
		8189495,
		8189399,
		8189302,
		8189206,
		8189110,
		8189013,
		8188917,
		8188821,
		8188725,
		8188628,
		8188532,
		8188436,
		8188339,
		8188243,
		8188147,
		8188050,
		8187954,
		8187858,
		8187762,
		8187665,
		8187569,
		8187473,
		8187377,
		8187280,
		8187184,
	},
	{//AUDIO_CLK_FREQ_8
		2612220,
		2612200,
		2612180,
		2612159,
		2612139,
		2612118,
		2612098,
		2612077,
		2612057,
		2612037,
		2612016,
		2611996,
		2611975,
		2611955,
		2611935,
		2611914,
		2611894,
		2611873,
		2611853,
		2611833,
		2611812,
		2611792,
		2611771,
		2611751,
		2611731,
		2611710,
		2611690,
		2611669,
		2611649,
		2611628,
		2611608,
		2611588,
		2611567,
		2611547,
		2611526,
		2611506,
		2611486,
		2611465,
		2611445,
		2611424,
		2611404,
		2611384,
		2611363,
		2611343,
		2611322,
		2611302,
		2611282,
		2611261,
		2611241,
		2611220,
		2611200,
		2611180,
		2611159,
		2611139,
		2611118,
		2611098,
		2611078,
		2611057,
		2611037,
		2611016,
		2610996,
		2610976,
		2610955,
		2610935,
		2610914,
		2610894,
		2610874,
		2610853,
		2610833,
		2610812,
		2610792,
		2610772,
		2610751,
		2610731,
		2610710,
		2610690,
		2610670,
		2610649,
		2610629,
		2610609,
		2610588,
		2610568,
		2610547,
		2610527,
		2610507,
		2610486,
		2610466,
		2610445,
		2610425,
		2610405,
		2610384,
		2610364,
		2610343,
		2610323,
		2610303,
		2610282,
		2610262,
		2610242,
		2610221,
		2610201,
		2610180,
	},
	{//AUDIO_CLK_FREQ_11
		3600359,
		3600323,
		3600287,
		3600251,
		3600215,
		3600179,
		3600143,
		3600107,
		3600071,
		3600035,
		3599999,
		3599963,
		3599927,
		3599891,
		3599855,
		3599819,
		3599783,
		3599747,
		3599711,
		3599675,
		3599639,
		3599603,
		3599567,
		3599531,
		3599495,
		3599460,
		3599424,
		3599388,
		3599352,
		3599316,
		3599280,
		3599244,
		3599208,
		3599172,
		3599136,
		3599100,
		3599064,
		3599028,
		3598992,
		3598956,
		3598920,
		3598884,
		3598848,
		3598812,
		3598776,
		3598740,
		3598704,
		3598668,
		3598632,
		3598596,
		3598560,
		3598524,
		3598489,
		3598453,
		3598417,
		3598381,
		3598345,
		3598309,
		3598273,
		3598237,
		3598201,
		3598165,
		3598129,
		3598093,
		3598057,
		3598021,
		3597985,
		3597949,
		3597913,
		3597877,
		3597841,
		3597806,
		3597770,
		3597734,
		3597698,
		3597662,
		3597626,
		3597590,
		3597554,
		3597518,
		3597482,
		3597446,
		3597410,
		3597374,
		3597338,
		3597302,
		3597267,
		3597231,
		3597195,
		3597159,
		3597123,
		3597087,
		3597051,
		3597015,
		3596979,
		3596943,
		3596907,
		3596871,
		3596835,
		3596799,
		3596764,
	},
	{//AUDIO_CLK_FREQ_12
		3918380,
		3918348,
		3918317,
		3918285,
		3918253,
		3918222,
		3918190,
		3918159,
		3918127,
		3918095,
		3918064,
		3918032,
		3918001,
		3917969,
		3917937,
		3917906,
		3917874,
		3917843,
		3917811,
		3917779,
		3917748,
		3917716,
		3917685,
		3917653,
		3917621,
		3917590,
		3917558,
		3917527,
		3917495,
		3917464,
		3917432,
		3917400,
		3917369,
		3917337,
		3917306,
		3917274,
		3917242,
		3917211,
		3917179,
		3917148,
		3917116,
		3917084,
		3917053,
		3917021,
		3916990,
		3916958,
		3916927,
		3916895,
		3916863,
		3916832,
		3916800,
		3916769,
		3916737,
		3916706,
		3916674,
		3916642,
		3916611,
		3916579,
		3916548,
		3916516,
		3916484,
		3916453,
		3916421,
		3916390,
		3916358,
		3916327,
		3916295,
		3916263,
		3916232,
		3916200,
		3916169,
		3916137,
		3916106,
		3916074,
		3916042,
		3916011,
		3915979,
		3915948,
		3915916,
		3915885,
		3915853,
		3915822,
		3915790,
		3915758,
		3915727,
		3915695,
		3915664,
		3915632,
		3915601,
		3915569,
		3915537,
		3915506,
		3915474,
		3915443,
		3915411,
		3915380,
		3915348,
		3915317,
		3915285,
		3915253,
		3915222,
	},
	{//AUDIO_CLK_FREQ_16
		5225666,
		5225601,
		5225535,
		5225470,
		5225405,
		5225339,
		5225274,
		5225209,
		5225143,
		5225078,
		5225013,
		5224947,
		5224882,
		5224816,
		5224751,
		5224686,
		5224620,
		5224555,
		5224490,
		5224424,
		5224359,
		5224294,
		5224228,
		5224163,
		5224098,
		5224033,
		5223967,
		5223902,
		5223837,
		5223771,
		5223706,
		5223641,
		5223575,
		5223510,
		5223445,
		5223379,
		5223314,
		5223249,
		5223183,
		5223118,
		5223053,
		5222988,
		5222922,
		5222857,
		5222792,
		5222726,
		5222661,
		5222596,
		5222531,
		5222465,
		5222400,
		5222335,
		5222269,
		5222204,
		5222139,
		5222074,
		5222008,
		5221943,
		5221878,
		5221813,
		5221747,
		5221682,
		5221617,
		5221551,
		5221486,
		5221421,
		5221356,
		5221290,
		5221225,
		5221160,
		5221095,
		5221029,
		5220964,
		5220899,
		5220834,
		5220769,
		5220703,
		5220638,
		5220573,
		5220508,
		5220442,
		5220377,
		5220312,
		5220247,
		5220181,
		5220116,
		5220051,
		5219986,
		5219921,
		5219855,
		5219790,
		5219725,
		5219660,
		5219594,
		5219529,
		5219464,
		5219399,
		5219334,
		5219268,
		5219203,
		5219138,
	},
	{//AUDIO_CLK_FREQ_22
		7202518,
		7202410,
		7202302,
		7202194,
		7202086,
		7201978,
		7201870,
		7201762,
		7201654,
		7201546,
		7201438,
		7201330,
		7201222,
		7201114,
		7201006,
		7200898,
		7200790,
		7200682,
		7200574,
		7200466,
		7200358,
		7200250,
		7200142,
		7200034,
		7199926,
		7199818,
		7199710,
		7199603,
		7199495,
		7199387,
		7199279,
		7199171,
		7199063,
		7198955,
		7198847,
		7198739,
		7198631,
		7198523,
		7198415,
		7198308,
		7198200,
		7198092,
		7197984,
		7197876,
		7197768,
		7197660,
		7197552,
		7197444,
		7197337,
		7197229,
		7197121,
		7197013,
		7196905,
		7196797,
		7196689,
		7196582,
		7196474,
		7196366,
		7196258,
		7196150,
		7196042,
		7195935,
		7195827,
		7195719,
		7195611,
		7195503,
		7195395,
		7195288,
		7195180,
		7195072,
		7194964,
		7194856,
		7194749,
		7194641,
		7194533,
		7194425,
		7194317,
		7194210,
		7194102,
		7193994,
		7193886,
		7193779,
		7193671,
		7193563,
		7193455,
		7193348,
		7193240,
		7193132,
		7193024,
		7192917,
		7192809,
		7192701,
		7192593,
		7192486,
		7192378,
		7192270,
		7192162,
		7192055,
		7191947,
		7191839,
		7191732,
	},
	{//AUDIO_CLK_FREQ_24
		7836760,
		7836696,
		7836633,
		7836570,
		7836507,
		7836444,
		7836380,
		7836317,
		7836254,
		7836191,
		7836128,
		7836064,
		7836001,
		7835938,
		7835875,
		7835812,
		7835748,
		7835685,
		7835622,
		7835559,
		7835496,
		7835433,
		7835369,
		7835306,
		7835243,
		7835180,
		7835117,
		7835053,
		7834990,
		7834927,
		7834864,
		7834801,
		7834738,
		7834674,
		7834611,
		7834548,
		7834485,
		7834422,
		7834358,
		7834295,
		7834232,
		7834169,
		7834106,
		7834043,
		7833979,
		7833916,
		7833853,
		7833790,
		7833727,
		7833664,
		7833600,
		7833537,
		7833474,
		7833411,
		7833348,
		7833285,
		7833222,
		7833158,
		7833095,
		7833032,
		7832969,
		7832906,
		7832843,
		7832780,
		7832716,
		7832653,
		7832590,
		7832527,
		7832464,
		7832401,
		7832338,
		7832274,
		7832211,
		7832148,
		7832085,
		7832022,
		7831959,
		7831896,
		7831832,
		7831769,
		7831706,
		7831643,
		7831580,
		7831517,
		7831454,
		7831391,
		7831327,
		7831264,
		7831201,
		7831138,
		7831075,
		7831012,
		7830949,
		7830886,
		7830823,
		7830759,
		7830696,
		7830633,
		7830570,
		7830507,
		7830444,
	},
}; 
#if MESON_CPU_TYPE >= MESON_CPU_TYPE_MESON6
int audio_clock_config_table[][13][2]=
{
	/*{HIU Reg , XD - 1)
	   //7.875k, 8K, 11.025k, 12k, 16k, 22.05k, 24k, 32k, 44.1k, 48k, 88.2k, 96k, 192k
	*/
	{
	//256
#if OVERCLOCK == 0
#if MESON_CPU_TYPE >= MESON_CPU_TYPE_MESON8	
		{0x0005cc08, (60-1)}, // 32
		{0x0005e965, (40-1)}, //44.1	
		{0x0004c9a0,	(50-1)},	//48K
#else		
		{0x0005cc08, (30-1)}, // 32
		{0x0004cdf3, (42-1)},  // 44.1
		{0x0007c4e6, (23-1)},  // 48
#endif	
		//{0x0006d0a4, (13-1)},  // 96
        {0x0005cc08,  (20-1)},// 96k ,24.576M
		//{0x0004e15a, (9 -1)},   // 192
        {0x0005cc08,    (10-1)},   // 192k, 49.152M
		{0x0007f400, (125-1)}, // 8k
		{0x0006c6f6, (116-1)}, // 11.025
		{0x0007e47f, (86-1)},  // 12
		{0x0004f880, (100-1)}, // 16
		{0x0004c4a4, (87-1)},  // 22.05
		{0x0007e47f, (43-1)},  // 24
		{0x0007f3f0, (127-1)}, // 7875
        {0x0005c88b, (22-1)}, // 88.2k ,22.579M
#else
	//512FS
		{0x0004f880, (25-1)},  // 32
		{0x0004cdf3, (21-1)},  // 44.1
		{0x0006d0a4, (13-1)},  // 48
		{0x0004e15a, (9 -1)},  // 96
		{0x0006f207, (3 -1)},   // 192
		{0x0004f880, (100-1)}, // 8k
		{0x0004c4a4, (87-1)}, // 11.025
		{0x0007e47f, (43-1)},  // 12
		{0x0004f880, (50-1)}, // 16
		{0x0004cdf3, (42-1)},  // 22.05
		{0x0007c4e6, (23-1)},  // 24
		{0x0006e1b6, (76-1)}, // 7875
#endif
	},
	{
	//384
		{0x0007c4e6, (23-1)},  // 32
		{0x0004c4a4, (29-1)},  // 44.1
		{0x0004cb18, (26-1)},  // 48
		{0x0004cb18, (13-1)},  // 96
		{0x0004e15a, (6 -1)},   // 192
		{0x0007e47f, (86-1)},  // 8k
		{0x0007efa5, (61-1)},  // 11.025
		{0x0006de98, (67-1)},  // 12
		{0x0007e47f, (43-1)},  // 16
		{0x0004c4a4, (58-1)},  // 22.05
		{0x0004c60e, (53-1)},  // 24
		{0x0007fdfa, (83-1)},  // 7875
	}
};
#else
int audio_clock_config_table[][11][2]=
{
  // 128*Fs
  //
	/*{M, N, OD, XD-1*/
	{
	//24M
        {(64<<0) | (3<<9) | (0<<14) , (125-1)}, // 32K, 4.096M
#if OVERCLOCK==0
        {(147<<0) | (5<<9) | (0<<14) , (125-1)}, // 44.1K, 5.6448M
        {(32<<0) | (1<<9) | (0<<14) , (125-1)}, // 48K, 6.144M
#else
        {(143<<0) | (8<<9) | (0<<14) , (19-1)}, // 44.1K, 5.6448M*4=22.5792M
        {(128<<0) | (5<<9) | (0<<14) , (25-1)}, // 48K, 6.144M*4=24.576M
#endif
        {(128<<0) | (5<<9) | (1<<14) , (25-1)}, // 96K, 12.288M
        {(128<<0) | (5<<9) | (0<<14) , (25-1)}, //192K, 24.576M
        {(64<<0) | (3<<9) | (1<<14) , (250-1)}, // 8K, 1.024M
        {(147<<0) | (5<<9) | (1<<14) , (250-1)}, //11.025K,1.4112M
        {(32<<0) | (1<<9) | (1<<14) , (250-1)}, // 12K, 1.536M
        {(64<<0) | (3<<9) | (1<<14) , (125-1)}, // 16K, 2.048M
        {(147<<0) | (5<<9) | (1<<14) , (125-1)}, //22.050K, 2.8224M
        {(32<<0) | (1<<9) | (1<<14) , (125-1)}, // 24K, 3.072M
	},
	{
	//25M
        {(29<<0) | (1<<9) | (0<<14) , (177-1)}, // 32K, 4.096M
#if OVERCLOCK==0
        {(21<<0) | (1<<9) | (0<<14) , (93-1)}, // 44.1K, 5.6448M
        {(29<<0) | (1<<9) | (1<<14) , (59-1)}, // 48K, 6.144M
#else
        {(28<<0) | (1<<9) | (0<<14) , (31-1)}, // 44.1K, 5.6448M*4=22.5792M
        {(173<<0) | (8<<9) | (1<<14) , (11-1)}, // 48K, 6.144M*4=24.576M
#endif
        {(29<<0) | (1<<9) | (0<<14) , (59-1)}, // 96K, 12.288M
        {(173<<0) | (8<<9) | (1<<14) , (11-1)}, //192K, 24.576M
        {(58<<0) | (3<<9) | (1<<14) , (236-1)}, // 8K, 1.024M
        {(162<<0) | (7<<9) | (1<<14) , (205-1)}, //11.025K,1.4112M
        {(29<<0) | (1<<9) | (1<<14) , (236-1)}, // 12K, 1.536M
        {(29<<0) | (1<<9) | (1<<14) , (177-1)}, // 16K, 2.048M
        {(162<<0) | (7<<9) | (0<<14) , (205-1)}, //22.050K, 2.8224M
        {(29<<0) | (1<<9) | (1<<14) , (118-1)}, // 24K, 3.072M
	}
};
#endif


void audio_set_aiubuf(u32 addr, u32 size, unsigned int channel)
{
    WRITE_MPEG_REG(AIU_MEM_I2S_START_PTR, addr & 0xffffffc0);
    WRITE_MPEG_REG(AIU_MEM_I2S_RD_PTR, addr & 0xffffffc0);
    if(channel == 8){
        WRITE_MPEG_REG_BITS(AIU_CLK_CTRL_MORE, 1, 6, 1);
        WRITE_MPEG_REG(AIU_MEM_I2S_END_PTR, (addr & 0xffffffc0) + (size & 0xffffffc0) - 256); 
    }else{
        WRITE_MPEG_REG_BITS(AIU_CLK_CTRL_MORE, 0, 6, 1);
        WRITE_MPEG_REG(AIU_MEM_I2S_END_PTR, (addr & 0xffffffc0) + (size & 0xffffffc0) - 64);   //this is for 16bit 2 channel
    }

    WRITE_MPEG_REG(AIU_I2S_MISC,		0x0004);	// Hold I2S
	WRITE_MPEG_REG(AIU_I2S_MUTE_SWAP,	0x0000);	// No mute, no swap
	// As the default amclk is 24.576MHz, set i2s and iec958 divisor appropriately so as not to exceed the maximum sample rate.
	WRITE_MPEG_REG(AIU_I2S_MISC,		0x0010 );	// Release hold and force audio data to left or right

	if(channel == 8){
		printk(" %s channel == 8\n",__FUNCTION__);
	WRITE_MPEG_REG(AIU_MEM_I2S_MASKS,		(24 << 16) |	// [31:16] IRQ block.
								(0xff << 8) |	// [15: 8] chan_mem_mask. Each bit indicates which channels exist in memory
								(0xff << 0));	// [ 7: 0] chan_rd_mask.  Each bit indicates which channels are READ from memory
		}
	else 
	WRITE_MPEG_REG(AIU_MEM_I2S_MASKS,		(24 << 16) |	// [31:16] IRQ block.
								(0x3 << 8) |	// [15: 8] chan_mem_mask. Each bit indicates which channels exist in memory
								(0x3 << 0));	// [ 7: 0] chan_rd_mask.  Each bit indicates which channels are READ from memory

    // 16 bit PCM mode
    //  WRITE_MPEG_REG_BITS(AIU_MEM_I2S_CONTROL, 1, 6, 1);
	// Set init high then low to initilize the I2S memory logic
	WRITE_MPEG_REG_BITS(AIU_MEM_I2S_CONTROL, 1, 0, 1 );
	WRITE_MPEG_REG_BITS(AIU_MEM_I2S_CONTROL, 0, 0, 1 );

	WRITE_MPEG_REG(AIU_MEM_I2S_BUF_CNTL, 1 | (0 << 1));
	WRITE_MPEG_REG(AIU_MEM_I2S_BUF_CNTL, 0 | (0 << 1));

    audio_out_buf_ready = 1;
}

void audio_set_958outbuf(u32 addr, u32 size,int flag)
{
    if (ENABLE_IEC958) {
        WRITE_MPEG_REG(AIU_MEM_IEC958_START_PTR, addr & 0xffffffc0);
	  	if(READ_MPEG_REG(AIU_MEM_IEC958_START_PTR) == READ_MPEG_REG(AIU_MEM_I2S_START_PTR)){
			WRITE_MPEG_REG(AIU_MEM_IEC958_RD_PTR, READ_MPEG_REG(AIU_MEM_I2S_RD_PTR));
		}
		else
        WRITE_MPEG_REG(AIU_MEM_IEC958_RD_PTR, addr & 0xffffffc0);
        if(flag == 0){
          WRITE_MPEG_REG(AIU_MEM_IEC958_END_PTR, (addr & 0xffffffc0) + (size & 0xffffffc0) - 64);    // this is for 16bit 2 channel
        }else{
          WRITE_MPEG_REG(AIU_MEM_IEC958_END_PTR, (addr & 0xffffffc0) + (size & 0xffffffc0) - 1);    // this is for RAW mode
        }
        WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_MASKS, 0x303, 0, 16);

        WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 1, 0, 1);
        WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 0, 0, 1);

        WRITE_MPEG_REG(AIU_MEM_IEC958_BUF_CNTL, 1 | (0 << 1));
        WRITE_MPEG_REG(AIU_MEM_IEC958_BUF_CNTL, 0 | (0 << 1));
    }
}
/*
i2s mode 0: master 1: slave
*/
static void i2sin_fifo0_set_buf(u32 addr, u32 size,u32 i2s_mode,u32 i2s_sync)
{
	unsigned char  mode = 0;
    unsigned int sync_mode = 0;
    if(i2s_sync)
        sync_mode = i2s_sync;
	if(i2s_mode &I2SIN_SLAVE_MODE)
		mode = 1;
	WRITE_MPEG_REG(AUDIN_FIFO0_START, addr & 0xffffffc0);
	WRITE_MPEG_REG(AUDIN_FIFO0_PTR, (addr&0xffffffc0));
	WRITE_MPEG_REG(AUDIN_FIFO0_END, (addr&0xffffffc0) + (size&0xffffffc0)-8);

	WRITE_MPEG_REG(AUDIN_FIFO0_CTRL, (1<<AUDIN_FIFO0_EN)	// FIFO0_EN
    								|(1<<AUDIN_FIFO0_LOAD)	// load start address./* AUDIN_FIFO0_LOAD */
								|(1<<AUDIN_FIFO0_DIN_SEL)	// DIN from i2sin./* AUDIN_FIFO0_DIN_SEL */
	    							//|(1<<6)	// 32 bits data in./*AUDIN_FIFO0_D32b */
									//|(0<<7)	// put the 24bits data to  low 24 bits./* AUDIN_FIFO0_h24b */16bit
								|(4<<AUDIN_FIFO0_ENDIAN)	// /*AUDIN_FIFO0_ENDIAN */
								|(2<<AUDIN_FIFO0_CHAN)//2 channel./* AUDIN_FIFO0_CHAN*/
		    						|(0<<16)	//to DDR
                                                       |(1<<AUDIN_FIFO0_UG)    // Urgent request.  DDR SDRAM urgent request enable.
                                                       |(0<<17)    // Overflow Interrupt mask
                                                       |(0<<18)    // Audio in INT
			                                	//|(1<<19)	//hold 0 enable
								|(0<<AUDIN_FIFO0_UG)	// hold0 to aififo
				  );

    WRITE_MPEG_REG(AUDIN_FIFO0_CTRL1,    0 << 4                       // fifo0_dest_sel
                                       | 2 << 2                       // fifo0_din_byte_num
                                       | 0 << 0);                      // fifo0_din_pos


	WRITE_MPEG_REG(AUDIN_I2SIN_CTRL, //(0<<I2SIN_SIZE)			///*bit8*/  16bit
									 (3<<I2SIN_SIZE)
									|(1<<I2SIN_CHAN_EN)		/*bit10~13*/ //2 channel
#if MESON_CPU_TYPE >= MESON_CPU_TYPE_MESON6TV
									|(sync_mode<<I2SIN_POS_SYNC)
#else
									|(1<<I2SIN_POS_SYNC)
#endif
									|(1<<I2SIN_LRCLK_SKEW)
                                    				|(1<<I2SIN_LRCLK_INVT)
									|(!mode<<I2SIN_CLK_SEL)
									|(!mode<<I2SIN_LRCLK_SEL)
				    				|(!mode<<I2SIN_DIR)
				  );

}

static void spdifin_reg_set(void)
{
	struct clk* clk_src = clk_get_sys("clk81", NULL);  // get clk81 clk_rate
	u32 clk_rate = clk_get_rate(clk_src);
	u32 spdif_clk_time = 54;   // 54us
	u32 spdif_mode_14bit = ((clk_rate /500000 +1 )>>1)* spdif_clk_time; // the reg spdif_mode(0x2800)last 14 bit
	u32 period_data = (clk_rate/64000 + 1 ) >> 1 ;   // sysclk/32(bit)/2(ch)/2(bmc)
	u32 period_32k = (period_data + (1<<4)) >> 5;     // 32k min period
	u32 period_44k = (period_data / 22 + 1) >> 1;   // 44k min period
	u32 period_48k = (period_data / 24 + 1) >> 1;   // 48k min period
	u32 period_96k = (period_data / 48 + 1) >> 1;   // 96k min period
	u32 period_192k = (period_data / 96 + 1) >> 1;  // 192k min period
	
	printk(KERN_INFO"spdifin_reg_set: clk_rate=%d\n", clk_rate);
		
	WRITE_MPEG_REG(AUDIN_SPDIF_MODE, (READ_MPEG_REG(AUDIN_SPDIF_MODE)&0x7fffc000)|(spdif_mode_14bit<<0));
	WRITE_MPEG_REG(AUDIN_SPDIF_FS_CLK_RLTN, (period_32k<<0)|(period_44k<<6)|(period_48k<<12) 
											|(period_96k<<18)|(period_192k<<24));  //Spdif_fs_clk_rltn
	
}

static void spdifin_fifo1_set_buf(u32 addr, u32 size)
{
	WRITE_MPEG_REG(AUDIN_SPDIF_MODE, READ_MPEG_REG(AUDIN_SPDIF_MODE)&0x7fffffff);
	WRITE_MPEG_REG(AUDIN_FIFO1_START, addr & 0xffffffc0);
	WRITE_MPEG_REG(AUDIN_FIFO1_PTR, (addr&0xffffffc0));
	WRITE_MPEG_REG(AUDIN_FIFO1_END, (addr&0xffffffc0) + (size&0xffffffc0)-8);
	WRITE_MPEG_REG(AUDIN_FIFO1_CTRL, (1<<AUDIN_FIFO1_EN)	// FIFO0_EN
    								|(1<<AUDIN_FIFO1_LOAD)	// load start address./* AUDIN_FIFO0_LOAD */
								|(0<<AUDIN_FIFO1_DIN_SEL)	// DIN from i2sin./* AUDIN_FIFO0_DIN_SEL */
	    							//|(1<<6)	// 32 bits data in./*AUDIN_FIFO0_D32b */
									//|(0<<7)	// put the 24bits data to  low 24 bits./* AUDIN_FIFO0_h24b */16bit
								|(4<<AUDIN_FIFO1_ENDIAN)	// /*AUDIN_FIFO0_ENDIAN */
								|(2<<AUDIN_FIFO1_CHAN)//2 channel./* AUDIN_FIFO0_CHAN*/
		    						|(0<<16)	//to DDR
                                                       |(1<<AUDIN_FIFO1_UG)    // Urgent request.  DDR SDRAM urgent request enable.
                                                       |(0<<17)    // Overflow Interrupt mask
                                                       |(0<<18)    // Audio in INT
			                                	//|(1<<19)	//hold 0 enable
								|(0<<AUDIN_FIFO1_UG)	// hold0 to aififo
				  );

	// according clk81 to set reg spdif_mode(0x2800) the last 14 bit and reg Spdif_fs_clk_rltn(0x2801)
	spdifin_reg_set();

	WRITE_MPEG_REG(AUDIN_FIFO1_CTRL1,0xc);
}
void audio_in_i2s_set_buf(u32 addr, u32 size,u32 i2s_mode, u32 i2s_sync)
{
	printk("i2sin_fifo0_set_buf \n");		
	i2sin_fifo0_set_buf(addr,size,i2s_mode,i2s_sync);
	audio_in_buf_ready = 1;
}
void audio_in_spdif_set_buf(u32 addr, u32 size)
{
	printk("spdifin_fifo1_set_buf \n");			
	spdifin_fifo1_set_buf(addr,size);
	
}
//extern void audio_in_enabled(int flag);

void audio_in_i2s_enable(int flag)
{
  	int rd = 0, start=0;
	if(flag){
          /* reset only when start i2s input */
reset_again:
	     WRITE_MPEG_REG_BITS(AUDIN_FIFO0_CTRL, 1, 1, 1); // reset FIFO 0
            WRITE_MPEG_REG(AUDIN_FIFO0_PTR, 0);
            rd = READ_MPEG_REG(AUDIN_FIFO0_PTR);
            start = READ_MPEG_REG(AUDIN_FIFO0_START);
            if(rd != start){
              printk("error %08x, %08x !!!!!!!!!!!!!!!!!!!!!!!!\n", rd, start);
              goto reset_again;
            }
			WRITE_MPEG_REG_BITS(AUDIN_I2SIN_CTRL, 1, I2SIN_EN, 1);

	}else{
			WRITE_MPEG_REG_BITS(AUDIN_I2SIN_CTRL, 0, I2SIN_EN, 1);
	}
}


void audio_in_spdif_enable(int flag)
{
  int rd = 0, start=0;

	if(flag){
reset_again:
	     WRITE_MPEG_REG_BITS(AUDIN_FIFO1_CTRL, 1, 1, 1); // reset FIFO 0
            WRITE_MPEG_REG(AUDIN_FIFO1_PTR, 0);
            rd = READ_MPEG_REG(AUDIN_FIFO1_PTR);
            start = READ_MPEG_REG(AUDIN_FIFO1_START);
            if(rd != start){
              printk("error %08x, %08x !!!!!!!!!!!!!!!!!!!!!!!!\n", rd, start);
              goto reset_again;
            }
		WRITE_MPEG_REG(AUDIN_SPDIF_MODE, READ_MPEG_REG(AUDIN_SPDIF_MODE)| (1<<31));		
	}else{
		WRITE_MPEG_REG(AUDIN_SPDIF_MODE, READ_MPEG_REG(AUDIN_SPDIF_MODE)& ~(1<<31));				
	}
}
int if_audio_in_i2s_enable()
{
	return READ_MPEG_REG_BITS(AUDIN_I2SIN_CTRL, I2SIN_EN, 1);
}
int if_audio_in_spdif_enable()
{
	return READ_MPEG_REG_BITS(AUDIN_SPDIF_MODE, 31, 1);
}
unsigned int audio_in_i2s_rd_ptr(void)
{
	unsigned int val;
	val = READ_MPEG_REG(AUDIN_FIFO0_RDPTR);
	printk("audio in i2s rd ptr: %x\n", val);
	return val;
}
unsigned int audio_in_spdif_rd_ptr(void)
{
	unsigned int val;
	val = READ_MPEG_REG(AUDIN_FIFO1_RDPTR);
	printk("audio in spdif rd ptr: %x\n", val);
	return val;
}
unsigned int audio_in_i2s_wr_ptr(void)
{
	unsigned int val;
      WRITE_MPEG_REG(AUDIN_FIFO0_PTR, 1);
	val = READ_MPEG_REG(AUDIN_FIFO0_PTR);
	return (val)&(~0x3F);
	//return val&(~0x7);
}
unsigned int audio_in_spdif_wr_ptr(void)
{
	unsigned int val;
      WRITE_MPEG_REG(AUDIN_FIFO1_PTR, 1);
	val = READ_MPEG_REG(AUDIN_FIFO1_PTR);
	return (val)&(~0x3F);
}
void audio_in_i2s_set_wrptr(unsigned int val)
{
	WRITE_MPEG_REG(AUDIN_FIFO0_RDPTR, val);
}
void audio_in_spdif_set_wrptr(unsigned int val)
{
	WRITE_MPEG_REG(AUDIN_FIFO1_RDPTR, val);
}
void audio_set_i2s_mode(u32 mode)
{
    const unsigned short mask[4] = {
        0x303,                  /* 2x16 */
        0x303,                  /* 2x24 */
        0x303,                 /* 8x24 */
        0x303,                  /* 2x32 */
    };

    if (mode < sizeof(mask)/ sizeof(unsigned short)) {
       /* four two channels stream */
        WRITE_MPEG_REG(AIU_I2S_SOURCE_DESC, 1);

        if (mode == AIU_I2S_MODE_PCM16) {
            WRITE_MPEG_REG_BITS(AIU_MEM_I2S_CONTROL, 1, 6, 1);
            WRITE_MPEG_REG_BITS(AIU_I2S_SOURCE_DESC, 0, 5, 1);
        } else if(mode == AIU_I2S_MODE_PCM32){
            WRITE_MPEG_REG_BITS(AIU_MEM_I2S_CONTROL, 0, 6, 1);
            WRITE_MPEG_REG_BITS(AIU_I2S_SOURCE_DESC, 1, 5, 1);
        }else if(mode == AIU_I2S_MODE_PCM24){
            WRITE_MPEG_REG_BITS(AIU_MEM_I2S_CONTROL, 0, 6, 1);
            WRITE_MPEG_REG_BITS(AIU_I2S_SOURCE_DESC, 1, 5, 1);
        }

        WRITE_MPEG_REG_BITS(AIU_MEM_I2S_MASKS, mask[mode], 0, 16);

        //WRITE_MPEG_REG_BITS(AIU_MEM_I2S_CONTROL, 1, 0, 1);
        //WRITE_MPEG_REG_BITS(AIU_MEM_I2S_CONTROL, 0, 0, 1);

        if (ENABLE_IEC958) {
       //     WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_MASKS, mask[mode], 0,
             //                   16);
            //WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 1, 0, 1);
            //WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 0, 0, 1);
        }
    }
}

/**
 *  if normal clock, i2s clock is twice of 958 clock, so the divisor for i2s is 8, but 4 for 958
 *  if over clock, the devisor for i2s is 8, but for 958 should be 1, because 958 should be 4 times speed according to i2s
 *  This is dolby digital plus's spec
 * */

void audio_util_set_dac_format(unsigned format)
{
  	WRITE_MPEG_REG(AIU_CLK_CTRL,		 (0 << 12) | // 958 divisor more, if true, divided by 2, 4, 6, 8.
							(1 <<  8) | // alrclk skew: 1=alrclk transitions on the cycle before msb is sent
							(1 <<  6) | // invert aoclk
							(1 <<  7) | // invert lrclk
#if OVERCLOCK == 1
							(3 <<  4) | // 958 divisor: 0=no div; 1=div by 2; 2=div by 3; 3=div by 4.
							(3 <<  2) | // i2s divisor: 0=no div; 1=div by 2; 2=div by 4; 3=div by 8.
#else
							(1 <<  4) | // 958 divisor: 0=no div; 1=div by 2; 2=div by 3; 3=div by 4.
							(2 <<  2) | // i2s divisor: 0=no div; 1=div by 2; 2=div by 4; 3=div by 8.
#endif
							(1 <<  1) | // enable 958 clock
							(1 <<  0)); // enable I2S clock
    if (format == AUDIO_ALGOUT_DAC_FORMAT_DSP) {
        WRITE_MPEG_REG_BITS(AIU_CLK_CTRL, 1, 8, 2);
    } else if (format == AUDIO_ALGOUT_DAC_FORMAT_LEFT_JUSTIFY) {
        WRITE_MPEG_REG_BITS(AIU_CLK_CTRL, 0, 8, 2);
    }
 	if(dac_mute_const == 0x800000)
    	WRITE_MPEG_REG(AIU_I2S_DAC_CFG, 	0x000f);	// Payload 24-bit, Msb first, alrclk = aoclk/64.mute const 0x800000
    else
    	WRITE_MPEG_REG(AIU_I2S_DAC_CFG, 	0x0007);	// Payload 24-bit, Msb first, alrclk = aoclk/64
	WRITE_MPEG_REG(AIU_I2S_SOURCE_DESC, 0x0001);	// four 2-channel
}

// iec958 and i2s clock are separated after M6TV.
void audio_util_set_dac_958_format(unsigned format)
{
	WRITE_MPEG_REG_BITS(AIU_CLK_CTRL,0,12,1);// 958 divisor more, if true, divided by 2, 4, 6, 8
#if IEC958_OVERCLOCK == 1
	WRITE_MPEG_REG_BITS(AIU_CLK_CTRL,3,4,2);// 958 divisor: 0=no div; 1=div by 2; 2=div by 3; 3=div by 4.
#else
	WRITE_MPEG_REG_BITS(AIU_CLK_CTRL,1,4,2);// 958 divisor: 0=no div; 1=div by 2; 2=div by 3; 3=div by 4.
#endif
	WRITE_MPEG_REG_BITS(AIU_CLK_CTRL,1,1,1);// enable 958 clock
}

void audio_util_set_dac_i2s_format(unsigned format)
{	
	WRITE_MPEG_REG_BITS(AIU_CLK_CTRL,1,6,1);//invert aoclk
	WRITE_MPEG_REG_BITS(AIU_CLK_CTRL,1,7,1);//invert lrclk
	WRITE_MPEG_REG_BITS(AIU_CLK_CTRL,1,8,2);// alrclk skew: 1=alrclk transitions on the cycle before msb is sent
#if OVERCLOCK == 1
	WRITE_MPEG_REG_BITS(AIU_CLK_CTRL,3,2,2);// i2s divisor: 0=no div; 1=div by 2; 2=div by 4; 3=div by 8.
#else
	WRITE_MPEG_REG_BITS(AIU_CLK_CTRL,2,2,2); // i2s divisor: 0=no div; 1=div by 2; 2=div by 4; 3=div by 8.
#endif
	WRITE_MPEG_REG_BITS(AIU_CLK_CTRL,1,0,1);// enable I2S clock
	
    if (format == AUDIO_ALGOUT_DAC_FORMAT_DSP) {
        WRITE_MPEG_REG_BITS(AIU_CLK_CTRL, 1, 8, 2);
    } else if (format == AUDIO_ALGOUT_DAC_FORMAT_LEFT_JUSTIFY) {
        WRITE_MPEG_REG_BITS(AIU_CLK_CTRL, 0, 8, 2);
    }
 	if(dac_mute_const == 0x800000)
    	WRITE_MPEG_REG(AIU_I2S_DAC_CFG, 	0x000f);	// Payload 24-bit, Msb first, alrclk = aoclk/64.mute const 0x800000
    else
    	WRITE_MPEG_REG(AIU_I2S_DAC_CFG, 	0x0007);	// Payload 24-bit, Msb first, alrclk = aoclk/64
	WRITE_MPEG_REG(AIU_I2S_SOURCE_DESC, 0x0001);	// four 2-channel	
}

enum clk_enum
{
	CLK_NONE = 0,
	CLK_MPLL0,
	CLK_MPLL1,
	CLK_MPLL2
};

// iec958 and i2s clock are separated after M6TV. 
void audio_set_i2s_clk(unsigned freq, unsigned fs_config, unsigned mpll)
{
    int i, index = 0, xtal = 0;
    int mpll_reg = 0, clk_src = 0;
    int (*audio_clock_config)[2];
    switch (mpll){
    case 0:
        mpll_reg = HHI_MPLL_CNTL7;
        clk_src = CLK_MPLL0;

        break;
    case 1:
        mpll_reg = HHI_MPLL_CNTL8;
        clk_src = CLK_MPLL1;
        break;
    case 2:
        mpll_reg = HHI_MPLL_CNTL9;
        clk_src = CLK_MPLL2;
        break;
    default:
        BUG();
    }
    i2s_mpll_reg = mpll_reg;
    printk("%s,i2s_mpll_reg:0x%x\n",__func__,i2s_mpll_reg);
    i2s_clk_src = clk_src;
    switch(freq)
    {
        case AUDIO_CLK_FREQ_192:
            index=4;
            break;
        case AUDIO_CLK_FREQ_96:
            index=3;
            break;
        case AUDIO_CLK_FREQ_48:
            index=2;
            break;
        case AUDIO_CLK_FREQ_441:
            index=1;
            break;
        case AUDIO_CLK_FREQ_32:
            index=0;
            break;
        case AUDIO_CLK_FREQ_8:
            index = 5;
            break;
        case AUDIO_CLK_FREQ_11:
            index = 6;
            break;
        case AUDIO_CLK_FREQ_12:
            index = 7;
            break;
        case AUDIO_CLK_FREQ_16:
            index = 8;
            break;
        case AUDIO_CLK_FREQ_22:
            index = 9;
            break;
        case AUDIO_CLK_FREQ_24:
            index = 10;
            break;
        case AUDIO_CLK_FREQ_882:
            index = 12;
            break;
        default:
            index=0;
            break;
    };
    i2s_freq_index = freq;
    if (fs_config == AUDIO_CLK_256FS) {
        // divide 256
        xtal = 0;
    }
    else if (fs_config == AUDIO_CLK_384FS) {
        // divide 384
        xtal = 1;
    }
    i2s_fs_config = fs_config;
    audio_clock_config = audio_clock_config_table[xtal];

    // gate the clock off
    WRITE_MPEG_REG( HHI_AUD_CLK_CNTL, READ_MPEG_REG(HHI_AUD_CLK_CNTL) & ~(1 << 8));
	WRITE_MPEG_REG(AIU_CLK_CTRL_MORE, 0);
	
	//Set filter register
	//WRITE_MPEG_REG(HHI_MPLL_CNTL3, 0x26e1250);

	/*--- DAC clock  configuration--- */
	// Disable mclk
    WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL, 0, 8, 1);
	// Select clk source, 0=none; 1=Multi-Phase PLL0; 2=Multi-Phase PLL1; 3=Multi-Phase PLL2.
	WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL, clk_src, 9, 2);

	// Configure Multi-Phase PLLX
	WRITE_MPEG_REG(mpll_reg, audio_clock_config[index][0]);
	// Set the XD value
	WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL, audio_clock_config[index][1], 0, 8);

	WRITE_MPEG_REG_BITS(AIU_CODEC_DAC_LRCLK_CTRL, 64-1, 0, 12);//set codec dac ratio---lrclk--64fs
	
	// delay 5uS
	//udelay(5);
	for (i = 0; i < 500000; i++) ;
	// gate the clock on
	WRITE_MPEG_REG( HHI_AUD_CLK_CNTL, READ_MPEG_REG(HHI_AUD_CLK_CNTL) | (1 << 8));

	//Audio DAC Clock enable
	WRITE_MPEG_REG(HHI_AUD_CLK_CNTL, READ_MPEG_REG(HHI_AUD_CLK_CNTL) |(1<<23));
	
	/* ---ADC clock  configuration--- */
	// Disable mclk
	WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL, 0, 8, 1);
    // Set pll over mclk ratio
    //we want 256fs ADC MLCK,so for over clock mode,divide more 2 than I2S  DAC CLOCK
#if OVERCLOCK == 0
	WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL, audio_clock_config[index][1], 0, 8);
#else
	WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL, (audio_clock_config[index][1]+1)*2-1, 0, 8);
#endif

    // Set mclk over sclk ratio
    WRITE_MPEG_REG_BITS(AIU_CLK_CTRL_MORE, 4-1, 8, 6);

    // Set sclk over lrclk ratio
    WRITE_MPEG_REG_BITS(AIU_CODEC_ADC_LRCLK_CTRL, 64-1, 0, 12); //set codec adc ratio---lrclk--64fs

    // Enable sclk
    WRITE_MPEG_REG_BITS(AIU_CLK_CTRL_MORE, 1, 14, 1);
    // Enable mclk
    WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL, 1, 8, 1);

    // delay 2uS
	//udelay(2);
	for (i = 0; i < 200000; i++) ;
}
/****** 0:up, 1:down***********/
unsigned int audio_set_i2s_mclk_tune(int up_down,unsigned ppm_value)
{
    int i;
    unsigned int mpll_reg_value = 0;
    unsigned int mpll_reg_pre_value = 0;
    unsigned int mpll_reg_value_offset = 0;
    unsigned int audio_xd = 0;
    unsigned int sdmin_value = 0;
    unsigned int set_sdmin_value = 0;
    unsigned int n2_integer = 0;
    unsigned int ppm_accuracy = 0;
    unsigned int set_mclk_value = 0;
    static unsigned long long mclk_offset_value = 0;
    audio_xd =((READ_MPEG_REG(HHI_AUD_CLK_CNTL) & 0xff) +1);
    mpll_reg_value = READ_MPEG_REG(i2s_mpll_reg);    
    mpll_reg_pre_value = mpll_reg_value;
    if(i2s_fs_config != AUDIO_CLK_256FS){
    	printk("%s, i2s_fs_config is not 256fs, return !!!\n",__func__);
	return 0;
    }
    switch(i2s_freq_index)
    {
        case AUDIO_CLK_FREQ_192:
	    mpll_reg_value = 0x5cc08;
	    ppm_accuracy = 1449; //ppm accuracy : 14.49
            break;
        case AUDIO_CLK_FREQ_96:
	    mpll_reg_value = 0x5cc08;
	    ppm_accuracy = 1234; //ppm accuracy : 12.34
	    break;
        case AUDIO_CLK_FREQ_48:
	    mpll_reg_value = 0x4c9a0;
	    ppm_accuracy = 1449; //ppm accuracy : 14.49
            break;
        case AUDIO_CLK_FREQ_441:
	    mpll_reg_value = 0x5e965;
	    ppm_accuracy = 1064; //ppm accuracy : 10.64
            break;
        case AUDIO_CLK_FREQ_32:
            mpll_reg_value = 0x5cc08;
	    ppm_accuracy = 1163; //ppm accuracy : 11.63
	    break;
        case AUDIO_CLK_FREQ_8:
            mpll_reg_value = 0x7f400;
	    ppm_accuracy = 775; //ppm accuracy : 7.75
	    break;
        case AUDIO_CLK_FREQ_11:
            mpll_reg_value = 0x6c6f6;
	    ppm_accuracy = 990; //ppm accuracy : 9.9
	    break;
        case AUDIO_CLK_FREQ_12:
            mpll_reg_value = 0x7e47f;
	    ppm_accuracy = 793; //ppm accuracy : 7.93
	    break;
        case AUDIO_CLK_FREQ_16:
            mpll_reg_value = 0x4f880;
	    ppm_accuracy = 1204; //ppm accuracy : 12.04
	    break;
        case AUDIO_CLK_FREQ_22:
            mpll_reg_value = 0x4c4a4;
	    ppm_accuracy = 1562; //ppm accuracy : 15.62
	    break;
        case AUDIO_CLK_FREQ_24:
            mpll_reg_value = 0x7e47f;
	    ppm_accuracy = 1351; //ppm accuracy : 13.51
	    break;
        case AUDIO_CLK_FREQ_882:
            mpll_reg_value = 0x5c88b;
	    ppm_accuracy = 1234; //ppm accuracy : 12.34
	    break;
        default:
	    printk("%s,i2s_freq_index error, return !!!\n",__func__);
    	    return 0;
    };
    
    sdmin_value = (mpll_reg_value & 0x3fff);
    n2_integer = (mpll_reg_value & 0x1ff0000) >> 16;

#if 0 
    mpll_reg_value_offset = (ppm_value * 100)/ppm_accuracy;  
    if(((ppm_value * 100)%ppm_accuracy) > ((ppm_accuracy*2)/3))
	    mpll_reg_value_offset += 1;
     
    if(up_down == 0)
	mpll_reg_value  -= mpll_reg_value_offset;
    else if (up_down == 1)
	mpll_reg_value  += mpll_reg_value_offset;
    
    set_sdmin_value = (mpll_reg_value & 0x3fff);

    printk("%s,audio_xd:%d,pre_mpll_reg:0x%x,sdmin:%d,set n2:%d,ppm_value:%d,mpll offset:%d,set mpll_reg:0x%x,set sdmin:%d\n",
    		__func__,audio_xd,mpll_reg_pre_value,sdmin_value,n2_integer,
		ppm_value,mpll_reg_value_offset,mpll_reg_value,set_sdmin_value);
#else
    mclk_offset_value = mclk_tune_table[i2s_freq_index][MCLK_BASE_INDEX];
    mclk_offset_value = mclk_offset_value*ppm_value;
    do_div(mclk_offset_value,1000000);
    printk("mclk_offset_value:%lld\n",mclk_offset_value);
    if(up_down == 0) {
	    set_mclk_value = mclk_tune_table[i2s_freq_index][MCLK_BASE_INDEX] 
	    		+ mclk_offset_value;
	    for(i = MCLK_BASE_INDEX;i>=1;i--){
	    	    printk("i:%d,set_mclk_value:%d,table[i]:%d,table[i-1]:%d\n",
		    	i,set_mclk_value,mclk_tune_table[i2s_freq_index][i],mclk_tune_table[i2s_freq_index][i-1]);
		    if(set_mclk_value >= mclk_tune_table[i2s_freq_index][i] && 
		    	set_mclk_value <=mclk_tune_table[i2s_freq_index][i-1])
			    break;
	    }
	    if(i == 0) {
		    printk("ppm value is beyond range error!!!\n");
		    return 0;
	    }
	    if(set_mclk_value > (mclk_tune_table[i2s_freq_index][i]+mclk_tune_table[i2s_freq_index][i-1])/2)
		    mpll_reg_value_offset = MCLK_BASE_INDEX-(i-1);
	    else
		    mpll_reg_value_offset = MCLK_BASE_INDEX-i;
	    mpll_reg_value  -= mpll_reg_value_offset;
    } else if (up_down == 1) {
	    set_mclk_value = mclk_tune_table[i2s_freq_index][MCLK_BASE_INDEX]-mclk_offset_value;
	    for(i = MCLK_BASE_INDEX;i< 100;i++){
	            printk("i:%d,set_mclk_value:%d,table[i]:%d,table[i+1]:%d\n",
		    	i,set_mclk_value,mclk_tune_table[i2s_freq_index][i],mclk_tune_table[i2s_freq_index][i+1]);
		    if(set_mclk_value <= mclk_tune_table[i2s_freq_index][i] && set_mclk_value >= mclk_tune_table[i2s_freq_index][i+1])
			    break;
	    }
	    if(i == 100){
		    printk("ppm value is beyond range error!!!\n");
		    return 0;
	    }
	    if(set_mclk_value > (mclk_tune_table[i2s_freq_index][i]+mclk_tune_table[i2s_freq_index][i+1])/2)
		    mpll_reg_value_offset = i-MCLK_BASE_INDEX;
	    else
		    mpll_reg_value_offset = i+1-MCLK_BASE_INDEX;
	    mpll_reg_value  += mpll_reg_value_offset;
    }

    set_sdmin_value = (mpll_reg_value & 0x3fff);

    printk("%s,audio_xd:%d,pre_mpll_reg:0x%x,sdmin:%d,set n2:%d,ppm_value:%d,mpll offset:%d,set mpll_reg:0x%x,set sdmin:%d\n",
    		__func__,audio_xd,mpll_reg_pre_value,sdmin_value,n2_integer,
		ppm_value,mpll_reg_value_offset,mpll_reg_value,set_sdmin_value);

#endif
    // gate the clock off
    WRITE_MPEG_REG( HHI_AUD_CLK_CNTL, READ_MPEG_REG(HHI_AUD_CLK_CNTL) & ~(1 << 8));
    WRITE_MPEG_REG(AIU_CLK_CTRL_MORE, 0);
    
    /*--- DAC clock  configuration--- */
    // Disable mclk
    WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL, 0, 8, 1);
    // Select clk source, 0=none; 1=Multi-Phase PLL0; 2=Multi-Phase PLL1; 3=Multi-Phase PLL2.
    WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL, i2s_clk_src, 9, 2);

    // Configure Multi-Phase PLLX
    WRITE_MPEG_REG(i2s_mpll_reg, mpll_reg_value);
    // Set the XD value
    WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL, audio_xd-1, 0, 8);

    WRITE_MPEG_REG_BITS(AIU_CODEC_DAC_LRCLK_CTRL, 64-1, 0, 12);//set codec dac ratio---lrclk--64fs
    
    //mdelay(5);
    for (i = 0; i < 500000; i++) ;
    // gate the clock on
    WRITE_MPEG_REG( HHI_AUD_CLK_CNTL, READ_MPEG_REG(HHI_AUD_CLK_CNTL) | (1 << 8));

    //Audio DAC Clock enable
    WRITE_MPEG_REG(HHI_AUD_CLK_CNTL, READ_MPEG_REG(HHI_AUD_CLK_CNTL) |(1<<23));
    
    /* ---ADC clock  configuration--- */
    // Disable mclk
    WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL, 0, 8, 1);
    // Set pll over mclk ratio
    //we want 256fs ADC MLCK,so for over clock mode,divide more 2 than I2S  DAC CLOCK
#if OVERCLOCK == 0
    WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL, audio_xd-1, 0, 8);
#else
    WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL, audio_xd*2-1, 0, 8);
#endif

    // Set mclk over sclk ratio
    WRITE_MPEG_REG_BITS(AIU_CLK_CTRL_MORE, 4-1, 8, 6);

    // Set sclk over lrclk ratio
    WRITE_MPEG_REG_BITS(AIU_CODEC_ADC_LRCLK_CTRL, 64-1, 0, 12); //set codec adc ratio---lrclk--64fs

    // Enable sclk
    WRITE_MPEG_REG_BITS(AIU_CLK_CTRL_MORE, 1, 14, 1);
    // Enable mclk
    WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL, 1, 8, 1);

    for (i = 0; i < 200000; i++) ;

    return 0;
}
// iec958 and i2s clock are separated after M6TV. Use PLL1 for iec958 clock
void audio_set_958_clk(unsigned freq, unsigned fs_config)
{
    int i;
    int xtal = 0;

    int (*audio_clock_config)[2];

	int index=0;
    printk("audio_set_958_clk, freq=%d,\n",freq);
	switch(freq)
	{
		case AUDIO_CLK_FREQ_192:
			index=4;
			break;
		case AUDIO_CLK_FREQ_96:
			index=3;
			break;
		case AUDIO_CLK_FREQ_48:
			index=2;
			break;
		case AUDIO_CLK_FREQ_441:
			index=1;
			break;
		case AUDIO_CLK_FREQ_32:
			index=0;
			break;
		case AUDIO_CLK_FREQ_8:
			index = 5;
			break;
		case AUDIO_CLK_FREQ_11:
			index = 6;
			break;
		case AUDIO_CLK_FREQ_12:
			index = 7;
			break;
		case AUDIO_CLK_FREQ_16:
			index = 8;
			break;
		case AUDIO_CLK_FREQ_22:
			index = 9;
			break;
		case AUDIO_CLK_FREQ_24:
			index = 10;
			break;
        case AUDIO_CLK_FREQ_882:
			index = 12;
			break;
		default:
			index=0;
			break;
	};

	if (fs_config == AUDIO_CLK_256FS) {
		// divide 256
		xtal = 0;
	}
	else if (fs_config == AUDIO_CLK_384FS) {
	    // divide 384
		xtal = 1;
	}
	audio_clock_config = audio_clock_config_table[xtal];

    // gate the clock off
    WRITE_MPEG_REG( HHI_AUD_CLK_CNTL, READ_MPEG_REG(HHI_AUD_CLK_CNTL) & ~(1 << 8));
	//WRITE_MPEG_REG(AIU_CLK_CTRL_MORE, 0);
	
	//Set filter register
	//WRITE_MPEG_REG(HHI_MPLL_CNTL3, 0x26e1250);

	/*--- IEC958 clock  configuration, use MPLL1--- */
	// Disable mclk
    WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL2, 0, 24, 1);
	//IEC958_USE_CNTL
	WRITE_MPEG_REG_BITS( HHI_AUD_CLK_CNTL2, 1, 27, 1);
	// Select clk source, 0=ddr_pll; 1=Multi-Phase PLL0; 2=Multi-Phase PLL1; 3=Multi-Phase PLL2.
	WRITE_MPEG_REG_BITS( HHI_AUD_CLK_CNTL2, I958_PLL_SRC, 25, 2);

	// Configure Multi-Phase PLL1
	WRITE_MPEG_REG(MPLL_958_CNTL, audio_clock_config[index][0]);
	// Set the XD value
#if IEC958_OVERCLOCK	==1
	WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL2, (audio_clock_config[index][1]+1)/2 -1, 16, 8);
#else
	WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL2, audio_clock_config[index][1], 16, 8);
#endif

	// delay 5uS
	//udelay(5);
	for (i = 0; i < 500000; i++) ;
	// gate the clock on
	WRITE_MPEG_REG( HHI_AUD_CLK_CNTL, READ_MPEG_REG(HHI_AUD_CLK_CNTL) | (1 << 8));
	// Enable mclk
    WRITE_MPEG_REG_BITS(HHI_AUD_CLK_CNTL2, 1, 24, 1);
}

//extern void audio_out_enabled(int flag);
void audio_hw_958_raw(void);

void audio_enable_ouput(int flag)
{
    if (flag) {
        WRITE_MPEG_REG(AIU_RST_SOFT, 0x05);
        READ_MPEG_REG(AIU_I2S_SYNC);
        WRITE_MPEG_REG_BITS(AIU_MEM_I2S_CONTROL, 3, 1, 2);

        if (0/*ENABLE_IEC958*/) {
            if(IEC958_MODE == AIU_958_MODE_RAW)
            {
              //audio_hw_958_raw();
            }
            //else
            {
              WRITE_MPEG_REG(AIU_958_FORCE_LEFT, 0);
              WRITE_MPEG_REG_BITS(AIU_958_DCU_FF_CTRL, 1, 0, 1);
              //WRITE_MPEG_REG(AIU_958_DCU_FF_CTRL, 1);

              WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 3, 1, 2);
            }
        }
        // Maybe cause POP noise
        // audio_i2s_unmute();
    } else {
        WRITE_MPEG_REG_BITS(AIU_MEM_I2S_CONTROL, 0, 1, 2);

        if (0/*ENABLE_IEC958*/) {
            WRITE_MPEG_REG(AIU_958_DCU_FF_CTRL, 0);
            WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 0, 1, 2);
        }
        // Maybe cause POP noise
        // audio_i2s_mute();
    }
    //audio_out_enabled(flag);
}

int if_audio_out_enable(void)
{
	return READ_MPEG_REG_BITS(AIU_MEM_I2S_CONTROL, 1, 2);
}
int if_958_audio_out_enable(void)
{
	return READ_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL,1,2);
}

unsigned int read_i2s_rd_ptr(void)
{
    unsigned int val;
    val = READ_MPEG_REG(AIU_MEM_I2S_RD_PTR);
    return val;
}
unsigned int read_iec958_rd_ptr(void)
{
    unsigned int val;
    val = READ_MPEG_REG(AIU_MEM_IEC958_RD_PTR);
    return val;
}
void aml_audio_i2s_unmute(void)
{
    WRITE_MPEG_REG_BITS(AIU_I2S_MUTE_SWAP, 0, 8, 8);
}

void aml_audio_i2s_mute(void)
{
    WRITE_MPEG_REG_BITS(AIU_I2S_MUTE_SWAP, 0xff, 8, 8);
}
void audio_i2s_unmute(void)
{
    WRITE_MPEG_REG_BITS(AIU_I2S_MUTE_SWAP, 0, 8, 8);
    WRITE_MPEG_REG_BITS(AIU_958_CTRL, 0, 3, 2);
}

void audio_i2s_mute(void)
{
    WRITE_MPEG_REG_BITS(AIU_I2S_MUTE_SWAP, 0xff, 8, 8);
    WRITE_MPEG_REG_BITS(AIU_958_CTRL, 3, 3, 2);
}

void audio_hw_958_reset(unsigned slow_domain, unsigned fast_domain)
{
	WRITE_MPEG_REG(AIU_958_DCU_FF_CTRL,0);
    WRITE_MPEG_REG(AIU_RST_SOFT,
                   (slow_domain << 3) | (fast_domain << 2));
}

void audio_hw_958_raw(void)
{
    if (ENABLE_IEC958) {
         WRITE_MPEG_REG(AIU_958_MISC, 1);
         WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 1, 8, 1);  // raw
         WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 0, 7, 1);  // 8bit
         WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 1, 3, 3); // endian
    }

    WRITE_MPEG_REG(AIU_958_BPF, IEC958_bpf);
    WRITE_MPEG_REG(AIU_958_BRST, IEC958_brst);
    WRITE_MPEG_REG(AIU_958_LENGTH, IEC958_length);
    WRITE_MPEG_REG(AIU_958_PADDSIZE, IEC958_padsize);
    WRITE_MPEG_REG_BITS(AIU_958_DCU_FF_CTRL, 0, 2, 2);// disable int

    if(IEC958_mode == 1){ // search in byte
      WRITE_MPEG_REG_BITS(AIU_958_DCU_FF_CTRL, 7, 4, 3);
    }else if(IEC958_mode == 2) { // search in word
      WRITE_MPEG_REG_BITS(AIU_958_DCU_FF_CTRL, 5, 4, 3);
    }else{
      WRITE_MPEG_REG_BITS(AIU_958_DCU_FF_CTRL, 0, 4, 3);
    }
    WRITE_MPEG_REG(AIU_958_CHSTAT_L0, IEC958_chstat0_l);
    WRITE_MPEG_REG(AIU_958_CHSTAT_L1, IEC958_chstat1_l);
    WRITE_MPEG_REG(AIU_958_CHSTAT_R0, IEC958_chstat0_r);
    WRITE_MPEG_REG(AIU_958_CHSTAT_R1, IEC958_chstat1_r);

    WRITE_MPEG_REG(AIU_958_SYNWORD1, IEC958_syncword1);
    WRITE_MPEG_REG(AIU_958_SYNWORD2, IEC958_syncword2);
    WRITE_MPEG_REG(AIU_958_SYNWORD3, IEC958_syncword3);
    WRITE_MPEG_REG(AIU_958_SYNWORD1_MASK, IEC958_syncword1_mask);
    WRITE_MPEG_REG(AIU_958_SYNWORD2_MASK, IEC958_syncword2_mask);
    WRITE_MPEG_REG(AIU_958_SYNWORD3_MASK, IEC958_syncword3_mask);

    printk("%s: %d\n", __func__, __LINE__);
    printk("\tBPF: %x\n", IEC958_bpf);
    printk("\tBRST: %x\n", IEC958_brst);
    printk("\tLENGTH: %x\n", IEC958_length);
    printk("\tPADDSIZE: %x\n", IEC958_length);
    printk("\tsyncword: %x, %x, %x\n\n", IEC958_syncword1, IEC958_syncword2, IEC958_syncword3);

}

void set_958_channel_status(_aiu_958_channel_status_t * set)
{
    if (set) {
		WRITE_MPEG_REG(AIU_958_CHSTAT_L0, set->chstat0_l);
		WRITE_MPEG_REG(AIU_958_CHSTAT_L1, set->chstat1_l);
		WRITE_MPEG_REG(AIU_958_CHSTAT_R0, set->chstat0_r);
		WRITE_MPEG_REG(AIU_958_CHSTAT_R1, set->chstat1_r);
    }
}

static void audio_hw_set_958_pcm24(_aiu_958_raw_setting_t * set)
{
    WRITE_MPEG_REG(AIU_958_BPF, 0x80); /* in pcm mode, set bpf to 128 */
    set_958_channel_status(set->chan_stat);
}

void audio_set_958_mode(unsigned mode, _aiu_958_raw_setting_t * set)
{
    if(mode == AIU_958_MODE_PCM_RAW){
    	mode = AIU_958_MODE_PCM16; //use 958 raw pcm mode
       WRITE_MPEG_REG(AIU_958_VALID_CTRL,3);//enable 958 invalid bit	
    } 
    else
	WRITE_MPEG_REG(AIU_958_VALID_CTRL,0);
    if (mode == AIU_958_MODE_RAW) {

        audio_hw_958_raw();
        if (ENABLE_IEC958) {
            WRITE_MPEG_REG(AIU_958_MISC, 1);
            WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 1, 8, 1);  // raw
            WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 0, 7, 1);  // 8bit
            WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 1, 3, 3); // endian
        }

        printk("IEC958 RAW\n");
    }else if(mode == AIU_958_MODE_PCM32){
        audio_hw_set_958_pcm24(set);
        if(ENABLE_IEC958){
            WRITE_MPEG_REG(AIU_958_MISC, 0x2020 | (1 << 7));
            WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 0, 8, 1);  // pcm
            WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 0, 7, 1);  // 16bit
            WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 0, 3, 3); // endian
        }
        printk("IEC958 PCM32 \n");
    }else if (mode == AIU_958_MODE_PCM24) {
        audio_hw_set_958_pcm24(set);
        if (ENABLE_IEC958) {
            WRITE_MPEG_REG(AIU_958_MISC, 0x2020 | (1 << 7));
            WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 0, 8, 1);  // pcm
            WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 0, 7, 1);  // 16bit
            WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 0, 3, 3); // endian

        }
        printk("IEC958 24bit\n");
    } else if (mode == AIU_958_MODE_PCM16) {
        audio_hw_set_958_pcm24(set);
        if (ENABLE_IEC958) {
            WRITE_MPEG_REG(AIU_958_MISC, 0x2042);
            WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 0, 8, 1);  // pcm
            WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 1, 7, 1);  // 16bit
            WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 0, 3, 3); // endian

        }
        printk("IEC958 16bit\n");
    }

    audio_hw_958_reset(0, 1);

    WRITE_MPEG_REG(AIU_958_FORCE_LEFT, 1);
}
void audio_out_i2s_enable(unsigned flag)
{
    if (flag) {
        WRITE_MPEG_REG(AIU_RST_SOFT, 0x01);
        READ_MPEG_REG(AIU_I2S_SYNC);
        WRITE_MPEG_REG_BITS(AIU_MEM_I2S_CONTROL, 3, 1, 2);
        // Maybe cause POP noise
        // audio_i2s_unmute();
    } else {
        WRITE_MPEG_REG_BITS(AIU_MEM_I2S_CONTROL, 0, 1, 2);

        // Maybe cause POP noise
        // audio_i2s_mute();
    }
    //audio_out_enabled(flag);
}
void audio_hw_958_enable(unsigned flag)
{
    if (ENABLE_IEC958)
    {
    		if(flag){
        		WRITE_MPEG_REG(AIU_RST_SOFT, 0x04);
	              WRITE_MPEG_REG(AIU_958_FORCE_LEFT, 0);
	              WRITE_MPEG_REG_BITS(AIU_958_DCU_FF_CTRL, 1, 0, 1);
	              WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 3, 1, 2);
    		}
		else{
	             WRITE_MPEG_REG(AIU_958_DCU_FF_CTRL, 0);
	             WRITE_MPEG_REG_BITS(AIU_MEM_IEC958_CONTROL, 0, 1, 2);			
		}	
    }
}

unsigned int read_i2s_mute_swap_reg(void)
{
	unsigned int val;
    	val = READ_MPEG_REG(AIU_I2S_MUTE_SWAP);
    	return val;
}

void audio_i2s_swap_left_right(unsigned int flag)
{
	if (ENABLE_IEC958)
	{
		WRITE_MPEG_REG_BITS(AIU_958_CTRL, flag, 1, 2);
	}
	WRITE_MPEG_REG_BITS(AIU_I2S_MUTE_SWAP, flag, 0, 2);
}
unsigned int audio_hdmi_init_ready()
{
	return 	READ_MPEG_REG_BITS(AIU_HDMI_CLK_DATA_CTRL, 0, 2);
}
/* power gate control for iec958 audio out */
unsigned audio_spdifout_pg_enable(unsigned char enable)
{
	if(enable){
			WRITE_MPEG_REG_BITS( MPLL_958_CNTL, 1,14, 1);					
			AUDIO_CLK_GATE_ON(AIU_IEC958);
			AUDIO_CLK_GATE_ON(AIU_ICE958_AMCLK);
	}
	else{
			AUDIO_CLK_GATE_OFF(AIU_IEC958);
			AUDIO_CLK_GATE_OFF(AIU_ICE958_AMCLK);	
			WRITE_MPEG_REG_BITS( MPLL_958_CNTL, 0,14, 1);								
	}
	return 0;
}
/*
	power gate control for normal aiu  domain including i2s in/out
	TODO: move i2s out /adc related gate to i2s cpu dai driver 
*/
unsigned audio_aiu_pg_enable(unsigned char enable)
{
	if(enable){
		AUDIO_CLK_GATE_ON(AIU_AMCLK_MEASURE);
		AUDIO_CLK_GATE_ON(AIU_AIFIFO2);
		AUDIO_CLK_GATE_ON(AIU_AUD_MIXER);
		AUDIO_CLK_GATE_ON(AIU_MIXER_REG);
		AUDIO_CLK_GATE_ON(AIU_AI_TOP_GLUE);
		AUDIO_CLK_GATE_ON(AIU_AOCLK);   		
		AUDIO_CLK_GATE_ON(AIU_I2S_OUT);
		AUDIO_CLK_GATE_ON(AIU_ADC);		
		AUDIO_CLK_GATE_ON(AUD_IN);
		AUDIO_CLK_GATE_ON(AIU_IEC958);
	#if MESON_CPU_TYPE != MESON_CPU_TYPE_MESON8B  	
		AUDIO_CLK_GATE_ON(AIU_PCLK);
	#endif
		AUDIO_CLK_GATE_ON(AIU_ICE958_AMCLK);
		AUDIO_CLK_GATE_ON(AIU_TOP_LEVEL);
	}
	else{
		AUDIO_CLK_GATE_OFF(AIU_AMCLK_MEASURE);
		AUDIO_CLK_GATE_OFF(AIU_AIFIFO2);
		AUDIO_CLK_GATE_OFF(AIU_AUD_MIXER);
		AUDIO_CLK_GATE_OFF(AIU_MIXER_REG);
		AUDIO_CLK_GATE_OFF(AIU_AI_TOP_GLUE);
		AUDIO_CLK_GATE_OFF(AIU_AOCLK);   		
		AUDIO_CLK_GATE_OFF(AIU_I2S_OUT);
		AUDIO_CLK_GATE_OFF(AIU_ADC);			
		AUDIO_CLK_GATE_OFF(AUD_IN);
		AUDIO_CLK_GATE_OFF(AIU_IEC958);
	#if MESON_CPU_TYPE != MESON_CPU_TYPE_MESON8B  
		AUDIO_CLK_GATE_OFF(AIU_PCLK);  
	#endif 
		AUDIO_CLK_GATE_OFF(AIU_ICE958_AMCLK);   
		AUDIO_CLK_GATE_OFF(AIU_TOP_LEVEL);
	}
    return 0;
}


