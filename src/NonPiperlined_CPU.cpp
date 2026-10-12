#include <bits/stdc++.h>
using namespace std;

enum OPcode{
    RType,ITypeArith,ITypeLoad,BType,SType,JALType,JALRType
};

unordered_map<string,OPcode> OPcode_map = {
    {"0110011",RType},{"0010011",ITypeArith},{"0000011",ITypeLoad},
    {"1100011",BType},{"0100011",SType},{"1101111",JALType},{"1100111",JALRType}
};

string getbits(const string &instr,int hi,int lo){
    int start = 31 - hi;
    int len = hi - lo + 1;
    return instr.substr(start, len);
}

class InstrMemory{
    private:
        vector<string> instructions;
    public:
        InstrMemory(const vector<string> &binaryInstructions){
            this->instructions = binaryInstructions;
            return;
        }

        string getInstruction(int index) const{
            if(index>=0 && index<instructions.size())
                return instructions[index];
            return "";
        }
};

// Stage -1
class InstrFetch{
    public:
        string opcode;
        string func7;
        string func3;
        string rs1;
        string rs2;
        string immediate;
        string rd;

        InstrFetch(const string &instr){
            if(instr.size() != 32){
                opcode = "";
                return;
            }

            this->opcode = getbits(instr,6,0);
            switch(OPcode_map[this->opcode]){
                case RType:
                    this->func7     = getbits(instr,31,25);
                    this->rs2       = getbits(instr,24,20);
                    this->rs1       = getbits(instr,19,15);
                    this->func3     = getbits(instr,14,12);
                    this->rd        = getbits(instr,11,7);
                    this->immediate = "";
                    break;

                case ITypeArith:
                    this->immediate = getbits(instr,31,20);
                    this->rs1       = getbits(instr,19,15);
                    this->func3     = getbits(instr,14,12);
                    this->rd        = getbits(instr,11,7);
                    this->func7     = "";
                    this->rs2       = "";
                    break;

                case ITypeLoad:
                    this->immediate = getbits(instr,31,20);
                    this->rs1       = getbits(instr,19,15);
                    this->func3     = getbits(instr,14,12);
                    this->rd        = getbits(instr,11,7);
                    this->func7     = "";
                    this->rs2       = "";
                    break;

                case SType:
                    this->immediate = getbits(instr,31,25) + getbits(instr,11,7);
                    this->rs2       = getbits(instr,24,20);
                    this->rs1       = getbits(instr,19,15);
                    this->func3     = getbits(instr,14,12);
                    this->func7     = "";
                    this->rd        = "";
                    break;

                case BType:
                    this->immediate = getbits(instr,31,31) + getbits(instr,7,7) + getbits(instr,30,25) + getbits(instr,11,8) + "0";
                    this->rs1       = getbits(instr,19,15);
                    this->func3     = getbits(instr,14,12);
                    this->rd        = ""; 
                    this->rs2       = getbits(instr,24,20);
                    this->func7     = "";
                    break;

                case JALType:
                    this->immediate = getbits(instr,31,31) + getbits(instr,19,12) + getbits(instr,20,20) + getbits(instr,30,21) + "0";
                    this->rd        = getbits(instr,11,7);
                    this->func7     = "";
                    this->rs2       = "";
                    this->rs1       = "";
                    this->func3     = "";
                    break;

                case JALRType:
                    this->immediate = getbits(instr,31,20);
                    this->rs1       = getbits(instr,19,15);
                    this->func3     = getbits(instr,14,12);
                    this->rd        = getbits(instr,11,7);
                    this->func7     = "";
                    this->rs2       = "";
                    break;

                default:
                    this->immediate = "";
                    this->rd        = "";
                    this->func7     = "";
                    this->rs2       = "";
                    this->rs1       = "";
                    this->func3     = "";

                return;
            }
        }
};

// Stage -2
class ControlUnit{
    public:
        string ALUop;
        bool ALUsrc;
        bool regRead;
        bool regWrite;
        bool memRead;
        bool memWrite;
        bool mem2Reg;
        bool branch;
        bool jump;

