#include <bits/stdc++.h>
using namespace std;

vector<string> loadInstructions(const string &filename) {
    vector<string> instructions;
    ifstream file(filename);
    string line;

    while (getline(file, line)) {
        if (!line.empty())
            instructions.push_back(line);
    }

    return instructions;
}

enum OPcode { RType, ITypeArith, ITypeLoad, BType, SType, JALType, JALRType, UnknownType };

unordered_map<string, OPcode> OPcode_map = {
    {"0110011", RType}, {"0010011", ITypeArith}, {"0000011", ITypeLoad},
    {"1100011", BType}, {"0100011", SType},     {"1101111", JALType},
    {"1100111", JALRType}
};

string getbits(const string &instr, int hi, int lo) {
    if(instr.size() != 32) return "";
    int start = 31 - hi;
    int len = hi - lo + 1;
    return instr.substr(start, len);
}

int signExtendToInt(const string &bits) {
    if(bits.empty()) return 0;
    int n = bits.size();
    int val = stoi(bits, nullptr, 2);
    if(bits[0] == '1') val -= (1 << n);
    return val;
}

int bitstrToIdx(const string &s) {
    if(s.empty()) return -1;
    return stoi(s, nullptr, 2);
}

class InstrMemory {
public:
    vector<string> instructions;
    InstrMemory() {}
    InstrMemory(const vector<string> &ins) : instructions(ins) {}
    string getInstruction(int idx) const {
        if(idx < 0 || idx >= (int)instructions.size()) return "";
        return instructions[idx];
    }
    int size() const { return (int)instructions.size(); }
};

class DataMemory {
public:
    vector<int> mem;
    DataMemory() { mem.assign(1024, 0); }
    int read(int address) {
        if(address < 0) return 0;
        int idx = address / 4;
        if(idx < 0 || idx >= (int)mem.size()) return 0;
        return mem[idx];
    }
    void write(int address, int value) {
        if(address < 0) return;
        int idx = address / 4;
        if(idx < 0 || idx >= (int)mem.size()) return;
        mem[idx] = value;
    }
};

class RegisterFile {
public:
    vector<int> regs;
    RegisterFile() { regs.assign(32, 0); }
    int readIdx(int idx) const {
        if(idx < 0 || idx >= 32) return 0;
        return regs[idx];
    }
    void writeIdx(int idx, int val) {
        if(idx <= 0 || idx >= 32) return; 
        regs[idx] = val;
    }
    void dump0to31() const {
        for(int i=0;i<32;i++){
            cout << "x" << i << ":" << regs[i] << endl;
        }
    }
};

struct IFID {
    bool valid = false;
    int pc = 0;
    string instr = "";
    string opcode="", func7="", func3="", rs1="", rs2="", rd="", immediate="";
    OPcode type = UnknownType;
};
 
struct IDEX {
    bool valid = false;
    int pc = 0;
    string opcode="", func7="", func3="", rs1="", rs2="", rd="", immediate="";
    OPcode type = UnknownType;
    string ALUop = "";
    bool ALUsrc=false, regRead=false, regWrite=false, memRead=false, memWrite=false, mem2Reg=false, branch=false, jump=false;
    int rs1_val=0, rs2_val=0, imm_val=0;
};

struct EXMEM {
    bool valid=false;
    int pc=0;
    OPcode type=UnknownType;
    string rd="";
    bool regWrite=false, memRead=false, memWrite=false, mem2Reg=false, branch=false, jump=false;
    int ALUresult=0;
    int rs2_val=0;
    bool branchTaken=false;
    int branchTarget=0;
};

struct MEMWB {
    bool valid=false;
    string rd="";
    bool regWrite=false, mem2Reg=false;
    int memData=0;
    int ALUresult=0;
};

class ALUcontrol {
    public:
    string ALUselect;
    ALUcontrol(string ALUOp="00", string func7="", string func3="") {
        if(ALUOp == "00") { ALUselect = "ADD"; }
        else if(ALUOp == "01") {
            if(func3 == "000") ALUselect = "BEQ";
            else if(func3 == "001") ALUselect = "BNE";
            else if(func3 == "100") ALUselect = "BLT";
            else if(func3 == "101") ALUselect = "BGE";
            else ALUselect = "BEQ";
        }
        else if(ALUOp == "10") {
            if(func3 == "000") {
                if(func7 == "0000000" || func7 == "") ALUselect = "ADD";
                else if(func7 == "0100000") ALUselect = "SUB";
            } else if(func3 == "111") ALUselect = "AND";
            else if(func3 == "110") ALUselect = "OR";
            else if(func3 == "100") ALUselect = "XOR";
            else if(func3 == "001") ALUselect = "SLL";
            else if(func3 == "101") {
                if(func7 == "0000000") ALUselect = "SRL";
                else ALUselect = "SRA";
            } else if(func3 == "010") ALUselect = "SLT";
            else ALUselect = "ADD";
        } else ALUselect = "ADD";
    }
};

