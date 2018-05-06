//
// Created by Dilemma丶 on 4/22/2018.
//
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cctype>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <stdio.h>
#include <string.h>

using namespace std;

struct instruction {
    string label = "";
    string opcode = "";

    vector<string> leftOperands;
    vector<string> rightOperands;
};

void helper(int &new_value, int &new_label, vector<instruction> &instructions);

void parse_input(string file_name, vector<instruction> &instructions);

void output(vector<instruction> &instructions, string file_name);

vector<int> block_start;
vector<int> block_end;

void value_numbering(vector<instruction> &instructions) {  // local value numbering
    int block_num = (int) block_start.size();
    for (int i = 0; i < block_num; i++) {
        int start = block_start[i], end = block_end[i];
        unordered_map<string, int> VN;   // for each single left operand
        unordered_map<string, int> VN_expr;  // for the whole left operands
        unordered_map<int, string> link; // link value number to right operand
        int new_vn = 0;
        for (int t = start; t <= end; t++) {
            int j = t - 1; // change to index form
            if (instructions[j].opcode == "halt") {
                break;
            } else if (instructions[j].opcode == "nop" || instructions[j].opcode == "not" ||
                       instructions[j].opcode == "store"
                       || instructions[j].opcode == "storeAI" || instructions[j].opcode == "storeAO" ||
                       instructions[j].opcode == "cstore"
                       || instructions[j].opcode == "cstoreAI" || instructions[j].opcode == "cstoreAO" ||
                       instructions[j].opcode == "i2i"
                       || instructions[j].opcode == "c2c" || instructions[j].opcode == "i2c" ||
                       instructions[j].opcode == "c2i"
                       || instructions[j].opcode == "br" || instructions[j].opcode == "cbr" ||
                       instructions[j].opcode == "read" || instructions[j].opcode == "cread" ||
                       instructions[j].opcode == "output"
                       || instructions[j].opcode == "coutput" || instructions[j].opcode == "write" ||
                       instructions[j].opcode == "cwrite"
                       || instructions[j].opcode == "loadI" || instructions[j].opcode == "load" ||
                       instructions[j].opcode == "cload")
                continue;
            else {

                // we use a bit manipulation to subsitute multI. reference of idea here from Internet
                if (instructions[j].opcode == "multI") {
                    if (((stoi(instructions[j].leftOperands[1])) &
                         ((stoi(instructions[j].leftOperands[1])) - 1)) == 0) {     // its power of 2
                        int res = (int) (sqrt(stoi(instructions[j].leftOperands[1])));
                        instruction instr;
                        instr.opcode = "lshiftI";
                        instr.leftOperands.push_back(instructions[j].leftOperands[0]);
                        instr.leftOperands.push_back(to_string(res));
                        instr.rightOperands.push_back(instructions[j].rightOperands[0]);
                        instructions[j] = instr;
                        cout << "Substitute multI with lshiftI on LINE: " << t << endl;
                    }
                }

                // r3 r3 => r6 (1->r3, 2->r3, 3->r6, (r3 * r3)->3)
                // eg mult r3, r3 => r8, r3 = left1, r3 = left2, r8 = right
                if (VN.find(instructions[j].leftOperands[0]) == VN.end())
                    VN[instructions[j].leftOperands[0]] = new_vn++;
                if (VN.find(instructions[j].leftOperands[1]) == VN.end())
                    VN[instructions[j].leftOperands[1]] = new_vn++;
                int left = VN[instructions[j].leftOperands[0]], right = VN[instructions[j].leftOperands[1]];
                string combine =
                        to_string(left) + instructions[j].opcode + to_string(right);
                if (VN_expr.find(combine) == VN_expr.end()) { // cannot find the left
                    VN[instructions[j].rightOperands[0]] = new_vn;
                    VN_expr[combine] = new_vn;
                    link[new_vn++] = instructions[j].rightOperands[0];
                } else {  // find it
                    // do I need another step judgement?
                    instruction new_ins;
                    new_ins.opcode = "i2i";
                    new_ins.label = instructions[j].label;
                    new_ins.rightOperands.push_back(instructions[j].rightOperands[0]);
                    new_ins.leftOperands.push_back(link[VN_expr[combine]]);
                    VN[instructions[j].rightOperands[0]] = VN_expr[combine];
                    instructions[j] = new_ins;
                    cout << "VN optimizes on LINE: " << t << endl;
                }

            }
        }
    }
};

