#include "ltype.h"

const char * ltype_lib_dflt() {
	static const char * ltype = ";;\n"
	";;  CadZinho Default Linetype Definition file\n"
	";;\n"
	";;  Edit this file to quick load linetypes at \n"
	";;  Linetype manager, according your\n"
	";;  preferences\n"
	";;\n"
	";;\n"
	"*Border,____ . ____ ____ . ____ ____ . ____\n"
	"A,.5,-.25,.5,-.25,0,-.25\n"
	"\n"
	"*Center,____ _ ____ _ ____ _ ____ _ ____ _\n"
	"A,1.25,-.25,.25,-.25\n"
	"\n"
	"*Dashdot,____ . ____ . ____ . ____ . ____ .\n"
	"A,.5,-.25,0,-.25\n"
	"\n"
	"*Dashed,____ ____ ____ ____ ____ ____ ____\n"
	"A,.5,-.25\n"
	"\n"
	"*Divide,____ . . ____ . . ____ . . ____ . .\n"
	"A,.5,-.25,0,-.25,0,-.25\n"
	"\n"
	"*Dot,. . . . . . . . . . . . . . . . . .\n"
	"A,0,-.25\n"
	"\n"
	"*Hidden,__ __ __ __ __ __ __ __ __ __ __ __\n"
	"A,.25,-.125\n"
	"\n"
	"*Phantom,_____ _ _ _____ _ _ _____ _ _ _____\n"
	"A,1.25,-.25,.25,-.25,.25,-.25\n"
	"\n";
	
	return ltype;
}

const char * ltype_lib_extra() {
	static const char * ltype = ";;\n"
	";;  CadZinho Extra Linetype Definition file\n"
	";;\n"
	";;  Edit this file to quick load linetypes at \n"
	";;  Linetype manager, according your\n"
	";;  preferences\n"
	";;\n"
	"\n"
	"\n"
	";;  Complex linetypes\n"
	";;\n"
	";;  These linetype definitions use LTYPESHP.SHX.\n"
	";;\n"
	"*Fence_circle,----0-----0----0-----0----0-----0--\n"
	"A,.25,-.1,[CIRC1,ltypeshp.shx,x=-.1,s=.1],-.1,1\n"
	"\n"
	"*Fence_square,----[]-----[]----[]-----[]----[]---\n"
	"A,.25,-.1,[BOX,ltypeshp.shx,x=-.1,s=.1],-.1,1\n"
	"\n"
	"*Fence_X,----X-----X----X-----X----X-----X--\n"
	"A,.25,-.1,[\"X\",STANDARD,S=.1,R=0.0,X=-0.04,Y=-.05],-.1,1\n"
	"\n"
	"*Tracks,-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-\n"
	"A,.15,[TRACK1,ltypeshp.shx,s=.25],.15\n"
	"\n"
	"*Batting,SSSSSSSSSSSSSSSSSSSSSS\n"
	"A,.0001,-.1,[BAT,ltypeshp.shx,x=-.1,s=.1],-.2,[BAT,ltypeshp.shx,r=180,x=.1,s=.1],-.1\n"
	"\n"
	"*Hot_water,---- HW ---- HW ---- HW ----\n"
	"A,.5,-.2,[\"HW\",STANDARD,S=.1,R=0.0,X=-0.1,Y=-.05],-.2\n"
	"\n"
	"*Gas,----GAS----GAS----GAS----GAS--\n"
	"A,.5,-.2,[\"GAS\",STANDARD,S=.1,R=0.0,X=-0.1,Y=-.05],-.25\n"
	"\n"
	"*Zigzag,/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\\n"
	"A,.0001,-.2,[ZIG,ltypeshp.shx,x=-.2,s=.2],-.4,[ZIG,ltypeshp.shx,r=180,x=.2,s=.2],-.2\n"
	"\n";
	
	return ltype;
}