class ALU {
    public:
    int result;
    bool Bout;
    ALU(const ALUcontrol &ctl, int a, int b) {
        result = 0; Bout = false;
        if(ctl.ALUselect == "ADD") result = a + b;
        else if(ctl.ALUselect == "SUB") result = a - b;
        else if(ctl.ALUselect == "AND") result = a & b;
        else if(ctl.ALUselect == "OR") result = a | b;
        else if(ctl.ALUselect == "XOR") result = a ^ b;
        else if(ctl.ALUselect == "SLL") result = a << (b & 0x1F);
        else if(ctl.ALUselect == "SRL") result = (unsigned int)a >> (b & 0x1F);
        else if(ctl.ALUselect == "SRA") result = a >> (b & 0x1F);
        else if(ctl.ALUselect == "SLT") result = (a < b) ? 1 : 0;
        else if(ctl.ALUselect == "BEQ") { result = a - b; Bout = (result == 0); }
        else if(ctl.ALUselect == "BNE") { result = a - b; Bout = (result != 0); }
        else if(ctl.ALUselect == "BLT") { result = a - b; Bout = (result < 0); }
        else if(ctl.ALUselect == "BGE") { result = a - b; Bout = (result >= 0); }
    }
};

class PipelineCPU {
public:
    InstrMemory instrMem;
    DataMemory dataMem;
    RegisterFile regFile;

    IFID ifid;
    IDEX idex;
    EXMEM exmem;
    MEMWB memwb;

    int pc = 0;
    int cycle = 0;

    PipelineCPU(const vector<string> &instrs) : instrMem(instrs) {}

    IFID decodeIF(const string &instr, int pcval) {
        IFID out;
        if(instr.empty()) return out;
        out.valid = true;
        out.pc = pcval;
        out.instr = instr;
        out.opcode = getbits(instr, 6, 0);
        if(OPcode_map.find(out.opcode) == OPcode_map.end()) { out.type = UnknownType; return out; }
        out.type = OPcode_map[out.opcode];

        switch(out.type) {
            case RType:
                out.func7 = getbits(instr,31,25);
                out.rs2   = getbits(instr,24,20);
                out.rs1   = getbits(instr,19,15);
                out.func3 = getbits(instr,14,12);
                out.rd    = getbits(instr,11,7);
                break;
            case ITypeArith:
            case ITypeLoad:
                out.immediate = getbits(instr,31,20);
                out.rs1 = getbits(instr,19,15);
                out.func3 = getbits(instr,14,12);
                out.rd = getbits(instr,11,7);
                break;
            case SType:
                out.immediate = getbits(instr,31,25) + getbits(instr,11,7);
                out.rs2 = getbits(instr,24,20);
                out.rs1 = getbits(instr,19,15);
                out.func3 = getbits(instr,14,12);
                break;
            case BType:
                out.immediate = getbits(instr,31,31) + getbits(instr,7,7) + getbits(instr,30,25) + getbits(instr,11,8) + "0";
                out.rs1 = getbits(instr,19,15);
                out.rs2 = getbits(instr,24,20);
                out.func3 = getbits(instr,14,12);
                break;
            case JALType:
                out.immediate = getbits(instr,31,31) + getbits(instr,19,12) + getbits(instr,20,20) + getbits(instr,30,21) + "0";
                out.rd = getbits(instr,11,7);
                break;
            case JALRType:
                out.immediate = getbits(instr,31,20);
                out.rs1 = getbits(instr,19,15);
                out.func3 = getbits(instr,14,12);
                out.rd = getbits(instr,11,7);
                break;
            default:
                break;
        }
        return out;
    }

    IDEX makeIDEXFromIFID(const IFID &in) {
        IDEX out;
        if(!in.valid) return out;
        out.valid = true;
        out.pc = in.pc;
        out.opcode = in.opcode; out.func3 = in.func3; out.func7 = in.func7;
        out.rs1 = in.rs1; out.rs2 = in.rs2; out.rd = in.rd; out.immediate = in.immediate;
        out.type = in.type;
        switch(in.type) {
            case RType: out.ALUop="10"; out.ALUsrc=false; out.regWrite=true; out.regRead=true; break;
            case ITypeArith: out.ALUop="10"; out.ALUsrc=true; out.regWrite=true; out.regRead=true; break;
            case ITypeLoad: out.ALUop="00"; out.ALUsrc=true; out.regWrite=true; out.regRead=true; out.memRead=true; out.mem2Reg=true; break;
            case SType: out.ALUop="00"; out.ALUsrc=true; out.regWrite=false; out.regRead=true; out.memWrite=true; break;
            case BType: out.ALUop="01"; out.ALUsrc=false; out.branch=true; out.regRead=true; break;
            case JALType: out.ALUop="00"; out.jump=true; out.regWrite=true; out.regRead=false; break;
            case JALRType: out.ALUop="00"; out.jump=true; out.regWrite=true; out.regRead=true; break;
            default: break;
        }
        int r1 = bitstrToIdx(out.rs1);
        int r2 = bitstrToIdx(out.rs2);
        out.rs1_val = regFile.readIdx(r1);
        out.rs2_val = regFile.readIdx(r2);
        out.imm_val = signExtendToInt(out.immediate);

        return out;
    }