void loop_unrolling(int &new_value, int &new_label, vector<instruction> &instructions) {
    vector<instruction> tmp;
    int block_num = (int) block_start.size();
    for (int i = 0; i < block_start[0] - 1; i++)
        tmp.push_back(instructions[i]);
    for (int i = 0; i < block_num; i++) {
        int start = block_start[i] - 1, end = block_end[i] - 1; // change to index form
        if (instructions[end].opcode == "halt") {
            for (int k = start; k <= end; k++)
                tmp.push_back(instructions[k]);
            instructions.swap(tmp);
            return;
        }
        if (!instructions[start].label.empty() &&
            (instructions[end].opcode == "cbr" || instructions[end].opcode == "br")) {

            if ((instructions[end].opcode == "cbr" &&
                 (instructions[start].label == instructions[end].rightOperands[0] ||
                  instructions[start].label == instructions[end].rightOperands[1])) ||
                (instructions[start].label == instructions[end].rightOperands[0])) {
                if (instructions[end - 2].opcode == "addI") {
                    cout << "loop unrolled by 4 on BLOCK: " << instructions[start].label << endl;
                    // do loop unrolling by 4

                    // calculate times for by-4 loop and by-1 loop
                    instruction instr1;
                    instr1.opcode = "nop";
                    instr1.label = instructions[start].label;
                    tmp.push_back(instr1);


                    instruction tmp1;
                    tmp1.opcode = "sub";
                    tmp1.leftOperands.push_back(instructions[end - 1].leftOperands[1]);
                    tmp1.leftOperands.push_back(instructions[end - 1].leftOperands[0]);
                    string new_r = "r" + to_string(++new_value);
                    tmp1.rightOperands.push_back(new_r);
                    tmp.push_back(tmp1);


                    instruction instr2;
                    instr2.opcode = "rshiftI";
                    instr2.leftOperands.push_back(new_r);
                    instr2.leftOperands.push_back("2");
                    instr2.rightOperands.push_back(new_r);
                    tmp.push_back(instr2);

                    
                    instruction instr3;
                    instr3.opcode = "lshiftI";
                    instr3.leftOperands.push_back(new_r);
                    instr3.leftOperands.push_back("2");
                    instr3.rightOperands.push_back(new_r);
                    tmp.push_back(instr3);


                    // record in a new value
                    instruction tmp2;
                    tmp2.opcode = "add";
                    tmp2.leftOperands.push_back(instructions[end - 1].leftOperands[0]);
                    tmp2.leftOperands.push_back(new_r);
                    string new_reg = "r" + to_string(++new_value);
                    tmp2.rightOperands.push_back(new_reg);
                    tmp.push_back(tmp2);


                    instruction instr4;
                    instr4.opcode = "cbr";
                    instr4.leftOperands.push_back(new_r);
                    string new_label1 = "L" + to_string(++new_label);
                    instr4.rightOperands.push_back(new_label1);
                    string new_label2 = "L" + to_string(++new_label);
                    instr4.rightOperands.push_back(new_label2);
                    tmp.push_back(instr4);

                    instruction instr5;
                    instr5.opcode = "nop";
                    instr5.label = new_label1;
                    tmp.push_back(instr5);

                    for (int s = 0; s < 4; s++)
                        for (int j = start + 1; j <= end - 2; j++)
                            tmp.push_back(instructions[j]);


                    instruction instr6;
                    if (instructions[end - 1].opcode == "cmp_LE")
                        instr6.opcode = "cmp_LT";
                    else if (instructions[end - 1].opcode == "cmp_LT")
                        instr6.opcode = "cmp_LE";
                    instr6.leftOperands.push_back(instructions[end - 1].leftOperands[0]);
                    instr6.leftOperands.push_back(new_reg);
                    instr6.rightOperands.push_back(instructions[end].leftOperands[0]);
                    tmp.push_back(instr6);

                    instruction instr7;
                    instr7.opcode = "cbr";
                    instr7.leftOperands.push_back(instructions[end].leftOperands[0]);
                    instr7.rightOperands.push_back(new_label1);
                    instr7.rightOperands.push_back(new_label2);
                    tmp.push_back(instr7);


                    //starting with L12:
                    instruction instr8;
                    instr8.opcode = "nop";
                    instr8.label = new_label2;
                    tmp.push_back(instr8);

                    tmp.push_back(instructions[end - 1]);

                    instruction instr9;
                    instr9.opcode = "cbr";
                    instr9.leftOperands.push_back(instructions[end].leftOperands[0]);
                    string new_label3 = "L" + to_string(new_label + 1);
                    new_label = new_label + 1;
                    instr9.rightOperands.push_back(new_label3);
                    instr9.rightOperands.push_back(instructions[end].rightOperands[1]);
                    tmp.push_back(instr9);

                    //starting with L13:
                    instruction instr10;
                    instr10.opcode = "nop";
                    instr10.label = new_label3;
                    tmp.push_back(instr10);

                    for (int c = start + 1; c <= end - 2; c++)
                        tmp.push_back(instructions[c]);
                    
                    tmp.push_back(instructions[end - 1]);
                    tmp.push_back(instr9);

                } else {
                    for (int k = start; k <= end; k++)
                        tmp.push_back(instructions[k]);
                    continue;
                }
            } else {
                for (int k = start; k <= end; k++)
                    tmp.push_back(instructions[k]);
                continue;
            }
        } else {
            for (int k = start; k <= end; k++)
                tmp.push_back(instructions[k]);
            continue;
        }
    }

}

