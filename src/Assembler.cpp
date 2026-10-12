#include<bits/stdc++.h>
using namespace std;

#define ln "\n";

unordered_map<string,string> opcodes = {{"R","0110011"},{"I","0010011"},{"B","1100011"},{"L","0000011"},{"S","0100011"},{"JAL","1101111"},{"JALR","1100111"}};

void file_i_o(){
    ios_base::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);
    freopen("input.txt","r",stdin);
}

string decToBinary(int num, int bits){
    unsigned int mask = (1u << bits) - 1;
    unsigned int value = num & mask;
    string rs = "";
    for(int i = bits-1; i >= 0; i--){
        rs += ((value >> i) & 1) ? '1' : '0';
    }
    return rs;
}

string regToBinary(string reg){
    string number= reg.substr(1,reg.size()-1);
    int temp = stoi(number);
    string rs = decToBinary(temp,5);
    return rs;
}

class Command{
    protected:
        string opcode;
    public:
        virtual string convert()=0;
};

class RType : public Command{
        string func7;
        string rs2;
        string rs1;
        string func3;
        string rd;
    public:
        RType(vector<string> instructions){
            enum R{
                ADD,SLT,SLTU,AND,OR,XOR,SLL,SRL,SUB,SRA,MUL,DIV,REM
            };
            unordered_map<string, R> strToR = {{"ADD",ADD},{"SLT",SLT},{"SLTU",SLTU},{"AND",AND},{"OR",OR},{"XOR",XOR},{"SLL",SLL},{"SRL",SRL},{"SUB",SUB},{"SRA",SRA},{"MUL",MUL},{"DIV",DIV},{"REM",REM}};
            opcode = opcodes["R"];
            int cmd = strToR[instructions[0]];
            switch(cmd){
                case ADD:
                    func7 = "0000000";
                    func3 = "000";
                    break;
                case SLT:
                    func7 = "0000000";
                    func3 = "010";
                    break;
                case SLTU:
                    func7 = "0000000";
                    func3 = "011";
                    break;
                case AND:
                    func7 = "0000000";
                    func3 = "111";
                    break;
                case OR:
                    func7 = "0000000";
                    func3 = "110";
                    break;
                case XOR:
                    func7 = "0000000";
                    func3 = "100";
                    break;
                case SLL:
                    func7 = "0000000";
                    func3 = "001";
                    break;
                case SRL:
                    func7 = "0000000";
                    func3 = "101";
                    break;
                case SUB:
                    func7 = "0000000";
                    func3 = "000";
                    break;
                case SRA:
                    func7 = "0000000";
                    func3 = "101";
                    break;
                case MUL:
                    func7= "0000001";
                    func3= "000";
                    break;
                case DIV:
                    func7= "0000001";
                    func3= "100";
                    break;
                case REM:
                    func7= "0000001";
                    func3= "110";
                    break;
                default:
                    cout << "ERROR : " << '"' << instructions[0] << '"' << " Command NOT found\n";
            }
            rd = regToBinary(instructions[1]);
            rs1 = regToBinary(instructions[2]);
            rs2 = regToBinary(instructions[3]);
        }
        
        string convert() override{
            return func7 + rs2 + rs1 + func3 + rd + opcode;
        }
};

class BType : public Command{
        string imm1;
        string rs2;
        string rs1;
        string func3;
        string imm2;
    public:
        BType(vector<string> instructions){
            enum B{
                BEQ=0,BNE=1,BLT=4,BGE=5
            };
            unordered_map<string, B> strToB = {{"BEQ",BEQ},{"BNE",BNE},{"BLT",BLT},{"BGE",BGE}};
            opcode = opcodes["B"];
            int cmd = strToB[instructions[0]];
            switch(cmd){
                case BEQ:
                    func3 = "000";
                    break;
                case BNE:
                    func3 = "001";
                    break;
                case BLT:
                    func3 = "100";
                    break;
                case BGE:
                    func3 = "101";
                    break;
                default:
                    cout << "ERROR : " << '"' << instructions[0] << '"' << " Command NOT found\n";
            }
            int imm = stoi(instructions[3]);
            string label = decToBinary(imm,12);
            imm1 = label.substr(7,5);
            imm2 = label.substr(0,7);
            rs1 = regToBinary(instructions[1]);
            rs2 = regToBinary(instructions[2]);
        }
        
