#include<stdio.h>
#include<string.h>
#include<stdlib.h>
////////////////////////////////////////////////////////////////////////////////////////////////////
//Declare states for comment remover.
typedef enum{
	code,slcread,mlcread,cschk,cechk
}readstate;
//Declare states for token identification.
typedef enum{
	unrec,var,varf,keyword,func,consr,consf,conserr,op,strr,charr
}tokentype;
//Struct to store identified tokens
typedef struct node{
	char str[52];
	struct node *ptr;
}string;
////////////////////////////////////////////////////////////////////////////////////////////////////
//Function declaration
readstate lexAn(readstate,char);
void rmcomments();
void generate_tokens();
tokentype tokenAnalyser(tokentype,char,FILE *);
void writetofile(tokentype,FILE *);
tokentype checkType(tokentype,char);
////////////////////////////////////////////////////////////////////////////////////////////////////
//Global variables
int bptr=0;
char buffer[200];
char keywords[33][11]={"auto","this","struct","break","case","const","continue",
"double","int","char","float","long","short","default",
"do","else","enum","extern","for","goto","if","register","return","signed","sizeof",
"static","switch","typedef","union","unsigned","void","volatile","while"};
int idfd_ptr=0;
string *identified=NULL;
////////////////////////////////////////////////////////////////////////////////////////////////////
void main(){
        rmcomments();  //remove comments from the source code.
        generate_tokens(); //generate tokens.
} //end of main()
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
		:: CODE TO REMOVE COMMENT LINES STARTS HERE ::