        ControlUnit(const InstrFetch &IFID){
            switch(OPcode_map[IFID.opcode]){
                case RType:
                    this->ALUop     = "10";
                    this->ALUsrc    = false;
                    this->regWrite  = true;
                    this->regRead   = true;
                    this->memRead   = false;
                    this->memWrite  = false;
                    this->mem2Reg   = false;
                    this->branch    = false;
                    this->jump      = false;
                    break;

                case ITypeArith:
                    this->ALUop     = "10";
                    this->ALUsrc    = true;
                    this->regWrite  = true;
                    this->regRead   = true;
                    this->memRead   = false;
                    this->memWrite  = false;
                    this->mem2Reg   = false;
                    this->branch    = false;
                    this->jump      = false;
                    break;

                case ITypeLoad:
                    this->ALUop     = "00";
                    this->ALUsrc    = true;
                    this->regWrite  = true;
                    this->regRead   = true;
                    this->memRead   = true;
                    this->memWrite  = false;
                    this->mem2Reg   = true;
                    this->branch    = false;
                    this->jump      = false;
                    break;

                case SType:
                    this->ALUop     = "00";
                    this->ALUsrc    = true;
                    this->regWrite  = false;
                    this->regRead   = true;
                    this->memRead   = false;
                    this->memWrite  = true;
                    this->mem2Reg   = false;
                    this->branch    = false;
                    this->jump      = false;
                    break;

                case BType:
                    this->ALUop     = "01";
                    this->ALUsrc    = false;
                    this->regWrite  = false;
                    this->regRead   = true;
                    this->memRead   = false;
                    this->memWrite  = false;
                    this->mem2Reg   = false;
                    this->branch    = true;
                    this->jump      = false;
                    break;

                case JALType:
                    this->ALUop     = "00";
                    this->ALUsrc    = false;
                    this->regWrite  = true;  
                    this->regRead   = false;
                    this->memRead   = false;
                    this->memWrite  = false;
                    this->mem2Reg   = false;
                    this->branch    = false;
                    this->jump      = true;
                    break;

                case JALRType:
                    this->ALUop     = "00";
                    this->ALUsrc    = false;
                    this->regWrite  = true;
                    this->regRead   = true;
                    this->memRead   = false;
                    this->memWrite  = false;
                    this->mem2Reg   = false;
                    this->branch    = false;
                    this->jump      = true;
                    break;

                default:
                    this->ALUop     = "";
                    this->ALUsrc    = false;
                    this->regWrite  = false;
                    this->regRead   = false;
                    this->memRead   = false;
                    this->memWrite  = false;
                    this->mem2Reg   = false;
                    this->branch    = false;
                    this->jump      = false;

                return;
            }
        }
};

int signExtendToInt(const string &bits){
    if(bits.empty()) return 0;
    int n = bits.size();
    int val = stoi(bits, nullptr, 2);
    if(bits[0] == '1') val -= (1 << n);
    return val;
}

int getImmediateValue(const string &imm){
    return signExtendToInt(imm);
}

class RegisterFile{
    public:
        vector<int> registers;

        RegisterFile(){
            registers = vector<int> (32,0);
            return;
        }

        int read(const string &reg){
            if(reg.empty())
                return 0;
            int idx = stoi(reg,nullptr,2);
            if(idx<0 || idx>=32)
                return 0;
            return registers[idx];
        }

        void write(const string &reg,int val){
            if(reg.empty())
                return;
            int idx = stoi(reg,nullptr,2);
            if(idx<0 || idx>=32)
                return;
            registers[idx] = val;
            return;
        }
};

// Stage -3
class ALUcontrol{
    public:
        string ALUselect;