        string convert() override{
            return imm2 + rs2 + rs1 + func3 + imm1 + opcode;
        }
};

class SType : public Command{
        string imm1;
        string rs2;
        string rs1;
        string func3;
        string imm2;
    public:
        SType(vector<string> instructions){
            enum S{
                SB,SHW,SW,SD
            };
            unordered_map<string, S> strToS = {{"SB",SB},{"SHW",SHW},{"SW",SW},{"SD",SD}};
            opcode = opcodes["S"];
            int cmd = strToS[instructions[0]];
            switch(cmd){
                case SB:
                    func3 = "000";
                    break;
                case SHW:
                    func3 = "001";
                    break;
                case SW:
                    func3 = "010";
                    break;
                case SD:
                    func3 = "011";
                    break;
                default:
                    cout << "ERROR : " << '"' << instructions[0] << '"' << " Command NOT found\n";
            }
            string temp = instructions[2];
            int imm=0,it=0;
            while(instructions[2][it]!='('){
                imm = imm*10 + (instructions[2][it]-'0');
                it++;
            }
            string offset = decToBinary(imm,12);
            imm1 = offset.substr(7,5);
            imm2 = offset.substr(0,7);
            rs1 = instructions[2].substr(it+1,instructions[2].size()-it-2);
            rs2 = regToBinary(instructions[1]);
        }
        
        string convert() override{
            return imm2 + rs2 + rs1 + imm1 + opcode;
        }
};

class IType : public Command{
    protected:
        string func3;
        string rd;
        string rs1;
        string imm;
    public:
        IType(){
            opcode = opcodes["I"];
        }
};

class ITypeArithmetic : public IType{
        string func7;
    public:
        ITypeArithmetic(vector<string> instructions){
            enum IR{
                SLLI,SRLI,SRAI
            };
            unordered_map<string,IR> strToI = {{"SLLI",SLLI},{"SRLI",SRLI},{"SRAI",SRAI}};
            int cmd = strToI[instructions[0]];
            switch(cmd){
                case SLLI:
                    func7 = "0000000";
                    func3 = "000";
                    break;
                case SRLI:
                    func7 = "0000000";
                    func3 = "101";
                    break;
                case SRAI:
                    func7 = "0100000";
                    func3 = "101";
                    break;
                default:
                    cout << "ERROR : " << '"' << instructions[0] << '"' << " Command NOT found\n";
            }
            rd = regToBinary(instructions[1]);
            rs1 = regToBinary(instructions[2]);
            int shamt = stoi(instructions[3]);
            string shamtBin = decToBinary(shamt,5);
            imm = func7 + shamtBin;
        }
        
        string convert() override{
            return imm + rs1 + func3 + rd + opcode;
        }
};

class ITypeShift : public IType{
    public:
        ITypeShift(vector<string> instructions){
            enum IS{
                ADDI,SLTI,SLTIU,XORI,ORI,ANDI
            };
            unordered_map<string,IS> strToI = {{"ADDI",ADDI},{"SLTI",SLTI},{"SLTIU",SLTIU},{"XORI",XORI},{"ORI",ORI},{"ANDI",ANDI}};
            int cmd = strToI[instructions[0]];
            switch(cmd){
                case ADDI:
                    func3 = "000";
                    break;
                case SLTI:
                    func3 = "010";
                    break;
                case SLTIU:
                    func3 = "011";
                    break;
                case XORI:
                    func3 = "100";
                    break;
                case ORI:
                    func3 = "110";
                    break;
                case ANDI:
                    func3 = "111";
                    break;
                default:
                    cout << "ERROR : " << '"' << instructions[0] << '"' << " Command NOT found\n";
            }
            rd = regToBinary(instructions[1]);
            rs1 = regToBinary(instructions[2]);
            int immediate = stoi(instructions[3]);
            imm = decToBinary(immediate,12);
        }
        
        string convert() override{
            return imm + rs1 + func3 + rd + opcode;
        }
};