void parse_instructions(vector<instruction> instructions) {
    block_end.clear();
    block_start.clear();
    int line_no = 1;
    for (auto i: instructions) {
        if (!i.label.empty()) {
            if (!block_start.empty())
                block_end.push_back(line_no - 1);
            block_start.push_back(line_no);
        }
        if (i.opcode == "halt")
            block_end.push_back(line_no);
        line_no++;
    }
}

int main(int argc, char *argv[]) {
    vector<instruction> instructions;
    string file_name;

    if (argc == 3) {
        file_name = argv[2];
        parse_input(file_name, instructions);
        if (argv[1] && (strcmp(argv[1], "-v") == 0)) {
            parse_instructions(instructions);
            value_numbering(instructions);
            output(instructions, "./output.i");
            cout << "Finish!  File saved in ./output.i" << endl;
        } else if (argv[1] && (strcmp(argv[1], "-u") == 0)) {
            parse_instructions(instructions);
            int new_value = -1;
            int new_label = -1;
            helper(new_value, new_label, instructions);
            loop_unrolling(new_value, new_label, instructions);
            output(instructions, "./output.i");
            cout << "Finish!  File saved in ./output.i" << endl;
        } else if (argv[1] && (strcmp(argv[1], "-i") == 0)) {
            cout << "code motion not implemented" << endl;
            return 1;
        } else {
            cout << "wrong format input" << endl;
            return 1;
        }
    } else if (argc == 4) {
        file_name = argv[3];
        parse_input(file_name, instructions);
        if (argv[1] && argv[2] && ((strcmp(argv[1], "-i") == 0) || (strcmp(argv[2], "-i") == 0))) {
            cout << "code motion not implemented" << endl;
            return 1;
        }
        if (argv[1] && argv[2] && (strcmp(argv[1], "-v") == 0) && (strcmp(argv[2], "-u") == 0)) {
            parse_instructions(instructions);
            value_numbering(instructions);
            parse_instructions(instructions);
            int new_value = -1;
            int new_label = -1;
            helper(new_value, new_label, instructions);
            loop_unrolling(new_value, new_label, instructions);
            output(instructions, "./output.i");
            cout << "Finish!  File saved in ./output.i" << endl;
        } else if (argv[1] && argv[2] && (strcmp(argv[1], "-u") == 0) && (strcmp(argv[2], "-v") == 0)) {
            parse_instructions(instructions);
            int new_value = -1;
            int new_label = -1;
            helper(new_value, new_label, instructions);
            loop_unrolling(new_value, new_label, instructions);
            parse_instructions(instructions);
            value_numbering(instructions);
            output(instructions, "./output.i");
            cout << "Finish!  File saved in ./output.i" << endl;
        } else {
            cout << "wrong format input" << endl;
            return 1;
        }
    } else {
        cout << "wrong parameter numbers" << endl;
        return 1;
    }

//    cout << argc << endl;
//    string file_name = "D:\\Rice\\PPT\\Spr 2018\\COMP 506\\Lab 2\\Opt\\fib.i";
//    vector<instruction> instructions;
//    parse_input(file_name, instructions);
//    parse_instructions(instructions);
//    int new_value = -1;
//    int new_label = -1;
//    helper(new_value, new_label, instructions);
////    sort(block_start.begin(), block_start.end());
////    sort(block_end.begin(), block_end.end());
//    loop_unrolling(new_value, new_label, instructions);
//    output(instructions, "./output.i");
//    return 0;
}