        ALUcontrol(string ALUOp,string func7,string func3){
            if(ALUOp == "00"){                                // L/S type, 
                this->ALUselect = "0000"; // ADD
            }
            else if(ALUOp == "01"){                           // Branch type:
                if(func3 == "000") this->ALUselect = "1001"; // BEQ
                else if(func3 == "001") this->ALUselect = "1010"; // BNE
                else if(func3 == "100") this->ALUselect = "1011"; // BLT
                else if(func3 == "101") this->ALUselect = "1100"; // BGE
                else this->ALUselect = "1001"; // default BEQ
            }
            else if(ALUOp == "10"){
                if(func3 == "000"){
                    if(func7 == "0000000" || func7 == "")
                        this->ALUselect = "0000";             // ADD / ADDI
                    else if(func7 == "0100000")
                        this->ALUselect = "0001";             // SUB
                }
                else if(func3 == "111") this->ALUselect = "0010";  // AND / ANDI
                else if(func3 == "110") this->ALUselect = "0011";  // OR  / ORI
                else if(func3 == "100") this->ALUselect = "0100";  // XOR / XORI
                else if(func3 == "001") this->ALUselect = "0101";  // SLL / SLLI
                else if(func3 == "101"){
                    if(func7 == "0000000")
                        this->ALUselect = "0110";             // SRL / SRLI
                    else if(func7 == "0100000")
                        this->ALUselect = "0111";             // SRA / SRAI
                }
                else if(func3 == "010") this->ALUselect = "1000";  // SLT / SLTI
                else this->ALUselect = "0000";
            }
            else {
                this->ALUselect = "0000";
            }
        }
};

class ALU{
    public:
        int ALUresult;
        bool Bout;

        ALU(const ALUcontrol &slct,const int &ALUsrc1,const int &ALUsrc2){
            this->ALUresult=0;
            this->Bout = false;

            if(slct.ALUselect == "0000"){                   // ADD ADDI
                this->ALUresult = ALUsrc1 + ALUsrc2;
            }
            else if(slct.ALUselect == "0001"){              // SUB
                this->ALUresult = ALUsrc1 - ALUsrc2;
            }
            else if(slct.ALUselect == "0010"){              // AND
                this->ALUresult = ALUsrc1 & ALUsrc2;
            }
            else if(slct.ALUselect == "0011"){              // OR
                this->ALUresult = ALUsrc1 | ALUsrc2;
            }
            else if(slct.ALUselect == "0100"){              // XOR
                this->ALUresult = ALUsrc1 ^ ALUsrc2;
            }
            else if(slct.ALUselect == "0101"){              // SLL
                this->ALUresult = ALUsrc1 << (ALUsrc2 & 0x1F);
            }
            else if(slct.ALUselect == "0110"){              // SRL
                this->ALUresult = (unsigned int)ALUsrc1 >> (ALUsrc2 & 0x1F);
            }
            else if(slct.ALUselect == "0111"){              // SRA
                this->ALUresult = ALUsrc1 >> (ALUsrc2 & 0x1F);
            }
            else if(slct.ALUselect == "1000"){              // SLT
                this->ALUresult = (ALUsrc1 < ALUsrc2);
            }
            else if(slct.ALUselect == "1001"){              // BEQ
                this->ALUresult = ALUsrc1 - ALUsrc2;
                this->Bout = (this->ALUresult == 0);
            }
            else if(slct.ALUselect == "1010"){              // BNE
                this->ALUresult = ALUsrc1 - ALUsrc2;
                this->Bout = (this->ALUresult != 0);
            }
            else if(slct.ALUselect == "1011"){              // BLT
                this->ALUresult = ALUsrc1 - ALUsrc2;
                this->Bout = (this->ALUresult < 0);
            }
            else if(slct.ALUselect == "1100"){              // BGE
                this->ALUresult = ALUsrc1 - ALUsrc2;
                this->Bout = (this->ALUresult >= 0);
            }
        }

};

// Stage -4
class DataMemory{
    public:
        vector<int> memory;
        DataMemory(){ memory.assign(1024,0); }
        int read(int address){
            if(address < 0) return 0;
            int idx = address / 4;
            if(idx < 0 || idx >= (int)memory.size()) return 0;
            return memory[idx];
        }
        void write(int address, int value){
            if(address < 0) return;
            int idx = address / 4;
            if(idx < 0 || idx >= (int)memory.size()) return;
            memory[idx] = value;
        }
};

// Stage -5
class CPU{
    public:
        InstrMemory instrMem;
        DataMemory dataMem;
        RegisterFile regFile;
        int pc;

