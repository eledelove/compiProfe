#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <list>
#include <string.h>
#include <sstream>
#include <vector>
#include <map>

using namespace std;

class cToken
{
protected:
    string nomToken;
    string lexema;

public:
    void setNomToken(string nT){
        this->nomToken=nT;
    }
    void setLexema(string lex="123.34"){
        this->lexema=lex;
    }
    string getNomToken(){
        return this->nomToken;
    }
    string getLexema(){
        return this->lexema;
    }

};

template <class T>
class cTokenNum : public cToken{
    T valorNum;

public:
    void setValor(T vN){
        this->valorNum=vN;
    }
    T getValor(){
        return this->valorNum;
    }

    T convierteToNum(){

        stringstream aux(this->lexema);
        aux>>valorNum;
        return valorNum;
    }

};

class cAnalisisLexico{

    ifstream in;
    ofstream out;
    ifstream palabrasReservadas;
    list<string> listaPalabras;
    vector<cToken> bufferToken;
    typedef void (cAnalisisLexico::*punteroFuncionToken)();
    char c;

    map<char,punteroFuncionToken> tipoToken;

    char* archivoSalida;

public:
    cAnalisisLexico(){}


    bool esReservada(char* id){
        bool reservada = false;

        for(list<string>::iterator it = listaPalabras.begin(); it != listaPalabras.end(); it ++){
            if(strcmp(id, (*it).c_str())==0)
            {
                reservada = true;
                break;

            }
        }
        return reservada;
    }

    cAnalisisLexico (char* s, char* salida){
        in.open(s);

        try{
            if(in.fail())
                throw 1;
            cout<<"Exito al abrir el archivo"<<s<<endl;

            palabrasReservadas.open("palabrasReservadas.txt");

            if(palabrasReservadas.fail())
                throw 2;
            cout<<"Exito al abrir el archivo palabrasReservadas.txt"<<endl;

            string cad;

            while(!palabrasReservadas.eof())
            {
                palabrasReservadas>>cad;
                listaPalabras.push_back(cad);
            }

            out.open(salida);
        }catch(int i){
            if(i==1)
                cout<<"Error al abrir el archivo para analizar"<<endl;
            else if(i==2)
                cout<<"Error al abrir archivo de palabrasReservadas.txt"<<endl;
            else
                cout<<"Error no definido"<<endl;
        }
        cout<<"Constructor dos"<<endl;
    }


    void tokenID(){
        string id;
        id = c;
        c = in.get();

        while(isalnum(c) || c=='_')
        {
            id += c;
            c = in.get();
        }

        if(!esReservada((char*)id.c_str()))
            out<<"(TokID,"<<id<<")";
        else
            out<<"(TokPR,"<<id<<")";

        in.unget();
    }

    void tokenComentario()
    {
        c = in.get();

        if(c=='/')
        {
            c = in.get();
            while(c!='\n')
                c = in.get();

            in.unget();
            out<<"TokComentarioCorto";
        }
        else if(isalpha(c))
        {
            out<<"TokDiv";
            in.unget();
        }
        else if(c=='*')
        {
            char d;
            do
            {
                while(in.get()!='*');
                in.unget();
                while(in.get()=='*');
                in.unget();
                if((d=in.get())=='/')
                {
                    out<<"TokComentarioLargo"<<endl;
                    in.unget();
                    break;
                }
            }while(d!='/');
        }
    }


    void tokenPI(){
        out<<"TokPI(";
    }

    void tokenPyC(){
        out<<"TokPyC"<<endl;
    }

    void tokenMenor(){
        out<<"TokMenor";
    }

    void tokenMayor(){
        out<<"TokMayor";
    }

    void tokenCorIzq(){
        out<<"TokLlave{"<<endl;
    }

    void tokenNumero(){

        string num;
        num = c;
        c = in.get();
        if(c == '+')
            out<<"(TokIncre,++)";
        else if(c == '-')
            out<<"(TokDecre,--)";
        else if(c == '=')
            out<<"(TokIncre,+=)";
        else if(isalpha(c) && toupper(c)!='E')
            out<<"(TokOpArit,"<<c;
        else
        {
            while(isdigit(c))
            {
                num += c;
                c = in.get();
            }
            if(c == '.')
            {
                num += c;
                c = in.get();

                while(isdigit(c))
                {
                    num += c;
                    c = in.get();
                }
                if (toupper(c)!='E')
                    out <<"TokFloat"<<num<<endl;
                else
                {
                    parteFinalAutomata:
                        num += c;
                        c = in.get();

                        if(c == '+' || c == '-' || isdigit(c))
                        {
                            num += c;
                            c = in.get();
                            while(isdigit(c))
                            {
                                num += c;
                                c = in.get();
                            }
                            out<<"TokNumExp"<<num<<endl;
                        }
                        else
                            out<<"Error en formato exponencial";
                }
            }

            else if(toupper(c) == 'E')
                goto parteFinalAutomata;

            else
                out<<"(TokInt,"<<num;

            in.unget();
        }
    }

    void tokenCadena(){

        string cad;
        cad = c;
        c = in.get();

        while((isalnum(c) || c == ' ') && c != '"')
        {
            cad += c;
            c = in.get();
        }
        //if(c == '"') cad += c;
        out<<"(TokenCadena,"<<cad<<")";

        in.unget();
    }


    void recuperaTokens(){

        string id;

        tipoToken['('] = &cAnalisisLexico::tokenPI;
        tipoToken[';'] = &cAnalisisLexico::tokenPyC;
        tipoToken['<'] = &cAnalisisLexico::tokenMenor;
        tipoToken['>'] = &cAnalisisLexico::tokenMayor;
        tipoToken['{'] = &cAnalisisLexico::tokenCorIzq;
        tipoToken['a'] = &cAnalisisLexico::tokenID;
        tipoToken['_'] = tipoToken['a'];

        for(int i = 97; i <= 122; i++)
            tipoToken[(char)i] = tipoToken['a'];
        for(int i = 65; i <= 90; i++)
            tipoToken[(char)i] = tipoToken['a'];

        tipoToken['/'] = &cAnalisisLexico::tokenComentario;
        tipoToken['+'] = &cAnalisisLexico::tokenNumero;
        tipoToken['-'] = tipoToken['+'];

        tipoToken['"'] = &cAnalisisLexico::tokenCadena;
        tipoToken[' '] = tipoToken['"'];


        for(int i = 48; i <= 57; i++)
            tipoToken[(char)i] = tipoToken['+'];

        map<char, punteroFuncionToken>::iterator it;
        while(!in.eof())
        {

            c = in.get();

            if(in.eof() || in.fail()) break;
            /*if(isdigit(c)) (this->*tipoToken['n'])();

            if(c == '(' || c == ';' || c == '>' || c == '<' || c == '{' || isalpha(c) || c == '_' || c == '/' || c == '+' || c == '-')
                (this->*tipoToken[c])();*/

            it=tipoToken.find(c);
            if(it!=tipoToken.end())
            (this->*tipoToken[c])();

            else
                out<<c;
        }
    }

};

class cAnalisisSintactica{

};


int main(int nargs, char** args){

    const char * sal = "miAnalisis.txt";

    if(nargs == 3)
        sal = args[2];

    cAnalisisLexico analisis1(args[1], (char*)sal);
    analisis1.recuperaTokens();


    return 0;
}