    // ----------- NEW RAW hazard detection (replaces load-use only) -----------
    bool detectRAWHazard(const IDEX &bufferd, const EXMEM &exmem, const MEMWB &memwb) {
    if (!bufferd.valid) return false;

    int src1 = bitstrToIdx(bufferd.rs1);
    int src2 = bitstrToIdx(bufferd.rs2);

    // --- Check hazard with EX/MEM ---
    if (exmem.valid && exmem.regWrite) {
        int writeRd = bitstrToIdx(exmem.rd);
        if (writeRd > 0 && (src1 == writeRd || src2 == writeRd))
            return true;
    }

    // --- Check hazard with MEM/WB ---
    if (memwb.valid && memwb.regWrite) {
        int writeRd = bitstrToIdx(memwb.rd);
        if (writeRd > 0 && (src1 == writeRd || src2 == writeRd))
            return true;
    }

    return false;
}

    // -------------------------------------------------------------------------

    void run() {
        cout << "----- 5-stage Pipelined CPU Simulation Start -----\n";
        while(true) {
            cycle++;
            cout << "\n=== Cycle " << cycle << " ===\n";
            MEMWB next_MEMWB = memwb;    
            EXMEM next_EXMEM = exmem;    
            IDEX  next_IDEX  = idex;
            IFID  next_IFID  = ifid;
            if(memwb.valid) {
                if(memwb.regWrite) {
                    int val = memwb.mem2Reg ? memwb.memData : memwb.ALUresult;
                    int rdIdx = bitstrToIdx(memwb.rd);
                    regFile.writeIdx(rdIdx, val);
                    cout << "WB: write x" << rdIdx << " = " << val << "\n";
                } else {
                    cout << "WB: (no reg write)\n";
                }
            } else {
                cout << "WB: (bubble)\n";
            }
            next_MEMWB = MEMWB(); 
            if(exmem.valid) {
                cout << "MEM: EX->ALUresult=" << exmem.ALUresult << ", memRead=" << exmem.memRead << ", memWrite=" << exmem.memWrite << "\n";
                next_MEMWB.valid = true;
                next_MEMWB.rd = exmem.rd;
                next_MEMWB.regWrite = exmem.regWrite;
                next_MEMWB.mem2Reg = exmem.mem2Reg;
                next_MEMWB.ALUresult = exmem.ALUresult;
                if(exmem.memRead) {
                    next_MEMWB.memData = dataMem.read(exmem.ALUresult);
                    cout << "MEM: Loaded mem[" << exmem.ALUresult << "] = " << next_MEMWB.memData << "\n";
                }
                if(exmem.memWrite) {
                    dataMem.write(exmem.ALUresult, exmem.rs2_val);
                    cout << "MEM: Stored mem[" << exmem.ALUresult << "] = " << exmem.rs2_val << "\n";
                }
            } else {
                cout << "MEM: (bubble)\n";
            }

            EXMEM newEXMEM; 
            if(idex.valid) {
                ALUcontrol aluctrl(idex.ALUop, idex.func7, idex.func3);
                int op1 = idex.rs1_val;
                int op2 = idex.ALUsrc ? idex.imm_val : idex.rs2_val;
                ALU alu(aluctrl, op1, op2);
                newEXMEM.valid = true;
                newEXMEM.pc = idex.pc;
                newEXMEM.type = idex.type;
                newEXMEM.rd = idex.rd;
                newEXMEM.regWrite = idex.regWrite;
                newEXMEM.memRead = idex.memRead;
                newEXMEM.memWrite = idex.memWrite;
                newEXMEM.mem2Reg = idex.mem2Reg;
                newEXMEM.ALUresult = alu.result;
                newEXMEM.rs2_val = idex.rs2_val;
                newEXMEM.branch = idex.branch;
                newEXMEM.jump = idex.jump;
                newEXMEM.branchTaken = false;
                newEXMEM.branchTarget = 0;
                cout << "EX: ALU op result = " << newEXMEM.ALUresult << " (op1=" << op1 << ", op2=" << op2 << ")\n";
                if(idex.branch) {
                    if(alu.Bout) {
                        newEXMEM.branchTaken = true;
                        newEXMEM.branchTarget = idex.pc + idex.imm_val;
                        cout << "EX: Branch TAKEN -> target = " << newEXMEM.branchTarget << "\n";
                    } else {
                        cout << "EX: Branch NOT taken\n";
                    }
                }
                if(idex.jump) {
                    if(idex.type == JALType) {
                        newEXMEM.branchTaken = true;
                        newEXMEM.branchTarget = idex.pc + idex.imm_val;
                        newEXMEM.ALUresult = idex.pc + 4;
                        cout << "EX: JAL -> link=" << newEXMEM.ALUresult << ", target=" << newEXMEM.branchTarget << "\n";
                    } else if(idex.type == JALRType) {
                        int target = (idex.rs1_val + idex.imm_val) & (~1);
                        newEXMEM.branchTaken = true;
                        newEXMEM.branchTarget = target;
                        newEXMEM.ALUresult = idex.pc + 4;
                        cout << "EX: JALR -> link=" << newEXMEM.ALUresult << ", target=" << newEXMEM.branchTarget << "\n";
                    }
                }
            } else {
                cout << "EX: (bubble)\n";
            }

            // Use RAW hazard detector (stall on any ID/EX regWrite -> IF/ID read)
           

            IDEX newIDEX; 
                if(ifid.valid) {
                    newIDEX = makeIDEXFromIFID(ifid);
                    cout << "ID: Decoded instr at PC=" << ifid.pc << (ifid.type==UnknownType? " (unknown type)" : "") << "\n";
                } else {
                    cout << "ID: (bubble)\n";
                    newIDEX = IDEX();
                }
            int oldPC = pc;

            IFID fetchedIFID;
                string instrBits = instrMem.getInstruction(pc / 4);
                fetchedIFID = decodeIF(instrBits, pc);
                if(fetchedIFID.valid) {
                    cout << "IF: Fetched instruction at PC=" << pc << "\n";
                } else {
                    cout << "IF: No instruction fetched at PC=" << pc << " (bubble or out-of-range)\n";
                }
                pc += 4; 

             bool insertStall = detectRAWHazard(idex, exmem, memwb);
            if(insertStall) {
                newIDEX= IDEX();
                fetchedIFID = ifid;
                pc = oldPC;
                cout << "Hazard: RAW dependency detected -> inserting one-cycle stall\n";
            }

            if(newEXMEM.valid && newEXMEM.branchTaken) {
                cout << "Control: Flush pipeline due to branch/jump taken. New PC = " << newEXMEM.branchTarget << "\n";
                pc = newEXMEM.branchTarget;
                fetchedIFID = IFID();
                newIDEX = IDEX();
            }
            memwb = next_MEMWB;
            exmem = newEXMEM;
            idex = newIDEX;
            ifid = fetchedIFID;
            cout << "\nPipeline registers after update:\n";
            cout << " IF/ID : " << (ifid.valid ? ("valid, PC=" + to_string(ifid.pc)) : "bubble") << "\n";
            cout << " ID/EX : " << (idex.valid ? ("valid, PC=" + to_string(idex.pc)) : "bubble") << "\n";
            cout << " EX/MEM: " << (exmem.valid ? ("valid, ALUres=" + to_string(exmem.ALUresult)) : "bubble") << "\n";
            cout << " MEM/WB: " << (memwb.valid ? "valid" : "bubble") << "\n";

            cout << "\nRegisters (x0..x31):\n";
            regFile.dump0to31();
            bool noMoreInstr = instrMem.getInstruction(pc/4).empty();
            bool pipelineEmpty = !ifid.valid && !idex.valid && !exmem.valid && !memwb.valid;
            if(noMoreInstr && pipelineEmpty) {
                cout << "\nAll instructions finished and pipeline empty -> halting.\n";
                break;
            }

            if(cycle > 1000) {
                cout << "Cycle limit reached - aborting simulation.\n";
                break;
            }
        } 
        cout << "\n----- Simulation End (cycles = " << cycle << ") -----\n";
    }
};

int main() {
    vector<string> instructions = loadInstructions("examples/sample_input.txt");

    PipelineCPU cpu(instructions);

    cpu.regFile.writeIdx(6, 10); // x6 = 10
    cpu.regFile.writeIdx(7, 3);  // x7 = 3
    cpu.regFile.writeIdx(3, 0);  // x3 = 0
    cpu.regFile.writeIdx(5, 0);  // x5 = 0

    cpu.run();
    return 0;
}