        CPU(vector<string>& instrs) : instrMem(instrs), dataMem(), regFile(), pc(0) {}
        void run(){
            cout<<"---------------- CPU Execution Start ----------------"<<endl;
            while(true){
                string instr = instrMem.getInstruction(pc/4);
                if(instr.empty()){
                    cout<<"---------------- CPU Execution End ----------------"<<endl;
                    break;
                }

                cout << "[PC = " << pc << "] Instruction: " << instr << endl;

                InstrFetch IF(instr);
                ControlUnit CU(IF);
                ALUcontrol ALUCtrl(CU.ALUop, IF.func7, IF.func3);
                int rs1 = regFile.read(IF.rs1);
                int rs2 = regFile.read(IF.rs2);
                int imm = getImmediateValue(IF.immediate);
                int ALUin2 = CU.ALUsrc ? imm : rs2;
                ALU alu(ALUCtrl, rs1, ALUin2);
                cout<<"inp1 = "<<rs1<<", inp2 = "<<ALUin2<<endl;
                cout << "ALU result = " << alu.ALUresult << endl;

                int memData = 0;
                if(CU.memRead){
                    memData = dataMem.read(alu.ALUresult);
                    cout << "Memory Read: Address = " << alu.ALUresult << ", Data = " << memData << endl;
                }
                if(CU.memWrite){
                    dataMem.write(alu.ALUresult, rs2);
                    cout << "Memory Write: Address = " << alu.ALUresult << ", Data = " << rs2 << endl;
                }

                int writeData = CU.mem2Reg ? memData : alu.ALUresult;

                
                if(CU.jump && OPcode_map[IF.opcode] == JALType){
                    writeData = pc + 4;
                }

                if(CU.regWrite){
                    regFile.write(IF.rd, writeData);
                    cout << "Register Write: Register = " << IF.rd << ", Data = " << writeData << endl;
                }

               
                if(CU.jump){
                    if(OPcode_map[IF.opcode] == JALType){
                        pc = pc + imm;
                    } else if(OPcode_map[IF.opcode] == JALRType){
                        // JALR target = (rs1 + imm) & ~1
                        int target = (rs1 + imm) & (~1);
                        pc = target;
                    }
                } else if(CU.branch && alu.Bout){
                    pc = pc + imm;
                } else {
                    pc = pc + 4;
                }

                cout<<"Registers"<<endl;
                for(int i=0;i<32;i++) cout<<"x"<<i<<": "<<regFile.registers[i]<<endl;
            }
            cout << "------------------ CPU HALT ------------------" << endl;
        }
};

int main(){
    // Correct 32-bit binary encodings for the intended instructions:
    // 1) ADDI x1, x0, 1
    // 2) ADDI x2, x0, 2
    // 3) ADDI x3, x0, 3
    // 4) ADD  x4, x1, x2
    // 5) ADD  x6, x3, x2
    // 6) SW   x4, 0(x5)
    // 7) LW   x9, 0(x5)   -- to read back stored x4
    vector<string> instructions = {
        // -------- R-TYPE --------
    "00000000011100110000001010110011", // add  x5, x6, x7
    "01000000011100110000001010110011", // sub  x5, x6, x7
    "00000000011100110111001010110011", // and  x5, x6, x7
    "00000000011100110110001010110011", // or   x5, x6, x7

    // -------- I-TYPE (ALU) --------
    "00000000001100000000000110010011", // addi x3, x0, 3
    "00000000010100011000001000010011", // addi x4, x3, 5
    "00000000110000100111010000010011", // andi x8, x4, 12
    "00000000011100100110010010010011", // ori  x9, x4, 7

    // -------- I-TYPE (LOAD) --------
    "00000000100000011010010100000011", // lw   x10, 8(x3)

    // -------- S-TYPE --------
    "00000000101000011010001100100011", // sw   x10, 12(x3)
    "00000000010100100010001000100011", // sw   x5, 4(x4)

    // -------- B-TYPE --------
    "00000000010000011000010001100011", // beq  x3, x4, 16
    "11111100011000101001111001100011", // bne  x5, x6, -8

    // -------- J-TYPE --------
    "00000000001000000000000011101111", // jal  x1, 32
    "00000000000000000000000000010011"  // jal  x0, -4

    };

    CPU cpu(instructions);
    cpu.run();
    return 0;
}