*/
void rmcomments(){
	FILE *frp;FILE *fwp;
	readstate st=code;
	frp=fopen("ipcode.txt","r");
	fwp=fopen("opcode.txt","w");
	char ch;
	char buff;
	while(!feof(frp)){
		ch=getc(frp);
		st=lexAn(st,ch);
		switch(st){
			case code:
				putc(ch,fwp);
				break;
			case cschk:
				buff=ch;
				ch=getc(frp);
				st=lexAn(st,ch);
				if(st==code){
					putc(buff,fwp);
					putc(ch,fwp);
				}
				buff='\t';
				break;
			case slcread:
				ch=getc(frp);
				while(ch!='\n'){
					ch=getc(frp);
				}
				putc(ch,fwp);
				st=lexAn(st,ch);
				break;
			case mlcread:
				ch=getc(frp);
				while(ch!='*'){
					ch=getc(frp);
				}
				st=lexAn(st,ch);
				ch=getc(frp);
				st=lexAn(st,ch);
				break;
			case cechk:
				ch=getc(frp);
				st=lexAn(st,ch);
				break;
		}
	}
	fclose(frp);fclose(fwp);
}
readstate lexAn(readstate s,char c){
	switch(s){
		case code:
			if(c=='/')
				return cschk;
			break;
		case cschk:
			if(c=='/')
				return slcread;
			else if(c=='*')
				return mlcread;
			else
				return code;
			break;
		case slcread:
			if(c=='\n')
				return code;
			break;
		case mlcread:
			if(c=='*')
				return cechk;
			break;
		case cechk:
			if(c=='/')
				return code;
			else
				return mlcread;
			break;
	}
	return s;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
/*
		:: CODE TO GENERATE TOKENS STARTS HERE ::
*/
void generate_tokens(){
	FILE *frp=fopen("opcode.txt","r");
	FILE *fwp=fopen("table.txt","w");
	char ch;
	tokentype tt=unrec;
	while(!feof(frp)){
		ch=getc(frp);
		tt=tokenAnalyser(tt,ch,fwp);
		switch(tt){
			case varf:
				writetofile(tt,fwp);
				tt=unrec;
				break;
			case consf:
				buffer[bptr++]=ch;
				buffer[bptr]='\0';
				writetofile(tt,fwp);
				tt=unrec;
				break;
			case strr:
				ch=getc(frp);
				while(((int)ch)!=34){
					buffer[bptr++]=ch;
					ch=getc(frp);
				}
				writetofile(tt,fwp);
				tt=unrec;
				break;
			case unrec:
				break;
			default:
				buffer[bptr++]=ch;
				
		}//end of switch()
	}//end of while(!feof(frp))
	fclose(frp);fclose(fwp);
}//end of generate tokens()
tokentype tokenAnalyser(tokentype t,char c,FILE *fp){
	int ascii=(int)c;
	switch(t){
		case unrec:
			switch(c){
				case '"':
					return strr;
				default:
					if((ascii>=65 && ascii<=90) || (ascii>=97 && ascii<=122)) //check for alphabets
						return var;
					else if(ascii>=48 && ascii<=57) //check for digits
						return consr;
					else if((ascii>=42 && ascii<=43) || ascii==47 || ascii==61) //check for operators
						return op;
					else if(ascii=10 || ascii==32) //check for newline and space
						return unrec;				
			} //switch(c) ends here.
		case var:
			switch(c){
				case '"':
					writetofile(checkType(varf,c),fp);
					return strr;
				default:
					if((ascii>=48 && ascii<=57) || (ascii>=65 && ascii<=90) || (ascii>=97 && ascii<=122)||ascii==95)//check for alphabets or digits
						return var;
					if((ascii>=40 && ascii<=43) || ascii==47 || ascii==61){ //check for operators
						writetofile(checkType(varf,c),fp);
						if(ascii!=40 && ascii!=41){
							strcpy(buffer,&c);
							return op;
						}
						return unrec;
					}
					if(ascii=10 || ascii==32 || c==';') //check for newline,space or end of statements
						return varf;				
			} //switch(c) ends here.
		case op:
			switch(c){
				case '"':
					writetofile(op,fp);		
					strcpy(buffer,&c);
					return strr;
				default:
					if((ascii>=42 && ascii<=43) || ascii==47 || ascii==61) //check for operators
						return op;
					else if((ascii>=65 && ascii<=90) || (ascii>=97 && ascii<=122)){ //check for alphabets
						writetofile(op,fp);
						strcpy(buffer,&c);
						return var;
					}
					else if(ascii>=48 && ascii<=57){ //check for digits
						writetofile(op,fp);
						strcpy(buffer,&c);
						return consr;
					}
					else{
						writetofile(op,fp);
						return unrec;
					}
			} //switch(c) ends here.
		case consr:
			switch(c){
				case '"':
					writetofile(consf,fp);
					return strr;
				default:
					if(ascii>=48 && ascii<=57) // check for digits
						return consr;
					if((ascii>=42 && ascii<=43) || ascii==47 || ascii==61){ //check for operators
						writetofile(consf,fp);
						strcpy(buffer,&c);
						return op;
					}
					if((ascii>=65 && ascii<=90) || (ascii>=97 && ascii<=122)) //check for alphabets
						return conserr;
					if(ascii=10 || ascii==32 || c==';'){ //check for newline,space or end of statements
						writetofile(consf,fp);
						strcpy(buffer,&c);
						return unrec;
					}
			} //switch(c) ends here.
		default:
			return t;
	} //end of switch()
}//end of tokenAnalyser()
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
		:: CODE FOR WRITING TO FILE ::
*/
void writetofile(tokentype t,FILE *fp){
		int i,c=0;
		buffer[bptr]='\0';
		if(t==varf)
			t=checkType(t,'\t');
		if(identified==NULL){
				string *newtoken=malloc(sizeof(string));
				identified=newtoken;
				newtoken->ptr=NULL;
				strcpy(newtoken->str,buffer);
			}
		else{
			string *temp=identified;
			while(temp!=NULL){
				if(!strcmp(buffer,temp->str)){
					c++;
					break;
				}
				temp=temp->ptr;
			}
		}
		if(c==0){
			switch(t){
					case 2:
						fprintf(fp,">> %-101s -- Variable\n",buffer);
						break;
					case 3:
						fprintf(fp,">> %-101s -- Keyword\n",buffer);
						break;
					case 4:
						fprintf(fp,">> %-101s -- Function\n",buffer);
						break;
					case 6:
						fprintf(fp,">> %-101s -- Constant\n",buffer);
						break;
					case 8:
						fprintf(fp,">> %-101s -- Operator\n",buffer);
						break;
					case 9:
						fprintf(fp,">> %-101s -- String\n",buffer);
						break;
					case 10:
						fprintf(fp,">> %-101s -- Character\n",buffer);
						break;
				}
			fprintf(fp,"\n");
			string *t=identified;
			while(t->ptr!=NULL)
				t=t->ptr;
			string *newtoken=malloc(sizeof(string));
			t->ptr=newtoken;
			newtoken->ptr=NULL;
			strcpy(newtoken->str,buffer);
		}
		strcpy(buffer,"");
		bptr=0,c=0;
} //end of writetofile,()
/////////////////////////////////////////////////////////////////////////////////////////////////////
/*
		SUPPORT FUNCTIONS
*/
tokentype checkType(tokentype t,char c){
		int i;
		if(t==varf && c=='(')
			return func;
		for(i=0;i<33;i++)
			if(!strcmp(buffer,keywords[i]))
				return keyword;
		return t;
}//end of checkType()