class ITypeLoad : public IType{
    public:
        ITypeLoad(vector<string> instructions){
            opcode=opcodes["L"];
            enum LI{LB,LH,LW,LBU,LHU};
            unordered_map<string,LI> strtoload={{"LB",LB},{"LH",LH},{"LW",LW},{"LBU",LBU},{"LHU",LHU}};
            int cmd=strtoload[instructions[0]];
            switch(cmd){
                case LB:
                    func3="000";
                    break;
                case LH:
                    func3="001";
                    break;
                case LW:
                    func3="010";
                    break;
                case LBU:
                    func3="100";
                    break;  
                case LHU:
                    func3="101";
                    break;
                default:
                    cout << "ERROR : " << '"' << instructions[0] << '"' << " Command NOT found\n";
            }
            rd=regToBinary(instructions[1]);
            int immVal=0,it=0;
            while(instructions[2][it] != '('){
                immVal=immVal*10+(instructions[2][it]-'0');
                it++;
            }
            imm=decToBinary(immVal,12);
            rs1=regToBinary(instructions[2].substr(it+1,instructions[2].size()-it-2));
        }
        string convert() override{
            return imm+rs1+func3+rd+opcode;
        }
};

class JType : public Command{
    protected:
        string rd;
        string imm;
};

class JALType : public JType{
    public:
        JALType(vector<string> instructions){
            opcode = opcodes["JAL"];
            rd = regToBinary(instructions[1]);
            int immediate = stoi(instructions[2]);
            string imm20 = decToBinary(immediate,20);
            string imm_20 = imm20.substr(19,1);
            string imm_10_1 = imm20.substr(0,10);
            string imm_11 = imm20.substr(10,1);
            string imm_19_12 = imm20.substr(11,8);
            imm = imm_20 + imm_10_1 + imm_11 + imm_19_12;
        }
        
        string convert() override{
            return imm + rd + opcode;
        }
};

class JALRType : public JType{
    string rs1;
    string func3;
    public:
        JALRType(vector<string> instructions){
            opcode = opcodes["JALR"];
            rd = regToBinary(instructions[1]);
            rs1 = regToBinary(instructions[2]);
            int immediate = stoi(instructions[3]);
            imm = decToBinary(immediate,12);
            func3 = "000";
        }
        
        string convert() override{
            return imm + rd + opcode;
        }
};

string processInstruction(vector<string> &instruction){
    string cmd = instruction[0];
    unordered_set<string> Rset = {"ADD","SLT","SLTU","AND","OR","XOR","SLL","SRL","SUB","SRA","MUL","DIV","REM"};
    unordered_set<string> IArithset = {"ADDI","SLTI","SLTIU","XORI","ORI","ANDI"};
    unordered_set<string> IShiftset = {"SLLI","SRLI","SRAI"};
    unordered_set<string> Sset = {"SB","SHW","SW","SD"};
    unordered_set<string> Bset = {"BEQ","BNE","BLT","BGE"};
    unordered_set<string> ILoadset = {"LW","LH","LW","LBU","LHU"};
    
    Command* cmdObj = NULL;
    if(Rset.count(cmd)){
        cmdObj = new RType(instruction);
    }
    else if(IArithset.count(cmd)){
        cmdObj = new ITypeArithmetic(instruction);
    }
    else if(IShiftset.count(cmd)){
        cmdObj = new ITypeShift(instruction);
    }
    else if(ILoadset.count(cmd)){
        cmdObj = new ITypeLoad(instruction);
    }
    else if(Sset.count(cmd)){
        cmdObj = new SType(instruction);
    }
    else if(Bset.count(cmd)){
        cmdObj = new BType(instruction);
    }
    else if(cmd == "JAL"){
        cmdObj = new JALType(instruction);
    }
    else if(cmd == "JALR"){
        cmdObj = new JALRType(instruction);
    }
    else{
        return "ERROR : " + '"' + instruction[0] + '"' + " Command NOT found\n";
    }
    string result = cmdObj->convert();
    delete cmdObj;
    return result;
}

string trim(const string& str){
    size_t first = str.find_first_not_of(" \t");
    if(first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

vector<string> tokenize(string& line){
    vector<string> tokens;
    string token;
    for(char ch : line){
        if(ch == ' ' || ch == ','){
            if(!token.empty()){
                tokens.push_back(trim(token));
                token.clear();
            }
        }
        else{
            token += ch;
        }
    }
    if (!token.empty()) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

int main(){
    file_i_o();
    vector<string> instructions;
    string instruct;
    while(getline(cin ,instruct)){
        if(trim(instruct).empty()) continue;
        vector<string> tokens = tokenize(instruct);
        string binaryInstruction = processInstruction(tokens);
        cout << binaryInstruction << ln;
    }
    return 0;
}