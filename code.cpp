#include <bits/stdc++.h>
#define uint unsigned int
#define fi first
#define se second
using namespace std;
//Lexer 
enum Ttype{
	END=-20,REM,LET,INPUT,EXIT,GOTO,IF,THEN,FOR,ENDFOR,ID,NUM,INT 
};
static string identifier;
static int numval;
bool flag=0;
int linenum,fornum,Lastfor[10006],isfor[10006],endfor[10006];//to match END FOR
static int gettok(int f=0)
{
	static int Lastchar=' ';
	if(f==1)Lastchar=' ';
	while(Lastchar==' ')Lastchar=getchar();
	if(isalpha(Lastchar)){
		identifier=Lastchar;
		while(isalpha(Lastchar=getchar()))identifier+=Lastchar;
		if(identifier=="REM"){
			do
				Lastchar=getchar();
			while(Lastchar!=EOF&&Lastchar!='\n'&&Lastchar!='\r');
			return REM;
		}
		if(identifier=="LET")return LET;
		if(identifier=="EXIT")return EXIT;
		if(identifier=="INPUT")return INPUT;
		if(identifier=="GOTO")return GOTO;
		if(identifier=="IF")return IF;
		if(identifier=="THEN")return THEN;
		if(identifier=="FOR")return FOR;
		if(identifier=="INT")return INT;
		if(identifier=="END"){
			do
				Lastchar=getchar();
			while(Lastchar!='R');
			Lastchar=getchar();
			return ENDFOR;
		}
		return ID;
	}
	if(isdigit(Lastchar)){
		numval=0;
		do{
			numval=numval*10+Lastchar-'0';
			Lastchar=getchar();
		}while(isdigit(Lastchar));
		return NUM;
	}
	if(Lastchar==EOF)return END;
	int Thischar=Lastchar;
	if(Thischar=='\n'||Thischar=='\r')return Thischar;
	Lastchar=getchar();
	if(Lastchar=='='){
		Lastchar=getchar();
		if(Thischar=='!')return '!';
		if(Thischar=='=')return '~';
		if(Thischar=='>')return '@';
		if(Thischar=='<')return '#';
	}else if(Lastchar=='&'||Lastchar=='|'){
		Lastchar=getchar();
		return Thischar;
	}
	return Thischar;
}
namespace{
	class ExprAST{
	public:
		virtual ~ExprAST()=default;
		virtual int key(){return 0;}
	};
	class NumExprAST:public ExprAST{
	public:
		int val;
		NumExprAST(int _val):val(_val){}
		int key(){return 1;}
	};
	class VarExprAST:public ExprAST{
	public:
		string name;
		vector<unique_ptr<ExprAST>>dim;
		VarExprAST(const string &_name,vector<unique_ptr<ExprAST>>_dim):name(_name),dim(move(_dim)){}
		int key(){return 2;}
	};
	class BinaryExprAST:public ExprAST{
	public:
		char op;
		unique_ptr<ExprAST>LHS,RHS;
		BinaryExprAST(char _op,unique_ptr<ExprAST>_LHS,unique_ptr<ExprAST>_RHS)
			:op(_op),LHS(move(_LHS)),RHS(move(_RHS)){}
		int key(){return 3;}
	};
	class IfExprAST:public ExprAST{
	public:
		unique_ptr<ExprAST>cond;
		int line;
		IfExprAST(unique_ptr<ExprAST>_cond,int _line):cond(move(_cond)),line(_line){}
		int key(){return 4;}
	};
	class ForExprAST:public ExprAST{
	public:
		unique_ptr<ExprAST>cond;
		ForExprAST(unique_ptr<ExprAST>_cond):cond(move(_cond)){}
		int key(){return 5;}
	};
	class EndforExprAST:public ExprAST{
	public:
		unique_ptr<ExprAST>variable,expr;
		int line;
		EndforExprAST(unique_ptr<ExprAST>_var,unique_ptr<ExprAST>_expr,int _line)
			:variable(move(_var)),expr(move(_expr)),line(_line){}
		int key(){return 6;}
	};
	class LetExprAST:public ExprAST{
	public:
		unique_ptr<ExprAST>variable,init;
		LetExprAST(unique_ptr<ExprAST>_var,unique_ptr<ExprAST>_init)
			:variable(move(_var)),init(move(_init)){}
		int key(){return 7;}
	};
	class GotoExprAST:public ExprAST{
	public:
		int line;
		GotoExprAST(int _line):line(_line){}
		int key(){return 8;}
	};
	class ExitExprAST:public ExprAST{
	public:
		unique_ptr<ExprAST>res;
		ExitExprAST(unique_ptr<ExprAST>_res):res(move(_res)){}
		int key(){return 9;}
	};
	class InputExprAST:public ExprAST{
	public:
		vector<unique_ptr<ExprAST>>vars;
		InputExprAST(vector<unique_ptr<ExprAST>>_vars):vars(move(_vars)){}
		int key(){return 10;}
	};
}
//Parser
static int curtok;
static unique_ptr<ExprAST>forstmt[10006][2];
static int getnext(int f=0)
{
	if(f==1)curtok=gettok(1);
	else curtok=gettok();
//	printf("%d\n",curtok);
	return curtok;
}
int precedence[256];
static int getprecedence()
{
	if(!isascii(curtok))return -1;
	return precedence[curtok]<=0?-1:precedence[curtok];
}
unique_ptr<ExprAST> LogError(const char *s)
{
	flag=1;
	fprintf(stderr,"Error:%s\n",s);
	return nullptr;
}
static unique_ptr<ExprAST> ParseExpression();
static unique_ptr<ExprAST> ParseNumExpr()
{
	if(curtok=='-'){
		getnext();
		numval=0-numval;
	}
	auto res=make_unique<NumExprAST>(numval);
	getnext();
	return move(res);
}
static unique_ptr<ExprAST> ParseParenExpr()
{
	getnext();
	auto V=ParseExpression();
	if(!V)return nullptr;
	if(curtok!=')')return LogError("expected ')'");
	getnext(); 
	return V;
}
static unique_ptr<ExprAST> ParseIDExpr()
{
	string IDname=identifier;
	vector<unique_ptr<ExprAST>>tmp;
	getnext();
	while(curtok=='['){
		getnext();
		auto tmpp=ParseExpression();
		tmp.push_back(move(tmpp));
		if(curtok!=']')return LogError("unknown statement");
		getnext();
	}
	return make_unique<VarExprAST>(IDname,move(tmp));
}
static unique_ptr<ExprAST> ParseIfExpr()
{
	getnext();
	auto cond=ParseExpression();
	if(!cond)return nullptr;
	if(curtok!=THEN)return LogError("expected THEN");
	getnext();
	if(curtok!=NUM)return LogError("expected number");
	return make_unique<IfExprAST>(move(cond),numval);
}
static unique_ptr<ExprAST> ParseForExpr()
{
	getnext();
	if(curtok!=ID)return LogError("expected var=expr after FOR");
	forstmt[fornum][0]=ParseIDExpr();
	if(curtok!='=')return LogError("expected var=expr after FOR");
	getnext();
	forstmt[fornum][1]=ParseExpression();
	if(curtok!=';')return LogError("expected ';'");
	getnext();
	auto cond=ParseExpression();
	if(!cond)return nullptr;
	return make_unique<ForExprAST>(move(cond));
}
static unique_ptr<ExprAST> ParseEndforExpr()
{
	getnext();
	if(fornum==0)return LogError("too many ENDFOR");
	endfor[Lastfor[fornum]]=linenum;
	fornum--;
	return make_unique<EndforExprAST>(move(forstmt[fornum+1][0]),
			move(forstmt[fornum+1][1]),Lastfor[fornum+1]);
}
static unique_ptr<ExprAST> ParseGotoExpr()
{
	getnext();
	if(curtok!=NUM)return LogError("expected number after GOTO");
	return  make_unique<GotoExprAST>(numval); 
}
static unique_ptr<ExprAST> ParseInputExpr()
{
	getnext();
	if(curtok!=ID)return LogError("expected identifier after INPUT");
	vector<unique_ptr<ExprAST>>res;
	auto name=ParseIDExpr();
	res.push_back(move(name));
	while(curtok==','){
		getnext();
		if(curtok!=ID)return LogError("expected identifier");
	//	name.release();
		name=ParseIDExpr();
		res.push_back(move(name));
	}
	return make_unique<InputExprAST>(move(res));
}
static unique_ptr<ExprAST> ParseExitExpr()
{
	getnext();
	auto res=ParseExpression();
	if(!res)return nullptr;
	return make_unique<ExitExprAST>(move(res));
}
static unique_ptr<ExprAST> ParseLetExpr()
{
	getnext();
	int q=0;
	while(curtok=='(')q++,getnext();
	if(curtok!=ID)return LogError("expected identifier after LET");
	auto qwq=ParseIDExpr();
	VarExprAST* ptr = dynamic_cast<VarExprAST*>(qwq.get());
	unique_ptr<VarExprAST>name;
	if(ptr!=nullptr){
		qwq.release();
		name.reset(ptr);
	}
	while(q){
		if(curtok!=')')return LogError("expected ')'");
		getnext();
		q--;
	}
	if(curtok!='=')return LogError("expected '='");
	getnext();
	if(curtok==INT){
		getnext();
		if(curtok!='[')return LogError("unknown statement");
		vector<unique_ptr<ExprAST>>dim;
		while(curtok=='['){
			getnext();
			auto t=ParseExpression();
			dim.push_back(move(t));
			if(curtok!=']')return LogError("unknown statement");
			getnext();
		}
		auto var=make_unique<VarExprAST>(name->name,move(dim));
		return make_unique<LetExprAST>(move(var),nullptr);
	}else{
		unique_ptr<ExprAST>init=ParseExpression();
		if(!init)return nullptr;
		return make_unique<LetExprAST>(move(name),move(init));
	}
}
static unique_ptr<ExprAST> ParsePrimary()
{
	switch(curtok){
		default:return LogError("unknown token when expecting an expression");
		case '-':return ParseNumExpr();
		case ID:return ParseIDExpr();
		case NUM:return ParseNumExpr();
		case '(':return ParseParenExpr();
		case IF:return ParseIfExpr();
		case FOR:Lastfor[++fornum]=linenum;isfor[linenum]=1;return ParseForExpr();
		case INPUT:return ParseInputExpr();
		case GOTO:return ParseGotoExpr();
		case LET:return ParseLetExpr();
		case EXIT:return ParseExitExpr();
		case ENDFOR:return ParseEndforExpr();
		case REM:return nullptr;
		case END:return nullptr;
	}
}
static unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,unique_ptr<ExprAST> LHS)
{
	while(1){
		int TokPrec=getprecedence();
		if(TokPrec<ExprPrec)return move(LHS);
		int op=curtok;
		getnext();
		auto RHS=ParsePrimary();
		if(!RHS)return nullptr;
		int NextPrec=getprecedence();
		if(TokPrec<NextPrec){
			RHS=ParseBinOpRHS(TokPrec+1,move(RHS));
			if(!RHS)return nullptr;
		}
		LHS=make_unique<BinaryExprAST>(op,move(LHS),move(RHS));
	}
}
static unique_ptr<ExprAST> ParseExpression()
{
	auto LHS=ParsePrimary();
	if(!LHS)return nullptr;
	return ParseBinOpRHS(0,move(LHS));
}
//translate
map<string,pair<int,int> >myvar;//fi->varpos se->varcnt
map<string,int>inputvar;
map<string,int>arraypos;//31->
vector<int>limit[100006];
const int varst=4*16*16*16;
int varcnt,spos[100006],jump[100006][5],nowpos=0,varpos=varst;
int arraynum=32,inputcnt,inputlist[100006],input[100006];
uint result[200006];
int tranbinary(auto x)
{
	if(x->key()==1){//num
		NumExprAST* ptr=dynamic_cast<NumExprAST*>(x.get());
		unique_ptr<NumExprAST>y;
		if(ptr!=nullptr)x.release(),y.reset(ptr);
		int val=y->val;
		result[varpos++]=val;
		y.release();
		return 4*(varpos-1-varst);
	}
	if(x->key()==2){//var
		VarExprAST* ptr=dynamic_cast<VarExprAST*>(x.get());
		unique_ptr<VarExprAST>y;
		if(ptr!=nullptr)x.release(),y.reset(ptr);
		string name=y->name;
		if(!myvar.count(name)){
			LogError("variable not declared");
			y.release();
			return 0;
		}
		if(!y->dim.size()){
			y.release();
			return 4*(myvar[name].fi-varst);
		}else{
			int start=myvar[name].fi,id=myvar[name].se;
			if(y->dim.size()!=limit[id].size()){
				LogError("array definition");
				y.release();
				return 0;
			}
			result[nowpos++]=51+(14<<7)+(2<<12)+(8<<15)+(0<<20);
			for(int i=0;i<y->dim.size();i++){
				int tmppos=tranbinary(move(y->dim[i]));
				int tmp=1;
				for(int j=i+1;j<limit[id].size();j++)tmp*=limit[id][j];
				result[varpos++]=4*tmp;
				int tmppos2=4*(varpos-1-varst);
				//lw reg11,reg9+tmppos
				result[nowpos++]=3+(11<<7)+(2<<12)+(9<<15)+(tmppos<<20);
				//lw reg12,reg9+tmppos2
				result[nowpos++]=3+(12<<7)+(2<<12)+(9<<15)+(tmppos2<<20);
				//*
				result[nowpos++]=51+(13<<7)+0+(11<<15)+(12<<20)+(1<<25);
				//+
				result[nowpos++]=51+(14<<7)+0+(14<<15)+(13<<20);
			}
			//+ reg14,reg[array[name]]
			result[nowpos++]=51+(14<<7)+0+(14<<15)+(arraypos[name]<<20);
			//lw reg15,reg14
			result[nowpos++]=3+(15<<7)+(2<<12)+(14<<15)+0;
			//sw reg15,reg9+4*(...)
			varpos++;
			int pos=4*(varpos-1-varst);
			result[nowpos++]=35+((pos%32)<<7)+(2<<12)+(9<<15)+(15<<20)+((pos/32)<<25);
			y.release();
			return pos;
		}
	}
	BinaryExprAST* ptr=dynamic_cast<BinaryExprAST*>(x.get());
	unique_ptr<BinaryExprAST>y;
	if(ptr!=nullptr)x.release(),y.reset(ptr);
	int lpos=tranbinary(move(y->LHS));
	int rpos=tranbinary(move(y->RHS));
	//lw reg1,reg9+lpos
	result[nowpos++]=3+(1<<7)+(2<<12)+(9<<15)+(lpos<<20);
	//lw reg2,reg9+rpos
	result[nowpos++]=3+(2<<7)+(2<<12)+(9<<15)+(rpos<<20);
	//L op R
	if(y->op=='+'){
		result[nowpos++]=51+(3<<7)+0+(1<<15)+(2<<20);
	}else if(y->op=='-'){
		result[nowpos++]=51+(3<<7)+0+(1<<15)+(2<<20)+(1<<30);
	}else if(y->op=='*'){
		result[nowpos++]=51+(3<<7)+0+(1<<15)+(2<<20)+(1<<25);
	}else if(y->op=='/'){
		result[nowpos++]=51+(3<<7)+(4<<12)+(1<<15)+(2<<20)+(1<<25);
	}else if(y->op=='&'){
		result[nowpos++]=51+(3<<7)+(7<<12)+(1<<15)+(2<<20);
	}else if(y->op=='|'){
		result[nowpos++]=51+(3<<7)+(6<<12)+(1<<15)+(2<<20);
	}else if(y->op=='!'){
		result[nowpos++]=51+(4<<7)+(2<<12)+(1<<15)+(2<<20);
		result[nowpos++]=51+(5<<7)+(2<<12)+(2<<15)+(1<<20);
		result[nowpos++]=51+(3<<7)+(6<<12)+(4<<15)+(5<<20);
	}else if(y->op=='<'){
		result[nowpos++]=51+(3<<7)+(2<<12)+(1<<15)+(2<<20);
	}else if(y->op=='>'){
		result[nowpos++]=51+(3<<7)+(2<<12)+(2<<15)+(1<<20);
	}else if(y->op=='~'){
		result[nowpos++]=51+(4<<7)+(2<<12)+(1<<15)+(2<<20);
		result[nowpos++]=51+(5<<7)+(2<<12)+(2<<15)+(1<<20);
		result[nowpos++]=51+(6<<7)+(2<<12)+(4<<15)+(8<<20);
		result[nowpos++]=51+(7<<7)+(2<<12)+(5<<15)+(8<<20);
		result[nowpos++]=51+(3<<7)+(7<<12)+(6<<15)+(7<<20);
	}else if(y->op=='@'){//>=
		result[nowpos++]=51+(4<<7)+(2<<12)+(1<<15)+(2<<20);
		result[nowpos++]=51+(3<<7)+(2<<12)+(4<<15)+(8<<20);
	}else if(y->op=='#'){//<=
		result[nowpos++]=51+(4<<7)+(2<<12)+(2<<15)+(1<<20);
		result[nowpos++]=51+(3<<7)+(2<<12)+(4<<15)+(8<<20);
	}
	//sw reg3,reg9+4*(...)
	varpos++;
	int pos=4*(varpos-1-varst);
	result[nowpos++]=35+((pos%32)<<7)+(2<<12)+(9<<15)+(3<<20)+((pos/32)<<25);
	y.release();
	return pos;
}
void tranif(auto x)
{
	IfExprAST* ptr=dynamic_cast<IfExprAST*>(x.get());
	unique_ptr<IfExprAST>y;
	if(ptr!=nullptr)x.release(),y.reset(ptr);
	int key=tranbinary(move(y->cond));
	jump[nowpos][0]=key;
	jump[nowpos][1]=y->line;
	jump[nowpos][2]=IF;
	jump[nowpos][3]=linenum;
	nowpos+=2;
	y.release();
}
void tranfor(auto x)
{
	ForExprAST* ptr=dynamic_cast<ForExprAST*>(x.get());
	unique_ptr<ForExprAST>y;
	if(ptr!=nullptr)x.release(),y.reset(ptr);
	int key=tranbinary(move(y->cond));
	jump[nowpos][0]=key;
	jump[nowpos][1]=linenum;
	jump[nowpos][2]=FOR;
	nowpos+=2;
	y.release();
}
void getLpos(auto x)
{
	VarExprAST* ptr=dynamic_cast<VarExprAST*>(x.get());
	unique_ptr<VarExprAST>y;
	if(ptr!=nullptr)x.release(),y.reset(ptr);
	string name=y->name;
	if(!myvar.count(name)){
		LogError("variable not declared");
		y.release();
		return;
	}
	if(!y->dim.size()){
		y.release();
		int Lpos=4*(myvar[name].fi-varst);
		//sw reg16,reg9+Lpos
		result[nowpos++]=35+((Lpos%32)<<7)+(2<<12)+(9<<15)+(16<<20)+((Lpos/32)<<25);
		return;
	}else{
		int start=myvar[name].fi,id=myvar[name].se;
		result[nowpos++]=51+(14<<7)+(2<<12)+(8<<15)+(0<<20);
		for(int i=0;i<y->dim.size();i++){
			int tmppos=tranbinary(move(y->dim[i]));
			int tmp=1;
			for(int j=i+1;j<limit[id].size();j++)tmp*=limit[id][j];
			result[varpos++]=4*tmp;
			int tmppos2=4*(varpos-1-varst);
			//lw reg11,reg9+tmppos
			result[nowpos++]=3+(11<<7)+(2<<12)+(9<<15)+(tmppos<<20);
			//lw reg12,reg9+tmppos2
			result[nowpos++]=3+(12<<7)+(2<<12)+(9<<15)+(tmppos2<<20);
			//*
			result[nowpos++]=51+(13<<7)+0+(11<<15)+(12<<20)+(1<<25);
			//+
			result[nowpos++]=51+(14<<7)+0+(14<<15)+(13<<20);
		}
		//+ reg14,reg[array[name]]
		result[nowpos++]=51+(14<<7)+0+(14<<15)+(arraypos[name]<<20);
		//sw reg16,reg14
		result[nowpos++]=35+0+(2<<12)+(14<<15)+(16<<20)+0;
		y.release();
	}
	y.release();
}
void tranendfor(auto x)
{
	EndforExprAST* ptr=dynamic_cast<EndforExprAST*>(x.get());
	unique_ptr<EndforExprAST>y;
	if(ptr!=nullptr)x.release(),y.reset(ptr);
	int Rpos=tranbinary(move(y->expr));
	//lw reg16,reg9+Rpos
	result[nowpos++]=3+(16<<7)+(2<<12)+(9<<15)+(Rpos<<20);
	getLpos(move(y->variable));
	jump[nowpos][1]=y->line;
	jump[nowpos++][2]=ENDFOR;
	y.release();
}
int getdim(auto x)
{
	if(x->key()==1){//num
		NumExprAST* ptr=dynamic_cast<NumExprAST*>(x.get());
		unique_ptr<NumExprAST>y;
		if(ptr!=nullptr)x.release(),y.reset(ptr);
		int val=y->val;
		y.release();
		return val;
	}
	if(x->key()==2){//var
		VarExprAST* ptr=dynamic_cast<VarExprAST*>(x.get());
		unique_ptr<VarExprAST>y;
		if(ptr!=nullptr)x.release(),y.reset(ptr);
		string name=y->name;
		y.release();
		return inputvar[name];
	}
}
void solve(auto x)
{
	VarExprAST* ptr=dynamic_cast<VarExprAST*>(x.get());
	unique_ptr<VarExprAST>y;
	if(ptr!=nullptr)x.release(),y.reset(ptr);
	string name=y->name;
	if(myvar.count(name)){
		LogError("define variables repeatedly");
		y.release();
		return;
	}
	myvar[name]=make_pair(varpos,++varcnt);
	arraypos[name]=--arraynum;
	//add reg[arraynum],reg9+4*(varpos-varst)
	result[nowpos++]=19+(arraynum<<7)+0+(9<<15)+((4*(varpos-varst))<<20);
	//cout<<name<<' '<<myvar.count(name)<<endl;
	int tmp=1;
	for(int i=0;i<y->dim.size();i++){
		int d=getdim(move(y->dim[i]));
		tmp*=d;
		limit[varcnt].push_back(d);
	}
	varpos+=tmp;
	y.release();
}
void getLpos2(auto x)
{
	VarExprAST* ptr=dynamic_cast<VarExprAST*>(x.get());
	unique_ptr<VarExprAST>y;
	if(ptr!=nullptr)x.release(),y.reset(ptr);
	string name=y->name;
	if(!myvar.count(name)&&y->dim.size()){
		LogError("variable not declared");
		y.release();
		return;
	}
	if(!myvar.count(name)){
		myvar[name]=make_pair(varpos++,++varcnt);
		y.release();
		int Lpos=4*(varpos-1-varst);
		//sw reg1,reg9+Lpos
		result[nowpos++]=35+((Lpos%32)<<7)+(2<<12)+(9<<15)+(16<<20)+((Lpos/32)<<25);
		return;
	}
	if(!y->dim.size()){
		y.release();
		int Lpos=4*(myvar[name].fi-varst);
		//sw reg1,reg9+Lpos
		result[nowpos++]=35+((Lpos%32)<<7)+(2<<12)+(9<<15)+(16<<20)+((Lpos/32)<<25);
		return;
	}else{
		int start=myvar[name].fi,id=myvar[name].se;
		result[nowpos++]=51+(14<<7)+(2<<12)+(0<<15)+(0<<20);
		for(int i=0;i<y->dim.size();i++){
			int tmppos=tranbinary(move(y->dim[i]));
			int tmp=1;
			for(int j=i+1;j<limit[id].size();j++)tmp*=limit[id][j];
			result[varpos++]=4*tmp;
			int tmppos2=4*(varpos-1-varst);
			//lw reg11,reg9+tmppos
			result[nowpos++]=3+(11<<7)+(2<<12)+(9<<15)+(tmppos<<20);
			//lw reg12,reg9+tmppos2
			result[nowpos++]=3+(12<<7)+(2<<12)+(9<<15)+(tmppos2<<20);
			//*
			result[nowpos++]=51+(13<<7)+0+(11<<15)+(12<<20)+(1<<25);
			//+
			result[nowpos++]=51+(14<<7)+0+(14<<15)+(13<<20);
		}
		//+ reg14,reg[array[name]]
		result[nowpos++]=51+(14<<7)+0+(14<<15)+(arraypos[name]<<20);
		//sw reg1,reg14
		result[nowpos++]=35+0+(2<<12)+(14<<15)+(16<<20)+0;
		y.release();
		return;
	}
	y.release();
}
void tranlet(auto x)
{
	LetExprAST* ptr=dynamic_cast<LetExprAST*>(x.get());
	unique_ptr<LetExprAST>y;
	if(ptr!=nullptr)x.release(),y.reset(ptr);
	if(y->init==nullptr){
		solve(move(y->variable));
	}else{
		int Rpos=tranbinary(move(y->init));
		//lw reg16,reg9+Rpos
		result[nowpos++]=3+(16<<7)+(2<<12)+(9<<15)+(Rpos<<20);
		getLpos2(move(y->variable));
	}
	y.release();
}
void trangoto(auto x)
{
	GotoExprAST* ptr=dynamic_cast<GotoExprAST*>(x.get());
	unique_ptr<GotoExprAST>y;
	if(ptr!=nullptr)x.release(),y.reset(ptr);
	jump[nowpos][0]=linenum;
	jump[nowpos][1]=y->line;
	jump[nowpos++][2]=GOTO;
	y.release();
}
void tranexit(auto x)
{
	ExitExprAST* ptr=dynamic_cast<ExitExprAST*>(x.get());
	unique_ptr<ExitExprAST>y;
	if(ptr!=nullptr)x.release(),y.reset(ptr);
	int pos=tranbinary(move(y->res));
	//lw reg10,reg9+pos
	result[nowpos++]=3+(10<<7)+(2<<12)+(9<<15)+(pos<<20);
	//end
	result[nowpos++]=0x0FF00513;
	y.release();
}
void getinputpos(auto x)
{
	VarExprAST* ptr=dynamic_cast<VarExprAST*>(x.get());
	unique_ptr<VarExprAST>y;
	if(ptr!=nullptr)x.release(),y.reset(ptr);
	string name=y->name;
	if(!myvar.count(name)&&y->dim.size()){
		LogError("variable not declared");
		y.release();
		return;
	}
	inputvar[name]=inputlist[inputcnt];
	if(!myvar.count(name)){
		myvar[name]=make_pair(varpos++,++varcnt);
		y.release();
		int pos=4*(varpos-1-varst);
		//sw reg16,reg9+pos
		result[nowpos++]=35+((pos%32)<<7)+(2<<12)+(9<<15)+(16<<20)+((pos/32)<<25);
		return;
	}
	if(!y->dim.size()){
		y.release();
		int pos=4*(myvar[name].fi-varst);
		//sw reg16,reg9+pos
		result[nowpos++]=35+((pos%32)<<7)+(2<<12)+(9<<15)+(16<<20)+((pos/32)<<25);
		return;
	}else{
		int start=myvar[name].fi,id=myvar[name].se;
		result[nowpos++]=51+(14<<7)+(2<<12)+(8<<15)+(0<<20);
		for(int i=0;i<y->dim.size();i++){
			int tmppos=tranbinary(move(y->dim[i]));
			int tmp=1;
			for(int j=i+1;j<limit[id].size();j++)tmp*=limit[id][j];
			result[varpos++]=4*tmp;
			int tmppos2=4*(varpos-1-varst);
			//lw reg11,reg9+tmppos
			result[nowpos++]=3+(11<<7)+(2<<12)+(9<<15)+(tmppos<<20);
			//lw reg12,reg9+tmppos2
			result[nowpos++]=3+(12<<7)+(2<<12)+(9<<15)+(tmppos2<<20);
			//*
			result[nowpos++]=51+(13<<7)+0+(11<<15)+(12<<20)+(1<<25);
			//+
			result[nowpos++]=51+(14<<7)+0+(14<<15)+(13<<20);
		}
		//+ reg14,reg[array[name]]
		result[nowpos++]=51+(14<<7)+0+(14<<15)+(arraypos[name]<<20);
		//sw reg16,reg14
		result[nowpos++]=35+0+(2<<12)+(14<<15)+(16<<20)+0;
		y.release();
		return;
	}
	y.release();
}
void traninput(auto x)
{
	InputExprAST* ptr=dynamic_cast<InputExprAST*>(x.get());
	unique_ptr<InputExprAST>y;
	if(ptr!=nullptr)x.release(),y.reset(ptr);
	for(int i=0;i<y->vars.size();i++){
		result[varpos++]=inputlist[++inputcnt];
		int keypos=4*(varpos-1-varst);
		//lw reg16,reg9+keypos
		result[nowpos++]=3+(16<<7)+(2<<12)+(9<<15)+(keypos<<20);
		getinputpos(move(y->vars[i]));
	}
	y.release();
}
void translate(auto x)
{
	//printf("%d %d\n",linenum,x->key());
	switch(x->key()){
		default:break;
		case 4:tranif(move(x));break;//if
		case 5:tranfor(move(x));break;//for
		case 6:tranendfor(move(x));break;//endfor
		case 7:tranlet(move(x));break;//let
		case 8:trangoto(move(x));break;//goto
		case 9:tranexit(move(x));break;//exit
		case 10:traninput(move(x));break;//input
	}
	x.release();
}
void write(uint x)
{
	int A=(int)x/16,B=(int)x%16;
	if(A<10)printf("%d",A);else printf("%c",'A'+A-10);
	if(B<10)printf("%d ",B);else printf("%c ",'A'+B-10);
}
void print(uint x)
{
	write(x%256u);x/=256u;
	write(x%256u);x/=256u;
	write(x%256u);x/=256u;
	write(x);
}
int main()
{
	int x;
	while(scanf("%d",&x)!=EOF){
		inputlist[++inputcnt]=x;
	}
	inputcnt=0;
	freopen("op_10.txt","r",stdin);
	freopen("qwq.out","w",stdout);
	//precedence['=']=0;
	//let && = &,|| = |,!= = !,== = ~,>= = @,<= = #
	precedence['|']=10;
	precedence['&']=20;
	precedence['<']=precedence['>']=precedence['!']=30;
	precedence['~']=precedence['@']=precedence['#']=30;
	precedence['+']=precedence['-']=40;
	precedence['*']=precedence['/']=50;
	//lui something(use reg9 to save varst, reg8 = 1)
	result[nowpos++]=55+(9<<7)+(1<<16);
	result[nowpos++]=51+(8<<7)+(2<<12)+(0<<15)+(9<<20);
	while(scanf("%d",&linenum)!=EOF){
		//build AST
		getnext(1);
		auto AST=ParseExpression();
		if(flag){
			AST.release();
			return 233;
		}
		//translate
		spos[linenum]=nowpos;
		if(AST!=nullptr)translate(move(AST));
		if(flag)return 233;
		//printf("%d %d\n",linenum,spos[linenum]);
	}
	//add end signal
	result[nowpos]=0x0FF00513;
	//jump & input
	for(int i=0;i<=nowpos;i++)if(jump[i][2]<0){
		int jumpline=0;uint Imm=0;
		if(jump[i][2]==ENDFOR){
			jumpline=spos[jump[i][1]];
			Imm=4*(jumpline-i);
			//beq reg0,reg0,Imm
			result[i]=99u+(((Imm&(1<<11))>0)<<7)+((Imm%(1<<5)/2)<<8)+0+0+0+((Imm%(1<<11)/(1<<5))<<25)+(((Imm&(1<<12))>0)<<31);
		}
		if(jump[i][2]==GOTO){
			if(jump[i][1]<jump[i][0]&&jump[i][0]<endfor[jump[i][1]]){
				jumpline=spos[endfor[jump[i][1]]];
			}else jumpline=spos[jump[i][1]];
			if(jumpline>spos[linenum]){
				LogError("jump across the border");
				return 233;
			}
			Imm=4*(jumpline-i);
			//beq reg0,reg0,Imm
			result[i]=99u+(((Imm&(1<<11))>0)<<7)+((Imm%(1<<5)/2)<<8)+0+0+0+((Imm%(1<<11)/(1<<5))<<25)+(((Imm&(1<<12))>0)<<31);
		}
		if(jump[i][2]==IF){
			if(jump[i][1]<jump[i][3]&&jump[i][3]<endfor[jump[i][1]]){
				jumpline=spos[endfor[jump[i][1]]];
			}else jumpline=spos[jump[i][1]];
			if(jumpline>spos[linenum]){
				LogError("jump across the border");
				return 233;
			}
			Imm=4*(jumpline-i-1);
			//lw reg1,reg9+jump[i][0]
			result[i]=3+(1<<7)+(2<<12)+(9<<15)+(jump[i][0]<<20);
			//beq reg1,reg8,Imm
			result[i+1]=99u+(((Imm&(1<<11))>0)<<7)+((Imm%(1<<5)/2)<<8)+0+(1<<15)+(8<<20)+((Imm%(1<<11)/(1<<5))<<25)+(((Imm&(1<<12))>0)<<31);
		}
		if(jump[i][2]==FOR){
			int tmp=endfor[jump[i][1]]+1;
			while(!spos[tmp])tmp++;
			jumpline=spos[tmp];
			Imm=4*(jumpline-i-1);
			//lw reg1,reg9+jump[i][0]
			result[i]=3+(1<<7)+(2<<12)+(9<<15)+(jump[i][0]<<20);
			//beq reg1,reg0,Imm
			result[i+1]=99u+(((Imm&(1<<11))>0)<<7)+((Imm%(1<<5)/2)<<8)+0+(1<<15)+0+((Imm%(1<<11)/(1<<5))<<25)+(((Imm&(1<<12))>0)<<31);
		}
	}else if(input[i]>0){
		
	}
	//out
	puts("@00000000");
	for(int i=0;i<=nowpos;i++)print(result[i]);
	puts("\n@00010000");
	for(int i=varst;i<varpos;i++)print(result[i]);
}