// find the next variable and next label
void helper(int &new_value, int &new_label, vector<instruction> &instructions) {
    for (auto x : instructions) {
        if (x.label != "") {
            int length = (int) x.label.length();
            string tmp_label = x.label.substr(1, length - 1);
            int tmp_label_value = stoi(tmp_label);
            if (tmp_label_value > new_label) {
                new_label = tmp_label_value;
            }
        }
        if (x.leftOperands.size() != 0) {
            for (auto y : x.leftOperands) {
                if (y[0] != 'r') {
                    continue;
                }
                int length = (int) y.length();
                string tmp_register = y.substr(1, length - 1);
                int tmp_register_value = stoi(tmp_register);
                if (tmp_register_value > new_value) {
                    new_value = tmp_register_value;
                }
            }
        }
        if (x.rightOperands.size() != 0) {
            for (auto z : x.rightOperands) {
                if (z[0] != 'r') {
                    continue;
                }
                int length = (int) z.length();
                string tmp_register = z.substr(1, length - 1);
                int tmp_register_value = stoi(tmp_register);
                if (tmp_register_value > new_value) {
                    new_value = tmp_register_value;
                }
            }
        }
    }
}
// in and out

void parse_input(string file_name, vector<instruction> &instructions) {
    ifstream infile;
    infile.open(file_name.c_str(), ifstream::in);
    string each_line = "";
    int line_no = 0;
    if (!infile) {
        cout << "open file errors!" << endl;
        exit(EXIT_FAILURE);
    }
    while (getline(infile, each_line)) {
        if (each_line.find("//") != string::npos)
            each_line = each_line.substr(0, each_line.find("//"));
        instruction instr;
        line_no++;
//        cout << "current line: " << each_line << endl;
        if (each_line.find(':') != string::npos) { // only label and nop
            string label_name = each_line.substr(0, each_line.find(':'));
            each_line = each_line.substr(0, each_line.find(':'));
            instr.opcode = "nop";
            instr.label = label_name;
            instructions.push_back(instr);
//            if (!block_start.empty())
//                block_end.push_back(line_no - 1);
//            block_start.push_back(line_no);
        } else {
            int i = 0;
            while (isalpha(each_line[i]) && i < each_line.length()) {
                i++;
            }
            i++;
            for (int j = i; j < each_line.length(); j++) {
                string opcode = "";
                opcode += each_line[j++];
                while ((isalnum(each_line[j]) || each_line[j] == '_') && j < each_line.length()) {
                    opcode += each_line[j];
                    j++;
                }
                instr.opcode = opcode;
                if (opcode == "halt") {
                    instructions.push_back(instr);
//                    block_end.push_back(line_no);
                    return;
                } else if (opcode == "output" || opcode == "coutput" || opcode == "write" || opcode == "cwrite") {
                    while (each_line[j] == ' ' && j < each_line.length()) {
                        j++;
                    }
                    string tmp = "";
                    tmp += each_line[j++];
                    while (each_line[j] != ' ' && j < each_line.length()) {
                        tmp += each_line[j];
                        j++;
                    }
                    instr.leftOperands.push_back(tmp);
                    instructions.push_back(instr);
                    break;
                } else if (opcode == "br") {
                    while (each_line[j] != '-' && j < each_line.length()) {
                        j++;
                    }
                    j++;
                    j++;
                    while (each_line[j] == ' ' && j < each_line.length()) {
                        j++;
                    }
                    string tmp = "";
                    while (isalnum(each_line[j]) && j < each_line.length()) {
                        tmp += each_line[j];
                        j++;
                    }
                    instr.rightOperands.push_back(tmp);
                    instructions.push_back(instr);
                    break;
                } else if (opcode == "cbr") {
                    while (each_line[j] == ' ' && j < each_line.length()) {
                        j++;
                    }
                    string left = "";
                    left += each_line[j++];
                    while (isalnum(each_line[j]) && j < each_line.length()) {
                        left += each_line[j];
                        j++;
                    }
                    instr.leftOperands.push_back(left);
                    while (each_line[j] != '-' && j < each_line.length()) {
                        j++;
                    }
                    j++;
                    j++;
                    while (each_line[j] == ' ' && j < each_line.length()) {
                        j++;
                    }
                    string right1 = "";
                    right1 += each_line[j++];
                    while (each_line[j] != ',' && j < each_line.length()) {
                        right1 += each_line[j];
                        j++;
                    }
                    j++;
                    while (each_line[j] == ' ' && j < each_line.length()) {
                        j++;
                    }
                    string right2 = "";
                    right2 += each_line[j++];
                    while (isalnum(each_line[j]) && j < each_line.length()) {
                        right2 += each_line[j];
                        j++;
                    }
                    instr.rightOperands.push_back(right1);
                    instr.rightOperands.push_back(right2);
                    instructions.push_back(instr);
                    break;
                } else if (opcode == "read" || opcode == "cread") {
                    while (each_line[j] == ' ' && j < each_line.length()) {
                        j++;
                    }
                    j++;
                    j++;
                    while (each_line[j] == ' ' && j < each_line.length()) {
                        j++;
                    }
                    string tmp = "";
                    while (isalnum(each_line[j]) && j < each_line.length()) {
                        tmp += each_line[j];
                        j++;
                    }
                    instr.rightOperands.push_back(tmp);
                    instructions.push_back(instr);
                    break;
                } else if (opcode == "not" || opcode == "loadI" || opcode == "load"
                           || opcode == "cload" || opcode == "store" || opcode == "cstore" || opcode == "i2i"
                           || opcode == "c2c" || opcode == "i2c" || opcode == "c2i") {
                    while (each_line[j] == ' ' && j < each_line.length()) {
                        j++;
                    }
                    string left = "";
                    left += each_line[j++];
                    while (isalnum(each_line[j]) && j < each_line.length()) {
                        left += each_line[j];
                        j++;
                    }
                    while (each_line[j] != '=' && j < each_line.length()) {
                        j++;
                    }
                    j++;
                    j++;
                    while (each_line[j] == ' ' && j < each_line.length()) {
                        j++;
                    }
                    string right = "";
                    right += each_line[j++];
                    while (isalnum(each_line[j]) && j < each_line.length()) {
                        right += each_line[j];
                        j++;
                    }
                    instr.leftOperands.push_back(left);
                    instr.rightOperands.push_back(right);
                    instructions.push_back(instr);
                    break;
                } else if (opcode == "storeAI" || opcode == "storeAO" || opcode == "cstoreAI" || opcode == "cstoreAO") {
                    while (each_line[j] == ' ' && j < each_line.length()) {
                        j++;
                    }
                    string left = "";
                    left += each_line[j++];
                    while (isalnum(each_line[j]) && j < each_line.length()) {
                        left += each_line[j];
                        j++;
                    }
                    while (each_line[j] != '=' && j < each_line.length()) {
                        j++;
                    }
                    j++;
                    j++;
                    while (each_line[j] == ' ' && j < each_line.length()) {
                        j++;
                    }
                    string right1 = "";
                    right1 += each_line[j++];
                    while (each_line[j] != ',' && j < each_line.length()) {
                        right1 += each_line[j];
                        j++;
                    }
                    j++;
                    while (each_line[j] == ' ' && j < each_line.length()) {
                        j++;
                    }
                    string right2 = "";
                    right2 += each_line[j++];
                    while (isalnum(each_line[j]) && j < each_line.length()) {
                        right2 += each_line[j];
                        j++;
                    }
                    instr.leftOperands.push_back(left);
                    instr.rightOperands.push_back(right1);
                    instr.rightOperands.push_back(right2);
                    instructions.push_back(instr);
                    break;
                } else {
                    while (each_line[j] == ' ' && j < each_line.length()) {
                        j++;
                    }
                    string left1 = "";
                    left1 += each_line[j++];
                    while (each_line[j] != ',' && j < each_line.length()) {
                        left1 += each_line[j];
                        j++;
                    }
                    j++;
                    while (each_line[j] == ' ' && j < each_line.length()) {
                        j++;
                    }
                    string left2 = "";
                    left2 += each_line[j++];
                    while (isalnum(each_line[j]) && j < each_line.length()) {
                        left2 += each_line[j];
                        j++;
                    }
                    while (each_line[j] != '=' && j < each_line.length()) {
                        j++;
                    }
                    j++;
                    j++;
                    while (each_line[j] == ' ' && j < each_line.length()) {
                        j++;
                    }
                    string right = "";
                    right += each_line[j++];
                    while (isalnum(each_line[j]) && j < each_line.length()) {
                        right += each_line[j];
                        j++;
                    }
                    instr.leftOperands.push_back(left1);
                    instr.leftOperands.push_back(left2);
                    instr.rightOperands.push_back(right);
                    instructions.push_back(instr);
                    break;
                }
            }
        }
    }
}

