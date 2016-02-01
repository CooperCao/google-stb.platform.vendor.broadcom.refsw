
/* test application command language parser */

%option always-interactive
%option noyywrap

DIGIT [0-9]

HDIGIT [a-fA-F0-9]

%s VPID

%s PSTRING

%s RATE

%s MODE

%s FILTER

%s POSITION

%s JUMPSIZE

%s CHUNKSIZE

%%

oinit {oinit(); }

ostart { ostart(); }

ostop { ostop(); }

orate { BEGIN(RATE); }

<RATE>[^ \n]+ { orate(atoi(yytext)); BEGIN(0); }

omode { BEGIN(MODE); }
<MODE>[^ \n]+ { omode(atoi(yytext)); BEGIN(0);}

offilter { BEGIN(FILTER); }
<FILTER>[^ \n]+ { offilter(atoi(yytext)); BEGIN(0);}

oforward { oforward(); }

orewind { orewind(); }

ofname { BEGIN(PSTRING); }
<PSTRING>[^ \n]+ { ofname(yytext); BEGIN(0); }

ofpos { BEGIN(POSITION);  }
<POSITION>[^ \n]+ { ofpos(atoi(yytext)); BEGIN(0); }

ofjumpsize { BEGIN(JUMPSIZE);  }
<JUMPSIZE>[^ \n]+ { ofjumpsize(atoi(yytext)); BEGIN(0); }

ochunksize { BEGIN(CHUNKSIZE);  }
<CHUNKSIZE>[^ \n]+ { ochunksize(atoi(yytext)); BEGIN(0); }

ovpid { BEGIN(VPID);  }
<VPID>[^ \n]+ { ovpid(atoi(yytext)); BEGIN(0); }
%%
