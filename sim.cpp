#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#define N 1000006
#define uint unsigned int
using namespace std;
char s[16];
int tot,yes;
int pos,PC,MEM_num,Mem[N],reg[32],reg_num[32],fc[N][16],his[N];
bool WB_oc,MEM_oc,EX_oc,ID_oc,isend;
int cnt1,cnt2,cnt3;
int getpos()
{
	int p=0;
	for(int i=1;i<=8;i++)
		if(s[i]>='A')p=p*16+s[i]-'A'+10;
		else p=p*16+s[i]-'0';
	return p;
}
int getnum()
{
	int x=0,y=0,num=0;
	if(s[1]>='A')y=s[1]-'A'+10;else y=s[1]-'0';
	if(s[0]>='A')x=s[0]-'A'+10;else x=s[0]-'0';
	int p=1,t=1;
	while(y){
		if(y&p)num+=t,y-=p;
		p<<=1;t<<=1;
	}
	p=1;t=1<<4;
	while(x){
		if(x&p)num+=t,x-=p;
		p<<=1;t<<=1;
	}
	return num;
}
struct relay{
	int Imm,fun,fun2,rd,rs1,rs2;
	relay(){
		Imm=fun=fun2=rd=rs1=rs2=0;
	}
}tmp;
struct date{
	bool IR[32],forecast;
	int Op,Imm,fun,fun2,rd,rs1,rs2,pc,A,B,C,M;
	date(){
		memset(IR,0,sizeof IR);
		forecast=Op=Imm=fun=rd=rs1=rs2=pc=A=B=C=M=0;
	}
}IF_ID,ID_EX,EX_MEM,MEM_WB;
void preU()
{
	tmp.rd=0;
	for(int i=11;i>=7;i--)tmp.rd=tmp.rd*2+IF_ID.IR[i];
	tmp.Imm=0;
	for(int i=31;i>=12;i--)tmp.Imm=tmp.Imm*2+IF_ID.IR[i];
	tmp.Imm<<=12;
}
void preJ()
{
	tmp.rd=0;
	for(int i=11;i>=7;i--)tmp.rd=tmp.rd*2+IF_ID.IR[i];
	tmp.Imm=0;
	for(int i=31;i>=20;i--)tmp.Imm=tmp.Imm*2+IF_ID.IR[31];
	for(int i=19;i>=12;i--)tmp.Imm=tmp.Imm*2+IF_ID.IR[i];
	tmp.Imm=tmp.Imm*2+IF_ID.IR[20];
	for(int i=10;i>=1;i--)tmp.Imm=tmp.Imm*2+IF_ID.IR[i+20];
	tmp.Imm<<=1;
}
void preI()
{
	tmp.rd=0;
	for(int i=11;i>=7;i--)tmp.rd=tmp.rd*2+IF_ID.IR[i];
	tmp.fun=0;
	for(int i=14;i>=12;i--)tmp.fun=tmp.fun*10+IF_ID.IR[i];
	tmp.rs1=0;
	for(int i=19;i>=15;i--)tmp.rs1=tmp.rs1*2+IF_ID.IR[i];
	tmp.Imm=0;
	for(int i=31;i>=11;i--)tmp.Imm=tmp.Imm*2+IF_ID.IR[31];
	for(int i=10;i>=0;i--)tmp.Imm=tmp.Imm*2+IF_ID.IR[i+20];
}
void preB()
{
	tmp.fun=0;
	for(int i=14;i>=12;i--)tmp.fun=tmp.fun*10+IF_ID.IR[i];
	tmp.rs1=0;
	for(int i=19;i>=15;i--)tmp.rs1=tmp.rs1*2+IF_ID.IR[i];
	tmp.rs2=0;
	for(int i=24;i>=20;i--)tmp.rs2=tmp.rs2*2+IF_ID.IR[i];
	tmp.Imm=0;
	uint Imm=0;
	for(int i=31;i>=12;i--)Imm=Imm*2+IF_ID.IR[31];
	Imm=Imm*2+IF_ID.IR[7];
	for(int i=10;i>=5;i--)Imm=Imm*2+IF_ID.IR[i+20];
	for(int i=4;i>=1;i--)Imm=Imm*2+IF_ID.IR[i+7];
	Imm<<=1;
	tmp.Imm=(int)Imm;
}
void preS()
{
	tmp.fun=0;
	for(int i=14;i>=12;i--)tmp.fun=tmp.fun*10+IF_ID.IR[i];
	tmp.rs1=0;
	for(int i=19;i>=15;i--)tmp.rs1=tmp.rs1*2+IF_ID.IR[i];
	tmp.rs2=0;
	for(int i=24;i>=20;i--)tmp.rs2=tmp.rs2*2+IF_ID.IR[i];
	tmp.Imm=0;
	for(int i=31;i>=11;i--)tmp.Imm=tmp.Imm*2+IF_ID.IR[31];
	for(int i=10;i>=5;i--)tmp.Imm=tmp.Imm*2+IF_ID.IR[i+20];
	for(int i=4;i>=0;i--)tmp.Imm=tmp.Imm*2+IF_ID.IR[i+7];
}
void preR()
{
	tmp.rd=0;
	for(int i=11;i>=7;i--)tmp.rd=tmp.rd*2+IF_ID.IR[i];
	tmp.fun=0;
	for(int i=14;i>=12;i--)tmp.fun=tmp.fun*10+IF_ID.IR[i];
	tmp.rs1=0;
	for(int i=19;i>=15;i--)tmp.rs1=tmp.rs1*2+IF_ID.IR[i];
	tmp.rs2=0;
	for(int i=24;i>=20;i--)tmp.rs2=tmp.rs2*2+IF_ID.IR[i];
	tmp.fun2=0;
	for(int i=31;i>=25;i--)tmp.fun2=tmp.fun2*10+IF_ID.IR[i];
}
void deal_fc(int pc,int h,int f)
{
	if(f){
		fc[pc][h]=min(fc[pc][h]+1,3);
	}else{
		fc[pc][h]=max(fc[pc][h]-1,0);
	}
}
void doOp5(date x)
{
	int fun=x.fun,B=x.B,C=x.C,pc=x.pc,Imm=x.Imm;tot++;
	if(fun==0){
		if(B==C){
			if(!x.forecast)PC=pc+Imm,ID_oc=0;else yes++;
			deal_fc(x.pc,his[x.pc],1);
			his[x.pc]=((his[x.pc]<<1)&0xF)+1;
		}else{
			if(x.forecast)PC=pc+4,ID_oc=0;else yes++;
			deal_fc(x.pc,his[x.pc],0);
			his[x.pc]=((his[x.pc]<<1)&0xF);
		}
	}else if(fun==1){
		if(B!=C){
			if(!x.forecast)PC=pc+Imm,ID_oc=0;else yes++;
			deal_fc(x.pc,his[x.pc],1);
			his[x.pc]=((his[x.pc]<<1)&0xF)+1;
		}else{
			if(x.forecast)PC=pc+4,ID_oc=0;else yes++;
			deal_fc(x.pc,his[x.pc],0);
			his[x.pc]=((his[x.pc]<<1)&0xF);
		}
	}else if(fun==100){
		if(B<C){
			if(!x.forecast)PC=pc+Imm,ID_oc=0;else yes++;
			deal_fc(x.pc,his[x.pc],1);
			his[x.pc]=((his[x.pc]<<1)&0xF)+1;
		}else{
			if(x.forecast)PC=pc+4,ID_oc=0;else yes++;
			deal_fc(x.pc,his[x.pc],0);
			his[x.pc]=((his[x.pc]<<1)&0xF);
		}
	}else if(fun==101){
		if(B>=C){
			if(!x.forecast)PC=pc+Imm,ID_oc=0;else yes++;
			deal_fc(x.pc,his[x.pc],1);
			his[x.pc]=((his[x.pc]<<1)&0xF)+1;
		}else{
			if(x.forecast)PC=pc+4,ID_oc=0;else yes++;
			deal_fc(x.pc,his[x.pc],0);
			his[x.pc]=((his[x.pc]<<1)&0xF);
		}
	}else if(fun==110){
		if((uint)B<(uint)C){
			if(!x.forecast)PC=pc+Imm,ID_oc=0;else yes++;
			deal_fc(x.pc,his[x.pc],1);
			his[x.pc]=((his[x.pc]<<1)&0xF)+1;
		}else{
			if(x.forecast)PC=pc+4,ID_oc=0;else yes++;
			deal_fc(x.pc,his[x.pc],0);
			his[x.pc]=((his[x.pc]<<1)&0xF);
		}
	}else if(fun==111){
		if((uint)B>=(uint)C){
			if(!x.forecast)PC=pc+Imm,ID_oc=0;else yes++;
			deal_fc(x.pc,his[x.pc],1);
			his[x.pc]=((his[x.pc]<<1)&0xF)+1;
		}else{
			if(x.forecast)PC=pc+4,ID_oc=0;else yes++;
			deal_fc(x.pc,his[x.pc],0);
			his[x.pc]=((his[x.pc]<<1)&0xF);
		}
	}
}
int doOp6(date x)
{
	int LMD=0;
	int fun=x.fun,M=x.M;
	if(fun==0){
		for(int i=31;i>=24;i--)LMD=LMD*2+Mem[M]/(1<<7);
		int t=Mem[M];
		for(int i=1<<7;i>=1;i>>=1)LMD=LMD*2+t/i,t%=i;
	}else if(fun==1){
		for(int i=31;i>=16;i--)LMD=LMD*2+Mem[M+1]/(1<<7);
		int t=Mem[M+1];
		for(int i=1<<7;i>=1;i>>=1)LMD=LMD*2+t/i,t%=i;
		t=Mem[M];
		for(int i=1<<7;i>=1;i>>=1)LMD=LMD*2+t/i,t%=i;
	}else if(fun==10){
		int t=Mem[M+3];
		for(int i=1<<7;i>=1;i>>=1)LMD=LMD*2+t/i,t%=i;
		t=Mem[M+2];
		for(int i=1<<7;i>=1;i>>=1)LMD=LMD*2+t/i,t%=i;
		t=Mem[M+1];
		for(int i=1<<7;i>=1;i>>=1)LMD=LMD*2+t/i,t%=i;
		t=Mem[M];
		for(int i=1<<7;i>=1;i>>=1)LMD=LMD*2+t/i,t%=i;
	}else if(fun==100){
		int t=Mem[M];
		for(int i=1<<7;i>=1;i>>=1)LMD=LMD*2+t/i,t%=i;
	}else if(fun==101){
		int t=Mem[M+1];
		for(int i=1<<7;i>=1;i>>=1)LMD=LMD*2+t/i,t%=i;
		t=Mem[M];
		for(int i=1<<7;i>=1;i>>=1)LMD=LMD*2+t/i,t%=i;
	}
	return LMD;
}
void doOp7(date x)
{
	int t=x.M,fun=x.fun,C=x.C;
	if(fun==0){
		int tmp=(uint)C%(1<<8);
		Mem[t]=tmp;
	}else if(fun==1){
		int tmp=(uint)C%(1<<8);
		Mem[t]=tmp;
		tmp=(uint)C%(1<<16)/(1<<8);
		Mem[t+1]=tmp;
	}else if(fun==10){
		int tmp=(uint)C%(1<<8);
		Mem[t]=tmp;
		tmp=(uint)C%(1<<16)/(1<<8);
		Mem[t+1]=tmp;
		tmp=(uint)C%(1<<24)/(1<<16);
		Mem[t+2]=tmp;
		tmp=(uint)C/(1<<24);
		Mem[t+3]=tmp;
	}
}
int doOp8(date x)
{
	int ALUOut=0;
	int B=x.B,Imm=x.Imm,fun=x.fun;
	if(fun==0){
		ALUOut=B+Imm;
	}else if(fun==10){
		ALUOut=B<Imm;
	}else if(fun==11){
		ALUOut=(uint)B<(uint)Imm;
	}else if(fun==100){
		ALUOut=B^Imm;
	}else if(fun==110){
		ALUOut=B|Imm;
	}else if(fun==111){
		ALUOut=B&Imm;
	}else if(fun==1){
		ALUOut=B<<Imm;
	}else if(fun==101&&Imm/(1<<6)==0){
		ALUOut=(uint)B>>Imm;
	}else if(fun==101&&Imm/(1<<6)!=0){
		int t=Imm%(1<<6);
		ALUOut=B>>Imm;
		if((uint)B&(1<<31)){
			for(int i=0;i<Imm;i++)ALUOut+=(1<<(31-i));
		}
	}
	return ALUOut;
}
int doOp9(date x)
{
	int ALUOut=0;
	int B=x.B,Imm=x.Imm,fun=x.fun,C=x.C,fun2=x.fun2;
	if(fun==0&&fun2==0){
		ALUOut=B+C;
	}else if(fun==0&&fun2==100000){
		ALUOut=B-C;
	}else if(fun==1){
		ALUOut=B<<(C%(1<<5));
	}else if(fun==10){
		ALUOut=B<C;
	}else if(fun==11){
		ALUOut=(uint)B<(uint)C;
	}else if(fun==100&&fun2!=1){
		ALUOut=B^C;
	}else if(fun==101&&fun2==0){
		ALUOut=(uint)B>>((uint)C%(1<<5));
	}else if(fun==101&&fun2==100000){
		ALUOut=(uint)B>>((uint)C%(1<<5));
		if((uint)B&(1<<31)){
			for(int i=0;i<(uint)C%(1<<5);i++)ALUOut+=(1<<(31-i));
		}
	}else if(fun==110){
		ALUOut=B|C;
	}else if(fun==111){
		ALUOut=B&C;
	}else if(fun==0&&fun2==1){
		ALUOut=B*C;
	}else if(fun==100&&fun2==1){
		ALUOut=B/C;
	}
	return ALUOut;
}
int judge(int Op)
{
	if(Op==110111)return preU(),1;
	if(Op==10111)return preU(),2;
	if(Op==1101111)return preJ(),3;
	if(Op==1100111)return preI(),4;
	if(Op==1100011)return preB(),5;
	if(Op==11)return preI(),6;
	if(Op==100011)return preS(),7;
	if(Op==10011)return preI(),8;
	if(Op==110011)return preR(),9;
}
void IF()
{
	if(ID_oc)return;
	if(Mem[PC]==19&&Mem[PC+1]==5&&Mem[PC+2]==240&&Mem[PC+3]==15){
		isend=1;
		return;
	}
	IF_ID=date();
	int t=Mem[PC+3],p=24;
	while(t){
		if(t&1)IF_ID.IR[p]=1;
		t>>=1;p++;
	}
	t=Mem[PC+2],p=16;
	while(t){
		if(t&1)IF_ID.IR[p]=1;
		t>>=1;p++;
	}
	t=Mem[PC+1],p=8;
	while(t){
		if(t&1)IF_ID.IR[p]=1;
		t>>=1;p++;
	}
	t=Mem[PC],p=0;
	while(t){
		if(t&1)IF_ID.IR[p]=1;
		t>>=1;p++;
	}
	IF_ID.pc=PC;PC+=4;
	ID_oc=1;
}
void ID()
{
	if(!ID_oc||EX_oc)return;
	int op=0;
	for(int i=6;i>=0;i--)op=op*10+IF_ID.IR[i];
	tmp=relay();
	op=judge(op);
	if(tmp.rs1&&reg_num[tmp.rs1]||tmp.rs2&&reg_num[tmp.rs2])return;
	ID_oc=0;EX_oc=1;
	if(tmp.rd)reg_num[tmp.rd]++;
	ID_EX=IF_ID;
	ID_EX.Op=op;
	//Imm,fun,fun2,rd,rs1,rs2
	ID_EX.Imm=tmp.Imm;
	ID_EX.fun=tmp.fun;
	ID_EX.fun2=tmp.fun2;
	ID_EX.rd=tmp.rd;
	ID_EX.rs1=tmp.rs1;
	ID_EX.rs2=tmp.rs2;
	ID_EX.B=reg[tmp.rs1];
	ID_EX.C=reg[tmp.rs2];
	if(op==3){
		PC=ID_EX.pc+ID_EX.Imm;
	}else if(op==4){
		PC=(ID_EX.B+ID_EX.Imm)&~1;
	}else if(op==5){
		ID_EX.forecast=fc[ID_EX.pc][his[ID_EX.pc]]&2;
		if(ID_EX.forecast)PC=ID_EX.pc+ID_EX.Imm;
	}
	//printf("%d %d %d  %d %d %d %d %d\n",op,ID_EX.pc,tmp.rd,tmp.rs1,tmp.rs2,ID_EX.B,ID_EX.C,ID_EX.Imm);
}
void EX()
{
	if(!EX_oc||MEM_oc)return;
	EX_MEM=ID_EX;
	MEM_num=EX_oc=0;MEM_oc=1;
	int op=ID_EX.Op;
	if(op==1){
		EX_MEM.A=ID_EX.Imm;
	}else if(op==2){
		EX_MEM.A=ID_EX.pc+ID_EX.Imm;
	}else if(op==3||op==4){
		EX_MEM.A=ID_EX.pc+4;
	}else if(op==5){
		doOp5(ID_EX);
	}else if(op==6||op==7){
		EX_MEM.M=ID_EX.B+ID_EX.Imm;
	}else if(op==8){
		EX_MEM.A=doOp8(ID_EX);
	}else if(op==9){
		EX_MEM.A=doOp9(ID_EX);
	}
}
void MEM()
{
	if(!MEM_oc||WB_oc)return;
	MEM_WB=EX_MEM;
	int op=EX_MEM.Op;
	if(op==6||op==7){
		if(MEM_num<3)MEM_num++;
		else{
			if(op==6){
				MEM_WB.A=doOp6(EX_MEM);
			}else{
				doOp7(EX_MEM);
			}
			MEM_oc=0;WB_oc=1;
		}
	}else{
		MEM_oc=0;WB_oc=1;
	}
}
void WB()
{
	if(!WB_oc)return;
	WB_oc=0;
	int op=MEM_WB.Op;
	if(op!=5&&op!=7&&MEM_WB.rd){
		reg[MEM_WB.rd]=MEM_WB.A;
		reg_num[MEM_WB.rd]--;
	}
	if(op==8||op==9)cnt1++;
	if(op==3||op==4||op==5)cnt2++;
	if(op==6||op==7)cnt3++;
}
int main()
{
	freopen("qwq.out","r",stdin);
	while(scanf("%s",s)!=EOF){
		if(s[0]=='@')pos=getpos();
		else Mem[pos]=getnum(),pos++;
	}
	//freopen("CON","r",stdin);
	while(!isend||WB_oc||MEM_oc||EX_oc||ID_oc){
		WB(),MEM(),EX(),ID(),IF();
	}
	printf("%d\n",reg[10]);
	printf("%d %d %d\n",cnt1,cnt2,cnt3);
//	printf("%lf\n",1.0*tot);
}