void output(vector<instruction> &instructions, string file_name) {
    ofstream outfile;
    outfile.open(file_name.c_str(), ios::trunc);
    outfile.close();
    for (instruction x : instructions) {
        outfile.open(file_name.c_str(), ios::app);
        if (x.opcode != "nop")
            outfile << "\t";
        if (x.opcode == "halt") {
            outfile << x.opcode << endl;
        } else if (x.opcode == "nop") {
            outfile << x.label << ": " << x.opcode << endl;
        } else if (x.opcode == "output" || x.opcode == "coutput" || x.opcode == "write" || x.opcode == "cwrite") {
            outfile << x.opcode << " " << x.leftOperands[0] << endl;
        } else if (x.opcode == "read" || x.opcode == "cread") {
            outfile << x.opcode << " => " << x.rightOperands[0] << endl;
        } else if (x.opcode == "br") {
            outfile << x.opcode << " -> " << x.rightOperands[0] << endl;
        } else if (x.opcode == "cbr") {
            outfile << x.opcode << " " << x.leftOperands[0] << " -> " << x.rightOperands[0] << ", "
                    << x.rightOperands[1] << endl;
        } else if (x.opcode == "not" || x.opcode == "loadI" || x.opcode == "load"
                   || x.opcode == "cload" || x.opcode == "store" || x.opcode == "cstore" || x.opcode == "i2i"
                   || x.opcode == "c2c" || x.opcode == "i2c" || x.opcode == "c2i") {
            outfile << x.opcode << " " << x.leftOperands[0] << " => " << x.rightOperands[0] << endl;
        } else if (x.opcode == "storeAI" || x.opcode == "storeAO" || x.opcode == "cstoreAI" ||
                   x.opcode == "cstoreAO") {
            outfile << x.opcode << " " << x.leftOperands[0] << " => " << x.rightOperands[0] << ", "
                    << x.rightOperands[1] << endl;
        } else {
            outfile << x.opcode << " " << x.leftOperands[0] << ", " << x.leftOperands[1] << " => "
                    << x.rightOperands[0]
                    << endl;
        }
        outfile.close();
    }
}
